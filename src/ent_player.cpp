#include "precompiled.h"

class PlayerStart : public Entity {
};

declare_entity_type(PlayerStart, "info_player_start", ENTITY_INFO_PLAYER_START);

class Player : public Entity {
	Field_Of_View fov;

	void spawn() {
		set_texture("data/textures/player.png");
		fov_init(&fov);
	}

	void shutdown() {
		fov_shutdown(&fov);
	}

	void update(float dt) {
		const Uint8* state = SDL_GetKeyboardState(nullptr);
		velocity = Vec2(0,0);
		if (state[SDL_SCANCODE_W]) {
			velocity = velocity + Vec2(0, -300);
		}
		if (state[SDL_SCANCODE_S]) {
			velocity = velocity + Vec2(0, 300);
		}
		if (state[SDL_SCANCODE_A]) {
			velocity = velocity + Vec2(-300, 0);
		}
		if (state[SDL_SCANCODE_D]) {
			velocity = velocity + Vec2(300, 0);
		}

		renderer.camera_position = position;

		angle = position.angle_to(to_world_pos(cursor_position));

		fov.position = position;
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