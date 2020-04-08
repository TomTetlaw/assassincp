#ifndef __RENDER_H__
#define __RENDER_H__

struct Render_Texture {
	Texture *texture = nullptr;
	Vec2 position;
	Vec2 size = Vec2(-1, -1);
	Vec2 scale = Vec2(1, 1);
	Vec4 colour = Vec4(1, 1, 1, 1);
	float angle = 0.0f;
	float sl = -1;
	float sh = -1;
	float tl = -1;
	float th = -1;
	bool repeat = false;
};

struct Render {
	Vec2 camera_position;
	int zoom_level = 0;
};

extern Render renderer;

Vec2 to_world_pos(Vec2 a);
float render_scale_for_zoom_level();
float render_inverse_scale_for_zoom_level();
void render_on_level_load();
void render_init();
void render_shutdown();
void render_begin_frame();
void render_end_frame();

// scissor means that everything outside of the box will not be rendered
void render_start_scissor(float top, float left, float bottom, float right);
void render_end_scissor();

// you must call one of these before rendering
void render_setup_for_world(); // middle of screen is (0,0) + boxes/textures etc are rendered from centrepoint
void render_setup_for_ui(bool use_camera, bool use_zoom); // top left of screen is (0,0) + boxes/textures etc are rendered from top left

// if rt->size or rt->sl/tl/sh/th are -1 then this function will use default values
void render_texture(Render_Texture *rt);

// note that all render_string_* functions ignore render_setup_centered
void render_string(Vec2 position, const char *text, Vec4 colour = Vec4(1, 1, 1, 1), Font *font = nullptr, float wrap = -1.0f);
void render_string_format(Vec2 position, Vec4 colour, Font *font, float wrap, const char *text, ...);

// if you don't want to think about colour/font/wrap the default values will be same as those in render_string
void render_string_format_lazy(Vec2 position, const char *text, ...);

// renders string at the top of the screen underneath last debug string
// usually used to print out debug info that changes every frame
// won't draw if var renderer_draw_debug is false
// note: expects to be drawn after a call to render_setup_for_ui
void render_debug_string(const char *text, ...);

void render_line(Vec2 a, Vec2 b, Vec4 colour = Vec4(1, 1, 1, 1));
void render_line(float ax, float ay, float bx, float by, Vec4 colour = Vec4(1, 1, 1, 1));

// render a line projected length units from a position 
void render_line2(Vec2 start, float length, float angle, Vec4 colour = Vec4(1, 1, 1, 1));
void render_line2(float x, float y, float length, float angle, Vec4 colour = Vec4(1, 1, 1, 1));

void render_box(Vec2 position, Vec2 size, bool fill = false, Vec4 colour = Vec4(1, 1, 1, 1));
void render_box(float x, float y, float w, float h, bool fill = false, Vec4 colour = Vec4(1, 1, 1, 1));
// render a box using top/left/bottom/right values (ignores render_setup_centered)
void render_box2(float top, float left, float bottom, float right, bool fill = false, Vec4 colour = Vec4(1, 1, 1, 1));

void render_point(Vec2 position, float size = 10.0f, Vec4 colour = Vec4(1, 1, 1, 1));
void render_point(float x, float y, float size = 10.0f, Vec4 colour = Vec4(1, 1, 1, 1));

#endif