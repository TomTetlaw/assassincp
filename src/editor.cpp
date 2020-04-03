#include "precompiled.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

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
	renderer.texture(&rt);

	Vec4 c = Vec4(1, 1, 1, 1);
	if (editor.selected_entities.find(this) || hovered) {
		c = Vec4(0, 1, 0, 1);
	}

	float hw = size.x / 2;
	float hh = size.y / 2;
	renderer.box(false, position.x-hw, position.y-hh, size.x*scale.x, size.y*scale.y, c);
}

bool Editor::gui_handle_event(SDL_Event *ev) {
	ImGui_ImplSDL2_ProcessEvent(ev);
	return false;
}

Texture *select_mode_on_image = nullptr;
Texture *select_mode_off_image = nullptr;
Texture *entity_mode_on_image = nullptr;
Texture *entity_mode_off_image = nullptr;
Texture *polygon_mode_on_image = nullptr;
Texture *polygon_mode_off_image = nullptr;

void Editor::init() {
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(sys.window, sys.context);
	ImGui_ImplOpenGL3_Init();

	select_mode_on_image = tex.load("data/textures/editor/select_mode_on.png");
	select_mode_off_image = tex.load("data/textures/editor/select_mode_off.png");
	entity_mode_on_image = tex.load("data/textures/editor/entity_mode_on.png");
	entity_mode_off_image = tex.load("data/textures/editor/entity_mode_off.png");
	polygon_mode_on_image = tex.load("data/textures/editor/polygon_mode_on.png");
	polygon_mode_off_image = tex.load("data/textures/editor/polygon_mode_off.png");

	edit_window_top = -(sys.window_size.y / 2);
	edit_window_left = -(sys.window_size.x / 2) + 110;
	edit_window_bottom = sys.window_size.y / 2 - 100 + 16 + 7;
	edit_window_right = (sys.window_size.x / 2) - 345;

	for (int i = 0; i < entity_manager.entity_types.num; i++) {
		entity_type_names.append(entity_manager.entity_types[i]->name);
	}
}

void Editor::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	remove_all();
}

void Editor::update() {
	For(entities) {
		if (!strcmp((*it)->type_name, "info_player_start")) {
			(*it)->texture = tex.load("data/textures/editor/info_player_start.png");
		}
		else {
			if ((*it)->texture_name[0]) {
				(*it)->texture = tex.load((*it)->texture_name);
			}
		}
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(sys.window);
	ImGui::NewFrame();

	ImGui::Begin("Mode", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
	if (ImGui::ImageButton((ImTextureID)select_mode_off_image->api_object, ImVec2(64, 64))) {
		mode = EDITOR_SELECT;
	}
	if (ImGui::ImageButton((ImTextureID)entity_mode_off_image->api_object, ImVec2(64, 64))) {
		mode = EDITOR_ENTITY;
	}
	if (ImGui::ImageButton((ImTextureID)polygon_mode_off_image->api_object, ImVec2(64, 64))) {
		mode = EDITOR_POLYGON;
	}
	ImGui::End();

	ImGui::Begin("Properties");
	if (selected_entities.num == 1) {
		Editor_Entity *entity = selected_entities[0];
		
		ImGui::InputText("Name", entity->name, 256);

		ImGui::Combo("Type", &entity->current_entity_type_num, entity_type_names.data, entity_type_names.num);

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
			sys.open_file_dialogue("data/textures/", "PNG Files\0*.png\0\0", entity->texture_name);
		}
	}
	ImGui::End();

	//static bool show_demo_window = true;
	//if (show_demo_window) {
	//	ImGui::ShowDemoWindow(&show_demo_window);
	//}
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
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glEnable(GL_SCISSOR_TEST);
	glScissor(edit_window_left + (sys.window_size.x/2), -edit_window_bottom + (sys.window_size.y / 2), edit_window_right - edit_window_left, edit_window_bottom - edit_window_top);

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

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);

	glVertex2f(edit_window_right, edit_window_top);
	glVertex2f(edit_window_left, edit_window_top);
	glVertex2f(edit_window_left, edit_window_bottom);
	glVertex2f(edit_window_right, edit_window_bottom);

	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPointSize(5);
	glBegin(GL_POINTS);
	glVertex2f(0, 0);
	glEnd();
}

void Editor::handle_mouse_press(int mouse_button, bool down, Vec2 _, bool is_double_click) {
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
				For(entities) { // don't start drag select if dragging a thing
					if (point_intersects_centered_box(renderer.to_world_pos(sys.cursor_position), (*it)->position, (*it)->size)) {
						entity = *it;
						break;
					}
				}
				if (entity && selected_entities.find(entity)) {
					dragging_thing = true;
				}
				else {
					drag_select = true;
					drag_start_point = renderer.to_world_pos(sys.cursor_position);
				}
			}
			else {
				if (dragging_thing) {
					dragging_thing = false;
				}
				else {
					// if clicked and didn't move mouse
					if (drag_start_point.x == 0) {
						drag_start_point.x = renderer.to_world_pos(sys.cursor_position).x;
					}
					if (drag_start_point.y == 0) {
						drag_start_point.y = renderer.to_world_pos(sys.cursor_position).y;
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
				//console.printf("added editor entity\n");
				new_entity->position = renderer.to_world_pos(sys.cursor_position);
				add_entity(new_entity);
				clear_selected_entities();
				selected_entities.append(new_entity);
			}
		}
	}
}

void Editor::handle_mouse_move(int relx, int rely) {
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
	//@todo: zoom to mouse position
	//renderer.camera_position = renderer.to_world_pos(sys.cursor_position);
	renderer.zoom_level += amount;
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
		//printf("saved: %d\n", (*it)->current_entity_type_num);
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
		//printf("loaded: %d\n", entity->current_entity_type_num);
		add_entity(entity);
	}

	save_close(&file);
}

void Editor::on_level_load() {
	remove_all();
}