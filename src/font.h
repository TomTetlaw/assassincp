#ifndef __FONT_H__
#define __FONT_H__

struct Glyph {
	bool available = false;
	char ch = '\0';
	int min_x = 0;
	int max_x = 0;
	int min_y = 0;
	int max_y = 0;
	int advance = 0;
	Texture *texture = nullptr;
	int name_length = 32;
	char name[32];
};

struct Font {
	int point_size = 0;
	int height = 0;
	int ascent = 0;
	int descent = 0;
	int line_skip = 0;
	Glyph glyphs[256]; //@todo: what size should this be?
	const char *filename = nullptr;
};

void font_shutdown();
Font *load_font(const char *filename, int point_size);
int font_get_string_length_in_pixels(Font *font, const char *string);

#endif