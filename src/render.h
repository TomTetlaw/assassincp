#ifndef __RENDER_H__
#define __RENDER_H__

struct Render_Texture {
	Texture *texture = nullptr;
	Vec2 position;
	Vec2 size;
	Vec2 scale;
	Vec4 colour;
	float angle = 0.0f;
	float sl = -1;
	float sh = -1;
	float tl = -1;
	float th = -1;
	bool centered = false;
	bool repeat = false;

	inline Render_Texture() {
		size = Vec2(-1, -1);
		scale = Vec2(1, 1);
		colour = Vec4(1, 1, 1, 1);
	}
};

struct Render {
	Vec2 camera_position;
	int zoom_level = 0;
};

extern Render renderer;

Vec2 render_to_world_pos(Vec2 a);
float render_scale_for_zoom_level();
float render_inverse_scale_for_zoom_level();
void render_on_level_load();
void render_init();
void render_shutdown();
void render_begin_frame();
void render_end_frame();
void render_setup_render_world(); // middle of screen is (0,0)
void render_setup_render_ui(); // top left of screen is (0,0)
void render_texture(Render_Texture *rt);
void render_debug_string(const char *text, ...);
void render_string_format(Font *font, Vec2 position, Vec4 colour, float wrap, const char *text, ...);
void render_string(Font *font, Vec2 position, Vec4 colour, float wrap, const char *text);
void render_line(Vec2 a, Vec2 b, Vec4 colour);
void render_line(Vec2 position, float length, float angle, Vec4 colour);
void render_box(bool fill, float x, float y, float width, float height, Vec4 colour);
void render_centered_box(bool fill, float x, float y, float width, float height, Vec4 colour);
void render_point(Vec2 position, float size, Vec4 colour);
void render_triangle(Vec4 colour, Vec2 point1, Vec2 point2, Vec2 point3);
void render_polygon(Vec4 colour, Vec2 *verts, int num_verts);

#endif