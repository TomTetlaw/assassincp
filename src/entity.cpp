#include "precompiled.h"

Entity_Manager entity_manager;

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
	rt.centered = true;
}

void Entity::set_texture(const char *filename, bool set_size) {
	texture = tex.load(filename);

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

void Entity::set_position(Vec2 pos) {
	if (body) {
		cpBodySetPosition(body, cpv(pos.x, pos.y));
	}

	position = pos;
}

void handle_collisions_post_solve(cpArbiter *arb, cpSpace *space, cpDataPointer data) {
	CP_ARBITER_GET_BODIES(arb, a, b);
	Entity *first = (Entity *)cpBodyGetUserData(a);
	Entity *second = (Entity *)cpBodyGetUserData(b);
	first->handle_collision(second, COLLISION_POST_SOLVE);
	second->handle_collision(first, COLLISION_POST_SOLVE);
}

cpBool handle_collisions_begin(cpArbiter *arb, cpSpace *space, cpDataPointer data) {
	CP_ARBITER_GET_BODIES(arb, a, b);
	Entity *first = (Entity *)cpBodyGetUserData(a);
	Entity *second = (Entity *)cpBodyGetUserData(b);
	first->handle_collision(second, COLLISION_BEGIN);
	second->handle_collision(first, COLLISION_BEGIN);
	return cpTrue;
}

void handle_collisions_seperate(cpArbiter *arb, cpSpace *space, cpDataPointer data) {
	CP_ARBITER_GET_BODIES(arb, a, b);
	Entity *first = (Entity *)cpBodyGetUserData(a);
	Entity *second = (Entity *)cpBodyGetUserData(b);
	first->handle_collision(second, COLLISION_SEPERATE);
	second->handle_collision(first, COLLISION_SEPERATE);
}

void Entity_Manager::init() {
	entity_manager.entity_types.ensure_size(Entity_Type_Decl::list_num);
	entity_manager.entity_types.num = Entity_Type_Decl::list_num;

	Entity_Type_Decl *decl = Entity_Type_Decl::list;
	while (decl) {
		entity_manager.entity_types[decl->classify] = decl;
		decl = decl->next;
	}

	space = cpSpaceNew();
	cpCollisionHandler *handler = cpSpaceAddDefaultCollisionHandler(space);
	handler->postSolveFunc = handle_collisions_post_solve;
	handler->beginFunc = handle_collisions_begin;
	handler->separateFunc = handle_collisions_seperate;
}

void Entity_Manager::shutdown() {
	for (int i = 0; i < entity_manager.entities.num; i++) {
		if (entity_manager.entities[i]) {
			entity_manager.entities[i]->shutdown();
		}
		delete entity_manager.entities[i];
	}
}

void Entity_Manager::on_level_load() {
	entity_manager.delete_all_entities();
}

void Entity_Manager::render() {
	For(entities, {
		if (it) {
			renderer.texture(&it->rt);
			it->render();
		}
	});
}

void Entity_Manager::update(float dt) {
	cpSpaceStep(space, 1.0f / 60.0f);

	for (int i = 0; i < entity_manager.entities.num; i++) {
		Entity *entity = entity_manager.entities[i];
		if (!entity) {
			continue;
		}

		if (entity->body) {
			cpVect physics_position = cpBodyGetPosition(entity->body);
			entity->position.x = physics_position.x;
			entity->position.y = physics_position.y;

			//@fixme: figure out how to properly make movement type customizable (angle, x velocity)
			entity->velocity = approach(entity->velocity, entity->goal_velocity, dt * entity->velocity_ramp_speed);
		//	cpBodySetVelocity(entity->body, cpv(entity->velocity.x, entity->velocity.y));
			cpBodySetAngle(entity->body, 0);
		}
	}

	for (int i = 0; i < entity_manager.entities.num; i++) {
		Entity *entity = entity_manager.entities[i];
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
}

Entity *Entity_Manager::get_entity_from_handle(Entity_Handle handle) {
	if (handle.num < 0 || handle.num >= entity_manager.entities.num) {
		return nullptr;
	}
	if (handle.parity < 0) {
		return nullptr;
	}

	Entity *entity = entity_manager.entities[handle.num];
	if (!entity) {
		return nullptr;
	}

	if (entity->parity == handle.parity) {
		return entity;
	}

	return nullptr;
}

void Entity_Manager::add_entity(Entity *entity) {
	bool added = false;

	for (int i = 0; i < entity_manager.entities.num; i++) {
		if (!entity_manager.entities[i]) {
			entity_manager.entities[i] = entity;
			entity->num = i;
			added = true;
		}
	}

	entity->parity = entity_manager.next_parity++;

	if (!added) {
		entity->num = entity_manager.entities.num;
		entity_manager.entities.append(entity);
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

void Entity_Manager::spawn_entity(Entity *entity) {
	if (entity->parity == -1) {
		entity_manager.add_entity(entity);
	}

	entity->spawn();
	entity->setup_physics(entity_manager.space);
	if (entity->body) {
		cpBodySetPosition(entity->body, cpv(entity->position.x, entity->position.y));
		cpBodySetUserData(entity->body, (void *)entity);
	}
}

Entity *Entity_Manager::create_entity(const char *type_name, const char *name, bool spawn, bool add) {
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

void Entity_Manager::delete_entity(Entity *entity) {
	if (!entity) {
		return;
	}
	if (entity->num < 0 || entity->num >= entity_manager.entities.num) {
		return;
	}

	entity_manager.entities[entity->num] = nullptr;
	entity_manager.entity_types[entity->classify]->entities[entity->num_in_type] = nullptr;

	entity->delete_physics(space);
	entity->shutdown();

	delete entity;
}

void Entity_Manager::delete_all_entities() {
	for (int i = 0; i < entity_manager.entities.num; i++) {
		entity_manager.delete_entity(entity_manager.entities[i]);
	}
	entity_manager.entities.num = 0;
	for (int i = 0; i < entity_manager.entity_types.num; i++) {
		Entity_Type_Decl *decl = entity_manager.entity_types[i];
		for (int j = 0; j < decl->entities.num; j++) {
			decl->entities[j] = nullptr;
		}
		decl->entities.num = 0;
	}
}