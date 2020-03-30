#ifndef __EDITOR_H__
#define __EDITOR_H__

enum Editor_Mode {
	EDITOR_SELECT,
	EDITOR_ENTITY,
};

enum Thing_Property_Type {
	THING_PROPERTY_INT,
	THING_PROPERTY_FLOAT,
	THING_PROPERTY_STRING,
	THING_PROPERTY_BOOL,
	THING_PROPERTY_VEC2,
	THING_PROPERTY_VEC3,
	THING_PROPERTY_VEC4,
	THING_PROPERTY_TEXTURE,
	THING_PROPERTY_ENTITY_TYPE,
};

struct Editor_Thing_Property {
	const char *name = nullptr;

	Thing_Property_Type type;

	union {
		int *int_dest;
		float *float_dest;
		char **string_dest;
		bool *bool_dest;
		Vec2 *vec2_dest;
		Vec3 *vec3_dest;
		Vec4 *vec4_dest;
		Texture **texture_dest;
	};

	union {
		int int_min;
		float float_min;
		Vec2 vec2_min;
		Vec3 vec3_min;
		Vec4 vec4_min;
	};

	union {
		int int_max;
		float float_max;
		Vec2 vec2_max;
		Vec3 vec3_max;
		Vec4 vec4_max;
	};

	union {
		int int_step;
		float float_step;
		Vec2 vec2_step;
		Vec3 vec3_step;
		Vec4 vec4_step;
	};

	union {
		int int_inc;
		float float_inc;
		Vec2 vec2_inc;
		Vec3 vec3_inc;
		Vec4 vec4_inc;
	};

	Editor_Thing_Property() {}

	Editor_Thing_Property(const char *name, int *var, int min_value, int max_value, int step, int inc) {
		this->name = name;
		type = THING_PROPERTY_INT;
		int_dest = var;
		int_min = min_value;
		int_max = max_value;
		int_step = step;
		int_inc = inc;
	}

	Editor_Thing_Property(const char *name, char **var) { this->name = name; type = THING_PROPERTY_STRING; string_dest = var; }
	Editor_Thing_Property(const char *name, bool *var) { this->name = name; type = THING_PROPERTY_BOOL; bool_dest = var; }

	Editor_Thing_Property(const char *name, float *var, float min_value, float max_value, float step, float inc) { 
		this->name = name;
		type = THING_PROPERTY_FLOAT;
		float_dest = var;
		float_min = min_value;
		float_max = max_value;
		float_step = step;
		float_inc = inc;
	}

	Editor_Thing_Property(const char *name, Vec2 *var, Vec2 min_value, Vec2 max_value, Vec2 step, Vec2 inc) {
		this->name = name; 
		type = THING_PROPERTY_VEC2; 
		vec2_dest = var;
		vec2_min = min_value;
		vec2_max = max_value;
		vec2_step = step;
		vec2_inc = inc;
	}

	Editor_Thing_Property(const char *name, Vec3 *var, Vec3 min_value, Vec3 max_value, Vec3 step, Vec3 inc) { 
		this->name = name; 
		type = THING_PROPERTY_VEC3; 
		vec3_dest = var;
		vec3_min = min_value;
		vec3_max = max_value;
		vec3_step = step;
		vec3_inc = inc;
	}

	Editor_Thing_Property(const char *name, Vec4 *var, Vec4 min_value, Vec4 max_value, Vec4 step, Vec4 inc) { 
		this->name = name; 
		type = THING_PROPERTY_VEC4; 
		vec4_dest = var;
		vec4_min = min_value;
		vec4_max = max_value;
		vec4_step = step;
		vec4_inc = inc;
	}
};

inline Editor_Thing_Property editor_thing_property_texture(const char *name, Texture **dest) {
	Editor_Thing_Property prop;
	prop.name = name;
	prop.type = THING_PROPERTY_TEXTURE;
	prop.texture_dest = dest;
	return prop;
}

inline Editor_Thing_Property editor_thing_property_entity_type() {
	Editor_Thing_Property prop;
	prop.name = "Entity Type";
	prop.type = THING_PROPERTY_ENTITY_TYPE;
	return prop;
}

class Editor_Thing {
public:
	int index = -1;

	Vec2 position;

	bool hovered = false;

	Vec4 hover_colour;
	Vec2 hover_size;
	Vec4 select_colour;
	Vec2 select_size;
	Vec4 normal_colour;
	Vec2 normal_size;
	bool draw_normal_border = true;

	bool draggable = true;

	virtual void on_select() {}
	virtual void on_hover() {}
	virtual void render();
	virtual void add_properties(Array<Editor_Thing_Property> &properties);
	virtual void update() {}
};

class Editor_Entity : public Editor_Thing {
public:
	Editor_Entity() {
		hover_colour = Vec4(0, 0.2f, 0, 1);
		hover_size = Vec2(32, 32);
		select_colour = Vec4(0, 1, 0, 1);
		select_size = Vec2(32, 32);
		normal_colour = Vec4(1, 1, 1, 1);
		normal_size = Vec2(32, 32);
		size = Vec2(32, 32);
		scale = Vec2(1, 1);
		colour = Vec4(1, 1, 1, 1);
	}

	dstr entity_name;
	dstr name;
	Vec2 size;
	Vec2 scale;
	Vec4 colour;

	Texture *texture = nullptr;

	void add_properties(Array<Editor_Thing_Property> &properties);
	void render();
};

struct Editor {
	bool debug_draw = true;

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

	Array<Editor_Thing *> things;
	Array<Editor_Thing *> selected_things;

	struct nk_context *context = nullptr;

	void init();
	void shutdown();
	void render();
	void update();

	Vec2 size_for_thing(Editor_Thing *thing);
	Vec4 colour_for_thing(Editor_Thing *thing);
	void add_thing(Editor_Thing *thing);
	void delete_thing(Editor_Thing *thing);
	void clear_selected_things();

	void gui_begin_input();
	void gui_end_input();
	bool gui_handle_event(SDL_Event *ev);

	void handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click);
	void handle_mouse_move(int relx, int rely);
	void handle_key_press(SDL_Scancode scancode, bool down, int mods);
	void handle_mouse_wheel(int amount);

	void save(const char *file_name);
	void load_map_into_editor(const char *file_name);

	void add_static_things();
	void on_level_load(); // called when we switch from editor to game so we can clear everything
	void remove_all();
};

extern Editor editor;

#endif