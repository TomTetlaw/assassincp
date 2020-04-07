#include "precompiled.h"

Game game;

internal float real_time = 0.0f;
internal float last_time = 0.0f;
internal float real_delta_time = 0.0f;
internal bool paused = false;
internal float time_paused = 0.0f;
internal float total_time_paused = 0.0f;
internal int nav_grid_width = 2000;
internal int nav_grid_height = 2000;

internal bool check_nav_point(Vec2 point) {
	return false;
}

internal void generate_nav_points() {
	int grid_size = game.current_level->nav_points_size;
	int num_x = nav_grid_width / grid_size;
	int num_y = nav_grid_height / grid_size;
	game.current_level->nav_points_width = num_x * 2;
	game.current_level->nav_points_height = num_y * 2;
	for (int y = -num_y; y < num_y; y++) {
		for (int x = -num_x; x < num_x; x++) {
			Vec2 p = Vec2(x * grid_size, y * grid_size);
			Nav_Mesh_Point point;
			point.point = p;
			point.valid = check_nav_point(p);
			point.grid_index = game.current_level->nav_points.num;
			game.current_level->nav_points.append(point);
		}
	}
}

internal void regen_nav_points(Config_Var *var) {
	game.current_level->nav_points.num = 0;
	generate_nav_points();
}

void game_init() {
	register_var("nav_grid_width", &nav_grid_width, regen_nav_points);
	register_var("nav_grid_height", &nav_grid_height, regen_nav_points);
}

void game_update() {
	float temp = last_time;
	last_time = SDL_GetTicks() / 1000.0f;

	if (!paused) {
		game.game_time = real_time - total_time_paused;
	}

	real_delta_time = last_time - temp;
	real_time += game.delta_time;

	if (paused) {
		game.delta_time = 0;
	}
	else {
		game.delta_time = real_delta_time;
	}
}

void get_neighbours(int index, Nav_Mesh_Point *neighbours[8]) {
	int offsets[8][2] = {
		{1,0},
		{-1,0},
		{0,1},
		{0,-1},

		{-1,-1},
		{1,-1},
		{1,1},
		{-1,1}
	};

	for (int i = 0; i < 8; i++) {
		int neighbour_index = index + (offsets[i][1] * game.current_level->nav_points_width) + (offsets[i][0]);
		if (neighbour_index > 0 && neighbour_index < game.current_level->nav_points.num) {
			neighbours[i] = &game.current_level->nav_points[neighbour_index];
		}
		else {
			neighbours[i] = nullptr;
		}
	}
}

void position_to_grid_index(Vec2 position, int *grid_x, int *grid_y) {
	int grid_index_x = (int)(position.x / game.current_level->nav_points_size);
	grid_index_x += (game.current_level->nav_points_width / 2);
	if (grid_index_x < 0) grid_index_x += game.current_level->nav_points_width;
	*grid_x = grid_index_x;

	int grid_index_y = (int)(position.y / game.current_level->nav_points_size);
	grid_index_y += (game.current_level->nav_points_height / 2);
	if (grid_index_y < 0) grid_index_y += game.current_level->nav_points_height;
	*grid_y = grid_index_y;
}

bool make_path(Nav_Path *path, Vec2 from, Vec2 to) {
	int num = game.current_level->nav_points.num;

	for (int i = 0; i < game.current_level->nav_points.num; i++) {
		game.current_level->nav_points[i].visited = false;
	}

	int grid_from_x = 0;
	int grid_from_y = 0;
	position_to_grid_index(from, &grid_from_x, &grid_from_y);

	int grid_to_x = 0;
	int grid_to_y = 0;
	position_to_grid_index(to, &grid_to_x, &grid_to_y);

	int grid_index_from = (grid_from_y * game.current_level->nav_points_width) + grid_from_x;
	int grid_index_to = (grid_to_y * game.current_level->nav_points_width) + grid_to_x;
	Vec2 goal_pos = game.current_level->nav_points[grid_index_to].point;

	int current = grid_index_from;
	while (current != grid_index_to) {
		Nav_Mesh_Point *neighbours[8] = { 0 };
		get_neighbours(current, neighbours);

		int lowest_index = -1;
		float lowest = 10000.0f;
		for (int i = 0; i < 8; i++) {
			if (!neighbours[i]) continue;
			if (!neighbours[i]->valid) continue;
			if (neighbours[i]->visited) continue;

			Vec2 next_pos = neighbours[i]->point;
			float dist = next_pos.distance_to(goal_pos);
			if (dist < lowest) {
				lowest = dist;
				lowest_index = i;
			}

			neighbours[i]->visited = true;
		}

		if (lowest_index == -1) {
			return false;
		}

		path->points.append(neighbours[lowest_index]);
		current = neighbours[lowest_index]->grid_index;
	}

	return true;
}

