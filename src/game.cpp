#include "precompiled.h"
Game game;

void Game::init() {
}

void Game::shutdown() {
}

void Game::update() {
	float temp = game.last_time;
	game.last_time = SDL_GetTicks() / 1000.0f;

	if (!game.paused) {
		game.game_time = game.real_time - game.total_time_paused;
	}

	game.real_delta_time = game.last_time - temp;
	game.real_time += game.delta_time;

	if (game.paused) {
		game.delta_time = 0;
	}
	else {
		game.delta_time = game.real_delta_time;
	}
}

struct Nav_Path {
	Array<Nav_Mesh_Point *> points;
};

int grid_index_plus(int index, int x, int y, int width) {
	return index + x + (-y * width);
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
			if (!neighbours[i]) {
				//printf("neighbour %d was nullptr\n", i);
				continue;
			}
			if (!neighbours[i]->valid) {
				printf("neighbour %d was not valid\n", i);
				continue;
			}
			if (neighbours[i]->visited) {
				printf("neighbour %d was visited\n", i);
				continue;
			}

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

void Game::render() {
	if (!current_level) return;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	renderer.setup_render();

	glPointSize(10.0f);
	glColor4f(1, 1, 1, 0.1f);
	glBegin(GL_POINTS);
	For(current_level->nav_points, {
		if (it.valid) {
			glVertex2f(it.point.x, it.point.y);
		}
	});
	glEnd();

	Nav_Path path;
	if (make_path(&path, game.player->position, renderer.to_world_pos(sys.cursor_position))) {
		printf("Path found: %d\n", path.points.num);
		glColor4f(1, 0, 0, 1);
		glPointSize(10.0f);
		glBegin(GL_POINTS);
		For(path.points, {
			glVertex2f(it->point.x, it->point.y);
		});
		glEnd();
	}

	glPopMatrix();
}

void Game::toggle_paused() {
	Game::set_paused(!game.paused);
}

void Game::set_paused(bool paused) {
	game.paused = paused;

	if (game.paused) {
		game.total_time_paused += game.real_time - game.time_paused;
	}
	else {
		game.time_paused = game.real_time;
	}
}

class Poly : public Entity {
public:
	cpShape *shape = nullptr;
	Array<Vec2> verts;

	void setup_physics(cpSpace *space) {
		body = cpBodyNewStatic();
		cpBodySetPosition(body, cpv(position.x, position.y));

		cpVect *cp_verts = new cpVect[verts.num];
		for (int i = 0; i < verts.num; i++) {
			cp_verts[i] = cpv(verts[i].x, verts[i].y);
		}

		cpTransform transform = cpTransformIdentity;
		shape = cpPolyShapeNew(body, verts.num, cp_verts, transform, 0.000001f);
		
		int count = cpPolyShapeGetCount(shape);
		for (int i = 0; i < count; i++) {
			cpVect v = cpPolyShapeGetVert(shape, i);
			game.current_level->fov_check_points.append(Vec2(v.x, v.y));
		}

		cpSpaceAddShape(space, shape);
		cpSpaceAddBody(space, body);
	}

	void delete_physics(cpSpace *space) {
		cpSpaceRemoveShape(space, shape);
		cpSpaceRemoveBody(space, body);
	}
};

declare_entity_type(Poly, "info_polygon", ENTITY_INFO_POLYGON);

class Poly_Point : public Entity {
};

declare_entity_type(Poly_Point, "info_poly_point", ENTITY_INFO_POLYGON_POINT);

void Game::on_level_load() {
	renderer.on_level_load();
	entity_manager.on_level_load();
	editor.on_level_load();
	delete current_level;
	current_level = nullptr;
}

void process_level();

void Game::load_level(const char *file_name) {
	current_level = new Level;

	Vec2 p[] = {
		Vec2(-10000, -10000),
		Vec2(-10000, 10000),
		Vec2(10000, -10000),
		Vec2(10000, 10000)
	};

	for (int i = 0; i < 4; i++) {
		current_level->fov_check_points.append(p[i]);
	}

	Save_File file;
	if (!save_open_read(file_name, &file)) {
		console.printf("Failed to open map file for reading from game: %d", errno);
		return;
	}

	int version = 0;
	save_read_int(&file, &version);
	if (version != map_file_version) {
		console.printf("Attempting to open map file with old version from game (wanted %d, got %d): %s\n", map_file_version, version, file_name);
		save_close(&file);
		return;
	}

	int num = 0;
	save_read_int(&file, &num);
	for (int i = 0; i < num; i++) {
		int type = 0;
		save_read_int(&file, &type);
		if (type == EDITOR_ENTITY_ENTITY) {
			Editor_Entity entity;
			entity.read_save(&file);

			Entity *ent = entity_manager.create_entity(entity.type_name, entity.name, false, false);
			if (ent) {
				ent->set_position(entity.position);
				ent->size = entity.size;
				ent->rt.scale = entity.scale;
				ent->set_texture(entity.texture_name, false);
				ent->colour = entity.colour;
				entity_manager.spawn_entity(ent);
			}
		}
		else if (type == EDITOR_ENTITY_POLYGON) {
			Editor_Polygon polygon;
			polygon.read_save(&file);

			Poly *poly = (Poly *)entity_manager.create_entity("info_polygon", nullptr, false, false);
			For(polygon.points, {
				poly->verts.append(it->position);
			});
			entity_manager.spawn_entity(poly);
		}
	}

	save_close(&file);

	process_level();
}

bool check_nav_point(Vec2 point) {
	cpSegmentQueryInfo info[8];
	float offset = game.current_level->nav_points_size / 2;
	Vec2 points[8] = {
		point + Vec2(offset, 0),
		point + Vec2(-offset, 0),
		point + Vec2(0, offset),
		point + Vec2(0, -offset),

		point + Vec2(-offset, -offset),
		point + Vec2(offset, -offset),
		point + Vec2(-offset, offset),
		point + Vec2(offset, offset),
	};

	for (int i = 0; i < 8; i++) {
		cpShapeFilter filter;
		filter.categories = 1;
		if (cpSpaceSegmentQueryFirst(entity_manager.space, cpv(point.x, point.y), cpv(points[i].x, points[i].y), 1, filter, &info[i])) {
			return false;
		}
	}

	return true;
}

void generate_nav_points() {
	int grid_size = game.current_level->nav_points_size;
	int num_x = 1000 / grid_size;
	int num_y = 1000 / grid_size;
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

void process_level() {
	bool has_one_player_start = true;
	Entity *player_start = nullptr;

	if (entity_manager.entity_types[ENTITY_INFO_PLAYER_START]->entities.num == 0) {
		console.printf("Warning: no info_player_start found!\n");
		has_one_player_start = false;
	}
	if (entity_manager.entity_types[ENTITY_INFO_PLAYER_START]->entities.num > 1) {
		console.printf("Warning: more than one info_player_start found!\n");
	}

	if (has_one_player_start) {
		player_start = entity_manager.entity_types[ENTITY_INFO_PLAYER_START]->entities[0];

		Entity *player = entity_manager.create_entity("ent_player", nullptr, false, false);
		player->position = player_start->position;
		entity_manager.spawn_entity(player);

		input.player = player;
		game.player = player;

		entity_manager.delete_entity(player_start);
	}

	generate_nav_points();
}