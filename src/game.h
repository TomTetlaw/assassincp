#ifndef __GAME_H__
#define __GAME_H__

struct Debug_Draw;
struct Contact_Listener;

struct Nav_Mesh_Point {
	Vec2 point;
	bool valid = false;
};

struct Level {
	Array<Vec2> fov_check_points;
	Array<Nav_Mesh_Point> nav_points;
	float nav_points_size = 64.0f;
	int nav_points_width = 0;
	int nav_points_height = 0;
	const char *file_name = nullptr;
};

struct Game {
	float game_time = 0.0f;
	float real_time = 0.0f;
	float last_time = 0.0f;
	float delta_time = 0.0f;
	float real_delta_time = 0.0f;

	bool paused = false;
	float time_paused = 0.0f;
	float total_time_paused = 0.0f;

	void init();
	void shutdown();
	void update();
	void render();
	void toggle_paused();
	void set_paused(bool paused);

	Level *current_level = nullptr;
	Entity *player = nullptr;
	void on_level_load();
	void load_level(const char *file_name);
};

extern Game game;

#endif