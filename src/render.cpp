#include "precompiled.h"

Render renderer;

void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

void Render::init() {
	glewInit();

	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback((GLDEBUGPROC)opengl_debug_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	config.register_var("renderer_clear_colour", &clear_colour);
	config.register_var("renderer_should_clear", &should_clear);
	config.register_var("renderer_draw_debug", &debug_draw);

	FILE *f = nullptr;
	fopen_s(&f, "data/opengl.log", "w");
	if(f) {
		fclose(f);
	}

	default_font = font_manager.load("data/fonts/consolas.ttf", 16);
	debug_string_position = Vec2(-sys.window_size.x, -sys.window_size.y);
}

void Render::render_physics_debug() {
	if (!debug_draw) {
		return;
	}
}

void Render::shutdown() {
}

void Render::on_level_load() {
	camera_position = Vec2(0, 0);
	zoom_level = 0;
}

void Render::begin_frame() {
	if(should_clear) {
		glClearColor(clear_colour.x, clear_colour.y, clear_colour.z, clear_colour.w);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float hw = sys.window_size.x / 2;
	float hh = sys.window_size.y / 2;
	glOrtho(-hw, hw, hh, -hh, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	debug_string_position = Vec2(-(sys.window_size.x / 2) + 10, -(sys.window_size.y / 2) + (float)default_font->line_skip);
}

void Render::end_frame() {
	SDL_GL_SwapWindow(sys.window);
}

void Render::update(float dt) {
}

float Render::scale_for_zoom_level(int level) {
	float scale = 0.0f;
	if (level == 0) {
		scale = 1.0f;
	}
	else if (level < 0) {
		scale = 1.0f / (((level * 0.125f)*-1) + 1);
	}
	else if (level > 0) {
		scale = (level * 0.25f) + 1;
	}
	return scale;
}

float Render::inverse_scale_for_zoom_level(int level) {
	float scale = 0.0f;
	if (level == 0) {
		scale = 1.0f;
	}
	else if (level < 0) {
		scale = ((level * 0.125f) * -1) + 1;
	}
	else if (level > 0) {
		scale = 1.0f / ((level * 0.25f) + 1);
	}
	return scale;
}

void Render::setup_render() {
	if (use_camera) {
		glTranslatef(-camera_position.x, -camera_position.y, 0);
	}

	if (use_zoom) {
		float scale = scale_for_zoom_level(zoom_level);
		glScalef(scale, scale, 1);
	}
}

void Render::texture(Render_Texture *rt) {
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

	glColor4f(V4PARMS(rt->colour));

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, rt->texture->api_object);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	setup_render();

	glTranslatef(x, y, 0);
	glRotatef(rad2deg(rt->angle), 0, 0, 1);
	glTranslatef(-x, -y, 0);

	glScalef(rt->scale.x, rt->scale.y, 1);

	float repeat_count_x = w / rt->texture->width;
	float repeat_count_y = h / rt->texture->height;
	if (!rt->repeat) {
		repeat_count_x = 1;
		repeat_count_y = 1;
	}

	glBegin(GL_QUADS);
	if (rt->centered) {
		glTexCoord2f(sl, tl);
		glVertex2f(x - hw, y - hh);
		glTexCoord2f(sl, th * repeat_count_y);
		glVertex2f(x - hw, y + hh);
		glTexCoord2f(sh * repeat_count_x, th * repeat_count_y);
		glVertex2f(x + hw, y + hh);
		glTexCoord2f(sh * repeat_count_x, tl);
		glVertex2f(x + hw, y - hh);
	}
	else {
		glTexCoord2f(sl, tl);
		glVertex2f(x, y);
		glTexCoord2f(sl, th * repeat_count_y);
		glVertex2f(x, y + h);
		glTexCoord2f(sh * repeat_count_x, th * repeat_count_y);
		glVertex2f(x + w, y + h);
		glTexCoord2f(sh * repeat_count_x, tl);
		glVertex2f(x + w, y);
	}
	glEnd();

	glScalef(1, 1, 1);

	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

void Render::debug_string(const char *text, ...) {
	va_list argptr;
	char message[1024]; //@fixme unsafe

	if (!debug_draw) {
		return;
	}

	va_start(argptr, text);
	vsnprintf_s(message, 1024, text, argptr);
	va_end(argptr);

	string(nullptr, debug_string_position, Vec4(1,1,1,1), -1, message);
	debug_string_position.y += (float)default_font->line_skip;
}

void Render::string_format(Font *font, Vec2 position, Vec4 colour, float wrap, const char *text, ...) {
	va_list argptr;
	char message[1024]; //@fixme unsafe

	va_start(argptr, text);
	vsnprintf_s(message, 1024, text, argptr);
	va_end(argptr);

	string(font, position, colour, wrap, message);
}

void Render::string(Font *font, Vec2 position, Vec4 colour, float wrap, const char *text) {
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
			texture(&rt);

			position.x += font->glyphs[(int)text[i]].advance;
		}
	}
}

void Render::line(Vec2 a, Vec2 b, Vec4 colour) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	setup_render();

	glColor4f(V4PARMS(colour));
	glBegin(GL_LINES);
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, b.y);
	glEnd();

	glPopMatrix();
}

void Render::line(Vec2 position, float length, float angle, Vec4 colour) {
	Vec2 b;

	b.x = position.x + (length * cos(angle));
	b.y = position.y + (length * -sin(angle));

	line(position, b, colour);
}

void Render::point(Vec2 position, float size, Vec4 colour) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	setup_render();

	glColor4f(V4PARMS(colour));

	glPointSize(size);
	glBegin(GL_POINTS);
	glVertex2f(position.x, position.y);
	glEnd();
	glPointSize(1);

	glPopMatrix();
}

void Render::triangle(Vec4 colour, Vec2 point1, Vec2 point2, Vec2 point3) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	setup_render();

	glColor4f(V4PARMS(colour));

	glBegin(GL_TRIANGLES);
	glVertex2f(point1.x, point1.y);
	glVertex2f(point2.x, point2.y);
	glVertex2f(point3.x, point3.y);
	glEnd();

	glPopMatrix();
}

void Render::polygon(Vec4 colour, Vec2 *verts, int num_verts) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	setup_render();

	glColor4f(V4PARMS(colour));

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < num_verts; i++) {
		glVertex2f(verts[i].x, verts[i].y);
	}
	glEnd();

	glPopMatrix();
}

void Render::box(bool fill, float x, float y, float width, float height, Vec4 colour) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	setup_render();

	if (fill) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glColor4f(V4PARMS(colour));
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x, y + height);
	glVertex2f(x + width, y + height);
	glVertex2f(x + width, y);
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPopMatrix();
}

void Render::centered_box(bool fill, float x, float y, float width, float height, Vec4 colour) {
	float hw = width / 2;
	float hh = height / 2;
	x = x - hw;
	y = y - hh;
	width += hw;
	height += hh;
	box(fill, x, y, width, height, colour);
}

Vec2 Render::get_string_size(const char *s) {
	return Vec2(font_manager.get_string_length_in_pixels(default_font, s),
		default_font->height);
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
				printf("Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", debSource, debType, id, debSev, message);
			}
			fprintf(f, "Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", debSource, debType, id, debSev, message);
			fclose(f);
       }
}

void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if(id == 131185) {
		return;
	}

    opengl_debug_output_to_file(source, type, id, severity, message, severity == GL_DEBUG_SEVERITY_MEDIUM_ARB || severity == GL_DEBUG_SEVERITY_HIGH_ARB);
}