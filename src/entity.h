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

struct Entity_Handle {
	int index = -1;
	int parity = -1;
};

struct Entity {
	bool _deleted = false;
	int _index = -1;

	Entity_Handle handle;
	bool delete_me = false;

	Physics_Object *po = nullptr;

	Texture *texture = nullptr;
	bool texture_repeat = false;

	int classify = 0;
	const char *type_name = nullptr;
	void *outer = nullptr;

	bool grid_aligned = false;
	int grid_x = 0;
	int grid_y = 0;
	int grid_w = 1;
	int grid_h = 1;
	int grid_size_x = 32;
	int grid_size_y = 32;

	void write(Save_File *file);
	void read(Save_File *file);
};

Entity *get_new_entity();
void add_entity(Entity *entity);

#define entity_stuff(x) \
	bool _deleted = false;\
	int _index = -1;\
	Entity *inner = nullptr;\
	Contiguous_Array<x, max_entities_by_type> *stored_in = nullptr;\

#define declare_entity_type(x) \
	Contiguous_Array<x, max_entities_by_type> _##x; \
	const char *_name_##x = #x; \
	const int _classify_##x = __LINE__

struct Entity_Callbacks {
	virtual void spawn() {}
	virtual void shutdown() {}
	virtual void update() {}
	virtual void render() {}
	virtual void write(Save_File *file) {}
	virtual void read(Save_File *file) {}
};

struct Wall : Entity_Callbacks {
	entity_stuff(Wall);

	void spawn() {
		inner->grid_aligned = true;
		inner->po->set_mass(0.0f);
	}
};

struct Entity_Types {
	declare_entity_type(Wall);
};

extern Entity_Types etypes;

#define create_entity(x) \
	([]() -> x* { \
		Entity *inner = get_new_entity();\
		x *ent = etypes._##x.alloc();\
		ent->stored_in = &etypes._##x; \
		ent->inner = inner; \
		inner->po = physics_add_object(); \
		inner->outer = (void *)ent; \
		inner->classify = etypes._classify_##x; \
		inner->type_name = etypes._name_##x; \
		add_entity(inner); \
		ent->spawn(); \
		return ent;\
	})()

void _remove_entity(Entity *entity);
#define remove_entity(x) \
	([&x]() -> void { \
		x->stored_in->remove(x); \
		_remove_entity(x->inner); \
	})()

#endif