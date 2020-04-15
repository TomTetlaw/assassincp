#include "precompiled.h"

Entity_Types etypes;

int next_parity = 0;

Entity_Manager entity_manager;

void Entity::write(Save_File *file) {
	save_write_bool(file, delete_me);

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

	if(texture) save_write_string(file, texture->filename);
	else save_write_string(file, "");
	save_write_bool(file, texture_repeat);

	save_write_bool(file, grid_aligned);
	save_write_int(file, grid_size_x);
	save_write_int(file, grid_size_y);
}

void Entity::read(Save_File *file) {
	save_read_bool(file, &delete_me);

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

	char texture_file_name[1024] = {0};
	save_read_string(file, texture_file_name);
	if(texture_file_name[0]) texture = load_texture(texture_file_name);
	save_read_bool(file, &texture_repeat);

	save_read_bool(file, &grid_aligned);
	save_read_int(file, &grid_size_x);
	save_read_int(file, &grid_size_y);
}

Entity *get_new_entity() {
	return entity_manager.entities.alloc();
}

void add_entity(Entity *entity) {
	entity->handle.parity = next_parity;
	entity->handle.index = entity->_index;
	next_parity++;
}

void _remove_entity(Entity *entity) {
	entity_manager.entities.remove(entity);
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
		if(!entity_manager.entities[i]) continue;

		Physics_Object *po = entity_manager.entities[i]->po;

		Render_Texture rt;
		rt.size.x = po->size.x;
		rt.size.y = po->size.y;
		rt.position.x = po->position.x;
		rt.position.y = po->position.y;
		rt.texture = entity_manager.entities[i]->texture;
		rt.repeat = entity_manager.entities[i]->texture_repeat;
		render_texture(&rt);

		physics_render_debug(po);
	}
}

void entity_update() {
	physics_step_world(game.delta_time);

	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;

		if(entity->grid_aligned) {
			entity->po->size.x = entity->grid_w * (float)entity->grid_size_x;
			entity->po->size.y = entity->grid_h * (float)entity->grid_size_y;
			entity->po->position.x = entity->grid_x * (float)entity->grid_size_x;
			entity->po->position.y = entity->grid_y * (float)entity->grid_size_y;
		}

		if(entity->po->mass == 0) {
			entity->po->inv_mass = 0;
		} else {
			entity->po->inv_mass = 1 / entity->po->mass;
		}
	}

	// @todo: handle entity->delete_me.
}

void entity_on_level_load() {
	entity_manager.entities.remove_all();
}

void entity_write(Save_File *file) {
	int num_entities = 0;
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		if(entity_manager.entities[i]) num_entities++;
	}

	save_write_int(file, num_entities);
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		if(!entity_manager.entities[i]) continue;
		save_write_int(file, entity_manager.entities[i]->classify);
		entity_manager.entities[i]->write(file);
		entity_manager.entities[i]->outer->write(file);
	}
}

internal void remove_all_entities() {
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		physics_remove_object(entity_manager.entities[i]->po);
	}
	entity_manager.entities.remove_all();
}
void entity_read(Save_File *file) {
	remove_all_entities();

	int num_entities = 0;
	save_read_int(file, &num_entities);
	entity_manager.entities.max_index = num_entities;

	for(int i = 0; i < num_entities; i++) {
		int classify = 0;
		save_read_int(file, &classify);
		Entity *entity = nullptr;
		if(classify == etypes._classify_Wall) {
			entity = create_entity(Wall)->inner;
		}
		entity->read(file);
		entity->outer->read(file);
	}
}