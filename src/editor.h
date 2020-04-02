#ifndef __EDITOR_H__
#define __EDITOR_H__

enum Editor_Mode {
	EDITOR_SELECT,
	EDITOR_ENTITY,
};

class Editor_Entity {
public:
	Editor_Entity();

	int index = -1;

	Vec2 position;
	char type_name[256];
	char name[256];
	char texture_name[256];
	Vec2 size;
	Vec2 scale;
	Vec4 colour;

	Texture *texture = nullptr;

	bool hovered = false;
	bool draggable = true;

	void render();

	int current_entity_type_num = 0;

	void write_save(Save_File *file);
	void read_save(Save_File *file);
};

#define map_file_version 1

struct Editor {
	bool left_button_down = false;
	bool middle_button_down = false;
	bool shift_down = false;

	bool dragging_thing = false;

	Vec2 edit_window_position;
	Vec2 edit_window_size;

	bool drag_select = false;
	Vec2 drag_start_point;
	Vec2 drag_size;

	Editor_Mode mode = EDITOR_SELECT;

	Array<Editor_Entity *> entities;
	Array<Editor_Entity *> selected_entities;

	Array<const char *> entity_type_names; // create on init because no new types after startup

	struct nk_context *context = nullptr;

	void init();
	void shutdown();
	void render();
	void update();

	void add_entity(Editor_Entity *entity);
	void delete_entity(Editor_Entity *entity);
	void clear_selected_entities();

	void gui_begin_input();
	void gui_end_input();
	bool gui_handle_event(SDL_Event *ev);

	void handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click);
	void handle_mouse_move(int relx, int rely);
	void handle_key_press(SDL_Scancode scancode, bool down, int mods);
	void handle_mouse_wheel(int amount);

	void save(const char *file_name);
	void load_map_into_editor(const char *file_name);

	void on_level_load(); // called when we switch from editor to game so we can clear everything
	void remove_all();
};

extern Editor editor;

#endif