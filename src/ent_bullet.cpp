#include "precompiled.h"

class Bullet : public Entity {
    cpBody *move_anchor = nullptr;
	cpConstraint *move_constraint = nullptr;
	cpShape *shape = nullptr;

    void spawn() {
        set_texture("data/textures/default.png");
    }

    void setup_physics(cpSpace *space) {
		body = cpBodyNew(1, cpMomentForBox(10, size.x, size.y));
		shape = cpBoxShapeNew(body, size.x, size.y, 1);

		cpShapeFilter filter;
		filter.categories = 8;
		filter.mask = CP_ALL_CATEGORIES;
		cpShapeSetFilter(shape, filter);

		move_anchor = cpBodyNewStatic();

		move_constraint = cpPivotJointNew(move_anchor, body, cpvzero);
		cpConstraintSetMaxBias(move_constraint, 2000.0f);
		cpConstraintSetMaxForce(move_constraint, 5000.0f);

		cpSpaceAddBody(space, body);
		cpSpaceAddShape(space, shape);
		cpSpaceAddBody(space, move_anchor);
		cpSpaceAddConstraint(space, move_constraint);

        cpBodySetPosition(move_anchor, cpv(goal_position.x, goal_position.y));
    }

    void delete_physics(cpSpace *space) { 
		cpSpaceRemoveConstraint(space, move_constraint);
		cpSpaceRemoveShape(space, shape);
		cpSpaceRemoveBody(space, move_anchor);
		cpSpaceRemoveBody(space, body);
    }

    void handle_collision(Entity *other, Collision_Type type) {
        if(type == COLLISION_BEGIN) {
            delete_me = true;
            set_texture("");
        }
    }
};

declare_entity_type(Bullet, "ent_bullet", ENTITY_BULLET);