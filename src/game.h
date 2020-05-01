#ifndef __GAME_H__
#define __GAME_H__

struct Debug_Draw;
struct Contact_Listener;

//@todo: refactor nav mesh stuff into its own struct/file
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
	int nav_points_width = 0;
	int nav_points_height = 0;
	const char *file_name = nullptr;
};

struct Game {
	float now = 0.0f;
	float delta_time = 0.0f;
	Level *current_level = nullptr;
	bool paused = false;
};

extern Game game;

void game_init();
void game_update();
void game_render();
void load_level();

void game_set_level(int index);

void on_level_load(); // the central function for changing level

#endif