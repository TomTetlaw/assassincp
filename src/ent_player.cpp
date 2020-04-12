#include "precompiled.h"

class PlayerStart : public Entity {
};

declare_entity_type(PlayerStart, "info_player_start", ENTITY_INFO_PLAYER_START);

class Player : public Entity {
	Field_Of_View fov;

	void spawn() {
		set_texture("data/textures/player.png");
		fov_init(&fov);

		po->size = rt.size;
		po->set_mass(1.0f);
		po->groups = phys_group_player;
	}

	void shutdown() {
		fov_shutdown(&fov);
	}

	void update() {
		const Uint8* state = SDL_GetKeyboardState(nullptr);
		Vec2 goal_velocity = Vec2(0,0);
		po->velocity_ramp_speed = 500.0f;
		if (state[SDL_SCANCODE_W]) {
			goal_velocity = goal_velocity + Vec2(0, -300);
			po->velocity_ramp_speed = 2000.0f;
		}
		if (state[SDL_SCANCODE_S]) {
			goal_velocity = goal_velocity + Vec2(0, 300);
			po->velocity_ramp_speed = 2000.0f;
		}
		if (state[SDL_SCANCODE_A]) {
			goal_velocity = goal_velocity + Vec2(-300, 0);
			po->velocity_ramp_speed = 2000.0f;
		}
		if (state[SDL_SCANCODE_D]) {
			goal_velocity = goal_velocity + Vec2(300, 0);
			po->velocity_ramp_speed = 2000.0f;
		}
		po->goal_velocity = goal_velocity;

		renderer.camera_position = po->position;

		rt.angle = po->position.angle_to(to_world_pos(cursor_position));

		fov.position = po->position;
		fov_update(&fov);
	}

	bool handle_key_press(SDL_Scancode scancode, bool down, int mods) {
		return false;
	}

	void render() {
		Entity::render();
		//fov_render(&fov);
	}
};

declare_entity_type(Player, "ent_player", ENTITY_PLAYER);