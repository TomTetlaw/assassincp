#include "precompiled.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL2_IMPLEMENTATION
#include "../include/nuklear/nuklear.h"
#include "../include/nuklear/nuklear_sdl_gl2.h"

Editor editor;

bool lines_intersect(Vec2 p1, Vec2 q1, Vec2 p2, Vec2 q2)
{
	return false;

	auto orientation = [](Vec2 p, Vec2 q, Vec2 r) -> int {
		int val = (q.y - p.y) * (r.x - q.x) -
			(q.x - p.x) * (r.y - q.y);
		if (val == 0) return 0;
		return (val > 0) ? 1 : 2;
	};

	auto point_on_segment = [](Vec2 p, Vec2 q, Vec2 r) -> bool {
		if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
			q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
			return true;

		return false;
	};

	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	// General case 
	if (o1 != o2 && o3 != o4)
		return true;

	if (o1 == 0 && point_on_segment(p1, p2, q1)) return true;
	if (o2 == 0 && point_on_segment(p1, q2, q1)) return true;
	if (o3 == 0 && point_on_segment(p2, p1, q2)) return true;
	if (o4 == 0 && point_on_segment(p2, q1, q2)) return true;

	return false;
}

bool point_intersects_box(Vec2 point, Vec2 position, Vec2 size) {
	if (point.x < position.x)
		return false;
	if (point.y < position.y)
		return false;
	if (point.x > position.x + size.x)
		return false;
	if (point.y > position.y + size.y)
		return false;
	return true;
}

bool point_intersects_centered_box(Vec2 point, Vec2 position, Vec2 size) {
	float hw = size.x / 2;
	float hh = size.y / 2;
	position.x -= hw;
	position.y -= hh;
	size.x += hw;
	size.y += hh;
	return point_intersects_box(point, position, size);
}

bool box_intersects_box(Vec2 position_a, Vec2 size_a, Vec2 position_b, Vec2 size_b) {
	if (position_a.x + size_a.x < position_b.x) {
		return false;
	}
	if (position_a.x > position_b.x + size_b.x) {
		return false;
	}
	if (position_a.y + size_a.y < position_b.y) {
		return false;
	}
	if (position_a.y > position_b.y + size_b.y) {
		return false;
	}

	return true;
}

bool box_intersects_centered_box(Vec2 position_a, Vec2 size_a, Vec2 position_b, Vec2 size_b) {
	float hw = size_b.x / 2;
	float hh = size_b.y / 2;
	position_b.x -= hw;
	position_b.y -= hh;
	size_b.x += hw;
	size_b.y += hh;
	return box_intersects_box(position_a, size_a, position_b, size_b);
}

