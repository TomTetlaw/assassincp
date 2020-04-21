#include "precompiled.h"

int main(int argc, char *argv[]) {
	//Array<Dir_Search_Entry> entries;
	//if(system_search_dir("data/textures/*.png")) {
	//	for(int i = 0; i < entries.num; i++) console_printf("entries[%d].file_name = %s\n", i, entries[i].file_name);
	//}

	system_init(argc, argv);

	editor.using_editor = true;
	game.paused = editor.using_editor;
	editor_load("data/levels/test.acp");

	float last_time = SDL_GetTicks() / 1000.0f;
	SDL_Event ev;
	while(sys.running) {
		while (SDL_PollEvent(&ev)) {
			bool editor_handled = editor.using_editor ? editor_gui_handle_event(&ev) : false;
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
						if(!editor.using_editor) {
							editor.using_editor = true;
							game.paused = true;
							editor_load(editor.current_file);
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
		if(editor.using_editor) editor_update();

		// render
		render_begin_frame();
		game_render();
		entity_render();
		render_deferred_textures();
		render_entity_physics_debug();
		if(editor.using_editor) editor_render();
		console_render();
		
		render_end_frame();
	}

	system_quit();

	return 0;
}