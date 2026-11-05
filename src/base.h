/* base.h */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#define uint unsigned int

#ifdef __SWITCH__
#  include <switch.h>
#endif

// SDL2 renderer. Needs to be declared here because several different units access to it
// directly to draw on it because there's no discrete graphics unit, but it works, so no complains :D
extern SDL_Renderer *renderer;

// Gamepad handling, needed here to support hotplugging / hotswapping
extern void gamepad_init (void);
extern void gamepad_remove (SDL_JoystickID joy_id);

#define SCREEN_W 256
#define SCREEN_H 192

#ifdef __amigaos__
/* The data sits beside the program; "PROGDIR:/x" would be its parent. A full
 * screen is a screen of the window's own size, not the Workbench's. */
#  undef DATADIR
#  define DATADIR "PROGDIR:"
#  define DATAPATH(p) "PROGDIR:" p
#  define FULLSCREEN_MODE SDL_WINDOW_FULLSCREEN
#else
#  define DATAPATH(p) DATADIR "/" p
#  define FULLSCREEN_MODE SDL_WINDOW_FULLSCREEN_DESKTOP
#endif