/*
bool polygon_is_concave(Editor_Polygon *poly)
{
	bool got_negative = false;
	bool got_positive = false;
	int num_points = poly->verts.num;
	int B, C;

	auto cross_product_length = [](float Ax, float Ay, float Bx, float By, float Cx, float Cy) -> float {
		float BAx = Ax - Bx;
		float BAy = Ay - By;
		float BCx = Cx - Bx;
		float BCy = Cy - By;
		return (BAx * BCy - BAy * BCx);
	};

	for (int A = 0; A < num_points; A++)
	{
		B = (A + 1) % num_points;
		C = (B + 1) % num_points;

		float cross_product =
			cross_product_length(
				poly->verts[A]->position.x, poly->verts[A]->position.y,
				poly->verts[B]->position.x, poly->verts[B]->position.y,
				poly->verts[C]->position.x, poly->verts[C]->position.y);
		if (cross_product < 0)
		{
			got_negative = true;
		}
		else if (cross_product > 0)
		{
			got_positive = true;
		}
		if (got_negative && got_positive) return false;
	}

	return true;
}

Vec2 polygon_centre_point(Editor_Polygon *poly) {
	Vec2 centroid = { 0, 0 };
	float signedArea = 0.0f;
	float x0 = 0.0f; // Current vertex X
	float y0 = 0.0f; // Current vertex Y
	float x1 = 0.0f; // Next vertex X
	float y1 = 0.0f; // Next vertex Y
	float a = 0.0f;  // Partial signed area
	int vertexCount = poly->verts.num;

	// For all vertices except last
	int i = 0;
	for (i = 0; i < vertexCount - 1; ++i)
	{
		x0 = poly->verts[i]->position.x;
		y0 = poly->verts[i]->position.y;
		x1 = poly->verts[i + 1]->position.x;
		y1 = poly->verts[i + 1]->position.y;
		a = x0 * y1 - x1 * y0;
		signedArea += a;
		centroid.x += (x0 + x1)*a;
		centroid.y += (y0 + y1)*a;
	}

	// Do last vertex separately to avoid performing an expensive
	// modulus operation in each iteration.
	x0 = poly->verts[i]->position.x;
	y0 = poly->verts[i]->position.y;
	x1 = poly->verts[0]->position.x;
	y1 = poly->verts[0]->position.y;
	a = x0 * y1 - x1 * y0;
	signedArea += a;
	centroid.x += (x0 + x1)*a;
	centroid.y += (y0 + y1)*a;

	signedArea *= 0.5;
	centroid.x /= (6.0*signedArea);
	centroid.y /= (6.0*signedArea);

	return centroid;
}

bool line_intersects_polygon(Vec2 a, Vec2 b, Editor_Polygon *poly, int ignore) {
	for (int j = 0; j < poly->verts.num; j++) {
		if (j + 1 >= poly->verts.num) {
			continue;
		}

		Vec2 p1 = poly->verts[j]->position;
		Vec2 p2 = poly->verts[j + 1]->position;
		Vec2 p3 = a;
		Vec2 p4 = b;

		if (lines_intersect(p1, p2, p3, p4) && j + 1 != ignore) {
			return true;
		}
	}

	if (poly->closed) {
		Vec2 p1 = poly->verts[0]->position;
		Vec2 p2 = poly->verts[poly->verts.num - 1]->position;
		Vec2 p3 = a;
		Vec2 p4 = b;

		if (lines_intersect(p1, p2, p3, p4)) {
			return true;
		}
	}

	return false;
}

void Editor_Polygon::calculate_properties() {
	center = polygon_centre_point(this);
	concave = !polygon_is_concave(this);
	valid = !concave;
}
*/

struct Editor_Mode_Select_Button : public UI_Button {
	Editor_Mode_Select_Button() {
		off_texture = tex.load("data/textures/editor/select_mode_off.png");
		on_texture = tex.load("data/textures/editor/select_mode_on.png");
		size = Vec2(32, 32);
	}

	void on_click() {
		editor.mode = EDITOR_SELECT;
	}
};

struct Editor_Mode_Entity_Button : public UI_Button {
	Editor_Mode_Entity_Button() {
		off_texture = tex.load("data/textures/editor/entity_mode_off.png");
		on_texture = tex.load("data/textures/editor/entity_mode_on.png");
		size = Vec2(32, 32);
	}

	void on_click() {
		editor.mode = EDITOR_ENTITY;
	}
};

void Editor_Thing::render() {
	Vec2 size = editor.size_for_thing(this);
	Vec4 colour = editor.colour_for_thing(this);
	renderer.centered_box(false, position.x, position.y, size.x, size.y, colour);
}

void Editor_Thing::add_properties(Array<Editor_Thing_Property> &properties) {
	properties.append(Editor_Thing_Property("Position", &position, Vec2(-10000, -10000), Vec2(10000, 10000), Vec2(1, 1), Vec2(0.5f, 0.5f)));
}

void Editor_Entity::add_properties(Array<Editor_Thing_Property> &properties) {
	Editor_Thing::add_properties(properties);
	properties.append(Editor_Thing_Property("Size", &size, Vec2(-10000, -10000), Vec2(10000, 10000), Vec2(1, 1), Vec2(0.5f, 0.5f)));
	properties.append(Editor_Thing_Property("Scale", &scale, Vec2(-10000, -10000), Vec2(10000, 10000), Vec2(1, 1), Vec2(0.5f, 0.5f)));
	properties.append(Editor_Thing_Property("Entity Type", &entity_name.data));
	properties.append(editor_thing_property_entity_type());
	properties.append(editor_thing_property_texture("Texture", &texture));
	properties.append(Editor_Thing_Property("Colour", &colour, Vec4(0, 0, 0, 0), Vec4(1, 1, 1, 1), Vec4(0.1f, 0.1f, 0.1f, 0.1f), Vec4(0.05f, 0.05f, 0.05f, 0.05f)));
}

