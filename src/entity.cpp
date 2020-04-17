#include "precompiled.h"

Entity_Types etypes;

int next_parity = 0;

Entity_Manager entity_manager;

void Entity::write(Save_File *file) {
	save_write_bool(file, delete_me);

	save_write_bool(file, po != nullptr);
	if(po) {
		save_write_vec2(file, po->position);
		save_write_vec2(file, po->velocity);
		save_write_vec2(file, po->goal_velocity);
		save_write_float(file, po->velocity_ramp_speed);
		save_write_float(file, po->extents.top);
    	save_write_float(file, po->extents.left);
    	save_write_float(file, po->extents.bottom);
    	save_write_float(file, po->extents.right);
		save_write_vec2(file, po->size);
    	save_write_float(file, po->hh);
		save_write_float(file, po->hw);
		for(int i = 0; i < 4; i++) {
			save_write_vec2(file, po->edges[i].a);
			save_write_vec2(file, po->edges[i].b);
		}
		save_write_float(file, po->mass);
		save_write_float(file, po->inv_mass);
		save_write_float(file, po->restitution);
		save_write_bool(file, po->colliding);
		save_write_uint(file, po->groups);
		save_write_uint(file, po->mask);
		save_write_bool(file, grid_aligned);
		save_write_int(file, grid_size_x);
		save_write_int(file, grid_size_y);
		save_write_int(file, grid_x);
		save_write_int(file, grid_y);
		save_write_int(file, grid_w);
		save_write_int(file, grid_h);
	} else {
		save_write_vec2(file, position);
		save_write_vec2(file, size);
	}

	if(texture) save_write_string(file, texture->filename);
	else save_write_string(file, "");
	save_write_bool(file, texture_repeat);
	save_write_int(file, z);
}

void Entity::read(Save_File *file) {
	save_read_bool(file, &delete_me);

	bool has_po = false;
	save_read_bool(file, &has_po);
	if(has_po) po = physics_add_object();

	if(po) {
		save_read_vec2(file, &po->position);
		save_read_vec2(file, &po->velocity);
		save_read_vec2(file, &po->goal_velocity);
		save_read_float(file, &po->velocity_ramp_speed);
		save_read_float(file, &po->extents.top);
    	save_read_float(file, &po->extents.left);
    	save_read_float(file, &po->extents.bottom);
    	save_read_float(file, &po->extents.right);
		save_read_vec2(file, &po->size);
    	save_read_float(file, &po->hh);
		save_read_float(file, &po->hw);
		for(int i = 0; i < 4; i++) {
			save_read_vec2(file, &po->edges[i].a);
			save_read_vec2(file, &po->edges[i].b);
		}
		save_read_float(file, &po->mass);
		save_read_float(file, &po->inv_mass);
		save_read_float(file, &po->restitution);
		save_read_bool(file, &po->colliding);
		save_read_uint(file, &po->groups);
		save_read_uint(file, &po->mask);

		save_read_bool(file, &grid_aligned);
		save_read_int(file, &grid_size_x);
		save_read_int(file, &grid_size_y);
		save_read_int(file, &grid_x);
		save_read_int(file, &grid_y);
		save_read_int(file, &grid_w);
		save_read_int(file, &grid_h);
	} else {
		save_read_vec2(file, &position);
		save_read_vec2(file, &size);
	}

	char texture_file_name[1024] = {0};
	save_read_string(file, texture_file_name);
	if(texture_file_name[0]) texture = load_texture(texture_file_name);
	save_read_bool(file, &texture_repeat);
	save_read_int(file, &z);
}

Entity *get_new_entity() {
	return entity_manager.entities.alloc();
}

void add_entity(Entity *entity) {
	entity->handle.parity = next_parity;
	entity->handle.index = entity->_index;
	next_parity++;
}

void remove_entity(Entity *entity) {
	entity->outer->remove();
	entity->outer->_remove();
	if(entity->po) physics_remove_object(entity->po);
	entity_manager.entities.remove(entity);
	entity->handle.index = -1;
	entity->handle.parity = -1;
}

internal void cmd_list_entities(Array<Command_Argument> &args) {
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;

		console_printf("(%d, %d), %s\n", entity->handle.index, entity->handle.parity, entity->type_name);
	}
}

void entity_init() {
	entity_manager.entities.init();

	register_command("list_entities", cmd_list_entities);
}

void entity_shutdown() {
}

