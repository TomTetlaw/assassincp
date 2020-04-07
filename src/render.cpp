#include "precompiled.h"

Render renderer;

internal bool should_clear = true;
internal Vec4 clear_colour;
internal bool debug_draw = true;
internal Font *default_font = nullptr;
internal Vec2 debug_string_position;
internal char *default_font_file = "data/fonts/consolas.ttf";
internal char **default_font_file_ptr = &default_font_file;
internal int default_font_size = 16;
internal bool centered = false;
internal double z_near = 0.0;
internal double z_far = 1.0;

internal void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

internal void reload_default_font(Config_Var *var) {
	default_font = load_font(*default_font_file_ptr, default_font_size);
	assert(default_font != nullptr);
}

void render_init() {
	glewInit();

	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback((GLDEBUGPROC)opengl_debug_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	register_var("clear_colour", &clear_colour);
	register_var("should_clear_screen", &should_clear);
	register_var("draw_debug", &debug_draw);
	register_var("z_near", &z_near);
	register_var("z_far", &z_far);
	register_var("default_font_file", default_font_file_ptr, reload_default_font);
	register_var("default_font_size", &default_font_size, reload_default_font);

	FILE *f = nullptr;
	fopen_s(&f, "data/opengl.log", "w");
	if(f) {
		fclose(f);
	}

	default_font = load_font(*default_font_file_ptr, default_font_size);
	debug_string_position = Vec2(-sys.window_size.x, -sys.window_size.y);
}

void render_shutdown() {
}

void render_on_level_load() {
	renderer.camera_position = Vec2(0, 0);
	renderer.zoom_level = 0;
}

void render_begin_frame() {
	if(should_clear) {
		glClearColor(v4parms(clear_colour));
		glClear(GL_COLOR_BUFFER_BIT);
	}

	glClear(GL_DEPTH_BUFFER_BIT);

	debug_string_position = Vec2(-(sys.window_size.x / 2) + 10, -(sys.window_size.y / 2) + (float)default_font->line_skip);
}

internal void setup_camera_and_zoom() {
	glTranslatef(-renderer.camera_position.x, -renderer.camera_position.y, 0);
	float scale = render_scale_for_zoom_level();
	glScalef(scale, scale, 1);
}

// scissor means that everything outside of the box will not be rendered
void render_start_scissor(float top, float left, float bottom, float right) {
	glEnable(GL_SCISSOR_TEST);
	glScissor(left + (sys.window_size.x / 2), -bottom + (sys.window_size.y / 2), right - left, bottom - top);
}

void render_end_scissor() {
	glDisable(GL_SCISSOR_TEST);
}

void render_setup_for_world() {
	centered = true;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float hw = sys.window_size.x / 2;
	float hh = sys.window_size.y / 2;
	glOrtho(-hw, hw, hh, -hh, z_near, z_far);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//setup_camera_and_zoom();
}

void render_setup_for_ui() {
	centered = false;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, sys.window_size.x, 0, sys.window_size.y, z_near, z_far);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

	//setup_camera_and_zoom();
}

void render_end_frame() {
	SDL_GL_SwapWindow(sys.window);
}

float render_scale_for_zoom_level() {
	float scale = 0.0f;
	if (renderer.zoom_level == 0) {
		scale = 1.0f;
	}
	else if (renderer.zoom_level < 0) {
		scale = 1.0f / (((renderer.zoom_level * 0.125f)*-1) + 1);
	}
	else if (renderer.zoom_level > 0) {
		scale = (renderer.zoom_level * 0.25f) + 1;
	}
	return scale;
}

float render_inverse_scale_for_zoom_level() {
	float scale = 0.0f;
	if (renderer.zoom_level == 0) {
		scale = 1.0f;
	}
	else if (renderer.zoom_level < 0) {
		scale = ((renderer.zoom_level * 0.125f) * -1) + 1;
	}
	else if (renderer.zoom_level > 0) {
		scale = 1.0f / ((renderer.zoom_level * 0.25f) + 1);
	}
	return scale;
}

Vec2 render_to_world_pos(Vec2 a) {
	return (a + renderer.camera_position) * render_inverse_scale_for_zoom_level();
}