void Editor_Entity::render() {
	Render_Texture rt;
	rt.position = position;
	rt.size = size;
	rt.scale = scale;
	rt.texture = texture;
	renderer.texture(&rt);

	Editor_Thing::render();
}

void Editor::gui_begin_input() {
	nk_input_begin(context);
}

void Editor::gui_end_input() {
	nk_input_end(context);
}

bool Editor::gui_handle_event(SDL_Event *ev) {
	nk_sdl_handle_event(ev);
	return false;
}

struct nk_image select_mode_on_image;
struct nk_image select_mode_off_image;
struct nk_image entity_mode_on_image;
struct nk_image entity_mode_off_image;

void Editor::init() {
	context = nk_sdl_init(sys.window);

	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	struct nk_font *font = nk_font_atlas_add_from_file(atlas, "data/fonts/consolas.ttf", 16, nullptr);
	nk_sdl_font_stash_end();	
	nk_style_set_font(context, &font->handle);

	select_mode_on_image = nk_image_id(tex.load("data/textures/editor/select_mode_on.png")->api_object);
	select_mode_off_image = nk_image_id(tex.load("data/textures/editor/select_mode_off.png")->api_object);
	entity_mode_on_image = nk_image_id(tex.load("data/textures/editor/entity_mode_on.png")->api_object);
	entity_mode_off_image = nk_image_id(tex.load("data/textures/editor/entity_mode_off.png")->api_object);

	edit_window_position = Vec2(128 - 16, 0);
	edit_window_size = Vec2(1920 - 456, 1080 - 80);
}

char buffer[2048] = { 0 };

