L'Abbaye des Morts for AmigaOS 3.x
==================================

A port of L'Abbaye des Morts (Locomalito's 2010 platformer, with music by
Gryzor87), the SDL 2 C version by David "Nevat" Lara and Carsten Teibes
(https://github.com/nevat/abbayedesmorts-gpl, master at commit
c971eb008f4eec43100e3474512c5b625fef02a9: release 2.0.5 plus gamepad
support and command line options), for AmigaOS 3.2 on a 68040 or 68060
with an FPU and an RTG screen, on OpenGPU's SDL 2.

Status: builds and links; not yet run on an Amiga.


What you need
-------------

- A 68040 or 68060 with an FPU (not an LC040 or EC040), AmigaOS 3.2, an RTG
  card (or the AmigaChrome lab).
- OpenGPU 0.6 or later: opengpu.library and LIBS:OpenGPU/SDL2.module,
  SDL2_image.module (the PNG pictures) and SDL2_mixer.module (the Ogg
  Vorbis music and effects), installed by OpenUp.
- About 2.5 MB of disk.


How to play
-----------

Double-click Abbaye in its drawer, or from a Shell:

    cd Abbaye
    Abbaye [-scale 1..4] [-fullscreen] [-graphset] [-filter] [-nosound]

You are Jean Raymond, a Cathar monk fleeing the crusaders through the
abbey. Find the twelve crosses.

- Title: Space starts, I shows the instructions, C switches the 8-bit and
  16-bit graphics sets, F window / full screen, Esc quits.
- Game: cursor left and right walk, cursor up jumps, cursor down ducks;
  Space or cursor left or right closes a parchment; C and F as on the
  title; Esc ends the game (back to the Shell).
- A game controller works too (SDL's mapping: A jumps, d-pad moves, Y
  switches graphics, Back quits).

The window is 512x384 (the game's 256x192 drawn twice the size);
-scale 1 is the game's own size, -scale 3 upstream's. -fullscreen opens a
screen of the window's size rather than the Workbench's. -filter smooths
the scaling (upstream's default; off here). -graphset starts with the
16-bit graphics; -nosound runs without SDL's audio.


Building
--------

GFOpenAmigaGCC 16.2.0-open4 or later and the OpenGPU developer kit:

    make -f amiga/Makefile.amigaos -j2 dist \
         STOVE=/path/to/openamigagcc-16.2.0-open4 \
         NDK=/path/to/OpenNDK/Include_H SDL2KIT=/path/to/opengpu-sdk

The defaults are CradleLab's paths. Output: build/amigaos/Abbaye; 'dist'
makes dist/Abbaye/ (Abbaye, graphics/, sounds/, data/, ReadMe,
ReadMe-abbaye, COPYING.txt, Licences.txt, with GlowIcons) and
dist/Abbaye.info. The icons are made from amiga/icons/icons.json (see
"about" there).


What was changed
----------------

- amiga/Makefile.amigaos: upstream's source list; links the kit's
  libSDL2_image.a, libSDL2_mixer.a and libSDL2.a.
- src/base.h: the data is read from PROGDIR: (DATAPATH(); upstream's
  DATADIR "/x" would be "PROGDIR:/x", the parent drawer). A full screen is
  SDL_WINDOW_FULLSCREEN (a screen of the window's size), not _DESKTOP.
- src/loading.c: makepath() builds its paths with DATAPATH().
- src/main.c: upstream draws through the window's renderer into a render
  target texture. Here the game draws with SDL's software renderer into a
  32-bit XRGB surface of its own (OpenGPU's SDL 2 reports a 24-bit window
  and render targets are untried on it); renderpresent() copies it into
  one streaming SDL_PIXELFORMAT_RGB888 texture on the window's renderer,
  which scales it. The game's speed came from a vsynced present: each
  frame now waits for its 1/60 s (SDL_GetTicks). Sound at 22050 Hz rather
  than 44100. The "data missing" message is an Intuition requester
  (amiga/amiga_message.c) and the Shell. -scale, -filter and -nosound are
  new options.
- amiga/amiga_screens.c (from the Seven Kingdoms port): SDL2.module 0.6
  leaves empty "SDL" screens after a full-screen window; an atexit()
  handler calls SDL_Quit() and then closes them, or sends them behind the
  Workbench (an early return skips upstream's SDL_Quit()).
- src/startscreen.c, src/game.c (keybpause): the title and the parchment
  wait polled for keys flat out; they wait 10 ms between polls.
- amiga/stack.c: 256 KB of stack (game() keeps the map and enemy tables,
  about 80 KB, in locals) and a $VER string.

Big-endian: data/map.txt and data/enemies.txt are text read with fgets and
sscanf; the pictures are PNG through SDL2_image and the sounds Ogg through
SDL2_mixer; nothing else is read from a file. No change needed.

SDL2_image.module's RGB PNG trap: every picture in graphics/ is already
RGBA (colour type 6) or a 2-bit palette picture (the parchments), so none
is converted.


Cut
---

Nothing of the game. Upstream's Switch, Wii and other platform files are
not built.


What a tester should check first
--------------------------------

1. It starts in a 512x384 window with the title picture in the right
   colours and the title music playing (Ogg through SDL2_mixer.module;
   watch the Shell for "No sound").
2. Space, then the history scene: Jean and the crusaders run across at the
   PC's speed (the 60 Hz pacing), then the game; walk, jump (cursor up),
   duck, pick up a cross (sound), touch a parchment (it waits for a key).
3. C switches the graphics sets in game; the frame rate in rooms with many
   enemies (all drawing is the 68k's, through SDL's software renderer).
4. F and -fullscreen; then Esc to quit leaves no grey screen in front of
   the Workbench.
5. -nosound and -scale 1 start and quit cleanly too; started from a drawer
   with no data, the requester shows.


Licence
-------

The program is under the GNU GPL, version 3 (COPYING.txt). The graphics
and the sounds and music are under CC BY 3.0 (Locomalito, Gryzor87), as
upstream's ReadMe.md says; see Licences.txt. The port's own files are GPL
3.
