#include "precompiled.h"

declare_entity_type(Entity, "ent_base", ENTITY_BASE);

Entity_Manager entity_manager;

internal Array<Entity *> entities;
internal int next_parity = 0;

#pragma init_seg(lib) 
Entity_Type_Decl *Entity_Type_Decl::list = nullptr;
int Entity_Type_Decl::list_num = 0;

void entity_init() {
	entity_manager.entity_types.ensure_size(Entity_Type_Decl::list_num);
	entity_manager.entity_types.num = Entity_Type_Decl::list_num;

	Entity_Type_Decl *decl = Entity_Type_Decl::list;
	while (decl) {
		entity_manager.entity_types[decl->classify] = decl;
		decl = decl->next;
	}
}

void entity_shutdown() {
	for (int i = 0; i < entities.num; i++) {
		if (entities[i]) {
			entities[i]->shutdown();
		}
		delete entities[i];
	}
}

void entity_on_level_load() {
	entity_delete_all_entities();
}

internal void update_render_texture(Entity *entity) {
	entity->rt.position = entity->po->position;
}

void entity_render() {
	render_setup_for_world();
	
	For(entities) {
		auto it = entities[it_index];
		if (it) {
			update_render_texture(it);
			render_texture(&it->rt);

			it->render();

			physics_render_debug(it->po);
		}
	}
}

void entity_update() {
	physics_step_world();

	for (int i = 0; i < entities.num; i++) {
		Entity *entity = entities[i];
		if (!entity) {
			continue;
		}

		entity->update();

		if (entity->think_time <= game.now) {
			entity->think_time = 0.0f;
			entity->think();
		}
	}

	//For(entities) {
	//	auto it = entities[it_index];
	//	if(it && it->delete_me) {
	//		delete_entity(it);
	//	}
	//}
}

Entity *find_entity(Entity_Handle handle) {
	if (handle.num < 0 || handle.num >= entities.num) {
		return nullptr;
	}
	if (handle.parity < 0) {
		return nullptr;
	}

	Entity *entity = entities[handle.num];
	if (!entity) {
		return nullptr;
	}

	if (entity->handle.parity == handle.parity) {
		return entity;
	}

	return nullptr;
}

void add_entity(Entity *entity) {
	if(entity->handle.parity != -1) {
		return;
	}

	bool added = false;

	for (int i = 0; i < entities.num; i++) {
		if (!entities[i]) {
			entities[i] = entity;
			entity->handle.num = i;
			added = true;
		}
	}

	entity->handle.parity = next_parity++;

	if (!added) {
		entity->handle.num = entities.num;
		entities.append(entity);
	}

	added = false;

	for (int i = 0; i < entity_manager.entity_types[entity->classify]->entities.num; i++) {
		if (!entity_manager.entity_types[entity->classify]->entities[i]) {
			entity_manager.entity_types[entity->classify]->entities[i] = entity;
			entity->num_in_type = i;
			added = true;
		}
	}

	if (!added) {
		entity->num_in_type = entity_manager.entity_types[entity->classify]->entities.num;
		entity_manager.entity_types[entity->classify]->entities.append(entity);
	}
}

void spawn_entity(Entity *entity) {
	if (entity->handle.parity == -1) {
		add_entity(entity);
	}

	entity->spawn();
}

Entity *create_entity(const char *type_name, const char *name, bool spawn, bool add) {
	if (!type_name) {
		return nullptr;
	}

	//console.printf("Created entity 'type=%s(name=%s)', added=%d\n", type_name, name, (int)add);
	for (int i = 0; i < entity_manager.entity_types.num; i++) {
		Entity_Type_Decl *type = entity_manager.entity_types[i];
		if (!strcmp(type_name, type->name)) {
			Entity *entity = type->callback();
			entity->type = type;
			if (name) {
				strcpy(entity->name, name);
			}
			entity->classify = type->classify;
			entity->po = physics_add_object();
			
			if (add) {
				add_entity(entity);
			}
			
			if (spawn) {
				spawn_entity(entity);
			}

			return entity;
		}
	}

	return nullptr;
}

void delete_entity(Entity *entity) {
	if (!entity) {
		return;
	}
	if (entity->handle.num < 0 || entity->handle.num >= entities.num) {
		return;
	}

	physics_remove_object(entity->po);

	entities[entity->handle.num] = nullptr;
	entity_manager.entity_types[entity->classify]->entities[entity->num_in_type] = nullptr;

	entity->shutdown();

	delete entity;
}

void entity_delete_all_entities() {
	for (int i = 0; i < entities.num; i++) {
		delete_entity(entities[i]);
	}
	entities.num = 0;
}

void Entity::set_texture(const char *file_name, bool set_render_size) {
	rt.texture = load_texture(file_name);

	if(set_render_size) {
		rt.size = Vec2(rt.texture->width, rt.texture->height);
	}
}