#include "precompiled.h"

Game game;

internal int nav_grid_width = 2000;
internal int nav_grid_height = 2000;
internal int nav_grid_size = 32.0f;

internal bool check_nav_point(Vec2 point) {
	float offset = nav_grid_size / 2;
	Vec2 offsets[8] = {
		{offset, 0},
		{-offset, 0},
		{0, offset},
		{0, -offset},

		{-offset, -offset},
		{offset, -offset},
		{offset, offset},
		{-offset, offset}
	};

	for(int i = 0; i < 8; i++) {
		Raycast_Hit hit;
		Collision_Filter filter;
		filter.mask = phys_group_wall;
		if(raycast(point, point + offsets[i], &hit, filter)) return false;
	}

	return true;
}

internal void generate_nav_points() {
	int grid_size = nav_grid_size;
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

internal void add_check_points() {
	Vec2 points[4] = { 
		{-10000, -10000},
		{10000, -10000},
		{-10000, 10000},
		{10000, 10000},
	};

	for(int i = 0; i < 4; i++) {
		game.current_level->fov_check_points.append(points[i]);
	}

	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
        if(!entity) continue;
		if(!(entity->flags & EFLAGS_NO_PHYSICS)) continue;
		if(!entity->added) continue;
		if(entity->classify != etypes._classify_Wall) continue;

		for(int j = 0; j < 4; j++) {
			game.current_level->fov_check_points.append(entity->po.edges[j].a);
		}
	}	
}

internal void regen_nav_points(Config_Var *var) {
	if(game.current_level) {
		game.current_level->nav_points.num = 0;
		generate_nav_points();
	}
}

void game_init() {
	register_var("nav_grid_width", &nav_grid_width, regen_nav_points);
	register_var("nav_grid_height", &nav_grid_height, regen_nav_points);
	register_var("nav_grid_size", &nav_grid_size, regen_nav_points);
}

void game_update() {
	game.now = sys.current_time;
	game.delta_time = sys.delta_time;
	if(game.paused) game.delta_time = 0;
}

internal void get_neighbours(int index, Nav_Mesh_Point *neighbours[8]) {
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

internal void position_to_grid_index(Vec2 position, int *grid_x, int *grid_y) {
	int grid_index_x = (int)(position.x / nav_grid_size);
	grid_index_x += (game.current_level->nav_points_width / 2);
	if (grid_index_x < 0) grid_index_x += game.current_level->nav_points_width;
	*grid_x = grid_index_x;

	int grid_index_y = (int)(position.y / nav_grid_size);
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
		float lowest = 10000.0f; //@todo large enough?
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

	render_setup_for_world();

	For(game.current_level->nav_points) {
		auto it = game.current_level->nav_points[it_index];
		if (it.valid) {
			render_point(it.point, 10.0f, Vec4(1, 1, 1, 0.1f));
		}
	}
}

void on_level_load() {
	render_on_level_load();
	entity_on_level_load();
	editor_on_level_load();
	delete game.current_level;
	game.current_level = nullptr;
}

void load_level() {
	game.current_level = new Level;
	add_check_points();
	generate_nav_points();
	entity_spawn_all();
}