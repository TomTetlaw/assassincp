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
};

extern Input input;

void input_handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click);
void input_handle_mouse_move(int relx, int rely);
bool input_handle_key_press(SDL_Scancode scancode, bool down, bool ctrl_pressed, bool alt_pressed, bool shift_pressed);
void input_handle_mouse_wheel(int amount);
bool input_get_key_state(SDL_Scancode scancode);
bool input_translate_scancode(SDL_Scancode scancode, bool shift_pressed, char *ch);

#endif