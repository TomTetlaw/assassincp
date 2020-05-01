#ifndef __SYSTEM_H__
#define __SYSTEM_H__

struct System {
	SDL_Window *window = nullptr;
	SDL_GLContext context = nullptr;
	Vec2 raw_cursor_position;
	Vec2 window_size;
	bool running = true;
	float delta_time = 0.0f;
	float current_time = 0.0f;
	int frame_num = 0;
};

// these are here instead of in System for convinience of typing
extern float screen_width;
extern float screen_height;
extern Vec2 screen_size;

extern Vec2 cursor_position;
extern Vec2 cursor_position_tl;
extern Vec2 cursor_position_world;

extern System sys;

void system_init(int argc, char *argv[]);
void system_quit();
// call this if you encounter a fatal error that the program can't recover from.
void system_error(const char *text, ...);

// file picker dialogue.

enum File_Open_Dialogue_Mode {
	FODM_OPEN,
	FODM_SAVE,
};

void system_open_file_dialogue(const char *dir, const char *filters, char *out, File_Open_Dialogue_Mode mode);

struct Dir_Search_Entry {
	char file_name[1024] = {0};
};

bool system_search_dir(const char *dir, Array<Dir_Search_Entry> &entries);

#endif