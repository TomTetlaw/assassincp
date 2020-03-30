#include "precompiled.h"

Input input;

void Input::handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) {
	if (target == INPUT_EDITOR) {
		editor.handle_mouse_press(mouse_button, down, position, is_double_click);
	}
	else {
		if (player) {
			player->handle_mouse_press(mouse_button, down, position, is_double_click);
		}
	}
}

void Input::handle_mouse_move(int relx, int rely) {
	if (target == INPUT_EDITOR) {
		editor.handle_mouse_move(relx, rely);
	}
	else {
		if (player) {
			player->handle_mouse_move(relx, rely);
		}
	}
}

void Input::handle_key_press(SDL_Scancode scancode, bool down, bool ctrl_pressed, bool alt_pressed, bool shift_pressed) {
	int mods = 0;
	if (ctrl_pressed) {
		mods |= KEY_MOD_CTRL;
	}
	if (alt_pressed) {
		mods |= KEY_MOD_ALT;
	}
	if (shift_pressed) {
		mods |= KEY_MOD_SHIFT;
	}

	if (target == INPUT_EDITOR) {
		editor.handle_key_press(scancode, down, mods);
	}
	else {
		if (player) {
			player->handle_key_press(scancode, down, mods);
		}
	}
}

void Input::handle_mouse_wheel(int amount) {
	if (target == INPUT_EDITOR) {
		editor.handle_mouse_wheel(amount);
	}
	else {
		if (player) {
			player->handle_mouse_wheel(amount);
		}
	}
}

bool Input::get_key_state(SDL_Scancode scancode) {
	return SDL_GetKeyboardState(nullptr)[scancode] == 1;
}