void game_render() {
	if (!game.current_level) return;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	render_setup_render_world();
	glPointSize(10.0f);
	glColor4f(1, 1, 1, 0.1f);
	glBegin(GL_POINTS);
	For(game.current_level->nav_points) {
		auto it = game.current_level->nav_points[it_index];
		if (it.valid) {
			glVertex2f(it.point.x, it.point.y);
		}
	}
	glEnd();

	glPopMatrix();
}

void game_toggle_paused() {
	game_set_paused(!paused);
}

void game_set_paused(bool paused) {
	paused = paused;

	if (paused) {
		total_time_paused += real_time - time_paused;
	}
	else {
		time_paused = real_time;
	}
}

class Poly : public Entity {
public:
	Array<Vec2> verts;

	void render() {
	}
};

declare_entity_type(Poly, "info_polygon", ENTITY_INFO_POLYGON);

class Poly_Point : public Entity {
};

declare_entity_type(Poly_Point, "info_poly_point", ENTITY_INFO_POLYGON_POINT);

void on_level_load() {
	render_on_level_load();
	entity_on_level_load();
	editor_on_level_load();
	delete game.current_level;
	game.current_level = nullptr;
}

internal void process_level() {
	bool has_one_player_start = true;
	Entity *player_start = nullptr;

	if (entity_manager.entity_types[ENTITY_INFO_PLAYER_START]->entities.num == 0) {
		console_printf("Warning: no info_player_start found!\n");
		has_one_player_start = false;
	}
	if (entity_manager.entity_types[ENTITY_INFO_PLAYER_START]->entities.num > 1) {
		console_printf("Warning: more than one info_player_start found!\n");
	}

	if (has_one_player_start) {
		player_start = entity_manager.entity_types[ENTITY_INFO_PLAYER_START]->entities[0];

		Entity *player = create_entity("ent_player", nullptr, false, false);
		player->position = player_start->position;
		spawn_entity(player);

		input.player = player;
		game.player = player;

		delete_entity(player_start);
	}

	generate_nav_points();
}

void load_level(const char *file_name) {
	game.current_level = new Level;

	Vec2 p[] = {
		Vec2(-10000, -10000),
		Vec2(-10000, 10000),
		Vec2(10000, -10000),
		Vec2(10000, 10000)
	};

	for (int i = 0; i < 4; i++) {
		game.current_level->fov_check_points.append(p[i]);
	}

	Save_File file;
	if (!save_open_read(file_name, &file)) {
		console_printf("Failed to open map file for reading from game: %d", errno);
		return;
	}

	int version = 0;
	save_read_int(&file, &version);
	if (version != map_file_version) {
		console_printf("Attempting to open map file with old version from game (wanted %d, got %d): %s\n", map_file_version, version, file_name);
		save_close(&file);
		return;
	}

	texture_begin_level_load();

	int num = 0;
	save_read_int(&file, &num);
	for (int i = 0; i < num; i++) {
		int type = 0;
		save_read_int(&file, &type);
		if (type == EDITOR_ENTITY_ENTITY) {
			Editor_Entity entity;
			entity.read_save(&file);

			Entity *ent = create_entity(entity.type_name, entity.name, false, false);
			if (ent) {
				ent->position = entity.position;
				ent->size = entity.size;
				ent->rt.scale = entity.scale;
				ent->set_texture(entity.texture_name, false);
				ent->colour = entity.colour;
				spawn_entity(ent);
			}
		}
		else if (type == EDITOR_ENTITY_POLYGON) {
			Editor_Polygon polygon;
			polygon.read_save(&file);

			Poly *poly = (Poly *)create_entity("info_polygon", nullptr, false, false);
			For(polygon.points) {
				auto it = polygon.points[it_index];
				poly->verts.append(it->position);
			}
			spawn_entity(poly);
		}
	}

	save_close(&file);

	texture_end_level_load();

	process_level();
}