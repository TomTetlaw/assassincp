#ifndef __SYSTEM_H__
#define __SYSTEM_H__

struct System {
	SDL_Window *window = nullptr;
	SDL_GLContext context = nullptr;
	Vec2 window_size;
	Vec2 cursor_position;
	bool running = true;
	float delta = 0.0f;
	float current_time = 0.0f;
	int frame_num = 0;

	void init(int argc, char *argv[]);
	void quit();
	void error(const char *text, ...);

	void open_file_dialogue(const char *dir, const char *filters, dstr *out);
};

extern System sys;

#endif