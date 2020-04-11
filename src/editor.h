#ifndef __EDITOR_H__
#define __EDITOR_H__

enum Editor_Mode {
	EDITOR_SELECT,
	EDITOR_ENTITY,
	EDITOR_POLYGON,
};

enum Editor_Entity_Type {
	EDITOR_ENTITY_ENTITY,
	EDITOR_ENTITY_POLYGON,
	EDITOR_ENTITY_POLYGON_POINT,
};

class Editor_Entity {
public:
	Editor_Entity();

	int index = -1;

	Editor_Entity_Type type = EDITOR_ENTITY_ENTITY;

	Vec2 position;
	char type_name[256] = { 0 };
	char name[256] = { 0 };
	char texture_name[256] = { 0 };
	Vec2 size;
	Vec2 scale;
	Vec4 colour;

	Texture *texture = nullptr;

	bool hovered = false;
	bool draggable = true;

	virtual void render();

	virtual void on_drag(Vec2 amount);

	int current_entity_type_num = 0;

	virtual void write_save(Save_File *file);
	virtual void read_save(Save_File *file);

	virtual void on_delete();
};

class Editor_Polygon;

class Editor_Polygon_Point : public Editor_Entity {
public:
	Editor_Polygon *parent = nullptr;

	Editor_Polygon_Point() { size = Vec2(5.0f, 5.0f); type = EDITOR_ENTITY_POLYGON_POINT; }
	void render();

	void on_drag(Vec2 amount);
	void on_delete();

	// leave these empty because they will be handled by Editor_Polygon
	void write_save(Save_File *file) {}
	void read_save(Save_File *file) {}
};

class Editor_Polygon : public Editor_Entity {
public:
	Vec2 center;
	Array<Editor_Polygon_Point *> points;
	bool closed = false;

	Editor_Polygon() { current_entity_type_num = ENTITY_INFO_POLYGON; strcpy(type_name, "info_polygon"); type = EDITOR_ENTITY_POLYGON; size = Vec2(5, 5); }

	void calculate_properties();
	void on_drag(Vec2 amount);
	void on_delete();
	void render();
	void write_save(Save_File *file);
	void read_save(Save_File *file);
};

constexpr int map_file_version = 3;

void editor_init();
void editor_shutdown();
void editor_render();
void editor_update();

bool editor_gui_handle_event(SDL_Event *ev);

void editor_handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click);
void editor_handle_mouse_move(int relx, int rely);
bool editor_handle_key_press(SDL_Scancode scancode, bool down, int mods);
void editor_handle_mouse_wheel(int amount);

void editor_save(const char *file_name);
void editor_load_map_into_editor(const char *file_name);

void editor_on_level_load(); // called when we switch from editor to game so we can clear everything
void editor_remove_all();

#endif