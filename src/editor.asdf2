#ifndef __EDITOR_H__
#define __EDITOR_H__

void editor_init();
void editor_shutdown();
void editor_render();
void editor_update();

void editor_on_level_load();

bool editor_gui_handle_event(SDL_Event *ev);

bool editor_handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click);
void editor_handle_mouse_move(int relx, int rely);
bool editor_handle_key_press(SDL_Scancode scancode, bool down, int mods);
void editor_handle_mouse_wheel(int amount);

void editor_load(const char *file_name);
void editor_save();

struct Editor {
    bool using_editor = false;
    char current_file[1024] = {0};
};

extern Editor editor;

#endif