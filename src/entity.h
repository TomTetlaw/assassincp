#ifndef __ENTITY_H__
#define __ENTITY_H__

enum Entity_Classify {
	ENTITY_BASE = 0,
	ENTITY_PLAYER = 1,
	ENTITY_POLYGON = 2,

	ENTITY_NUM_TYPES,
};

struct Entity_Handle {
	int parity = -1;
	int num = -1;
	int num_in_type = -1;
};

struct Entity_Type_Decl;

enum Collision_Type {
	COLLISION_POST_SOLVE,
	COLLISION_BEGIN,
	COLLISION_SEPERATE,
};

struct Entity {
	Entity();

	int parity = -1;
	int num = -1;
	int num_in_type = -1;
	Entity_Type_Decl *type = nullptr;
	Entity_Classify classify = ENTITY_BASE;
	const char *name = nullptr;

	float think_time = 0.0f;

	virtual void spawn() {}
	virtual void shutdown() {}
	virtual void update(float dt) {}
	virtual void render() {}
	virtual void think() {}
	virtual void setup_physics(cpSpace *space) {}
	virtual void delete_physics(cpSpace *space) {}
	virtual void handle_collision(Entity *other, Collision_Type type) {}

	virtual void handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) {}
	virtual void handle_mouse_move(int relx, int rely) {}
	virtual void handle_key_press(SDL_Scancode scancode, bool down, int mods) {}
	virtual void handle_mouse_wheel(int amount) {}

	void set_position(Vec2 pos);

	//@refactor: need these passthrough variables?
	Texture *texture = nullptr;
	Vec2 size;
	Vec4 colour;
	float angle = 0.0f;
	float sl = -1;
	float sh = -1;
	float tl = -1;
	float th = -1;
	Render_Texture rt;
	void set_texture(const char *filename, bool set_size = true);
	void update_render_texture();

	cpBody *body = nullptr;
	Vec2 position;
	Vec2 velocity;
	Vec2 goal_velocity;
	float velocity_ramp_speed = 1.0f;
};

struct Entity_Type_Decl {
	static Entity_Type_Decl *list;
	static int list_num;

	Entity_Type_Decl *next = nullptr;

	const char *name = nullptr;
	Entity_Classify classify = ENTITY_BASE;
	Entity *(*callback)() = nullptr;
	Array<Entity *> entities;

	Entity_Type_Decl(const char *name, Entity *(*callback)(), Entity_Classify classify) {
		this->name = name;
		this->callback = callback;
		this->classify = classify;
		this->next = list;
		list = this;
		list_num++;
	}
};

#define declare_entity_type(class_name, classify) \
	Entity *class_name##_creation_callback() { return new class_name; } \
	Entity_Type_Decl class_name##_type_decl(#class_name, class_name##_creation_callback, classify)

struct Entity_Manager {
	Array<Entity_Type_Decl *> entity_types;
	Array<Entity *> entities;
	int next_parity = 0;

	cpSpace *space = nullptr;

	void init();
	void shutdown();
	void on_level_load();
	void render();
	void update(float dt);
	void add_entity(Entity *entity);
	void spawn_entity(Entity *entity);
	void delete_entity(Entity *entity);
	void delete_all_entities();
	Entity *get_entity_from_handle(Entity_Handle handle);
	Entity *create_entity(const char *type_name, const char *name = nullptr, bool spawn = true, bool add = true);
};

extern Entity_Manager entity_manager;

declare_entity_type(Entity, ENTITY_BASE);

#endif