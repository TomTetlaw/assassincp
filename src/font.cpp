#include "precompiled.h"

Array<Font *> fonts;

void font_shutdown() {
	for (int i = 0; i < fonts.num; i++) {
		Font *font = fonts[i];
		delete font;
	}
}

internal void load_data_into_font(Font *font_info, TTF_Font *font) {
	font_info->height = TTF_FontHeight(font);
	font_info->ascent = TTF_FontAscent(font);
	font_info->descent = TTF_FontDescent(font);
	font_info->line_skip = TTF_FontLineSkip(font);

	for (int i = 32; i < 127; i++) {
		int index = TTF_GlyphIsProvided(font, (char)i);
		if (index > 0) {
			font_info->glyphs[i].available = true;
			TTF_GlyphMetrics(font, (char)i, &font_info->glyphs[i].min_x, &font_info->glyphs[i].max_x, &font_info->glyphs[i].min_y, &font_info->glyphs[i].max_y, &font_info->glyphs[i].advance);
			font_info->glyphs[i].ch = (char)i;
			font_info->advance = font_info->glyphs[i].advance;
			
			memset(font_info->glyphs[i].name, 0, font_info->glyphs[i].name_length);
			sprintf_s(font_info->glyphs[i].name, "%s_%c_%d", font_info->filename, (char)i, font_info->point_size);

			SDL_Surface *surface = TTF_RenderGlyph_Blended(font, (char)i, {255, 255, 255, 255});
			font_info->glyphs[i].texture = create_texture_from_surface(font_info->glyphs[i].name, surface);
		}
	}
}

internal void font_hotload_callback(const char *filename, void *data) {
	for (int i = 0; i < fonts.num; i++) {
		Font *font_info = fonts[i];
		if (!strcmp(filename, font_info->filename)) {
			TTF_Font *font = TTF_OpenFont(filename, font_info->point_size);
			if (!font) {
				console_printf("Failed to hotload font %s: %s\n", filename, TTF_GetError());
				break;
			}
			else {
				load_data_into_font(font_info, font);
				TTF_CloseFont(font);
				break;
			}
		}
	}
}

Font *load_font(const char *filename, int point_size) {
	Font *font_info = nullptr;

	for (int i = 0; i < fonts.num; i++) {
		font_info = fonts[i];
		if (!strcmp(font_info->filename, filename) && font_info->point_size == point_size) {
			return font_info;
		}
	}

	TTF_Font *font = TTF_OpenFont(filename, point_size);
	if (!font) {
		console_printf("Failed to open font %s: %s\n", filename, TTF_GetError());
		return nullptr;
	}

	font_info = new Font;
	font_info->filename = filename;
	font_info->point_size = point_size;
	load_data_into_font(font_info, font);
	fonts.append(font_info);

	hotload_add_file(filename, nullptr, font_hotload_callback);

	TTF_CloseFont(font);

	return font_info;
}

int font_get_string_length_in_pixels(Font *font, const char *string) {
	int length = (int)strlen(string);
	int pixel_length = 0;

	for (int i = 0; i < length; i++) {
		Glyph *glyph = &font->glyphs[string[i]];
		pixel_length += glyph->advance;
	}

	return pixel_length;
}