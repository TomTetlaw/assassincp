#include "precompiled.h"

Game game;

void Game::init() {
}

void Game::shutdown() {
}

void Game::update() {
	float temp = game.last_time;
	game.last_time = SDL_GetTicks() / 1000.0f;

	if (!game.paused) {
		game.game_time = game.real_time - game.total_time_paused;
	}


	game.real_delta_time = game.last_time - temp;
	game.real_time += game.delta_time;

	if (game.paused) {
		game.delta_time = 0;
	}
	else {
		game.delta_time = game.real_delta_time;
	}
}

void Game::render() {
}

void Game::toggle_paused() {
	Game::set_paused(!game.paused);
}

void Game::set_paused(bool paused) {
	game.paused = paused;

	if (game.paused) {
		game.total_time_paused += game.real_time - game.time_paused;
	}
	else {
		game.time_paused = game.real_time;
	}
}

class Poly : public Entity {
public:
	cpShape *shape = nullptr;
	Array<Vec2> verts;

	void setup_physics(cpSpace *space) {
		body = cpBodyNewStatic();
		cpBodySetPosition(body, cpv(position.x, position.y));

		cpVect *cp_verts = new cpVect[verts.num];
		for (int i = 0; i < verts.num; i++) {
			cp_verts[i] = cpv(verts[i].x, verts[i].y);
		}

		cpTransform transform = cpTransformIdentity;
		shape = cpPolyShapeNew(body, verts.num, cp_verts, transform, 0.000001f);
		
		int count = cpPolyShapeGetCount(shape);
		for (int i = 0; i < count; i++) {
			cpVect v = cpPolyShapeGetVert(shape, i);
			game.current_level->fov_check_points.append(Vec2(v.x, v.y));
		}

		cpSpaceAddShape(space, shape);
		cpSpaceAddBody(space, body);
	}

	void delete_physics(cpSpace *space) {
		cpSpaceRemoveShape(space, shape);
		cpSpaceRemoveBody(space, body);
	}
};

declare_entity_type(Poly, "info_polygon", ENTITY_POLYGON);

void Game::on_level_load() {
	renderer.on_level_load();
	entity_manager.on_level_load();
	editor.on_level_load();
	delete current_level;
	current_level = nullptr;
}

void process_level();

void Game::load_level(const char *file_name) {
	current_level = new Level;

	Vec2 p[] = {
		Vec2(-10000, -10000),
		Vec2(-10000, 10000),
		Vec2(10000, -10000),
		Vec2(10000, 10000)
	};

	for (int i = 0; i < 4; i++) {
		current_level->fov_check_points.append(p[i]);
	}

	Save_File file;
	if (!save_open_read(file_name, &file)) {
		console.printf("Failed to open map file for reading from game: %d", errno);
		return;
	}

	int version = 0;
	save_read_int(&file, &version);
	if (version != map_file_version) {
		console.printf("Attempting to open map file with old version from game (wanted %d, got %d): %s\n", map_file_version, version, file_name);
		save_close(&file);
		return;
	}

	int num = 0;
	save_read_int(&file, &num);
	for (int i = 0; i < num; i++) {
		int type = 0;
		save_read_int(&file, &type);
		if (type == EDITOR_ENTITY_ENTITY) {
			Editor_Entity entity;
			entity.read_save(&file);

			Entity *ent = entity_manager.create_entity(entity.type_name, entity.name, false, false);
			if (ent) {
				ent->set_position(entity.position);
				ent->size = entity.size;
				ent->rt.scale = entity.scale;
				ent->set_texture(entity.texture_name, false);
				ent->colour = entity.colour;
				entity_manager.spawn_entity(ent);
			}
		}
		else if (type == EDITOR_ENTITY_POLYGON) {
			Editor_Polygon polygon;
			polygon.read_save(&file);

			Poly *poly = (Poly *)entity_manager.create_entity("info_polygon", nullptr, false, false);
			For(polygon.points, {
				poly->verts.append(it->position);
			});
			entity_manager.spawn_entity(poly);
		}
	}

	save_close(&file);

	process_level();
}

void process_level() {
	bool has_one_player_start = true;
	Entity *player_start = nullptr;

	if (entity_manager.entity_types[ENTITY_INFO_PLAYER_START]->entities.num == 0) {
		console.printf("Warning: no info_player_start found!\n");
		has_one_player_start = false;
	}
	if (entity_manager.entity_types[ENTITY_INFO_PLAYER_START]->entities.num > 1) {
		console.printf("Warning: more than one info_player_start found!\n");
	}

	if (has_one_player_start) {
		player_start = entity_manager.entity_types[ENTITY_INFO_PLAYER_START]->entities[0];

		Entity *player = entity_manager.create_entity("ent_player", nullptr, false, false);
		player->position = player_start->position;
		player->set_texture("data/textures/player.png");
		entity_manager.spawn_entity(player);

		input.player = player;

		entity_manager.delete_entity(player_start);
	}
}