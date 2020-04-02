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

Editor_Entity::Editor_Entity() {
	type_name[0] = 0;
	name[0] = 0;
	texture_name[0] = 0;

	size = Vec2(32, 32);
	scale = Vec2(1, 1);
	colour = Vec4(1, 1, 1, 1);

	strcpy(type_name, "Entity");
	strcpy(texture_name, "data/textures/default.png");
}

void Editor_Entity::write_save(Save_File *file) {
	// !!!!! if you add anything to this you must increment map_file_version in editor.h !!!!!

	save_write_string(file, type_name);
	save_write_string(file, name);
	save_write_string(file, texture_name);
	save_write_vec2(file, position);
	save_write_vec2(file, size);
	save_write_vec2(file, scale);
	save_write_vec4(file, colour);
}

void Editor_Entity::read_save(Save_File *file) {
	save_read_string(file, type_name);
	save_read_string(file, name);
	save_read_string(file, texture_name);
	save_read_vec2(file, &position);
	save_read_vec2(file, &size);
	save_read_vec2(file, &scale);
	save_read_vec4(file, &colour);
}

void Editor_Entity::render() {
	Render_Texture rt;
	rt.position = position;
	rt.size = size;
	rt.scale = scale;
	rt.texture = texture;
	rt.centered = true;
	renderer.texture(&rt);

	Vec4 c = Vec4(1, 1, 1, 1);
	if (editor.selected_entities.find(this) || hovered) {
		c = Vec4(0, 1, 0, 1);
	}

	float hw = size.x / 2;
	float hh = size.y / 2;
	renderer.box(false, position.x-hw, position.y-hh, size.x*scale.x, size.y*scale.y, c);
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

	for (int i = 0; i < entity_manager.entity_types.num; i++) {
		entity_type_names.append(entity_manager.entity_types[i]->name);
	}
}

void Editor::shutdown() {
	nk_sdl_shutdown();
	remove_all();
}

void Editor::update() {
	For(entities) {
		if ((*it)->texture_name[0]) {
			(*it)->texture = tex.load((*it)->texture_name);
		}
	}

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
	nk_labelf(context, NK_TEXT_LEFT, "Mode: %s (zoom: %f)", mode == EDITOR_SELECT ? "select" : "entity", renderer.scale_for_zoom_level(renderer.zoom_level));
	nk_end(context);

	nk_begin(context, "Properties", nk_rect(1920 - 340 - 1, 10, 356, 1080 - 66 - 64), NK_WINDOW_BORDER | NK_WINDOW_TITLE);

	int tree_id = 1;
	if (selected_entities.num == 1) {
		Editor_Entity *entity = selected_entities[0];

		nk_layout_row_dynamic(context, 20, 1);

		// type
		if (nk_tree_push_id(context, NK_TREE_TAB, "Type", NK_MINIMIZED, tree_id++)) {
			entity->current_entity_type_num = nk_combo(
				context, entity_type_names.data, entity_type_names.num, entity->current_entity_type_num, 25, nk_vec2(200, 200));
			strcpy(entity->type_name, entity_type_names[entity->current_entity_type_num]);
			nk_tree_pop(context);
		}

		// name
		nk_label(context, "Name", NK_LEFT);
		nk_edit_string_zero_terminated(context, NK_EDIT_FIELD, entity->name, 256, nullptr);

		// texture
		if (nk_tree_push_id(context, NK_TREE_TAB, "Texture", NK_MINIMIZED, tree_id++)) {
			nk_layout_row_static(context, 64, 64, 1);
			if (entity->texture) {
				nk_button_image(context, nk_image_id(entity->texture->api_object));
			}
			else {
				nk_button_image(context, nk_image_id(-1));
			}
			nk_layout_row_dynamic(context, 30, 1);

			if (nk_button_label(context, "Find texture...")) {
				sys.open_file_dialogue("data/textures/", "PNG Files\0*.png\0\0", entity->texture_name);
			}
			nk_tree_pop(context);
		}

		// position
		if (nk_tree_push_id(context, NK_TREE_TAB, "Position", NK_MINIMIZED, tree_id++)) {
			nk_property_float(context, "#x", -10000, &entity->position.x, 10000, 1, 1);
			nk_property_float(context, "#y", -10000, &entity->position.y, 10000, 1, 1);
			nk_tree_pop(context);
		}

		// size
		if (nk_tree_push_id(context, NK_TREE_TAB, "Size", NK_MINIMIZED, tree_id++)) {
			nk_property_float(context, "#x", -10000, &entity->size.x, 10000, 1, 1);
			nk_property_float(context, "#y", -10000, &entity->size.y, 10000, 1, 1);
			nk_tree_pop(context);
		}

		// scale
		if (nk_tree_push_id(context, NK_TREE_TAB, "Scale", NK_MINIMIZED, tree_id++)) {
			nk_property_float(context, "#x", -10000, &entity->scale.x, 10000, 1, 1);
			nk_property_float(context, "#y", -10000, &entity->scale.y, 10000, 1, 1);
			nk_tree_pop(context);
		}
		
		// colour
		if (nk_tree_push_id(context, NK_TREE_TAB, "Colour", NK_MINIMIZED, tree_id++)) {
			nk_property_float(context, "#r", 0, &entity->colour.x, 1, 0.01f, 0.01f);
			nk_property_float(context, "#g", 0, &entity->colour.y, 1, 0.01f, 0.01f);
			nk_property_float(context, "#b", 0, &entity->colour.y, 1, 0.01f, 0.01f);
			nk_property_float(context, "#a", 0, &entity->colour.y, 1, 0.01f, 0.01f);
			nk_tree_pop(context);
		}
	}

	nk_end(context);
}

