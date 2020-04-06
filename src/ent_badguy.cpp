#include "precompiled.h"

class Bad_Guy : public Entity {
	Nav_Path path;
	cpShape *shape = nullptr;
	cpBody *move_anchor = nullptr;
	cpConstraint *move_constraint = nullptr;
	
	int current_point_index = 0;

	void think() {
		path.points.num = 0;
		current_point_index = 0;
		make_path(&path, position, game.player->position);
		think_time = game.game_time + 1.0f;
	}

	void setup_physics(cpSpace *space) {
		body = cpBodyNew(1, cpMomentForBox(10, size.x, size.y));
		shape = cpBoxShapeNew(body, size.x, size.y, 1);

		cpShapeFilter filter;
		filter.categories = 2;
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

	void update(float dt) {
		if (path.points.num > 0) {
			if (position.distance_to(path.points[current_point_index]->point) < game.current_level->nav_points_size) {
				current_point_index += 1;
				if (current_point_index >= path.points.num) {
					path.points.num = 0;
					current_point_index = 0;
				}
			}

			cpVect v = cpv(path.points[current_point_index]->point.x, path.points[current_point_index]->point.y);
			cpBodySetPosition(move_anchor, v);
		}
		else {
			cpBodySetPosition(move_anchor, cpv(position.x, position.y));
		}
	}

	void render() {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glPushMatrix();
		renderer.setup_render();

		glPointSize(10.0f);
		glBegin(GL_POINTS);
		For(path.points, {
			if (it_index == current_point_index) {
				glColor4f(0, 1, 0, 1);
			}
			else {
				glColor4f(1, 0, 0, 1);
			}

			glVertex2f(it->point.x, it->point.y);
		});
		glEnd();

		glPopMatrix();
	}
};

declare_entity_type(Bad_Guy, "ent_bad_guy", ENTITY_BAD_GUY);