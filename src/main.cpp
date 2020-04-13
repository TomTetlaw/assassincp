#include "precompiled.h"

struct test_entity {
	int a, b;
	float c, d;
	char buffer[1024] = {0};
	bool deleted = false;
};

void test1() {
	test_entity entities[10000];
	float now = SDL_GetTicks() / 1000.0f;
	for(int i = 0; i < 10000; i++) {
		entities[i].a = rand() % 100;
		entities[i].b = rand() % 1000;
		entities[i].c = entities[i].a / entities[i].b * (entities[i].a * entities[i].a) + sqrt(entities[i].a);
		entities[i].d = (entities[i].b*entities[i].b)*(entities[i].a*entities[i].a)*(entities[i].b*entities[i].b)*(entities[i].a*entities[i].a);
	}
	float t2 = (SDL_GetTicks() / 1000.0f) - now;
	printf("best case time: %f\n", t2);
}

void test2() {
	test_entity entities[10000];
	for(int i = 0; i < 10000; i++) {
		if(i%2) entities[i].deleted = true;
	}

	float now = SDL_GetTicks() / 1000.0f;
	for(int i = 0; i < 10000; i++) {
		if(entities[i].deleted) continue;
		entities[i].a = rand() % 100;
		entities[i].b = rand() % 1000;
		entities[i].c = entities[i].a / entities[i].b * (entities[i].a * entities[i].a) + sqrt(entities[i].a);
		entities[i].d = (entities[i].b*entities[i].b)*(entities[i].a*entities[i].a)*(entities[i].b*entities[i].b)*(entities[i].a*entities[i].a);
	}
	float t2 = (SDL_GetTicks() / 1000.0f) - now;
	printf("some deleted time: %f\n", t2);
}

void test3() {
	test_entity *entities[10000] = {0};
	for(int i = 0; i < 10000; i++) {
		entities[i] = new test_entity;
		if(i%2) entities[i]->deleted = true;
	}

	float now = SDL_GetTicks() / 1000.0f;
	for(int i = 0; i < 10000; i++) {
		if(entities[i]->deleted) continue;
		entities[i]->a = rand() % 100;
		entities[i]->b = rand() % 1000;
		entities[i]->c = entities[i]->a / entities[i]->b * (entities[i]->a * entities[i]->a) + sqrt(entities[i]->a);
		entities[i]->d = (entities[i]->b*entities[i]->b)*(entities[i]->a*entities[i]->a)*(entities[i]->b*entities[i]->b)*(entities[i]->a*entities[i]->a);
	}
	float t2 = (SDL_GetTicks() / 1000.0f) - now;
	printf("best case time: %f\n", t2);
}

int main(int argc, char *argv[]) {
	test1(); test2(); test3();
	
	system_init(argc, argv);

	Test_Entity *test = create_entity(Test_Entity);
	add_entity(test->base);

	float last_time = SDL_GetTicks() / 1000.0f;
	SDL_Event ev;
	while(sys.running) {
		while (SDL_PollEvent(&ev)) {
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

		console_update();
		
		game_update();
		entity_update();

		render_begin_frame();

		game_render();
		entity_render();

		console_render();
		
		render_end_frame();
	}

	system_quit();

	return 0;
}