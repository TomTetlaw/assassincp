#ifndef __ENTITY_H__
#define __ENTITY_H__

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
	bool added = false;

	Physics_Object *po = nullptr;
	Render_Texture rt;
};

Entity *get_new_entity();
void add_entity(Entity *entity);

#define entity_stuff() bool _deleted = false; int _index = -1; Entity *base = nullptr

struct Test_Entity {
	entity_stuff();

	int test = 3;
};

#define declare_entity_type(x) Contiguous_Array<x, max_entities_by_type> _##x

struct Entity_Types {
	declare_entity_type(Test_Entity);
};

extern Entity_Types etypes;

#define create_entity(x) \
	([]() -> x* { \
		Entity *base = get_new_entity();\
		x *ent = etypes._##x.alloc();\
		ent->base = base; return ent;\
	})()

#endif