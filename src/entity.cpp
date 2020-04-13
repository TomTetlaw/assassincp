#include "precompiled.h"

Entity_Types etypes;

int next_parity = 0;
internal Contiguous_Array<Entity, max_entities> entities;

Entity *get_new_entity() {
	return entities.alloc();
}

void add_entity(Entity *entity) {
	entity->handle.parity = next_parity;
	entity->handle.index = entity->_index;
	entity->added = true;
	next_parity++;
}

void entity_init() {
	entities.init();
}

void entity_shutdown() {
}

void entity_render() {
	for(int i = 0; i < entities.max_index; i++) {
		if(entities[i]->_deleted) continue;
		if(!entities[i]->added) continue;
		render_texture(&entities[i]->rt);
	}
}

void entity_update() {
	physics_step_world(game.delta_time);

	for(int i = 0; i < entities.max_index; i++) {
		if(entities[i]->_deleted) continue;
		if(!entities[i]->added) continue;
	}
}

void entity_on_level_load() {
	entities.remove_all();
}