#include "precompiled.h"

Entity_Types etypes;

int next_parity = 0;

Entity_Manager entity_manager;

Entity *get_new_entity() {
	return entity_manager.entities.alloc();
}

void add_entity(Entity *entity, bool add) {
	entity->parity = next_parity;
	next_parity++;
	entity->added = add;
}

void copy_entity(Entity *source, Entity *dest) {
	int parity = dest->parity;
	int _index = dest->_index;
	Entity_Callbacks *outer = dest->outer;
	memcpy(dest, source, sizeof(Entity));
	dest->parity = parity;
	dest->_index = _index;
	dest->outer = outer;
}

void remove_entity(Entity *entity) {
	entity->outer->remove();
	entity->outer->_remove();
	entity_manager.entities.remove(entity);
	entity->parity = -1;
}

internal void cmd_list_entities(Array<Command_Argument> &args) {
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;

		console_printf("(%d, %d), %s\n", entity->_index, entity->parity, entity->type_name);
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
		if(!entity->added) continue;

		if(game.current_level) entity->outer->render();

		Render_Texture *rt = render_add_rt();
		if(entity->flags & EFLAGS_NO_PHYSICS) {
			rt->size = entity->size;
			rt->position = entity->position;
		} else {
			rt->size = entity->po.size;
			rt->position = entity->po.position;
		}
		rt->texture = get_texture(entity_manager.entities[i]->texture);
		rt->repeat = entity_manager.entities[i]->texture_repeat;
		rt->z = entity->z;
		rt->angle = entity->angle;
	}
}

void render_entity_physics_debug() {
	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;
		if(!entity->added) continue;
		
		if(!(entity->flags & EFLAGS_NO_PHYSICS)) {
			physics_render_debug(&entity->po);
		}
	}
}

