#ifndef __ENTITY_H__
#define __ENTITY_H__

// this is enough for 64 different entity types,
// will need to up this limit if there are more.
constexpr int max_entities = 65536;
constexpr int max_entities_by_type = 1024;

void entity_init();
void entity_shutdown();
void entity_render();
void entity_update();
void entity_on_level_load();
void entity_write(Save_File *file);
void entity_read(Save_File *file);
void entity_spawn_all();

void render_entity_physics_debug();

struct Entity_Handle {
	int index = -1;
	int parity = -1;
};

struct Entity_Callbacks {
	virtual void _remove() {}
	// gets called when the entity is created. Do not put anything here that depends on game state.
	virtual void setup() {}
	// gets called when the entity is live for the first time in the game world. Put things in here that depend on game state.
	virtual void spawn() {}
	virtual void remove() {}
	virtual void update() {}
	virtual void render() {}
	virtual void write(Save_File *file) {}
	virtual void read(Save_File *file) {}

	void handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) { }
};

enum Entity_Flags {
	EFLAGS_NO_PHYSICS = 1 << 0,
};

struct Entity {
	bool _deleted = false;
	int _index = -1;

	int parity = -1;
	bool delete_me = false;
	bool added = false;
	int flags = 0;

	bool texture_repeat = false;
	int z = 0;
	Vec2 position = Vec2(0, 0); // only used if the entity doesn't have a physics object.
	Vec2 size = Vec2(0, 0); // only used if the entity doesn't have a physics object.

	int classify = 0;
	Entity_Callbacks *outer = nullptr;

	bool grid_aligned = false;
	int grid_x = 0;
	int grid_y = 0;
	int grid_w = 1;
	int grid_h = 1;
	int grid_size_x = 16;
	int grid_size_y = 16;

	char type_name[1024] = {0};
	Physics_Object po;
	int texture = -1;
	char texture_filename[1024] = {0}; // for saving/loading of entity textures.

	inline void set_texture(const char *filename) {
		texture = load_texture(filename);
		if(texture != -1) strcpy(texture_filename, filename);
	}
};

void copy_entity(Entity *source, Entity *dest);

struct Entity_Manager {
	Contiguous_Array<Entity, max_entities> entities;
};

extern Entity_Manager entity_manager;

Entity *get_new_entity();
void add_entity(Entity *entity, bool add);

#define entity_stuff(x) \
	bool _deleted = false;\
	int _index = -1;\
	Entity *inner = nullptr;\
	Contiguous_Array<x, max_entities_by_type> *stored_in = nullptr;\
	void _remove() { stored_in->remove(this); }

struct Wall : Entity_Callbacks {
	entity_stuff(Wall);

	void setup() {
		inner->set_texture("data/textures/wall.png");
		inner->texture_repeat = true;
		inner->z = 1;
		
		inner->grid_aligned = true;

        inner->po.size = Vec2(32, 32);
		inner->po.set_mass(0.0f);
		inner->po.groups = phys_group_wall;
	}
};

struct Weapon {
	float refire_time = 0.0f;
	float last_fire_time = 0.0f;
	int texture = -1;
	Vec2 position;

	virtual void fire() {}
};

enum Weapon_Type {
	WEAPON_NONE = 0,
	WEAPON_GUN = 1 << 0,
};

struct Gun : Weapon {
	Gun() { 
		refire_time = 1.0f;
		texture = load_texture("data/textures/gun.png");
		position = Vec2(30, 30);
	}

	void fire() {
	}
};

struct Player : Entity_Callbacks {
	entity_stuff(Player);

	Field_Of_View fov;

	float last_fire_time = 0.0f;
	bool firing = false;

	u8 weapons_in_inventory = 0;
	u8 currently_equiped_weapon = 0;
	Gun weapon_gun;

	inline Weapon *get_weapon() { 
		if(weapons_in_inventory & currently_equiped_weapon) {
			if(currently_equiped_weapon == 1) {
				return &weapon_gun;
			}
		}

		return nullptr;
	}

	void handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click);

	void setup();
	void spawn();
	void update();
	void render();
	void remove();

	void write(Save_File *file);
	void read(Save_File *file);
};

struct Parallax : Entity_Callbacks {
	entity_stuff(Parallax);

	void setup() {
		inner->z = 0;
		inner->set_texture("data/textures/parallax_test.png");
		inner->flags = EFLAGS_NO_PHYSICS;
	}

	void update() {
		inner->position = -renderer.camera_position * 0.05f;
	}
};

struct Floor : Entity_Callbacks {
	entity_stuff(Floor);

	void setup() {
		inner->z = 1;
		inner->set_texture("data/textures/floor.png");
		inner->flags = EFLAGS_NO_PHYSICS;
	}
};

#define declare_entity_type(x) \
	Contiguous_Array<x, max_entities_by_type> _##x; \
	const char *_name_##x = #x; \
	const int _classify_##x = __COUNTER__ 

struct Entity_Types {
	declare_entity_type(Wall);
	declare_entity_type(Player);
	declare_entity_type(Parallax);
	declare_entity_type(Floor);
};

extern Entity_Types etypes;

// if anything gets added to this is should probably also be added to create_entity_at in entity.cpp.
#define create_entity(x, add) \
	([]() -> x* { \
		Entity *inner = get_new_entity();\
		x *ent = etypes._##x.alloc();\
		ent->stored_in = &etypes._##x; \
		ent->inner = inner; \
		inner->outer = ent; \
		inner->classify = etypes._classify_##x; \
		strcpy(inner->type_name, etypes._name_##x); \
		add_entity(inner, add); \
		ent->setup(); \
		return ent;\
	})()

void remove_entity(Entity *entity);

#endif