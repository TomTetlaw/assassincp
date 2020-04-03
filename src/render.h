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
	bool should_clear = true;
	Vec4 clear_colour;
	bool debug_draw = true;

	Font *default_font = nullptr;
	Vec2 debug_string_position;

	bool use_camera = true;
	Vec2 camera_position;

	bool use_zoom = true;
	int zoom_level = 0;

	float scale_for_zoom_level(int level);
	float inverse_scale_for_zoom_level(int level);

	Vec2 to_world_pos(Vec2 a) { 
		return (a + camera_position) * inverse_scale_for_zoom_level(zoom_level);
	}

	void on_level_load();

	void init();
	void shutdown();
	void begin_frame();
	void end_frame();
	void update(float dt);

	void setup_render();

	void render_physics_debug();
	void texture(Render_Texture *rt);
	void debug_string(const char *text, ...);
	void string_format(Font *font, Vec2 position, Vec4 colour, float wrap, const char *text, ...);
	void string(Font *font, Vec2 position, Vec4 colour, float wrap, const char *text);
	void line(Vec2 a, Vec2 b, Vec4 colour);
	void line(Vec2 position, float length, float angle, Vec4 colour);
	void box(bool fill, float x, float y, float width, float height, Vec4 colour);
	void centered_box(bool fill, float x, float y, float width, float height, Vec4 colour);
	void point(Vec2 position, float size, Vec4 colour);
	void triangle(Vec4 colour, Vec2 point1, Vec2 point2, Vec2 point3);
	void polygon(Vec4 colour, Vec2 *verts, int num_verts);

	Vec2 get_string_size(const char *s);
};

extern Render renderer;

#endif