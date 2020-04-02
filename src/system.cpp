#include "precompiled.h"

System sys;

void System::init(int argc, char *argv[]) {
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		error("Failed to initialize SDL: %s", SDL_GetError());
	}

	int x = SDL_WINDOWPOS_CENTERED;
	int y = SDL_WINDOWPOS_CENTERED;
	window_size = Vec2(1920, 1080);
	
	SDL_Rect rect;
	SDL_GetDisplayBounds(0, &rect);

	unsigned int flags = SDL_WINDOW_OPENGL;
	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-width")) {
			window_size.x = (float)atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-height")) {
			window_size.y = (float)atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-fullscreen")) {
			flags = flags | SDL_WINDOW_FULLSCREEN;
			window_size = Vec2((float)rect.w, (float)rect.h);
		}
		else if (!strcmp(argv[i], "-borderless")) {
			flags = flags | SDL_WINDOW_BORDERLESS;
		}
	}

	window = SDL_CreateWindow("", x, y, (int)window_size.x, (int)window_size.y, flags);
	if (!window) {
		error("Failed to create window at [%d, %d]: %s", x, y, SDL_GetError());
	}

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

	context = SDL_GL_CreateContext(window);
	if (!context) {
		error("Failed to create opengl context: %s", SDL_GetError());
	}

	if (TTF_Init()) {
		error("Failed to initialize SDL_TTF: %s", TTF_GetError());
	}

	if (!IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG)) {
		error("Failed to initialize SDL_IMG: %s", IMG_GetError());
	}

	console.init();
	hotload.init();
	config.init();
	renderer.init();
	tex.init();
	font_manager.init();
	entity_manager.init();
	editor.init();
}

void System::quit() {
	editor.shutdown();
	entity_manager.shutdown();
	font_manager.shutdown();
	tex.shutdown();
	renderer.shutdown();
	config.shutdown();
	hotload.shutdown();

	TTF_Quit();
	IMG_Quit();
	SDL_GL_DeleteContext(sys.context);
	SDL_DestroyWindow(sys.window);
	SDL_Quit();

	running = false;
}

void System::error(const char *text, ...) {
	va_list argptr;
	char message[1024];

	va_start(argptr, text);
	vsnprintf_s(message, _TRUNCATE, text, argptr);
	va_end(argptr);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", message, window);
	quit();
}