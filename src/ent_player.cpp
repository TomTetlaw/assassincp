#include "precompiled.h"

class PlayerStart : public Entity {
};

declare_entity_type(PlayerStart, "info_player_start", ENTITY_INFO_PLAYER_START);

class Player : public Entity {
	Field_Of_View fov;

	void spawn() {
		set_texture("data/textures/player.png");
		fov.init();
	}

	void shutdown() {
		fov.shutdown();
	}

	void update(float dt) {
		const Uint8* state = SDL_GetKeyboardState(nullptr);
		if (state[SDL_SCANCODE_W]) {
			velocity = Vec2(0, -100);
		}
		if (state[SDL_SCANCODE_S]) {
			velocity = Vec2(0, 100);
		}
		if (state[SDL_SCANCODE_A]) {
			velocity = Vec2(-100, 0);
		}
		if (state[SDL_SCANCODE_D]) {
			velocity = Vec2(100, 0);
		}

		renderer.camera_position = position;

		angle = position.angle_to(renderer.to_world_pos(sys.cursor_position));

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