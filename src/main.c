/* Abbaye des Morts */
/* Version 2.0 */

/* (c) 2010 - Locomalito & Gryzor87 */
/* 2013-2023 - David "Nevat" Lara */
/* 2015-2023 - Carsten Teibes */

/* GPL v3 license */

 
#include "main.h"
#include "startscreen.h"
#include "history.h"
#include "game.h"
#include "gameover.h"
#include "ending.h"

SDL_Renderer *renderer;

#ifdef __amigaos__
/* OpenGPU's SDL2.module reports a 24-bit window and may not keep render
 * targets: the game draws with SDL's software renderer into its own 32-bit
 * XRGB picture (frame), which renderpresent() hands to the window's renderer
 * as one streaming texture of the same format, scaled there. */
static SDL_Surface *frame;
static SDL_Renderer *winrenderer;
static SDL_Texture *frametex;
void amiga_screens_snapshot(void);
void amiga_screens_close_leftovers(void);
void amiga_message(const char *title, const char *text);
static void amiga_quit(void) {
	/* SDL first (an early return skips it), then the empty "SDL" screens
	 * SDL2.module 0.6 leaves after a full screen. */
	SDL_Quit();
	amiga_screens_close_leftovers();
}
#endif

static SDL_GameController *gamepad = NULL;

static void gamepad_shutdown (void) {
	if (gamepad) {
		SDL_GameControllerClose(gamepad);
		gamepad = NULL;
	}
}

void gamepad_init (void) {
	const int num_joysticks = SDL_NumJoysticks();
	if (gamepad || num_joysticks == 0) return;

	for (int i=0; i < num_joysticks; i++) {
		if (!SDL_IsGameController(i))
			continue;
		gamepad = SDL_GameControllerOpen(i);
		if (gamepad)
			break;
	}
}

void gamepad_remove (SDL_JoystickID joy_id) {
	if (gamepad && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad)) == joy_id) {
		gamepad_shutdown();
		gamepad_init();
	}
}

int main (int argc, char *argv[]) {
	static const char *filter_modes[] = {"nearest", "linear"};

	uint8_t exit = 0;
	uint8_t state = 0; /* 0-intro,1-history,2-game,3-gameover,4-ending,5-exit */
	uint8_t grapset = 0; /* 0-8bits, 1-16bits */
	uint8_t fullscreen = 0; /* 0-Windowed,1-Fullscreen */
#ifdef __amigaos__
	uint8_t filtering = 0; /* scaling is cheaper and sharper without */
	int scale = 2;
#else
	uint8_t filtering = 1;
	int scale = 3;
#endif
	uint8_t sound = 1;

	for (int i = 1; i < argc; i++) {
		if ( !strcmp("-nofilter", argv[i]) )
			filtering = 0;
		else if ( !strcmp("-filter", argv[i]) )
			filtering = 1;
		else if ( !strcmp("-nosound", argv[i]) )
			sound = 0;
		else if ( !strcmp("-scale", argv[i]) && i + 1 < argc ) {
			scale = atoi(argv[++i]);
			if (scale < 1) scale = 1;
			if (scale > 4) scale = 4;
		}
		else if ( !strcmp("-fullscreen", argv[i]) )
			fullscreen = 1;
		else if ( !strcmp("-graphset", argv[i]) )
			grapset = 1;
	}

	/* SDL2 initialization */
#ifdef __amigaos__
	amiga_screens_snapshot();
	atexit(amiga_quit);
#endif
	SDL_Init(SDL_INIT_VIDEO | (sound ? SDL_INIT_AUDIO : 0));

#ifdef __SWITCH__
	romfsInit();
#endif

	/* Missing data files, exit with error */
	if(!check_data()) return 1;

	/* Creating window */
	SDL_Window *screen = SDL_CreateWindow(GAME_TITLE,
		SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_W * scale,SCREEN_H * scale,
		fullscreen ? FULLSCREEN_MODE : SDL_WINDOW_RESIZABLE);
	if (!screen) {
		printf("ERROR: no window: %s\n", SDL_GetError());
		return 1;
	}

#ifdef __amigaos__
	winrenderer = SDL_CreateRenderer(screen, -1, 0);
	frame = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_W, SCREEN_H, 32, SDL_PIXELFORMAT_RGB888);
	renderer = frame ? SDL_CreateSoftwareRenderer(frame) : NULL;
	if (winrenderer) {
		SDL_RenderSetLogicalSize(winrenderer, SCREEN_W, SCREEN_H);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filter_modes[filtering]);
		frametex = SDL_CreateTexture(winrenderer, SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
		SDL_SetRenderDrawColor(winrenderer,0,0,0,255);
	}
	if (!renderer || !frametex) {
		printf("ERROR: no renderer: %s\n", SDL_GetError());
		return 1;
	}
#else

	/* Create renderer (with VSync, nice !) */
	renderer = SDL_CreateRenderer(screen, -1,
		SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	SDL_RenderSetLogicalSize(renderer, SCREEN_W, SCREEN_H);

	/* Create a render target for smooth scaling */
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filter_modes[filtering]);
	SDL_Texture *target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET, SCREEN_W, SCREEN_H);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_SetRenderTarget(renderer, target);
