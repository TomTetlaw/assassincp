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
#include <filesystem>

#define SDL_MAIN_HANDLED 1
#include "../include/sdl2/SDL.h"
#include "../include/sdl2/SDL_syswm.h"
#include "../include/sdl2/SDL_image.h"
#include "../include/sdl2/SDL_ttf.h"
#include "../include/sdl2/SDL_mixer.h"
#include "../include/glew/glew.h"

#ifdef assert
#undef assert
#endif

#define assert(x) { if(!(x)) *(int *)0 = 0; }

#ifdef break_point
#undef break_point
#endif

#define break_point(x) { *(int *)0 = 0; }

#define internal static

typedef unsigned int uint;
typedef unsigned char u8;

#include "mathlib.h"
#include "common.h"
#include "array.h"
#include "console.h"
#include "input.h"
#include "hotload.h"
#include "system.h"
#include "config.h"
#include "texture.h"
#include "font.h"
#include "render.h"
#include "physics.h"
#include "fov.h"
#include "entity.h"
#include "game.h"

#endif