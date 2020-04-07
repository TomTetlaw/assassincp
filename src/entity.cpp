#include "precompiled.h"

declare_entity_type(Entity, "ent_base", ENTITY_BASE);

Entity_Manager entity_manager;

internal Array<Entity *> entities;
internal int next_parity = 0;

#pragma init_seg(lib) 
Entity_Type_Decl *Entity_Type_Decl::list = nullptr;
int Entity_Type_Decl::list_num = 0;

Entity::Entity() {
	colour = Vec4(1, 1, 1, 1);
	size = Vec2(1, 1);
}

void Entity::update_render_texture() {
	rt.texture = texture;
	rt.position = position;
	rt.size = size;
	rt.colour = colour;
	rt.angle = angle;
	rt.sl = sl;
	rt.sh = sh;
	rt.tl = tl;
	rt.th = th;
}

void Entity::set_texture(const char *filename, bool set_size) {
	texture = load_texture(filename);

	if (set_size) {
		if (texture) {
			size.x = (float)texture->width;
			size.y = (float)texture->height;
		}
		else {
			size.x = 1.0f;
			size.y = 1.0f;
		}
	}
}

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

void entity_render() {
	For(entities) {
		auto it = entities[it_index];
		if (it) {
			render_texture(&it->rt);
			it->render();
		}
	}
}

void entity_update(float dt) {
	for (int i = 0; i < entities.num; i++) {
		Entity *entity = entities[i];
		if (!entity) {
			continue;
		}

		entity->velocity = approach(entity->velocity, entity->goal_velocity, dt * entity->velocity_ramp_speed);
		entity->position = entity->position + (entity->velocity * dt);
	}

	for (int i = 0; i < entities.num; i++) {
		Entity *entity = entities[i];
		if (!entity) {
			continue;
		}

		entity->update(dt);
		entity->update_render_texture();

		if (entity->think_time <= game.game_time) {
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

Entity *entity_get_entity_from_handle(Entity_Handle handle) {
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
	for (int i = 0; i < entity_manager.entity_types.num; i++) {
		Entity_Type_Decl *decl = entity_manager.entity_types[i];
		for (int j = 0; j < decl->entities.num; j++) {
			decl->entities[j] = nullptr;
		}
		decl->entities.num = 0;
	}
}