void Editor::update() {
	nk_begin(context, "Mode", nk_rect(10, 10, 96, 1080 - 66-64), NK_WINDOW_BORDER | NK_WINDOW_TITLE);
	nk_layout_row_static(context, 64, 64, 1);
	if (nk_button_image(context, select_mode_off_image)) {
		mode = EDITOR_SELECT;
	}
	nk_layout_row_static(context, 64, 64, 1);
	if (nk_button_image(context, entity_mode_off_image)) {
		mode = EDITOR_ENTITY;
	}
	nk_end(context);

	nk_begin(context, "", nk_rect(10, 1080 - 10 - 64, 1920 - 10 - 10, 64), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR);
	nk_layout_row_dynamic(context, 64, 1);
	if (mode == EDITOR_SELECT) {
		nk_label(context, "Mode: select", NK_TEXT_LEFT);
	}
	else {
		nk_label(context, "Mode: entity", NK_TEXT_LEFT);
	}
	nk_end(context);

	nk_begin(context, "Properties", nk_rect(1920 - 340 - 1, 10, 356, 1080 - 66 - 64), NK_WINDOW_BORDER | NK_WINDOW_TITLE);
	if (mode == EDITOR_SELECT) {
		if (selected_things.num == 1) {
			Array<Editor_Thing_Property> properties;
			selected_things[0]->add_properties(properties);
			
			For(properties) {
				nk_layout_row_dynamic(context, 30, 1);

				Editor_Thing_Property *prop = it;
				if (it->type == THING_PROPERTY_INT) {
					nk_label(context, it->name, NK_LEFT);
					nk_property_int(context, it->name, it->int_min, it->int_dest, it->int_max, it->int_step, it->int_inc);
				}
				else if (it->type == THING_PROPERTY_FLOAT) {
					nk_label(context, it->name, NK_LEFT);
					nk_property_float(context, it->name, it->float_min, it->float_dest, it->float_max, it->float_step, it->float_inc);
				}
				else if (it->type == THING_PROPERTY_STRING) {
					nk_label(context, it->name, NK_LEFT);
					nk_edit_string_zero_terminated(context, NK_EDIT_FIELD, *it->string_dest, 2048, nullptr);
				}
				else if (it->type == THING_PROPERTY_BOOL) {
					nk_checkbox_label(context, it->name, (int*)it->bool_dest);
				}
				else if (it->type == THING_PROPERTY_VEC2) {
					nk_label(context, it->name, NK_LEFT);
					nk_property_float(context, "#x", it->vec2_min.x, &(it->vec2_dest->x), it->vec2_max.x, it->vec2_step.x, it->vec2_inc.x);
					nk_property_float(context, "#y", it->vec2_min.y, &(it->vec2_dest->y), it->vec2_max.y, it->vec2_step.y, it->vec2_inc.y);
				}
				else if (it->type == THING_PROPERTY_VEC3) {
					nk_label(context, it->name, NK_LEFT);
					nk_property_float(context, "#x", it->vec3_min.x, &(it->vec3_dest->x), it->vec3_max.x, it->vec3_step.x, it->vec3_inc.x);
					nk_property_float(context, "#y", it->vec3_min.y, &(it->vec3_dest->y), it->vec3_max.y, it->vec3_step.y, it->vec3_inc.y);
					nk_property_float(context, "#z", it->vec3_min.z, &(it->vec3_dest->z), it->vec3_max.z, it->vec3_step.z, it->vec3_inc.z);
				}
				else if (it->type == THING_PROPERTY_VEC4) {
					nk_label(context, it->name, NK_LEFT);
					nk_property_float(context, "#x", it->vec4_min.x, &(it->vec4_dest->x), it->vec4_max.x, it->vec4_step.x, it->vec4_inc.x);
					nk_property_float(context, "#y", it->vec4_min.y, &(it->vec4_dest->y), it->vec4_max.y, it->vec4_step.y, it->vec4_inc.y);
					nk_property_float(context, "#z", it->vec4_min.z, &(it->vec4_dest->z), it->vec4_max.z, it->vec4_step.z, it->vec4_inc.z);
					nk_property_float(context, "#w", it->vec4_min.w, &(it->vec4_dest->w), it->vec4_max.w, it->vec4_step.w, it->vec4_inc.w);
				}
				else if (it->type == THING_PROPERTY_TEXTURE) {
					nk_label(context, it->name, NK_LEFT);

					if ((*prop->texture_dest)) {
						nk_layout_row_static(context, 64, 64, 1);
						nk_button_image(context, nk_image_id((*prop->texture_dest)->api_object));
						nk_layout_row_dynamic(context, 30, 1);
					}
					else {
						nk_layout_row_static(context, 64, 64, 1);
						nk_button_image(context, nk_image_id(-1));
						nk_layout_row_dynamic(context, 30, 1);
					}

					if (nk_button_label(context, "Find texture...")) {
						dstr out;
						sys.open_file_dialogue(".\\data\\textures\\", "PNG Files\0*.png\0\0", &out);
						if (out.data[0]) {
							*prop->texture_dest = tex.load(out.data);
						}
					}
				}
				else if (it->type == THING_PROPERTY_ENTITY_TYPE) {
					
				}
			}
		}
	}
	else if (mode == EDITOR_ENTITY) {

	}

	nk_end(context);

	For(things) {
		(*it)->update();
	}
}

void Editor::shutdown() {
	nk_sdl_shutdown();
	remove_all();
}

void Editor::remove_all() {
	for (int i = 0; i < things.num; i++) {
		delete things[i];
	}

	things.num = 0;
	clear_selected_things();
}

Vec2 Editor::size_for_thing(Editor_Thing *thing) {
	Vec2 size = Vec2(0, 0);
	if (thing->hovered) {
		size = thing->hover_size;
	}
	if (!thing->hovered && !selected_things.find(thing)) {
		size = thing->normal_size;
	}
	if (selected_things.find(thing)) {
		size = thing->select_size;
	}
	return size;
}

