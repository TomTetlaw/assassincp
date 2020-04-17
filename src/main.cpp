#include "precompiled.h"

int main(int argc, char *argv[]) {
	system_init(argc, argv);

	bool use_editor = true;
	game.paused = use_editor;
	on_level_load();
	Save_File file;
	save_open_read("data/levels/test.acp", &file);
	entity_read(&file);
	save_close(&file);

	float last_time = SDL_GetTicks() / 1000.0f;
	SDL_Event ev;
	while(sys.running) {
		while (SDL_PollEvent(&ev)) {
			bool editor_handled = editor_gui_handle_event(&ev);
			switch (ev.type) {
			case SDL_QUIT:
				sys.running = false;
				break;
			case SDL_KEYDOWN: {
				bool ctrl_pressed = (ev.key.keysym.mod & KMOD_CTRL) > 0;
				bool alt_pressed = (ev.key.keysym.mod & KMOD_ALT) > 0;
				bool shift_pressed = (ev.key.keysym.mod & KMOD_SHIFT) > 0;
				if(!input_handle_key_press(ev.key.keysym.scancode, true, ctrl_pressed, alt_pressed, shift_pressed)) {
					if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						sys.running = false;
					} else if (ev.key.keysym.scancode == SDL_SCANCODE_GRAVE) {
						console_toggle_open();
					} else if (ev.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						use_editor = !use_editor;
						if(use_editor) {
							game.paused = true;
							on_level_load();
							Save_File file;
							save_open_read("data/levels/test.acp", &file);
							entity_read(&file);
							save_close(&file);
						} else {
							Save_File file;
							save_open_write("data/levels/test.acp", &file);
							entity_write(&file);
							save_close(&file);
							game.paused = false;
							load_level();
						}
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
				cursor_position_world = to_world_pos(cursor_position);
				cursor_position_tl = sys.raw_cursor_position;
				if(!editor_handled) input_handle_mouse_move(ev.motion.xrel, ev.motion.yrel);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(!editor_handled) input_handle_mouse_press(ev.button.button, true, Vec2((float)ev.button.x, (float)ev.button.y), ev.button.clicks == 2);
				break;
			case SDL_MOUSEBUTTONUP:
				if(!editor_handled) input_handle_mouse_press(ev.button.button, false, Vec2((float)ev.button.x, (float)ev.button.y), ev.button.clicks == 2);
				break;
			case SDL_MOUSEWHEEL:
				if(!editor_handled) input_handle_mouse_wheel(ev.wheel.y);
				break;
			}
		}

		if(!sys.running) {
			break;
		}

		srand((unsigned)sys.current_time);

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

		// update
		console_update();
		game_update();
		entity_update();
		if(use_editor) editor_update();

		// render
		render_begin_frame();
		game_render();
		entity_render();
		render_deferred_textures();
		render_entity_physics_debug();
		if(use_editor) editor_render();
		console_render();
		render_end_frame();
	}

	system_quit();

	return 0;
}