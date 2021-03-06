#include "precompiled.h"

internal int compare_fov_vert(const void *a, const void *b) {
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

void fov_init(Field_Of_View *fov) {
	fov->num_verts = (game.current_level->fov_check_points.num * num_rays);
	fov->verts = new Vec2[fov->num_verts];
	fov->sorted = new Sorted_FOV_Vert[fov->num_verts];
}

void fov_update(Field_Of_View *fov) {
	int vert_num = 0;
	for (int i = 0; i < game.current_level->fov_check_points.num; i++) {
		for (int j = 0; j < num_rays; j++) {
			Vec2 points[num_rays];
			points[0] = game.current_level->fov_check_points[i];
			points[1] = fov->position + Vec2::from_angle(fov->position.angle_to(game.current_level->fov_check_points[i]) - 0.000572958f) * 10000;
			points[2] = fov->position + Vec2::from_angle(fov->position.angle_to(game.current_level->fov_check_points[i]) + 0.000572958f) * 10000;
			Collision_Filter filter;
			filter.mask = phys_group_wall;
			Raycast_Hit hit;
			if (raycast(fov->position, points[j], &hit, filter)) {
				fov->verts[vert_num] = hit.point;
			}
			else {
				fov->verts[vert_num] = points[j];
			}

			vert_num++;
		}
	}

	for (int i = 0; i < fov->num_verts; i++) {
		fov->sorted[i].angle = fov->position.angle_to(fov->verts[i]);
		fov->sorted[i].position = fov->verts[i];
	}

	qsort(fov->sorted, fov->num_verts, sizeof(Sorted_FOV_Vert), compare_fov_vert);
}

void fov_shutdown(Field_Of_View *fov) {
	if(fov->verts) { delete[] fov->verts; }
	if(fov->sorted) { delete[] fov->sorted; }
}

void fov_render(Field_Of_View *fov) {
	render_setup_for_world();

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < fov->num_verts - 1; i++) {
		glVertex2f(fov->position.x, fov->position.y);
		glVertex2f(fov->sorted[i].position.x, fov->sorted[i].position.y);
		glVertex2f(fov->sorted[i + 1].position.x, fov->sorted[i + 1].position.y);
	}
	glVertex2f(fov->position.x, fov->position.y);
	glVertex2f(fov->sorted[0].position.x, fov->sorted[0].position.y);
	glVertex2f(fov->sorted[fov->num_verts - 1].position.x, fov->sorted[fov->num_verts - 1].position.y);
	glEnd();

	glColor4f(1, 0, 0, 1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < fov->num_verts - 1; i++) {
		glVertex2f(fov->position.x, fov->position.y);
		glVertex2f(fov->sorted[i].position.x, fov->sorted[i].position.y);
		glVertex2f(fov->sorted[i + 1].position.x, fov->sorted[i + 1].position.y);
	}
	glVertex2f(fov->position.x, fov->position.y);
	glVertex2f(fov->sorted[0].position.x, fov->sorted[0].position.y);
	glVertex2f(fov->sorted[fov->num_verts - 1].position.x, fov->sorted[fov->num_verts - 1].position.y);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}