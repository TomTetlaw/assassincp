#include "precompiled.h"

int main(int argc, char *argv[]) {
	system_init(argc, argv);

	bool use_editor = false;
	input.target = INPUT_GAME;
	on_level_load();
	load_level("data/levels/test.acp");

	float last_time = SDL_GetTicks() / 1000.0f;
	SDL_Event ev;
	while(sys.running) {
		while (SDL_PollEvent(&ev)) {
			if (!editor_gui_handle_event(&ev)) {
				switch (ev.type) {
				case SDL_QUIT:
					sys.running = false;
					break;
				case SDL_KEYDOWN: {
					bool ctrl_pressed = (ev.key.keysym.mod & KMOD_CTRL) > 0;
					bool alt_pressed = (ev.key.keysym.mod & KMOD_ALT) > 0;
					bool shift_pressed = (ev.key.keysym.mod & KMOD_SHIFT) > 0;
					if(!input_handle_key_press(ev.key.keysym.scancode, true, ctrl_pressed, alt_pressed, shift_pressed)) {
						if (ev.key.keysym.scancode == SDL_SCANCODE_SPACE) {
							use_editor = !use_editor;
							if (use_editor) {
								input.target = INPUT_EDITOR;
								on_level_load();
								editor_load_map_into_editor("data/levels/test.acp");
							}
							else {
								editor_save("data/levels/test.acp");
								input.target = INPUT_GAME;
								on_level_load();
								load_level("data/levels/test.acp");
							}
						}
						else if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
							sys.running = false;
						} else if (ev.key.keysym.scancode == SDL_SCANCODE_GRAVE) {
							console_toggle_open();
						}
					}
				} break;
				case SDL_KEYUP: {
					bool ctrl_pressed = (ev.key.keysym.mod & KMOD_CTRL) > 0;
					bool alt_pressed = (ev.key.keysym.mod & KMOD_ALT) > 0;
					bool shift_pressed = (ev.key.keysym.mod & KMOD_SHIFT) > 0;
					input_handle_key_press(ev.key.keysym.scancode, false, ctrl_pressed, alt_pressed, shift_pressed);
				} break;
				case SDL_MOUSEMOTION:
					sys.raw_cursor_position = Vec2((float)ev.motion.x, (float)ev.motion.y);
					cursor_position = sys.raw_cursor_position - (sys.window_size * 0.5f);
					cursor_position_tl = sys.raw_cursor_position;
					input_handle_mouse_move(ev.motion.xrel, ev.motion.yrel);
					break;
				case SDL_MOUSEBUTTONDOWN:
					input_handle_mouse_press(ev.button.button, true, Vec2((float)ev.button.x, (float)ev.button.y), ev.button.clicks == 2);
					break;
				case SDL_MOUSEBUTTONUP:
					input_handle_mouse_press(ev.button.button, false, Vec2((float)ev.button.x, (float)ev.button.y), ev.button.clicks == 2);
					break;
				case SDL_MOUSEWHEEL:
					input_handle_mouse_wheel(ev.wheel.y);
					break;
				}
			}
		}

		if(!sys.running) {
			break;
		}

		srand((unsigned)time(nullptr));

		hotload_check_files_non_blocking();

		float now = SDL_GetTicks() / 1000.0f;
		sys.delta_time = now - last_time;
		sys.current_time = now;
		last_time = now;

		if(sys.delta_time < 0.001f) {
			sys.delta_time = 0.001f;
		}
		if(sys.delta_time > 0.25f) {
			sys.delta_time = 0.25f;
		}

		sys.frame_num++;

		console_update();

		if (use_editor) {
			editor_update();
		}
		
		game_update();
		entity_update();

		render_begin_frame();

		game_render();
		entity_render();
		
		if (use_editor) {
			editor_render();
		}

		console_render();
		
		render_end_frame();
	}

	system_quit();

	return 0;
}