void Editor::remove_all() {
	for (int i = 0; i < entities.num; i++) {
		delete entities[i];
	}

	entities.num = 0;
	clear_selected_entities();
}

void Editor::add_entity(Editor_Entity *entity) {
	entity->index = entities.num;
	entities.append(entity);
}

void Editor::delete_entity(Editor_Entity *entity) {
	if (entity->index < 0) {
		return;
	}

	entities.remove(entity->index);
	delete entity;

	For(entities) {
		(*it)->index = it_index;
	}
}

void Editor::clear_selected_entities() {
	selected_entities.num = 0;
}

void Editor::render() {
	nk_sdl_render(NK_ANTI_ALIASING_OFF);

	glEnable(GL_SCISSOR_TEST);
	glScissor(edit_window_position.x, edit_window_position.y + (1080 - edit_window_size.y), edit_window_size.x, edit_window_size.y);

	renderer.use_zoom = true;

	For(entities) {
		(*it)->render();
	}

	if (drag_select) {
		renderer.box(false, drag_start_point.x, drag_start_point.y, drag_size.x, drag_size.y, Vec4(0, 1, 0, 1));
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	renderer.setup_render();

	glColor4f(1, 1, 1, 0.1f);
	glBegin(GL_LINES);

	int grid_size = 32;
	int num_x = 10000 / grid_size;
	int num_y = 10000 / grid_size;
	for (int x = -num_x; x < num_x; x++) {
		glVertex2f(x * grid_size, -10000);
		glVertex2f(x * grid_size, 10000);
	}
	for (int y = -num_y; y < num_y; y++) {
		glVertex2f(-10000, y * grid_size);
		glVertex2f(10000, y * grid_size);
	}

	glEnd();

	glPopMatrix();

	glDisable(GL_SCISSOR_TEST);

	renderer.use_camera = false;
	renderer.use_zoom = false;
	renderer.box(false, 128-16, 0, 1920 - 456, 1080 - 80, Vec4(1, 1, 1, 1));
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
				Editor_Entity *entity = nullptr;
				For(entities) { // don't start drag select if dragging a thing
					if (point_intersects_centered_box(renderer.to_world_pos(position), (*it)->position, (*it)->size)) {
						entity = *it;
						break;
					}
				}
				if (entity && selected_entities.find(entity)) {
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

					// if drawing a box to the left or above where initially clicked
					if (drag_size.x == 0) {
						drag_size.x = 1;
					}
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
						clear_selected_entities();
					}

					For(entities) {
						if (box_intersects_centered_box(drag_start_point, drag_size, (*it)->position, (*it)->size)) {
							selected_entities.append(*it);
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
				add_entity(new_entity);
				clear_selected_entities();
				selected_entities.append(new_entity);
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
			For(selected_entities) {
				if ((*it)->draggable) {
					(*it)->position = (*it)->position + (Vec2(relx, rely) * renderer.inverse_scale_for_zoom_level(renderer.zoom_level));
				}
			}
		}

		if (drag_select) {
			drag_size = renderer.to_world_pos(sys.cursor_position) - drag_start_point;

			Vec2 start_point = drag_start_point;
			Vec2 size = drag_size;

			// if clicked and didn't move mouse
			if (start_point.x == 0) {
				start_point.x = renderer.to_world_pos(sys.cursor_position).x;
			}
			if (start_point.y == 0) {
				start_point.y = renderer.to_world_pos(sys.cursor_position).y;
			}

			// if drawing a box to the left or above where initially clicked
			if (size.x == 0) {
				size.x = 1;
			}
			else if (size.x < 0) {
				size.x = size.x * -1;
				start_point.x = start_point.x - size.x;
			}
			if (size.y == 0) {
				size.y = 1;
			}
			else if (size.y < 0) {
				size.y = size.y * -1;
				start_point.y = start_point.y - size.y;
			}

			For(entities) {
				float hw = ((*it)->size.x / 2);
				float hh = ((*it)->size.y / 2);
				if (box_intersects_box(start_point, size, (*it)->position - Vec2(hw, hh), (*it)->size)) {
					(*it)->hovered = true;
				}
				else {
					(*it)->hovered = false;
				}
			}
		}
		else {
			For(entities) {
				float hw = ((*it)->size.x / 2);
				float hh = ((*it)->size.y / 2);
				if (point_intersects_box(renderer.to_world_pos(sys.cursor_position), (*it)->position - Vec2(hw, hh), (*it)->size)) {
					(*it)->hovered = true;
				}
				else {
					(*it)->hovered = false;
				}
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
			if (selected_entities.num > 0) {
				For(selected_entities) {
					delete_entity((*it));
				}
				clear_selected_entities();
			}
		}
	}
}

void Editor::handle_mouse_wheel(int amount) {
	//renderer.zoom_level += amount;
	Vec2 diff = ((sys.window_size * 0.5f) - (sys.cursor_position)) * renderer.scale_for_zoom_level(renderer.zoom_level);
	if (amount > 0) {
		renderer.camera_position = renderer.camera_position - diff;
	}
	else {
		renderer.camera_position = renderer.camera_position + diff;
	}
	console.printf("added (%.2f, %.2f)\n", diff.x, diff.y);
}

void Editor::save(const char *file_name) {
	// !!!!! if you add anything to this you must increment map_file_version in editor.h !!!!!

	Save_File file;
	if (!save_open_write(file_name, &file)) {
		console.printf("Failed to open map file for writing from editor: %d\n", errno);
		return;
	}

	save_write_int(&file, map_file_version);

	save_write_int(&file, entities.num);
	For(entities) {
		(*it)->write_save(&file);
	}

	save_close(&file);
}

void Editor::load_map_into_editor(const char *file_name) {
	Save_File file;
	if (!save_open_read(file_name, &file)) {
		console.printf("Failed to open map file for reading from editor: %d\n", errno);
		return;
	}

	int version = 0;
	save_read_int(&file, &version);
	if (version != map_file_version) {
		console.printf("Attempting to open map file with old version from editor (wanted %d, got %d): %s\n", map_file_version, version, file_name);
		save_close(&file);
		return;
	}

	int num = 0;
	save_read_int(&file, &num);
	for (int i = 0; i < num; i++) {
		Editor_Entity *entity = new Editor_Entity;
		entity->read_save(&file);
		add_entity(entity);
	}

	save_close(&file);
}

void Editor::on_level_load() {
	remove_all();
}