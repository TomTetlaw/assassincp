#include "precompiled.h"

Texture_Manager tex;

void delete_api_object_for_texture(Texture *texture) {
	if (texture->api_object) {
		glDeleteTextures(1, &texture->api_object);
		texture->api_object = 0;
	}
}

void Texture_Manager::init() {
}

void Texture_Manager::shutdown() {
	For(textures, {
		delete_api_object_for_texture(it);
		delete it;
	});
}

void Texture_Manager::begin_level_load() {
	For(textures, {
		if (!it->never_unload) {
			it->used = false;
		}
	});
}

void Texture_Manager::end_level_load() {
	For(textures, {
		if (!it->used && !it->never_unload) {
			delete_api_object_for_texture(it);
		}
	});
}

bool load_texture_data(Texture *texture) {
	SDL_Surface *surf = IMG_Load(texture->filename);
	if (!surf) {
		return false;
	}

	if (surf->format->format != SDL_PIXELFORMAT_RGBA8888) {
		SDL_PixelFormat format = { 0 };
		format.BitsPerPixel = 32;
		format.BytesPerPixel = 4;
		format.format = SDL_PIXELFORMAT_RGBA8888;
		format.Rshift = 0;
		format.Gshift = 8;
		format.Bshift = 16;
		format.Ashift = 24;
		format.Rmask = 0xff << format.Rshift;
		format.Gmask = 0xff << format.Gshift;
		format.Bmask = 0xff << format.Bshift;
		format.Amask = 0xff << format.Ashift;

		SDL_Surface *newSurf = SDL_ConvertSurface(surf, &format, 0);
		SDL_FreeSurface(surf);
		surf = newSurf;
	}

	unsigned int t;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
	glObjectLabel(GL_TEXTURE, t, -1, texture->filename);

	texture->width = surf->w;
	texture->height = surf->h;
	texture->api_object = t;
	SDL_FreeSurface(surf);

	return true;
}

void create_texture_data(Texture *texture, const unsigned char *data, int width, int height) {
	unsigned int t;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)data);
	glObjectLabel(GL_TEXTURE, t, -1, texture->filename);

	texture->width = width;
	texture->height = height;
	texture->api_object = t;
}

void create_texture_data_from_surface(Texture *texture, SDL_Surface *surf) {
	if (!surf) {
		return;
	}

	if (surf->format->format != SDL_PIXELFORMAT_RGBA8888) {
		SDL_PixelFormat format = { 0 };
		format.BitsPerPixel = 32;
		format.BytesPerPixel = 4;
		format.format = SDL_PIXELFORMAT_RGBA8888;
		format.Rshift = 0;
		format.Gshift = 8;
		format.Bshift = 16;
		format.Ashift = 24;
		format.Rmask = 0xff << format.Rshift;
		format.Gmask = 0xff << format.Gshift;
		format.Bmask = 0xff << format.Bshift;
		format.Amask = 0xff << format.Ashift;

		SDL_Surface *newSurf = SDL_ConvertSurface(surf, &format, 0);
		SDL_FreeSurface(surf);
		surf = newSurf;
	}

	unsigned int t;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
	glObjectLabel(GL_TEXTURE, t, -1, texture->filename);

	SDL_FreeSurface(surf);

	texture->width = surf->w;
	texture->height = surf->h;
	if (texture->width <= 0 || texture->height <= 0) {
		console.printf("Texture with zero width but valid SDL surface! [%s: (%d, %d) (%d, %d)]\n", texture->filename, texture->width, texture->height, surf->clip_rect.w, surf->clip_rect.h);
		texture->width = surf->clip_rect.w;
		texture->height = surf->clip_rect.h;
	}
	texture->api_object = t;
}

void texture_hotload_callback(const char *filename, void *data) {
	Texture *texture = (Texture *)data;
	delete_api_object_for_texture(texture);
	load_texture_data(texture);
}

Texture *Texture_Manager::load(const char *filename) {
	if (!filename || filename[0] == 0) {
		return nullptr;
	}

	For(textures, {
		if (!strcmp(it->filename, filename)) {
			it->used = true;
			if (!it->api_object) {
				load_texture_data(it);
			}
			return it;
		}
	});

	Texture *texture = new Texture;
	strcpy(texture->filename, filename);
	if (load_texture_data(texture)) {
		texture->used = true;
		tex.textures.append(texture);

		hotload.add_file(filename, texture, texture_hotload_callback);

		return texture;
	}
	else {
		console.printf("Failed to load texture %s: %s\n", filename, IMG_GetError());
		delete texture;
		return nullptr;
	}
}

Texture *Texture_Manager::create(const char *name, const unsigned char *data, int width, int height) {
	Texture *texture = new Texture;
	strcpy(texture->filename, name);
	texture->used = true;
	texture->never_unload = true;
	textures.append(texture);

	create_texture_data(texture, data, width, height);

	return texture;
}

Texture *Texture_Manager::create_from_surface(const char *name, SDL_Surface *surface) {
	Texture *texture = new Texture;
	strcpy(texture->filename, name);
	texture->used = true;
	texture->never_unload = true;
	textures.append(texture);

	create_texture_data_from_surface(texture, surface);

	return texture;
}