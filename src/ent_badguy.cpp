#include "precompiled.h"

class Bad_Guy : public Entity {
	Nav_Path path;
	
	int current_point_index = 0;

	void think() {
		path.points.num = 0;
		current_point_index = 0;
		make_path(&path, po->position, game.player->po->position);

		think_time = game.now + 1.0f;
	}

	void update() {
		if (path.points.num > 0) {
			if (po->position.distance_to(path.points[current_point_index]->point) < game.current_level->nav_points_size) {
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
	}
};

declare_entity_type(Bad_Guy, "ent_bad_guy", ENTITY_BAD_GUY);