void render_texture(Render_Texture *rt) {
	if (!rt->texture) {
		return;
	}

	float x = rt->position.x;
	float y = rt->position.y;
	float w = rt->size.x;
	float h = rt->size.y;
	float hw = w / 2;
	float hh = h / 2;

	if (rt->size.x == 0) {
		w = 1;
	}
	else if (rt->size.x < 0) {
		w = (float)rt->texture->width;
	}

	if (rt->size.y == 0) {
		h = 1;
	}
	else if (rt->size.y < 0) {
		h = (float)rt->texture->height;
	}

	float sl = rt->sl;
	float sh = rt->sh;
	float tl = rt->tl;
	float th = rt->th;

	if (sl == -1) { sl = 0; }
	if (sh == -1) { sh = 1; }
	if (tl == -1) { tl = 0; }
	if (th == -1) { th = 1; }

	float repeat_count_x = w / rt->texture->width;
	float repeat_count_y = h / rt->texture->height;
	if (!rt->repeat) {
		repeat_count_x = 1;
		repeat_count_y = 1;
	}

	//@todo_now test that this works
	if(centered) {
		x -= hw;
		y -= hh;
		w += hw;
		h += hh;
	}

	glColor4f(v4parms(rt->colour));

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, rt->texture->api_object);

	//@todo_now: test if this makes opengl ignore out render_setup calls
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	//@todo_now: test if this works for both centered/not centered
	glTranslatef(x, y, 0);
	glRotatef(rad2deg(rt->angle), 0, 0, 1);
	glTranslatef(-x, -y, 0);

	glScalef(rt->scale.x, rt->scale.y, 1);

	glBegin(GL_QUADS);
	glTexCoord2f(sl, tl);
	glVertex2f(x, y);
	glTexCoord2f(sl, th * repeat_count_y);
	glVertex2f(x, y + h);
	glTexCoord2f(sh * repeat_count_x, th * repeat_count_y);
	glVertex2f(x + w, y + h);
	glTexCoord2f(sh * repeat_count_x, tl);
	glVertex2f(x + w, y);
	glEnd();

	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

void render_string(Vec2 position, const char *text, Vec4 colour, Font *font, float wrap) {
	if (!font) {
		font = default_font;
	}

	float origin_x = position.x;

	Render_Texture rt;

	int length = (int)strlen(text);
	for (int i = 0; i < length; i++) {
		if (text[i] == '\n' || ((position.x - origin_x + font->glyphs[(int)text[i]].advance) >= wrap && wrap > 0)) {
			position.x = origin_x;
			position.y += font->line_skip;
		}

		if (font->glyphs[(int)text[i]].available) {
			rt.texture = font->glyphs[(int)text[i]].texture;
			rt.position = position;
			rt.colour = colour;
			
			bool old_centered = centered;
			centered = false;
			render_texture(&rt);
			centered = old_centered;

			position.x += font->glyphs[(int)text[i]].advance;
		}
	}
}

void render_string_format(Vec2 position, Vec4 colour, Font *font, float wrap, const char *text, ...) {
	//@todo: make this better?
	constexpr int max_string_length = 1024;

	va_list argptr;
	char message[max_string_length];

	assert(strlen(text) < max_string_length);

	va_start(argptr, text);
	vsnprintf_s(message, 1024, text, argptr);
	va_end(argptr);

	render_string(position, message, colour, font, wrap);
}

// if you don't want to think about colour/font/wrap the default values will be same as those in render_string
void render_string_format_lazy(Vec2 position, const char *text, ...) {
	//@todo: make this better?
	constexpr int max_string_length = 1024;

	va_list argptr;
	char message[max_string_length];

	assert(strlen(text) < max_string_length);

	va_start(argptr, text);
	vsnprintf_s(message, 1024, text, argptr);
	va_end(argptr);

	render_string(position, message);
}

// renders string at the top of the screen underneath last debug string
// usually used to print out debug info that changes every frame.
// won't draw if var renderer_draw_debug is false
void render_debug_string(const char *text, ...) {
	//@todo: make this better?
	constexpr int max_string_length = 1024;

	va_list argptr;
	char message[max_string_length];

	assert(strlen(text) < max_string_length);

	if (!debug_draw) {
		return;
	}

	va_start(argptr, text);
	vsnprintf_s(message, 1024, text, argptr);
	va_end(argptr);

	render_string(debug_string_position, message);
	debug_string_position.y += (float)default_font->line_skip;
}

void render_line(Vec2 a, Vec2 b, Vec4 colour) {
	glColor4f(v4parms(colour));
	glBegin(GL_LINES);
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, b.y);
	glEnd();
}

void render_line(float ax, float ay, float bx, float by, Vec4 colour) {
	render_line(Vec2(ax, ay), Vec2(bx, by), colour);
}

