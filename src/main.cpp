#include "precompiled.h"

int main(int argc, char *argv[]) {
	sys.init(argc, argv);

	config_load_file("data/config.txt", nullptr);
	hotload.add_file("data/config.txt", nullptr, config_load_file);

	float last_time = 0.0f;

	bool use_editor = true;
	input.target = INPUT_EDITOR;
	editor.load_map_into_editor("data/levels/test.acp");

	system("cd");
	SDL_Event ev;
	while(sys.running) {
		editor.gui_begin_input();

		while (SDL_PollEvent(&ev)) {
			if (!editor.gui_handle_event(&ev)) {
				switch (ev.type) {
				case SDL_QUIT:
					sys.running = false;
					break;
				case SDL_KEYDOWN: {
					bool ctrl_pressed = (ev.key.keysym.mod & KMOD_CTRL) > 0;
					bool alt_pressed = (ev.key.keysym.mod & KMOD_ALT) > 0;
					bool shift_pressed = (ev.key.keysym.mod & KMOD_SHIFT) > 0;
					input.handle_key_press(ev.key.keysym.scancode, true, ctrl_pressed, alt_pressed, shift_pressed);
					if (ev.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						use_editor = !use_editor;
						if (use_editor) {
							input.target = INPUT_EDITOR;
							editor.load_map_into_editor("data/levels/test.acp");
						}
						else {
							editor.save("data/levels/test.acp");
							input.target = INPUT_GAME;
							game.load_level("data/levels/test.acp");
						}
					}
					else if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						sys.running = false;
						break;
					}
				}
								  break;
				case SDL_KEYUP: {
					bool ctrl_pressed = (ev.key.keysym.mod & KMOD_CTRL) > 0;
					bool alt_pressed = (ev.key.keysym.mod & KMOD_ALT) > 0;
					bool shift_pressed = (ev.key.keysym.mod & KMOD_SHIFT) > 0;
					input.handle_key_press(ev.key.keysym.scancode, false, ctrl_pressed, alt_pressed, shift_pressed);
				}
								break;
				case SDL_MOUSEMOTION:
					sys.cursor_position = Vec2((float)ev.motion.x, (float)ev.motion.y);
					input.handle_mouse_move(ev.motion.xrel, ev.motion.yrel);
					break;
				case SDL_MOUSEBUTTONDOWN:
					input.handle_mouse_press(ev.button.button, true, Vec2((float)ev.button.x, (float)ev.button.y), ev.button.clicks == 2);
					break;
				case SDL_MOUSEBUTTONUP:
					input.handle_mouse_press(ev.button.button, false, Vec2((float)ev.button.x, (float)ev.button.y), ev.button.clicks == 2);
					break;
				case SDL_MOUSEWHEEL:
					input.handle_mouse_wheel(ev.wheel.y);
					break;
				}
			}
		}

		editor.gui_end_input();

		if(!sys.running) {
			break;
		}

		srand((unsigned)time(nullptr));

		hotload.check_files_non_blocking();

		float now = SDL_GetTicks() / 1000.0f;
		sys.delta = now - last_time;
		sys.current_time = now;
		last_time = now;

		if(sys.delta < 0.001f) {
			sys.delta = 0.001f;
		}

		sys.frame_num++;

		if (use_editor) {
			editor.update();
		}
		game.update();
		entity_manager.update(game.delta_time);
		renderer.update(game.delta_time);

		renderer.begin_frame();

		renderer.use_camera = true;
		game.render();
		entity_manager.render();
		renderer.render_physics_debug();

		if (use_editor) {
			renderer.use_camera = true;
			editor.render();
		}
		else {
			renderer.use_camera = false;
			renderer.debug_string("game.game_time: %f", game.game_time);
			renderer.debug_string("game.delta_time: %f", game.delta_time);
			renderer.debug_string("entity_manager.entities.num: %d", entity_manager.entities.num);
		}

		renderer.end_frame();
	}

	sys.quit();

	return 0;
}