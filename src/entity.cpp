#include "precompiled.h"

Entity_Types etypes;

int next_parity = 0;
internal Contiguous_Array<Entity, max_entities> entities;

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

	save_write_int(file, classify);

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
	save_write_float(file, po->mass);
	save_write_float(file, po->inv_mass);
	save_write_float(file, po->restitution);
	save_write_bool(file, po->colliding);
	save_write_uint(file, po->groups);
	save_write_uint(file, po->mask);

	char texture_file_name[1024] = {0};
	save_read_string(file, texture_file_name);
	if(texture_file_name[0]) texture = load_texture(texture_file_name);
	save_read_bool(file, &texture_repeat);

	save_read_int(file, &classify);

	save_read_bool(file, &grid_aligned);
	save_read_int(file, &grid_size_x);
	save_read_int(file, &grid_size_y);
}

Entity *get_new_entity() {
	return entities.alloc();
}

void add_entity(Entity *entity) {
	entity->handle.parity = next_parity;
	entity->handle.index = entity->_index;
	next_parity++;
}

void _remove_entity(Entity *entity) {
	entities.remove(entity);
}

void entity_init() {
	entities.init();
}

void entity_shutdown() {
}

void entity_render() {
	render_setup_for_world();

	for(int i = 0; i < entities.max_index; i++) {
		if(!entities[i]) continue;

		Physics_Object *po = entities[i]->po;

		Render_Texture rt;
		rt.size.x = po->size.x;
		rt.size.y = po->size.y;
		rt.position.x = po->position.x;
		rt.position.y = po->position.y;
		rt.texture = entities[i]->texture;
		rt.repeat = entities[i]->texture_repeat;
		render_texture(&rt);

		physics_render_debug(po);
	}
}

void entity_update() {
	physics_step_world(game.delta_time);

	for(int i = 0; i < entities.max_index; i++) {
		if(!entities[i]) continue;

		if(entities[i]->grid_aligned) {
			entities[i]->po->size.x = entities[i]->grid_w * (float)entities[i]->grid_size_x;
			entities[i]->po->size.y = entities[i]->grid_h * (float)entities[i]->grid_size_y;
			entities[i]->po->position.x = entities[i]->grid_x * (float)entities[i]->grid_size_x;
			entities[i]->po->position.y = entities[i]->grid_y * (float)entities[i]->grid_size_y;
		}

		if(entities[i]->po->mass == 0) {
			entities[i]->po->inv_mass = 0;
		} else {
			entities[i]->po->inv_mass = 1 / entities[i]->po->mass;
		}
	}

	for(int i = 0; i < entities.max_index; i++) {
		if(!entities[i]) continue;
	}
}

void entity_on_level_load() {
	entities.remove_all();
}