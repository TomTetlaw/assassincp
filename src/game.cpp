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

		cpSpaceAddShape(space, shape);
		cpSpaceAddBody(space, body);
	}

	void delete_physics(cpSpace *space) {
		cpSpaceRemoveShape(space, shape);
		cpSpaceRemoveBody(space, body);
	}
};

declare_entity_type(Poly, ENTITY_POLYGON);

void Game::on_level_load() {
	renderer.on_level_load();
	entity_manager.on_level_load();
	editor.on_level_load();
	delete current_level;
	current_level = nullptr;
}

void Game::load_level(const char *file_name) {
	Load_File_Result in = load_file(file_name);
	char token[2048] = { 0 };
	const char *text_position = in.data;

	if (!in.data) {
		return;
	}

	on_level_load();

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

	delete[] in.data;
}