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

	void init();
	void update();
	void shutdown();
	void render();
};

#endif