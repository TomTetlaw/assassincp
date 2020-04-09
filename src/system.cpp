#include "precompiled.h"

System sys;
float screen_width = 0.0f;
float screen_height = 0.0f;
Vec2 cursor_position = Vec2(0.0f, 0.0f);
Vec2 cursor_position_tl = Vec2(0.0f, 0.0f);;

internal void hotload_config_file(const char *filename, void *data) {
	config_load(filename);
}

void system_init(int argc, char *argv[]) {
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		system_error("Failed to initialize SDL: %s", SDL_GetError());
	}

	int x = SDL_WINDOWPOS_CENTERED;
	int y = SDL_WINDOWPOS_CENTERED;
	sys.window_size = Vec2(1920, 1080);
	
	SDL_Rect rect;
	SDL_GetDisplayBounds(0, &rect);

	unsigned int flags = SDL_WINDOW_OPENGL;
	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-width")) {
			sys.window_size.x = (float)atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-height")) {
			sys.window_size.y = (float)atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-fullscreen")) {
			flags = flags | SDL_WINDOW_FULLSCREEN;
			sys.window_size = Vec2((float)rect.w, (float)rect.h);
		}
		else if (!strcmp(argv[i], "-borderless")) {
			flags = flags | SDL_WINDOW_BORDERLESS;
		}
	}

	sys.window = SDL_CreateWindow("", x, y, (int)sys.window_size.x, (int)sys.window_size.y, flags);
	if (!sys.window) {
		system_error("Failed to create window at [%d, %d]: %s", x, y, SDL_GetError());
	}

	screen_width = sys.window_size.x;
	screen_height = sys.window_size.y;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	sys.context = SDL_GL_CreateContext(sys.window);
	if (!sys.context) {
		system_error("Failed to create opengl context: %s", SDL_GetError());
	}

	if (TTF_Init()) {
		system_error("Failed to initialize SDL_TTF: %s", TTF_GetError());
	}

	if (!IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG)) {
		system_error("Failed to initialize SDL_IMG: %s", IMG_GetError());
	}

	console_init();
	hotload_init();
	render_init();
	entity_init();
	editor_init();
	game_init();
	physics_init();

	config_load("data/config.txt");
	hotload_add_file("data/config.txt", nullptr, hotload_config_file);
}

void system_quit() {
	config_write_file("data/config.txt");

	editor_shutdown();
	entity_shutdown();
	font_shutdown();
	texture_shutdown();
	render_shutdown();
	config_shutdown();
	hotload_shutdown();

	TTF_Quit();
	IMG_Quit();
	SDL_GL_DeleteContext(sys.context);
	SDL_DestroyWindow(sys.window);
	SDL_Quit();

	sys.running = false;
}

void system_error(const char *text, ...) {
	va_list argptr;
	char message[1024];

	va_start(argptr, text);
	vsnprintf_s(message, _TRUNCATE, text, argptr);
	va_end(argptr);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", message, sys.window);
	system_quit();
}