void entity_update() {
	physics_step_world(game.delta_time);

	for(int i = 0; i < entity_manager.entities.max_index; i++) {
		Entity *entity = entity_manager.entities[i];
		if(!entity) continue;
		if(!entity->added) continue;

		if(!(entity->flags & EFLAGS_NO_PHYSICS)) {
			if(entity->grid_aligned) {
				entity->po.size.x = entity->grid_w * (float)entity->grid_size_x;
				entity->po.size.y = entity->grid_h * (float)entity->grid_size_y;
				entity->po.position.x = entity->grid_x * (float)entity->grid_size_x;
				entity->po.position.y = entity->grid_y * (float)entity->grid_size_y;
			} else {
				entity->grid_w = (int)(entity->po.size.x / (float)entity->grid_size_x);
				entity->grid_h = (int)(entity->po.size.y / (float)entity->grid_size_y);
				entity->grid_x = (int)(entity->po.position.x / (float)entity->grid_size_x);
				entity->grid_y = (int)(entity->po.position.y / (float)entity->grid_size_y);
			}

			if(entity->grid_w <= 0) entity->grid_w = 1;
			if(entity->grid_h <= 0) entity->grid_h = 1;

			if(entity->po.mass == 0) {
				entity->po.inv_mass = 0;
			} else {
				entity->po.inv_mass = 1 / entity->po.mass;
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
			save_write_int(file, entity_manager.entities[i]->parity);
			save_write_int(file, entity_manager.entities[i]->_index);

			save_write(file, entity_manager.entities[i], sizeof(Entity));

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
		inner->parity = parity; \
		inner->classify = etypes._classify_##x; \
		inner->outer = etypes._##x.alloc(); \
		strcpy(inner->type_name, etypes._name_##x); \
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
			} else if(classify == etypes._classify_Bullet) {
				inner = create_entity_at(Bullet, index, parity);
			} else if(classify == etypes._classify_Badguy) {
				inner = create_entity_at(Badguy, index, parity);
			}

			Entity_Callbacks *outer = inner->outer;
			save_read(file, inner, sizeof(Entity));
			inner->outer = outer;
			if(inner->texture_filename[0]) inner->texture = load_texture(inner->texture_filename);
			inner->po.owner = inner;

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

//========================================================

void Player::setup() {
	inner->set_texture("data/textures/player.png");
	inner->z = 2;
	inner->po.size = Vec2(32, 32);
	inner->po.set_mass(1);
	inner->po.groups = phys_group_player;

	weapons_in_inventory = WEAPON_GUN;
	currently_equiped_weapon = 1;
}

void Player::spawn() {
	//fov_init(&fov);
}

void Player::update() {
	renderer.camera_position = inner->po.position;
	inner->po.goal_velocity = Vec2(0, 0);
	inner->po.velocity_ramp_speed = 300.0f;
	if(input_get_key_state(SDL_SCANCODE_W)) {
		inner->po.velocity_ramp_speed = 2000.0f;
		inner->po.goal_velocity = inner->po.goal_velocity + Vec2(0.0f, -150.0f);
	}
	if(input_get_key_state(SDL_SCANCODE_A)) {
		inner->po.velocity_ramp_speed = 2000.0f;
		inner->po.goal_velocity = inner->po.goal_velocity + Vec2(-150.0f, 0.0f);
	}
	if(input_get_key_state(SDL_SCANCODE_S)) {
		inner->po.velocity_ramp_speed = 2000.0f;
		inner->po.goal_velocity = inner->po.goal_velocity + Vec2(0.0f, 150.0f);
	}
	if(input_get_key_state(SDL_SCANCODE_D)) {
		inner->po.velocity_ramp_speed = 2000.0f;
		inner->po.goal_velocity = inner->po.goal_velocity + Vec2(150.0f, 0.0f);
	}

	//fov.position = inner->po.position;
	//fov_update(&fov);
}

void Player::render() {
	Raycast_Hit hit;
	Vec2 end = Vec2::from_angle(inner->po.position.angle_to(to_world_pos(cursor_position))) * Vec2(10000, 10000);
	Collision_Filter filter;
	filter.mask = ~phys_group_player;
	if(raycast(inner->po.position, end, &hit, filter)) {
		end = hit.point;
	}

	Weapon *weapon = get_weapon();
	if(weapon) {
		Render_Texture *rt = render_add_rt();
		rt->texture = get_texture(weapon->texture);
		rt->z = inner->z;

		float angle = inner->po.position.angle_to(to_world_pos(cursor_position));
		rt->position = inner->po.position + (Vec2::from_angle(angle) * weapon->position);
		rt->angle = angle;
		
		render_texture(rt);
	}
	//fov_render(&fov); // this won't work until we have z sorting for all rendering.
}

void Player::remove() {
	//fov_shutdown(&fov);
}

void Player::handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) {
	if(mouse_button == 1 && down) {
		Weapon *weapon = get_weapon();
		if(weapon) {
			if(game.now - weapon->last_fire_time >= weapon->refire_time) {
				weapon->fire();
				weapon->last_fire_time = game.now;
			}
		}
	}
}

void Player::write(Save_File *file) {
	save_write_u8(file, weapons_in_inventory);
	save_write_u8(file, currently_equiped_weapon);
}

void Player::read(Save_File *file) {
	save_read_u8(file, &weapons_in_inventory);
	save_read_u8(file, &currently_equiped_weapon);
}

void Gun::fire() {
	Bullet *b = create_entity(Bullet, true);

	float angle = etypes._Player[0]->inner->po.position.angle_to(to_world_pos(cursor_position));
	b->inner->angle = angle;
	b->inner->po.position = etypes._Player[0]->inner->po.position + (Vec2::from_angle(angle) * position);
	b->inner->po.velocity = Vec2::from_angle(angle) * Vec2(2000, 2000);
}

void Bullet::update() {
}

void Badguy::setup() {
	inner->set_texture("data/textures/badguy.png");

	inner->z = 2;
	inner->po.size = Vec2(32, 32);
	inner->po.set_mass(1);
	inner->po.groups = phys_group_badguy;
}

void Badguy::render() {
	render_box(inner->po.position + Vec2(0, -100), Vec2(100, 20), true, Vec4(1, 0, 0, 1));
	render_box(inner->po.position + Vec2(0, -100), Vec2(10 * health, 20), true, Vec4(0, 1, 0, 1));
}

void Badguy::handle_collision(Entity *other) {
	if(other->classify == etypes._classify_Bullet) {
		health -= 1;
		if(health <= 0) inner->delete_me = true;
	}
}