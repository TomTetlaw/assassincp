#include "precompiled.h"

class Bad_Guy : public Entity {
	Nav_Path path;
	
	int current_point_index = 0;

	void spawn() {
		po->groups = phys_group_badguy;
	}

	void think() {
		path.points.num = 0;
		current_point_index = 0;
		make_path(&path, po->position, game.player->po->position);

		think_time = game.now + 1.0f;
	}

	void update() {
		if (path.points.num > 0) {
			if (po->position.distance_to(path.points[current_point_index]->point) < 64.0f) { //@todo: figure out the proper distance.
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
		For(path.points) {
			auto it = path.points[it_index];
			render_point(it->point, 10.0f, Vec4(1, 0, 0, 1));
		}
	}
};

declare_entity_type(Bad_Guy, "ent_bad_guy", ENTITY_BAD_GUY);