#endif

	SDL_SetRenderDrawColor(renderer,0,0,0,255);
	SDL_RenderClear(renderer);

	/* Init gamepad subsystem */
#ifdef SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS
	// Use button positions instead of labels, like SDL3
	SDL_SetHint( SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0" );
#endif

	if (SDL_Init(SDL_INIT_GAMECONTROLLER) >= 0) {
		SDL_GameControllerAddMappingsFromFile(DATAPATH("gamecontrollerdb.txt"));
		gamepad_init();
	}

	/* Hide mouse cursor */
	SDL_ShowCursor(SDL_DISABLE);

	/* Init audio */
#ifdef __amigaos__
	/* Half the PC's rate: less mixing for the 68k. */
	if (sound && Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,2048) < 0)
#else
	if (sound && Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,4096) < 0)
#endif
		printf("No sound: %s\n", Mix_GetError());
	Mix_AllocateChannels(5);

	while (exit != 1) {
		switch (state) {
			case 0:
				update_title(screen, NULL);
				startscreen(screen,&state,&grapset,&fullscreen);
				break;
			case 1:
				update_title(screen, "History");
				history(screen,&state,&grapset,&fullscreen);
				break;
			case 2:
				game(screen,&state,&grapset,&fullscreen);
				break;
			case 3:
				update_title(screen, "GAME OVER");
				gameover(screen,&state);
				break;
			case 4:
				update_title(screen, "THE END");
				ending(screen,&state);
				break;
			case 6:
				exit = 1;
				break;
		}
	}

	/* Cleaning */
	gamepad_shutdown();
	if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) == SDL_INIT_GAMECONTROLLER)
		SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
#ifdef __amigaos__
	Mix_CloseAudio();
	SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(frame);
	SDL_DestroyTexture(frametex);
	SDL_DestroyRenderer(winrenderer);
#else
	SDL_DestroyTexture(target);
	SDL_DestroyRenderer(renderer);
#endif
	SDL_DestroyWindow(screen);
	SDL_Quit();	

#ifdef __SWITCH__
	romfsExit();
#endif

	/* Exiting normally */
	return 0;

}

int check_data() {
	const int NUM_CHECKS = 4;

	/* Check every kind of data */
	const char* files[] = {
		DATAPATH("graphics/tiles.png"),
		DATAPATH("sounds/MainTitleN.ogg"),
		DATAPATH("data/map.txt"),
		DATAPATH("data/enemies.txt")
	};

	/* Try to open each file */
	int missing = NUM_CHECKS;
	SDL_RWops *rw;
	for(int i = 0; i < NUM_CHECKS; i++) {
		rw = SDL_RWFromFile(files[i], "rb");
		if (rw) {
			SDL_RWclose(rw);
			missing--;
		}
	}

	/* Everything found */
	if (!missing) return 1;

	const char* errormsg = "Cannot find data!\nFiles are expected under \"" DATADIR
		"\",\nbut nothing was found.\n\nPlease check your installation.";

#ifdef __amigaos__
	/* An Intuition requester and the Shell: no SDL window yet. */
	printf("ERROR: %s\n", errormsg);
	amiga_message(GAME_TITLE, errormsg);
	if (0) {
#else
	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, GAME_TITLE, errormsg, NULL)) {
#endif
		/* Fallback error message to terminal */
		printf("ERROR: %s\n", errormsg);
	}

	return 0;
}

void update_title(SDL_Window *screen, const char* title) {
	static char buf[64];
	strcpy(buf, GAME_TITLE);
	if (title) {
		strcat(buf, " - ");
		strcat(buf, title);
	}
	SDL_SetWindowTitle(screen, buf);
}

void renderpresent(SDL_Renderer *renderer) {
#ifdef __amigaos__
	/* The game's speed came from a vsynced present at 60 Hz: here a frame
	 * waits for its 1/60 s whatever the present does. */
	static Uint32 start, frames;
	Uint32 now, due;

	(void)renderer;
	SDL_UpdateTexture(frametex, NULL, frame->pixels, frame->pitch);
	SDL_RenderClear(winrenderer);
	SDL_RenderCopy(winrenderer, frametex, NULL, NULL);
	SDL_RenderPresent(winrenderer);

	now = SDL_GetTicks();
	frames++;
	due = start + frames * 50 / 3; /* 1000/60 ms a frame */
	if (!start || (Sint32)(now - due) > 100) {
		/* first frame, or far behind (a pause, a slow machine): start over */
		start = now;
		frames = 0;
	} else if ((Sint32)(due - now) > 0)
		SDL_Delay(due - now);
#else
	SDL_Texture *target = SDL_GetRenderTarget(renderer);
	SDL_SetRenderTarget(renderer, NULL);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, target, NULL, NULL);
	SDL_RenderPresent(renderer);

	SDL_SetRenderTarget(renderer, target);
#endif
}
