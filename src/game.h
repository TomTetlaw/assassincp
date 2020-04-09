#ifndef __GAME_H__
#define __GAME_H__

// if you want to use path finding to follow a path, first generate the path like this:
//
// Nav_Path my_path;
// make_path(my_position, player_position);
//
// then in your update function you can call: //@todo: implement this
// follow_path(&my_path);

struct Debug_Draw;
struct Contact_Listener;

struct Nav_Mesh_Point {
	Vec2 point;
	bool valid = false;
	bool visited = false;
	int grid_index = -1;
};

struct Nav_Path {
	Array<Nav_Mesh_Point *> points;
};

bool make_path(Nav_Path *path, Vec2 from, Vec2 to);

struct Level {
	Array<Vec2> fov_check_points;
	Array<Nav_Mesh_Point> nav_points;
	float nav_points_size = 32.0f;
	int nav_points_width = 0;
	int nav_points_height = 0;
	const char *file_name = nullptr;
};

struct Game {
	float now = 0.0f;
	float delta_time = 0.0f;
	Level *current_level = nullptr;
	Entity *player = nullptr;
};

extern Game game;

void game_init();
void game_update();
void game_render();
void game_toggle_paused();
void game_set_paused(bool paused);
void load_level(const char *file_name);

void on_level_load(); // the central function for changing level

#endif