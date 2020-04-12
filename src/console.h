#ifndef __CONSOLE_H__
#define __CONSOLE_H__

void console_init();

// print out to the console.
// these will not put a newline at the end.
void console_print(const char *text);
void console_printf(const char *text, ...);

void console_update();
void console_render();
void console_toggle_open();
bool console_handle_key_press(SDL_Scancode scancode, bool down, bool ctrl_pressed, bool alt_pressed, bool shift_pressed);

typedef void(*Console_Command_Callback)(const char *args[], int num_args);
void register_command(const char *name, Console_Command_Callback callback);

#endif