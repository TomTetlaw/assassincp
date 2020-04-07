#include "precompiled.h"

class Bad_Guy : public Entity {
	Nav_Path path;
	
	int current_point_index = 0;

	void think() {
		path.points.num = 0;
		current_point_index = 0;
		make_path(&path, position, game.player->position);

		think_time = game.game_time + 1.0f;
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
		}
		else {
		}
	}

	void render() {
		Entity::render();
		
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