#ifndef __TEXTURE_H__
#define __TEXTURE_H__

struct Texture {
	char filename[256] = { 0 };
	bool used = false;
	bool never_unload = false;
	unsigned int api_object = 0;
	int width = 0;
	int height = 0;
};

void texture_shutdown();
void texture_begin_level_load();
void texture_end_level_load();
// if never_unload is true, the file won't be unloaded after the level
// finishes loading even if it wasn't used. Currently used for font glyphs
// and ui images
int load_texture(const char *filename, bool never_unload = false);

// create texture from data you already have, for both of these, never_unload = true.

// frees the surface after it creates the texture
int create_texture_from_surface(const char *name, SDL_Surface *surface);

int create_texture(const char *name, const unsigned char *data, int width, int height);

Texture *get_texture(int index);

#endif