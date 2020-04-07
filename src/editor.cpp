#include "precompiled.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

internal bool left_button_down = false;
internal bool middle_button_down = false;
internal bool shift_down = false;

internal bool dragging_entity = false;

internal bool drag_select = false;
internal Vec2 drag_start_point;
internal Vec2 drag_size;

internal Editor_Polygon *currently_editing_polygon = nullptr;
internal bool find_polygon_point_at(Vec2 position, Editor_Polygon **poly_out, int *point_index);

internal Editor_Mode mode = EDITOR_SELECT;

internal Array<Editor_Entity *> entities;
internal Array<Editor_Entity *> selected_entities;

internal Array<int> selected_polygon_points; // indices into selected_entities[0].points

internal Array<const char *> entity_type_names; // create on init because no new types after startup

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
*/

Vec2 polygon_centre_point(Editor_Polygon *poly) {
	if (poly->points.num <= 0) {
		return poly->center;
	}

	Vec2 centroid = { 0, 0 };
	float signedArea = 0.0f;
	float x0 = 0.0f; // Current vertex X
	float y0 = 0.0f; // Current vertex Y
	float x1 = 0.0f; // Next vertex X
	float y1 = 0.0f; // Next vertex Y
	float a = 0.0f;  // Partial signed area
	int vertexCount = poly->points.num;

	// For all vertices except last
	int i = 0;
	for (i = 0; i < vertexCount - 1; ++i)
	{
		x0 = poly->points[i]->position.x;
		y0 = poly->points[i]->position.y;
		x1 = poly->points[i + 1]->position.x;
		y1 = poly->points[i + 1]->position.y;
		a = x0 * y1 - x1 * y0;
		signedArea += a;
		centroid.x += (x0 + x1)*a;
		centroid.y += (y0 + y1)*a;
	}

	// Do last vertex separately to avoid performing an expensive
	// modulus operation in each iteration.
	x0 = poly->points[i]->position.x;
	y0 = poly->points[i]->position.y;
	x1 = poly->points[0]->position.x;
	y1 = poly->points[0]->position.y;
	a = x0 * y1 - x1 * y0;
	signedArea += a;
	centroid.x += (x0 + x1)*a;
	centroid.y += (y0 + y1)*a;

	signedArea *= 0.5;
	centroid.x /= (6.0*signedArea);
	centroid.y /= (6.0*signedArea);

	return centroid;
}

