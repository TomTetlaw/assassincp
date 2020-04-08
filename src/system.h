#ifndef __SYSTEM_H__
#define __SYSTEM_H__

struct System {
	SDL_Window *window = nullptr;
	SDL_GLContext context = nullptr;
	Vec2 raw_cursor_position;
	Vec2 window_size;
	bool running = true;
	float delta = 0.0f;
	float current_time = 0.0f;
	int frame_num = 0;
};

// these are here instead of in System for convinience of typing
extern float screen_width;
extern float screen_height;

extern Vec2 cursor_position;
extern Vec2 cursor_position_tl;

extern System sys;

void system_init(int argc, char *argv[]);
void system_quit();
// call this if you encounter a fatal error that the program can't recover from.
void system_error(const char *text, ...);
// file picker dialogue.
void system_open_file_dialogue(const char *dir, const char *filters, char *out);

#endif