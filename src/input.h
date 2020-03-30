#ifndef __INPUT_H__
#define __INPUT_H__

enum Key_Mods {
	KEY_MOD_CTRL = 1,
	KEY_MOD_ALT = 2,
	KEY_MOD_SHIFT = 4,
};

struct Entity;

enum Input_Target {
	INPUT_EDITOR,
	INPUT_GAME,
};

struct Input {
	Input_Target target = INPUT_EDITOR;

	Entity *player = nullptr;

	void handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click);
	void handle_mouse_move(int relx, int rely);
	void handle_key_press(SDL_Scancode scancode, bool down, bool ctrl_pressed, bool alt_pressed, bool shift_pressed);
	void handle_mouse_wheel(int amount);
	bool get_key_state(SDL_Scancode scancode);
};

extern Input input;

#endif