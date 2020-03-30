#ifndef __PRECOMPILED_H__
#define __PRECOMPILED_H__

#define _CRT_SECURE_NO_WARNINGS

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#define SDL_MAIN_HANDLED 1
#include "../include/sdl2/SDL.h"
#include "../include/sdl2/SDL_syswm.h"
#include "../include/sdl2/SDL_image.h"
#include "../include/sdl2/SDL_ttf.h"
#pragma comment(lib, "../../lib/sdl2/SDL2.lib")
#pragma comment(lib, "../../lib/sdl2/SDL2_image.lib")
#pragma comment(lib, "../../lib/sdl2/SDL2_ttf.lib")

#include "../include/glew/glew.h"
#pragma comment(lib, "../../lib/glew/glew32.lib")
#pragma comment(lib, "opengl32.lib")

#include "../include/chipmunk/chipmunk.h"
#pragma comment(lib, "../../lib/chipmunk/chipmunk.lib")

#ifdef assert
#undef assert
#endif

#define assert(x) { if(!(x)) *(int *)0 = 0; }

#ifdef break_point
#undef break_point
#endif

#define break_point(x) { *(int *)0 = 0; }

#include "common.h"
#include "mathlib.h"
#include "array.h"
#include "input.h"
#include "hotload.h"
#include "system.h"
#include "config.h"
#include "texture.h"
#include "font.h"
#include "console.h"
#include "render.h"
#include "entity.h"
#include "game.h"
#include "ui_render.h"
#include "editor.h"

#include "fov.h"

#endif