// render a line projected length units from a position 
void render_line2(Vec2 start, float length, float angle, Vec4 colour) {
	Vec2 end;
	end.x = start.x + (length * cos(angle));
	end.y = start.y + (length * -sin(angle)); // using -sin because y is up in our coord system
	render_line(start, end, colour);
}

void render_line2(float x, float y, float length, float angle, Vec4 colour) {
	render_line2(Vec2(x, y), length, angle, colour);
}

void render_box(Vec2 position, Vec2 size, bool fill, Vec4 colour) {
	float hw = size.x / 2;
	float hh = size.y / 2;
	
	if(centered) {
		position.x -= hw;
		position.y -= hh;
		size.x += hw;
		size.y += hh;
	}

	if (fill) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glColor4f(v4parms(colour));
	glBegin(GL_QUADS);
	glVertex2f(position.x, position.y);
	glVertex2f(position.x, position.y + size.y);
	glVertex2f(position.x + size.x, position.y + size.y);
	glVertex2f(position.x + size.x, position.y);
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void render_box(float x, float y, float w, float h, bool fill, Vec4 colour) {
	render_box(Vec2(x, y), Vec2(w, h), fill, colour);
}

void render_box2(float top, float left, float bottom, float right, bool fill, Vec4 colour) {
	if (fill) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}	

	glColor4f(v4parms(colour));
	glBegin(GL_QUADS);
	glVertex2f(left, top);
	glVertex2f(left, bottom);
	glVertex2f(right, bottom);
	glVertex2f(right, top);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void render_point(Vec2 position, float size, Vec4 colour) {
	glColor4f(v4parms(colour));
	glPointSize(size);
	glBegin(GL_POINTS);
	glVertex2f(position.x, position.y);
	glEnd();
}

void render_point(float x, float y, float size, Vec4 colour) {
	render_point(Vec2(x, y), size, colour);
}

void opengl_debug_output_to_file(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, const char* message, bool should_print) {
       FILE* f = nullptr;
       fopen_s(&f, "data/opengl.log", "a");
       if(f) {
			char debSource[16] = {};
			char debType[20] = {};
			char debSev[5] = {};

			switch(source) {
			case GL_DEBUG_SOURCE_API_ARB:
				strcpy_s(debSource, "OpenGL");
				break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
				strcpy_s(debSource, "Shader Compiler");
				break;
			case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
			    strcpy_s(debSource, "Third Party");
				break;
			case GL_DEBUG_SOURCE_APPLICATION_ARB:
			    strcpy_s(debSource, "Application");
				break;
			case GL_DEBUG_SOURCE_OTHER_ARB:
			    strcpy_s(debSource, "Other");
				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
				strcpy_s(debSource, "Windows");
				break;
			}
			
			switch(type) {
			case GL_DEBUG_TYPE_ERROR_ARB:
			    strcpy_s(debType, "Error");
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
			    strcpy_s(debType, "Deprecated behavior");
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
			    strcpy_s(debType, "Undefined behavior");
				break;
			case GL_DEBUG_TYPE_PORTABILITY_ARB:
			    strcpy_s(debType, "Portability");
				break;
			case GL_DEBUG_TYPE_PERFORMANCE_ARB:
			    strcpy_s(debType, "Performance");
				break;
			case GL_DEBUG_TYPE_OTHER_ARB:
			    strcpy_s(debType, "Other");
				break;
			}
			   
			switch(severity) {
			case GL_DEBUG_SEVERITY_HIGH_ARB:
			    strcpy_s(debSev, "High");
				break;
			case GL_DEBUG_SEVERITY_MEDIUM_ARB:
			    strcpy_s(debSev, "Medium");
				break;
			case GL_DEBUG_SEVERITY_LOW_ARB:
			    strcpy_s(debSev, "Low");
				break;
			}
			
			if(should_print) {
				printf("%d: Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", sys.frame_num, debSource, debType, id, debSev, message);
			}
			fprintf(f, "%d: Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", sys.frame_num, debSource, debType, id, debSev, message);
			fclose(f);
       }
}

void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if(id == 131185) {
		/* Message:Buffer detailed info: Buffer object 1 (bound to GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (0), GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (1), GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (2), and GL_ARRAY_BUFFER_ARB, usage hint is GL_STREAM_DRAW) will use VIDEO memory as the source for buffer object operations.*/
		return;
	}

    opengl_debug_output_to_file(source, type, id, severity, message, severity == GL_DEBUG_SEVERITY_MEDIUM_ARB || severity == GL_DEBUG_SEVERITY_HIGH_ARB);
}