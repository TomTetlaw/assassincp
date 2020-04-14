#include "precompiled.h"

bool input_handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) {
	if(console_handle_mouse_press(mouse_button, down, position, is_double_click))
		return true;

	if(editor_handle_mouse_press(mouse_button, down, position, is_double_click))
		return true;

	return false;
}

void input_handle_mouse_move(int relx, int rely) {
}

bool input_handle_key_press(SDL_Scancode scancode, bool down, bool ctrl_pressed, bool alt_pressed, bool shift_pressed) {
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
	
	if(console_handle_key_press(scancode, down, mods))
		return true;

	if(editor_handle_key_press(scancode, down, mods))
		return true;

	return false;
}

void input_handle_mouse_wheel(int amount) {
}

bool input_get_key_state(SDL_Scancode scancode) {
	return SDL_GetKeyboardState(nullptr)[scancode] == 1;
}

bool input_translate_scancode(SDL_Scancode scancode, bool shift_pressed, char *ch) {
		 if(scancode == SDL_SCANCODE_A)            { *ch = shift_pressed ? 'A'  : 'a';  return true; }
	else if(scancode == SDL_SCANCODE_B)            { *ch = shift_pressed ? 'B'  : 'b';  return true; }
	else if(scancode == SDL_SCANCODE_C)            { *ch = shift_pressed ? 'C'  : 'c';  return true; }
	else if(scancode == SDL_SCANCODE_D)            { *ch = shift_pressed ? 'D'  : 'd';  return true; }
	else if(scancode == SDL_SCANCODE_E)            { *ch = shift_pressed ? 'E'  : 'e';  return true; }
	else if(scancode == SDL_SCANCODE_F)            { *ch = shift_pressed ? 'F'  : 'f';  return true; }
	else if(scancode == SDL_SCANCODE_G)            { *ch = shift_pressed ? 'G'  : 'g';  return true; }
	else if(scancode == SDL_SCANCODE_H)            { *ch = shift_pressed ? 'H'  : 'h';  return true; }
	else if(scancode == SDL_SCANCODE_I)            { *ch = shift_pressed ? 'I'  : 'i';  return true; }
	else if(scancode == SDL_SCANCODE_J)            { *ch = shift_pressed ? 'J'  : 'j';  return true; }
	else if(scancode == SDL_SCANCODE_K)            { *ch = shift_pressed ? 'K'  : 'k';  return true; }
	else if(scancode == SDL_SCANCODE_L)            { *ch = shift_pressed ? 'L'  : 'l';  return true; }
	else if(scancode == SDL_SCANCODE_M)            { *ch = shift_pressed ? 'M'  : 'm';  return true; }
	else if(scancode == SDL_SCANCODE_N)            { *ch = shift_pressed ? 'N'  : 'n';  return true; }
	else if(scancode == SDL_SCANCODE_O)            { *ch = shift_pressed ? 'O'  : 'o';  return true; }
	else if(scancode == SDL_SCANCODE_P)            { *ch = shift_pressed ? 'P'  : 'p';  return true; }
	else if(scancode == SDL_SCANCODE_Q)            { *ch = shift_pressed ? 'Q'  : 'q';  return true; }
	else if(scancode == SDL_SCANCODE_R)            { *ch = shift_pressed ? 'R'  : 'r';  return true; }
	else if(scancode == SDL_SCANCODE_S)            { *ch = shift_pressed ? 'S'  : 's';  return true; }
	else if(scancode == SDL_SCANCODE_T)            { *ch = shift_pressed ? 'T'  : 't';  return true; }
	else if(scancode == SDL_SCANCODE_U)            { *ch = shift_pressed ? 'U'  : 'u';  return true; }
	else if(scancode == SDL_SCANCODE_V)            { *ch = shift_pressed ? 'V'  : 'v';  return true; }
	else if(scancode == SDL_SCANCODE_W)            { *ch = shift_pressed ? 'W'  : 'w';  return true; }
	else if(scancode == SDL_SCANCODE_X)            { *ch = shift_pressed ? 'X'  : 'x';  return true; }
	else if(scancode == SDL_SCANCODE_Y)            { *ch = shift_pressed ? 'Y'  : 'y';  return true; }
	else if(scancode == SDL_SCANCODE_Z)            { *ch = shift_pressed ? 'Z'  : 'z';  return true; }
	else if(scancode == SDL_SCANCODE_0)            { *ch = shift_pressed ? ')'  : '0';  return true; }
	else if(scancode == SDL_SCANCODE_1)            { *ch = shift_pressed ? '!'  : '1';  return true; }
	else if(scancode == SDL_SCANCODE_2)            { *ch = shift_pressed ? '@'  : '2';  return true; }
	else if(scancode == SDL_SCANCODE_3)            { *ch = shift_pressed ? '#'  : '3';  return true; }
	else if(scancode == SDL_SCANCODE_4)            { *ch = shift_pressed ? '$'  : '4';  return true; }
	else if(scancode == SDL_SCANCODE_5)            { *ch = shift_pressed ? '%'  : '5';  return true; }
	else if(scancode == SDL_SCANCODE_6)            { *ch = shift_pressed ? '^'  : '6';  return true; }
	else if(scancode == SDL_SCANCODE_7)            { *ch = shift_pressed ? '&'  : '7';  return true; }
	else if(scancode == SDL_SCANCODE_8)            { *ch = shift_pressed ? '*'  : '8';  return true; }
	else if(scancode == SDL_SCANCODE_9)            { *ch = shift_pressed ? '('  : '9';  return true; }
	else if(scancode == SDL_SCANCODE_MINUS) 	   { *ch = shift_pressed ? '_'  : '-';  return true; }
	else if(scancode == SDL_SCANCODE_EQUALS) 	   { *ch = shift_pressed ? '+'  : '=';  return true; }
	else if(scancode == SDL_SCANCODE_LEFTBRACKET)  { *ch = shift_pressed ? '{'  : '[';  return true; }
	else if(scancode == SDL_SCANCODE_RIGHTBRACKET) { *ch = shift_pressed ? '}'  : ']';  return true; }
	else if(scancode == SDL_SCANCODE_BACKSLASH)    { *ch = shift_pressed ? '|'  : '\\'; return true; }
	else if(scancode == SDL_SCANCODE_SEMICOLON)    { *ch = shift_pressed ? ':'  : ';';  return true; }
	else if(scancode == SDL_SCANCODE_APOSTROPHE)   { *ch = shift_pressed ? '"'  : '\''; return true; }
	else if(scancode == SDL_SCANCODE_COMMA) 	   { *ch = shift_pressed ? '<'  : ',';  return true; }
	else if(scancode == SDL_SCANCODE_PERIOD) 	   { *ch = shift_pressed ? '>'  : '.';  return true; }
	else if(scancode == SDL_SCANCODE_SLASH) 	   { *ch = shift_pressed ? '?'  : '/';  return true; }
	else if(scancode == SDL_SCANCODE_SPACE) 	   { *ch =                        ' ';  return true; }
	else return false;
}