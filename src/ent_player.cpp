#include "precompiled.h"

class PlayerStart : public Entity {
};

declare_entity_type(PlayerStart, "info_player_start", ENTITY_INFO_PLAYER_START);

class Player : public Entity {
	cpPivotJoint *joint = nullptr;
	cpBody *move_anchor = nullptr;
	cpConstraint *move_constraint = nullptr;
	cpShape *shape = nullptr;
	Field_Of_View fov;

	void spawn() {
		set_texture("data/textures/player.png");
		velocity_ramp_speed = 1000;
		fov.init();
	}

	void shutdown() {
		fov.shutdown();
	}

	void update(float dt) {
		const Uint8* state = SDL_GetKeyboardState(nullptr);
		Vec2 anchor_pos = position;
		if (state[SDL_SCANCODE_W]) {
			anchor_pos = anchor_pos + Vec2(0, -100);
		}
		if (state[SDL_SCANCODE_S]) {
			anchor_pos = anchor_pos + Vec2(0, 100);
		}
		if (state[SDL_SCANCODE_A]) {
			anchor_pos = anchor_pos + Vec2(-100, 0);
		}
		if (state[SDL_SCANCODE_D]) {
			anchor_pos = anchor_pos + Vec2(100, 0);
		}

		cpBodySetPosition(move_anchor, cpv(anchor_pos.x, anchor_pos.y));

		renderer.camera_position = position;

		angle = position.angle_to(renderer.to_world_pos(sys.cursor_position));

		fov.position = position;
		fov.update();
	}

	void handle_key_press(SDL_Scancode scancode, bool down, int mods) {
	}

	void render() {
		//fov.render();
	}

	void setup_physics(cpSpace *space) {
		body = cpBodyNew(1, cpMomentForBox(10, size.x, size.y));
		shape = cpBoxShapeNew(body, size.x, size.y, 1);

		cpShapeFilter filter;
		filter.categories = 1;
		cpShapeSetFilter(shape, filter);

		move_anchor = cpBodyNewStatic();

		move_constraint = cpPivotJointNew(move_anchor, body, cpvzero);
		cpConstraintSetMaxBias(move_constraint, 300.0f);
		cpConstraintSetMaxForce(move_constraint, 3000.0f);

		cpSpaceAddShape(space, shape);
		cpSpaceAddBody(space, body);
		cpSpaceAddBody(space, move_anchor);
		cpSpaceAddConstraint(space, move_constraint);
	}

	void delete_physics(cpSpace *space) {
		cpSpaceRemoveConstraint(space, move_constraint);
		cpSpaceRemoveShape(space, shape);
		cpSpaceRemoveBody(space, move_anchor);
		cpSpaceRemoveBody(space, body);
	}
};

declare_entity_type(Player, "ent_player", ENTITY_PLAYER);