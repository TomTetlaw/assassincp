#ifndef __ENTITY_H__
#define __ENTITY_H__

// if you want to create your own entity type:
//
// class My_Entity : public Entity {
//     void spawn() {} // gets called on spawn
//     void shutdown() {} // gets called when deleted
//     void update() {} // gets called every frame, don't put rendering code in here
//     void render() {} // gets called every frame, use for rendering code only
//     void think() {} // gets called at think_time, which you can set like this: think_time = current_time + 1.0f; to run think() again one second from when it was called
//     // these handle input if input.player is your entity
//     void handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) {}
//     void handle_mouse_move(int relx, int rely) {}
//     void handle_key_press(SDL_Scancode scancode, bool down, int mods) {}
//     void handle_mouse_wheel(int amount) {}
// };
//
// you must then declare your entity type to make it available in the editor.
// do this in the global scope just after your entity class:
//
// declare_entity_type(My_Entity, "ent_my_entity", ENTITY_MY_ENTITY);
//
// ENTITY_MY_ENTITY is an enum member that you must add to Entity_Classify in entity.h.
//
// if you want to create an entity and put it in the game immediately without specifying its properties use:
//
// create_entity("my_entity_type");
//
// if you want to do the same but specify properties first use:
//
// Entity *entity = create_entity("my_entity_type", "individual entity's name", false, false);
// entity->position = ...
// entity->scale = ...
// spawn_entity(entity);
//
// if you want to hold a reference to an entity for longer than one frame
// you cannot hold the pointer because it might not be valid the next frame.
// instead you must hold its handle, and check the handle when you want to use 
// the entity, like this:
//
// Entity_Handle player_handle;
// 
// void start_following_player() {
//     player_handle = player->handle;
// }
//
// void follow_player() {
//     Entity *player = find_entity(player_handle);
//     if(player) {
//         // follow the player
//     }
// }
//
// there is also delete_entity which will mark it for removal and deletion at the end of the frame.
// eg: delete_entity(my_entity);

enum Entity_Classify {
	ENTITY_BASE = 0,
	ENTITY_PLAYER = 1,
	ENTITY_INFO_POLYGON = 2,
	ENTITY_INFO_POLYGON_POINT = 3,
	ENTITY_INFO_PLAYER_START = 4,
	ENTITY_BAD_GUY = 5,
	ENTITY_BULLET = 6,

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
	Entity_Handle handle;
	bool delete_me = false;
	int num_in_type = -1;
	Entity_Type_Decl *type = nullptr;
	Entity_Classify classify = ENTITY_BASE;
	char name[256] = { 0 };

	float think_time = 0.0f;

	Render_Texture rt;
	Physics_Object *po = nullptr;

	void set_texture(const char *file_name, bool set_render_size = true);

	virtual void spawn() {}
	virtual void shutdown() {}
	virtual void update() {}
	virtual void render() {}
	virtual void think() {}

	virtual void handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) {}
	virtual void handle_mouse_move(int relx, int rely) {}
	virtual void handle_key_press(SDL_Scancode scancode, bool down, int mods) {}
	virtual void handle_mouse_wheel(int amount) {}
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

#define declare_entity_type(class_name, type_name, classify) \
	Entity *class_name##_creation_callback() { return new class_name; } \
	Entity_Type_Decl class_name##_type_decl(type_name, class_name##_creation_callback, classify)

struct Entity_Manager {
	Array<Entity_Type_Decl *> entity_types;
};

extern Entity_Manager entity_manager;

void entity_init();
void entity_shutdown();
void entity_on_level_load();
void entity_render();
void entity_update();
void entity_delete_all_entities();

void add_entity(Entity *entity);
void spawn_entity(Entity *entity);
Entity *create_entity(const char *type_name, const char *name = nullptr, bool spawn = true, bool add = true);
void delete_entity(Entity *entity);
Entity *find_entity(Entity_Handle handle);

#endif