/*
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

	strcpy(type_name, "ent_base");
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
	save_write_int(file, current_entity_type_num);
}

void Editor_Entity::read_save(Save_File *file) {
	save_read_string(file, type_name);
	save_read_string(file, name);
	save_read_string(file, texture_name);
	save_read_vec2(file, &position);
	save_read_vec2(file, &size);
	save_read_vec2(file, &scale);
	save_read_vec4(file, &colour);
	save_read_int(file, &current_entity_type_num);
}

void Editor_Entity::render() {
	Render_Texture rt;
	rt.position = position;
	rt.size = size;
	rt.scale = scale;
	rt.texture = texture;
	rt.centered = true;
	render_texture(&rt);

	Vec4 c = Vec4(1, 1, 1, 1);
	if (selected_entities.find(this) || hovered) {
		c = Vec4(0, 1, 0, 1);
	}

	float hw = size.x / 2;
	float hh = size.y / 2;
	render_box(false, position.x-hw, position.y-hh, size.x*scale.x, size.y*scale.y, c);
}

void Editor_Entity::on_drag(Vec2 amount) {
	position = position + amount;
}

void Editor_Entity::on_delete() { 
	entities.remove(entities.find_index(this));
}

void Editor_Polygon::render() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	render_setup_render_ui();

	glColor4f(1, 1, 1, 1);
	glBegin(GL_LINES);
	for (int i = 0; i < points.num; i++) {
		if (i + 1 < points.num) {
			glVertex2f(points[i]->position.x, points[i]->position.y);
			glVertex2f(points[i + 1]->position.x, points[i + 1]->position.y);
		}
	}
	glEnd();

	if (closed) {
		glBegin(GL_LINES);
		glVertex2f(points[points.num - 1]->position.x, points[points.num - 1]->position.y);
		glVertex2f(points[0]->position.x, points[0]->position.y);
		glEnd();
	}

	if (currently_editing_polygon == this) {
		glBegin(GL_LINES);
		glVertex2f(points[points.num - 1]->position.x, points[points.num - 1]->position.y);
		Vec2 end = render_to_world_pos(sys.cursor_position);
		glVertex2f(end.x, end.y);
		glEnd();
	}

	if (selected_entities.find(this)) {
		glColor4f(0, 1, 0, 1);
	}
	else {
		glColor4f(1, 1, 1, 1);
	}
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glVertex2f(center.x, center.y);
	glEnd();

	glPopMatrix();
}

void Editor_Polygon::on_drag(Vec2 amount) {
	For(points, {
		it->position = it->position + amount;
	});
	calculate_properties();
}

void Editor_Polygon::write_save(Save_File *file) {
	save_write_bool(file, closed);
	save_write_int(file, points.num);
	For(points, {
		save_write_vec2(file, it->position);
	});
}

internal void editor_add_entity(Editor_Entity *entity) {
	entity->index = entities.num;
	entities.append(entity);
}

void Editor_Polygon::read_save(Save_File *file) {
	strcpy(type_name, "info_polygon");
	save_read_bool(file, &closed);

	int num = 0;
	save_read_int(file, &num);
	for (int i = 0; i < num; i++) {
		Vec2 v;
		save_read_vec2(file, &v);
		Editor_Polygon_Point *point = new Editor_Polygon_Point;
		point->current_entity_type_num = ENTITY_INFO_POLYGON_POINT;
		strcpy(point->type_name, "info_poly_point");
		point->parent = this;
		point->position = v;
		editor_add_entity(point);
		points.append(point);
	}
}

void Editor_Polygon::calculate_properties() {
	center = polygon_centre_point(this);
	position = center;
}

void Editor_Polygon::on_delete() {
	For(points, {
		entities.remove(entities.find_index(it));
	});

	For(points, {
		delete it;
	});

	Editor_Entity::on_delete();
}

void Editor_Polygon_Point::render() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	render_setup_render_ui();

	glBegin(GL_POINTS);
	if (selected_entities.find(this)) {
		glColor4f(0, 1, 0, 1);
	}
	else {
		glColor4f(1, 1, 1, 1);
	}
	glVertex2f(position.x, position.y);
	glEnd();

	glPopMatrix();
}

void Editor_Polygon_Point::on_drag(Vec2 amount) {
	// only drag if our parent isn't selected
	// because that would cause it to drag twice
	if (!selected_entities.find(parent)) {
		position = position + amount;
		parent->calculate_properties();
	}
}

void Editor_Polygon_Point::on_delete() {
	// make sure we remove ourself from parent->points
	parent->points.remove(parent->points.find_index(this));
	parent->calculate_properties();

	Editor_Entity::on_delete();
}

bool editor_gui_handle_event(SDL_Event *ev) {
	ImGui_ImplSDL2_ProcessEvent(ev);
	return false;
}

internal Texture *select_mode_on_image = nullptr;
internal Texture *select_mode_off_image = nullptr;
internal Texture *entity_mode_on_image = nullptr;
internal Texture *entity_mode_off_image = nullptr;
internal Texture *polygon_mode_on_image = nullptr;
internal Texture *polygon_mode_off_image = nullptr;

internal float edit_window_top = 0.0f;
internal float edit_window_left = 0.0f;
internal float edit_window_bottom = 0.0f;
internal float edit_window_right = 0.0f;

void editor_init() {
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(sys.window, sys.context);
	ImGui_ImplOpenGL3_Init();

	select_mode_on_image = load_texture("data/textures/editor/select_mode_on.png");
	select_mode_off_image = load_texture("data/textures/editor/select_mode_off.png");
	entity_mode_on_image = load_texture("data/textures/editor/entity_mode_on.png");
	entity_mode_off_image = load_texture("data/textures/editor/entity_mode_off.png");
	polygon_mode_on_image = load_texture("data/textures/editor/polygon_mode_on.png");
	polygon_mode_off_image = load_texture("data/textures/editor/polygon_mode_off.png");

	edit_window_top = -(sys.window_size.y / 2);
	edit_window_left = -(sys.window_size.x / 2) + 110;
	edit_window_bottom = sys.window_size.y / 2 - 100 + 16 + 7;
	edit_window_right = (sys.window_size.x / 2) - 345;

	for (int i = 0; i < entity_manager.entity_types.num; i++) {
		entity_type_names.append(entity_manager.entity_types[i]->name);
	}
}

void editor_shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	editor_remove_all();
}

internal void clear_selected_entities() {
	selected_entities.num = 0;
	selected_polygon_points.num = 0;
}

internal bool find_polygon_point_at(Vec2 position, Editor_Polygon **poly_out, int *point_index) {
	For(entities, {
		if (it->type == EDITOR_ENTITY_POLYGON) {
			for (int i = 0; i < ((Editor_Polygon *)it)->points.num; i++) {
				if (((Editor_Polygon *)it)->points[i]->position.distance_to(position) < 10.0f) {
					*poly_out = ((Editor_Polygon *)it);
					*point_index = i;
					return true;
				}
			}
		}
	});

	return false;
}

void editor_update() {
	// @cleanup: don't do this every frame, only on change or selected new image
	For(entities, {
		if (!strcmp(it->type_name, "info_player_start")) {
			strcpy(it->texture_name, "data/textures/editor/info_player_start.png");
			it->texture = load_texture("data/textures/editor/info_player_start.png");
		}
		else {
			if (it->texture_name[0]) {
				it->texture = load_texture(it->texture_name);
			}
		}
	});

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(sys.window);
	ImGui::NewFrame();

	ImGui::Begin("Mode", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
	if (ImGui::ImageButton((ImTextureID)select_mode_off_image->api_object, ImVec2(64, 64))) {
		mode = EDITOR_SELECT;
		clear_selected_entities();
	}
	if (ImGui::ImageButton((ImTextureID)entity_mode_off_image->api_object, ImVec2(64, 64))) {
		mode = EDITOR_ENTITY;
		clear_selected_entities();
	}
	if (ImGui::ImageButton((ImTextureID)polygon_mode_off_image->api_object, ImVec2(64, 64))) {
		mode = EDITOR_POLYGON;
		clear_selected_entities();
	}
	ImGui::End();

	if (selected_entities.num == 1) {
		ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
		Editor_Entity *entity = selected_entities[0];

		ImGui::InputText("Name", entity->name, 256);

		if (ImGui::Combo("Type", &entity->current_entity_type_num, entity_type_names.data, entity_type_names.num)) {
			strcpy(entity->type_name, entity_type_names[entity->current_entity_type_num]);
		}

		{
			float data[2] = { entity->position.x, entity->position.y };
			if (ImGui::DragFloat2("Position", data)) {
				entity->position.x = data[0];
				entity->position.y = data[1];
			}
		}

		{
			float data[2] = { entity->size.x, entity->size.y };
			if (ImGui::DragFloat2("Size", data)) {
				entity->size.x = data[0];
				entity->size.y = data[1];
			}
		}

		{
			float data[2] = { entity->scale.x, entity->scale.y };
			if (ImGui::DragFloat2("Scale", data)) {
				entity->scale.x = data[0];
				entity->scale.y = data[1];
			}
		}

		{
			float data[4] = { entity->colour.x, entity->colour.y, entity->colour.z, entity->colour.w };
			if (ImGui::DragFloat4("Colour", data)) {
				entity->colour.x = data[0];
				entity->colour.y = data[1];
				entity->colour.z = data[2];
				entity->colour.w = data[3];
			}
		}

		int id = entity->texture ? entity->texture->api_object : -1;
		if (ImGui::ImageButton((ImTextureID)id, ImVec2(32, 32))) {
			system_open_file_dialogue("data/textures/", "PNG Files\0*.png\0\0", entity->texture_name);
		}
		ImGui::End();
	}
	else if (selected_entities.num > 1) {
		ImGui::Begin("Selected");
		for (int i = 0; i < selected_entities.num; i++) {
			char buffer[256] = { 0 };
			sprintf(buffer, "%s (%d): %s", selected_entities[i]->name, selected_entities[i]->index, selected_entities[i]->type_name);
			if (ImGui::Selectable(buffer, false)) {
				selected_entities[0] = selected_entities[i];
				selected_entities.num = 1;
			}
		}
		ImGui::End();
	}

	static bool show_demo_window = true;
	if (show_demo_window) {
		ImGui::ShowDemoWindow(&show_demo_window);
	}
}

void editor_remove_all() {
	for (int i = 0; i < entities.num; i++) {
		delete entities[i];
	}

	entities.num = 0;
	clear_selected_entities();
}

void editor_delete_entity(Editor_Entity *entity) {
	if (entity->index < 0) {
		return;
	}

	entities.remove(entity->index);
	delete entity;

	For(entities, {
		it->index = it_index;
	});
}

void editor_render() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glEnable(GL_SCISSOR_TEST);
	glScissor(edit_window_left + (sys.window_size.x / 2), -edit_window_bottom + (sys.window_size.y / 2), edit_window_right - edit_window_left, edit_window_bottom - edit_window_top);

	For(entities, {
		it->render();
	});

	if (drag_select) {
		render_box(false, drag_start_point.x, drag_start_point.y, drag_size.x, drag_size.y, Vec4(0, 1, 0, 1));
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	render_setup_render_ui();

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

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);

	glVertex2f(edit_window_right, edit_window_top);
	glVertex2f(edit_window_left, edit_window_top);
	glVertex2f(edit_window_left, edit_window_bottom);
	glVertex2f(edit_window_right, edit_window_bottom);

	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void editor_handle_mouse_press(int mouse_button, bool down, Vec2 _, bool is_double_click) {
	if (mouse_button == SDL_BUTTON_LEFT) {
		left_button_down = down;
	}
	if (mouse_button == SDL_BUTTON_MIDDLE) {
		middle_button_down = down;
	}

	if (sys.cursor_position.x < edit_window_left) {
		return;
	}
	if (sys.cursor_position.x > edit_window_right) {
		return;
	}
	if (sys.cursor_position.y < edit_window_top) {
		return;
	}
	if (sys.cursor_position.y > edit_window_bottom) {
		return;
	}

	if (mode == EDITOR_SELECT) {
		if (mouse_button == SDL_BUTTON_LEFT) {
			if (down) {
				Editor_Entity *entity = nullptr;
				For(entities, { // don't start drag select if dragging a thing
					if (point_intersects_centered_box(render_to_world_pos(sys.cursor_position), it->position, it->size)) {
						entity = it;
						break;
					}
				});
				if (entity && selected_entities.find(entity)) {
					dragging_entity = true;
				}
				else {
					drag_select = true;
					drag_start_point = render_to_world_pos(sys.cursor_position);
				}
			}
			else {
				if (dragging_entity) {
					dragging_entity = false;
				}
				else {
					// if clicked and didn't move mouse
					if (drag_start_point.x == 0) {
						drag_start_point.x = render_to_world_pos(sys.cursor_position).x;
					}
					if (drag_start_point.y == 0) {
						drag_start_point.y = render_to_world_pos(sys.cursor_position).y;
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

					For(entities, {
						if (box_intersects_centered_box(drag_start_point, drag_size, it->position, it->size)) {
							selected_entities.append(it);
						}
					});

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
				new_entity->position = render_to_world_pos(sys.cursor_position);
				editor_add_entity(new_entity);
				clear_selected_entities();
				selected_entities.append(new_entity);
			}
		}
	}
	else if (mode == EDITOR_POLYGON) {
		if (mouse_button == SDL_BUTTON_LEFT && down) {
			if (currently_editing_polygon) {
				Editor_Polygon *polygon = nullptr;
				int point_index = -1;

				if(find_polygon_point_at(render_to_world_pos(sys.cursor_position), &polygon, &point_index)) {
					polygon->closed = true;
					currently_editing_polygon = nullptr;
				}
				else {
					Editor_Polygon_Point *point = new Editor_Polygon_Point;
					point->parent = currently_editing_polygon;
					point->position = render_to_world_pos(sys.cursor_position);
					point->current_entity_type_num = ENTITY_INFO_POLYGON_POINT;
					editor_add_entity(point);
					currently_editing_polygon->points.append(point);
					currently_editing_polygon->calculate_properties();
				}
			}
			else {
				Editor_Polygon *polygon = new Editor_Polygon;
				currently_editing_polygon = polygon;
				Editor_Polygon_Point *point = new Editor_Polygon_Point;
				point->current_entity_type_num = ENTITY_INFO_POLYGON_POINT;
				point->parent = polygon;
				point->position = render_to_world_pos(sys.cursor_position);
				editor_add_entity(point);
				polygon->points.append(point);
				polygon->calculate_properties();
				editor_add_entity(polygon);
			}
		}
	}
}

void editor_handle_mouse_move(int relx, int rely) {
	if (middle_button_down) {
		renderer.camera_position = renderer.camera_position - Vec2(relx, rely);
	}

	if (mode == EDITOR_SELECT) {
		if (dragging_entity) {
			For(selected_entities, {
				if (it->draggable) {
					it->on_drag(Vec2(relx, rely) * render_inverse_scale_for_zoom_level());
				}
			});
		}

		if (drag_select) {
			drag_size = render_to_world_pos(sys.cursor_position) - drag_start_point;

			Vec2 start_point = drag_start_point;
			Vec2 size = drag_size;

			// if clicked and didn't move mouse
			if (start_point.x == 0) {
				start_point.x = render_to_world_pos(sys.cursor_position).x;
			}
			if (start_point.y == 0) {
				start_point.y = render_to_world_pos(sys.cursor_position).y;
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

			For(entities, {
				float hw = (it->size.x / 2);
				float hh = (it->size.y / 2);
				if (box_intersects_box(start_point, size, it->position - Vec2(hw, hh), it->size)) {
					it->hovered = true;
				}
				else {
					it->hovered = false;
				}
			});
		}
		else {
			For(entities, {
				float hw = (it->size.x / 2);
				float hh = (it->size.y / 2);
				if (point_intersects_box(render_to_world_pos(sys.cursor_position), it->position - Vec2(hw, hh), it->size)) {
					it->hovered = true;
				}
				else {
					it->hovered = false;
				}
			});
		}
	}
	else if (mode == EDITOR_ENTITY) {
	}
}

void editor_handle_key_press(SDL_Scancode scancode, bool down, int mods) {
	if (scancode == SDL_SCANCODE_LSHIFT) {
		shift_down = down;
	}

	if (mode == EDITOR_SELECT) {
		if (down && scancode == SDL_SCANCODE_DELETE) {
			if (selected_entities.num > 0) {
				For(selected_entities, {
					it->on_delete();
					delete it;
				});
				clear_selected_entities();
			}
		}
	}
}

void editor_handle_mouse_wheel(int amount) {
	//@todo: zoom to mouse position
	//renderer.camera_position = renderer.to_world_pos(sys.cursor_position);
	renderer.zoom_level += amount;
}

void editor_save(const char *file_name) {
	// !!!!! if you add anything to this you must increment map_file_version in editor.h !!!!!

	Save_File file;
	if (!save_open_write(file_name, &file)) {
		console_printf("Failed to open map file for writing from editor: %d\n", errno);
		return;
	}

	save_write_int(&file, map_file_version);

	save_write_int(&file, entities.num);
	For(entities, {
		save_write_int(&file, it->type);
		it->write_save(&file);
	});

	save_close(&file);
}

void editor_load_map_into_editor(const char *file_name) {
	Save_File file;
	if (!save_open_read(file_name, &file)) {
		console_printf("Failed to open map file for reading from editor: %d\n", errno);
		return;
	}

	int version = 0;
	save_read_int(&file, &version);
	if (version != map_file_version) {
		console_printf("Attempting to open map file with old version from editor (wanted %d, got %d): %s\n", map_file_version, version, file_name);
		save_close(&file);
		return;
	}

	int num = 0;
	save_read_int(&file, &num);
	for (int i = 0; i < num; i++) {
		int type = 0;
		save_read_int(&file, &type);
		if (type == EDITOR_ENTITY_ENTITY) {
			Editor_Entity *entity = new Editor_Entity;
			entity->read_save(&file);
			//printf("loaded: %d\n", entity->current_entity_type_num);
			editor_add_entity(entity);
		}
		else if (type == EDITOR_ENTITY_POLYGON) {
			Editor_Polygon *polygon = new Editor_Polygon;
			polygon->read_save(&file);
			polygon->calculate_properties();
			editor_add_entity(polygon);
		}
	}

	save_close(&file);
}

void editor_on_level_load() {
	editor_remove_all();
}