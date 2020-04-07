#ifndef __FOV_H__
#define __FOV_H__

struct Sorted_FOV_Vert {
	float angle;
	Vec2 position;
};

struct Field_Of_View {
	int num_verts = 0;
	Vec2 *verts = nullptr;
	Sorted_FOV_Vert *sorted = nullptr;
	Vec2 position;
};

void fov_init(Field_Of_View *fov);
void fov_update(Field_Of_View *fov);
void fov_shutdown(Field_Of_View *fov);
void fov_render(Field_Of_View *fov);

#endif