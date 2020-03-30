#ifndef __TEXTURE_H__
#define __TEXTURE_H__

struct Texture {
	dstr filename;
	bool used = false;
	bool never_unload = false;
	unsigned int api_object = 0;
	int width = 0;
	int height = 0;
};

struct Texture_Manager {
	Array<Texture *> textures;

	void init();
	void shutdown();
	void begin_level_load();
	void end_level_load();
	Texture *load(const char *filename);
	Texture *create_from_surface(const char *name, SDL_Surface *surface);
	Texture *create(const char *name, const unsigned char *data, int width, int height);
};

extern Texture_Manager tex;

#endif