#include "precompiled.h"

class PlayerStart : public Entity {
};

declare_entity_type(PlayerStart, "info_player_start", ENTITY_INFO_PLAYER_START);

class Player : public Entity {
	Field_Of_View fov;

	void spawn() {
		set_texture("data/textures/player.png");
		fov_init(&fov);

		po->shape_type = PHYSICS_SHAPE_BOX;
		po->box.size = rt.size;
		po->velocity_ramp_speed = 500.0f;
	}

	void shutdown() {
		fov_shutdown(&fov);
	}

	void update() {
		const Uint8* state = SDL_GetKeyboardState(nullptr);
		Vec2 goal_velocity = Vec2(0,0);
		if (state[SDL_SCANCODE_W]) {
			goal_velocity = goal_velocity + Vec2(0, -300);
		}
		if (state[SDL_SCANCODE_S]) {
			goal_velocity = goal_velocity + Vec2(0, 300);
		}
		if (state[SDL_SCANCODE_A]) {
			goal_velocity = goal_velocity + Vec2(-300, 0);
		}
		if (state[SDL_SCANCODE_D]) {
			goal_velocity = goal_velocity + Vec2(300, 0);
		}
		po->goal_velocity = goal_velocity;

		renderer.camera_position = po->position;

		rt.angle = po->position.angle_to(to_world_pos(cursor_position));

		fov.position = po->position;
		//fov.update();
	}

	void handle_key_press(SDL_Scancode scancode, bool down, int mods) {
	}

	void render() {
		Entity::render();
		//fov.render();
	}
};

declare_entity_type(Player, "ent_player", ENTITY_PLAYER);