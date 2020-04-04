#include "precompiled.h"

int compare_fov_vert(const void *a, const void *b) {
	const Sorted_FOV_Vert *va = (const Sorted_FOV_Vert *)a;
	const Sorted_FOV_Vert *vb = (const Sorted_FOV_Vert *)b;
	float angle_a = va->angle;
	float angle_b = vb->angle;
	angle_a = rad2deg(angle_a);
	angle_b = rad2deg(angle_b);
	if (angle_a < 0) angle_a += 360;
	if (angle_b < 0) angle_b += 360;
	if (angle_a > angle_b) {
		return 1;
	}
	else if (angle_a < angle_b) {
		return -1;
	}
	return 0;
}

const int num_rays = 3;

void Field_Of_View::init() {
	num_verts = (game.current_level->fov_check_points.num * num_rays);
	verts = new Vec2[num_verts];
	sorted = new Sorted_FOV_Vert[num_verts];
}

void Field_Of_View::update() {
	int vert_num = 0;
	for (int i = 0; i < game.current_level->fov_check_points.num; i++) {
		cpSegmentQueryInfo info[num_rays];
		cpShapeFilter filter;
		filter.categories = 1;

		for (int j = 0; j < num_rays; j++) {
			Vec2 points[num_rays];
			points[0] = game.current_level->fov_check_points[i];
			points[1] = position + Vec2::from_angle(position.angle_to(game.current_level->fov_check_points[i]) - 0.00001f) * 10000;
			points[2] = position + Vec2::from_angle(position.angle_to(game.current_level->fov_check_points[i]) + 0.00001f) * 10000;
			if (cpSpaceSegmentQueryFirst(entity_manager.space, cpv(position.x, position.y), cpv(points[j].x, points[j].y), 0.00001f, filter, &info[j])) {
				verts[vert_num] = Vec2(info[j].point.x, info[j].point.y);
			}
			else {
				verts[vert_num] = points[j];
			}

			vert_num++;
		}
	}

	for (int i = 0; i < num_verts; i++) {
		sorted[i].angle = position.angle_to(verts[i]);
		sorted[i].position = verts[i];
	}

	qsort(sorted, num_verts, sizeof(Sorted_FOV_Vert), compare_fov_vert);
}

void Field_Of_View::shutdown() {
	delete[] verts;
	delete[] sorted;
}

void Field_Of_View::render() {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	renderer.setup_render();
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < num_verts - 1; i++) {
		glVertex2f(position.x, position.y);
		glVertex2f(sorted[i].position.x, sorted[i].position.y);
		glVertex2f(sorted[i + 1].position.x, sorted[i + 1].position.y);
	}
	glVertex2f(position.x, position.y);
	glVertex2f(sorted[0].position.x, sorted[0].position.y);
	glVertex2f(sorted[num_verts - 1].position.x, sorted[num_verts - 1].position.y);
	glEnd();
	glPopMatrix();

	if (!renderer.debug_draw) {
		return;
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	renderer.setup_render();
	glColor4f(1, 0, 0, 1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < num_verts - 1; i++) {
		glVertex2f(position.x, position.y);
		glVertex2f(sorted[i].position.x, sorted[i].position.y);
		glVertex2f(sorted[i + 1].position.x, sorted[i + 1].position.y);
	}
	glVertex2f(position.x, position.y);
	glVertex2f(sorted[0].position.x, sorted[0].position.y);
	glVertex2f(sorted[num_verts - 1].position.x, sorted[num_verts - 1].position.y);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopMatrix();
}