Vec4 Editor::colour_for_thing(Editor_Thing *thing) {
	Vec4 colour;
	if (thing->hovered) {
		colour = thing->hover_colour;
	}
	if (!thing->hovered && !selected_things.find(thing)) {
		colour = thing->normal_colour;
	}
	if (selected_things.find(thing)) {
		colour = thing->select_colour;
	}
	return colour;
}

void Editor::add_thing(Editor_Thing *thing) {
	thing->index = things.num;
	things.append(thing);
}

void Editor::delete_thing(Editor_Thing *thing) {
	if (thing->index < 0) {
		return;
	}

	things.remove(thing->index);
	delete thing;

	For(things) {
		(*it)->index = it_index;
	}
}

void Editor::clear_selected_things() {
	selected_things.num = 0;
}

void Editor::render() {
	nk_sdl_render(NK_ANTI_ALIASING_OFF);

	glEnable(GL_SCISSOR_TEST);
	glScissor(edit_window_position.x, edit_window_position.y + (1080 - edit_window_size.y), edit_window_size.x, edit_window_size.y);

	For2(things, t) {
		Vec4 colour = colour_for_thing(*t);
		Vec2 size = size_for_thing(*t);
		(*t)->render();
	}

	//////////////////////////////

	if (drag_select) {
		renderer.box(false, drag_start_point.x, drag_start_point.y, drag_size.x, drag_size.y, Vec4(0, 1, 0, 1));
	}

	//////////////////////////////

	glDisable(GL_SCISSOR_TEST);

	renderer.use_camera = false;
	renderer.use_zoom = false;
	renderer.box(false, 128-16, 0, 1920 - 456, 1080 - 80, Vec4(1, 1, 1, 1));
	renderer.use_zoom = true;

	if (debug_draw) {
		renderer.debug_string("%d", mode);
		renderer.debug_string("zoom_level: %d", renderer.zoom_level);
	}
}

bool point_intersects_panel(Vec2 position, UI_Panel *panel) {
	return point_intersects_box(position, ui_get_absolute_position(panel), panel->size);
}

bool point_intersects_button(Vec2 position, UI_Button *button) {
	return point_intersects_box(position, ui_get_absolute_position(button), button->size);
}

void Editor::handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) {
	if (mouse_button == SDL_BUTTON_LEFT) {
		left_button_down = down;
	}
	if (mouse_button == SDL_BUTTON_MIDDLE) {
		middle_button_down = down;
	}

	if (!point_intersects_box(position, edit_window_position, edit_window_size)) {
		return;
	}

	if (mode == EDITOR_SELECT) {
		if (mouse_button == SDL_BUTTON_LEFT) {
			if (down) {
				Editor_Thing *thing = nullptr;
				For(things) { // don't start drag select if dragging a thing
					if (point_intersects_centered_box(renderer.to_world_pos(position), (*it)->position, size_for_thing(*it))) {
						thing = *it;
						break;
					}
				}
				if (thing && selected_things.find(thing)) {
					dragging_thing = true;
				}
				else {
					drag_select = true;
					drag_start_point = renderer.to_world_pos(position);
				}
			}
			else {
				if (dragging_thing) {
					dragging_thing = false;
				}
				else {
					// if clicked and didn't move mouse
					if (drag_start_point.x == 0) {
						drag_start_point.x = renderer.to_world_pos(position).x;
					}
					if (drag_start_point.y == 0) {
						drag_start_point.y = renderer.to_world_pos(position).y;
					}
					if (drag_size.x == 0) {
						drag_size.x = 1;
					}

					// if drawing a box to the left or above where initially clicked
					else if (drag_size.x < 0) {
						drag_size.x = drag_size.x * -1;
						drag_start_point.x = drag_start_point.x - drag_size.x;
					}
					if (drag_size.y == 0) {
						drag_size.y = 1;
					}
					else if (drag_size.y < 0) {
						drag_size.y = drag_size.y * -1;
						drag_start_point.y = drag_start_point.y - drag_size.y;
					}

					if (!shift_down) {
						clear_selected_things();
					}

					For(things) {
						if (box_intersects_centered_box(drag_start_point, drag_size, (*it)->position, size_for_thing(*it))) {
							selected_things.append(*it);
						}
					}

					drag_select = false;
					drag_start_point = Vec2(0, 0);
					drag_size = Vec2(0, 0);
				}
			}
		}
	}
	else if (mode == EDITOR_ENTITY) {
		if (mouse_button == SDL_BUTTON_LEFT) {
			if (down) {
				Editor_Entity *new_entity = new Editor_Entity;
				new_entity->position = renderer.to_world_pos(position);
				add_thing(new_entity);
			}
		}
	}
}