void entity_render() {
	render_setup_for_world();

	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;

		if(game.current_level) entity->outer->render();

		Physics_Object *po = entity_manager.entities[i]->po;
		Render_Texture *rt = render_add_rt();
		if(po) {
			rt->size.x = po->size.x;
			rt->size.y = po->size.y;
			rt->position.x = po->position.x;
			rt->position.y = po->position.y;
		} else {
			rt->size = entity->size;
			rt->position = entity->position;
		}
		rt->texture = entity_manager.entities[i]->texture;
		rt->repeat = entity_manager.entities[i]->texture_repeat;
		rt->z = entity->z;
	}
}

void render_entity_physics_debug() {
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;
		
		if(entity->po) {
			physics_render_debug(entity->po);
		}
	}
}

void entity_update() {
	physics_step_world(game.delta_time);

	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;

		if(entity->po) {
			if(entity->grid_aligned) {
				entity->po->size.x = entity->grid_w * (float)entity->grid_size_x;
				entity->po->size.y = entity->grid_h * (float)entity->grid_size_y;
				entity->po->position.x = entity->grid_x * (float)entity->grid_size_x;
				entity->po->position.y = entity->grid_y * (float)entity->grid_size_y;
			} else {
				entity->grid_w = (int)(entity->po->size.x / (float)entity->grid_size_x);
				entity->grid_h = (int)(entity->po->size.y / (float)entity->grid_size_y);
				entity->grid_x = (int)(entity->po->position.x / (float)entity->grid_size_x);
				entity->grid_y = (int)(entity->po->position.y / (float)entity->grid_size_y);
			}

			if(entity->grid_w <= 0) entity->grid_w = 1;
			if(entity->grid_h <= 0) entity->grid_h = 1;

			if(entity->po->mass == 0) {
				entity->po->inv_mass = 0;
			} else {
				entity->po->inv_mass = 1 / entity->po->mass;
			}
		}

		if(game.current_level) entity->outer->update();
	}

	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;
		if(entity->delete_me) remove_entity(entity);
	}
}

internal void remove_all_entities() {
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		if(!entity_manager.entities[i]) continue;
		remove_entity(entity_manager.entities[i]);
	}
}

void entity_on_level_load() {
	remove_all_entities();
	next_parity = 0;
}

void entity_write(Save_File *file) {
	save_write_int(file, next_parity);
	save_write_int(file, entity_manager.entities.max_index);
	save_write_int(file, entity_manager.entities.last_freed_index);
	save_write_int(file, entity_manager.entities.elements.num);
	
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		if(entity_manager.entities[i]) {
			save_write_int(file, 1);

			save_write_int(file, entity_manager.entities[i]->classify);
			save_write_int(file, entity_manager.entities[i]->handle.parity);
			save_write_int(file, entity_manager.entities[i]->handle.index);

			entity_manager.entities[i]->write(file);
			entity_manager.entities[i]->outer->write(file);
		}
		else {
			save_write_int(file, 0);
			continue;
		}
	}
}

// if anything gets added to this is should probably also be added to create_entity in entity.h.
#define create_entity_at(x, index, parity) \
	([&]() -> Entity * { \
		Entity *inner = &entity_manager.entities.elements.data[index]; \
		inner->_deleted = false; \
		inner->_index = index; \
		inner->handle.index = index; \
		inner->handle.parity = parity; \
		inner->classify = etypes._classify_##x; \
		inner->outer = etypes._##x.alloc(); \
		inner->type_name = etypes._name_##x; \
		((x *)inner->outer)->stored_in = &etypes._##x; \
		((x *)inner->outer)->inner = inner; \
		return inner; \
	})()

void entity_read(Save_File *file) {
	remove_all_entities();

	save_read_int(file, &next_parity);
	save_read_int(file, &entity_manager.entities.max_index);
	save_read_int(file, &entity_manager.entities.last_freed_index);
	save_read_int(file, &entity_manager.entities.elements.num);

	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		int entity_there = 0;
		save_read_int(file, &entity_there);

		if(entity_there == 1) {
			int classify = 0;
			int parity = 0;
			int index = 0;
			save_read_int(file, &classify);
			save_read_int(file, &parity);
			save_read_int(file, &index);

			Entity *inner = nullptr;
			if(classify == etypes._classify_Wall) {
				inner = create_entity_at(Wall, index, parity);
			} else if(classify == etypes._classify_Player) {
				inner = create_entity_at(Player, index, parity);
			} else if(classify == etypes._classify_Parallax) {
				inner = create_entity_at(Parallax, index, parity);
			} else if(classify == etypes._classify_Floor) {
				inner = create_entity_at(Floor, index, parity);
			}

			inner->read(file);
			inner->outer->read(file);
		}
	}
}

void entity_spawn_all() {
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;
		entity->outer->spawn();
	}
}