void Editor::handle_mouse_move(int relx, int rely) {
	if (!point_intersects_box(sys.cursor_position, edit_window_position, edit_window_size)) {
		return;
	}

	if (middle_button_down) {
		renderer.camera_position = renderer.camera_position - Vec2(relx, rely);
	}

	if (mode == EDITOR_SELECT) {
		if (dragging_thing) {
			For(selected_things) {
				if ((*it)->draggable) {
					(*it)->position = (*it)->position + Vec2(relx, rely);
				}
			}
		}

		if (drag_select) {
			drag_size = renderer.to_world_pos(sys.cursor_position) - drag_start_point;
		}

		For(things) {
			Vec2 size = size_for_thing(*it);
			if (point_intersects_centered_box(sys.cursor_position, (*it)->position, size)) {
				(*it)->hovered = true;
			}
			else {
				(*it)->hovered = false;
			}
		}
	}
	else if (mode == EDITOR_ENTITY) {
	}
}

void Editor::handle_key_press(SDL_Scancode scancode, bool down, int mods) {
	if (scancode == SDL_SCANCODE_LSHIFT) {
		shift_down = down;
	}

	if (mode == EDITOR_SELECT) {
		if (down && scancode == SDL_SCANCODE_DELETE) {
			if (selected_things.num > 0) {
				For(selected_things) {
					delete_thing((*it));
				}
				clear_selected_things();
			}
		}
	}
}

void Editor::handle_mouse_wheel(int amount) {
	renderer.zoom_level += amount;
}

void Editor::save(const char *file_name) {
}

void Editor::add_static_things() {
	Editor_Thing *thing1 = new Editor_Thing;
	thing1->position = Vec2(100, 100);
	thing1->normal_colour = Vec4(0, 0, 1, 1);
	thing1->normal_size = Vec2(10, 10);
	thing1->hover_colour = Vec4(1, 0, 1, 1);
	thing1->hover_size = Vec2(15, 15);
	thing1->select_colour = Vec4(1, 1, 1, 1);
	thing1->select_size = Vec2(20, 20);

	Editor_Thing *thing2 = new Editor_Thing;
	thing2->position = Vec2(200, 200);
	thing2->normal_colour = Vec4(1, 0, 0, 1);
	thing2->normal_size = Vec2(10, 10);
	thing2->hover_colour = Vec4(1, 0, 1, 1);
	thing2->hover_size = Vec2(15, 15);
	thing2->select_colour = Vec4(1, 1, 1, 1);
	thing2->select_size = Vec2(20, 20);

	Editor_Thing *thing3 = new Editor_Thing;
	thing3->position = Vec2(300, 300);
	thing3->normal_colour = Vec4(0, 1, 0, 1);
	thing3->normal_size = Vec2(10, 10);
	thing3->hover_colour = Vec4(1, 0, 1, 1);
	thing3->hover_size = Vec2(15, 15);
	thing3->select_colour = Vec4(1, 1, 1, 1);
	thing3->select_size = Vec2(20, 20);

	add_thing(thing1);
	add_thing(thing2);
	add_thing(thing3);
}

void Editor::on_level_load() {
	remove_all();

	add_static_things();
}

void Editor::load_map_into_editor(const char *file_name) {
	Load_File_Result in = load_file(file_name);
	char token[2048] = { 0 };
	const char *text_position = in.data;

	if (!text_position) {
		return;
	}

	remove_all();
	game.on_level_load();

	delete[] in.data;
}