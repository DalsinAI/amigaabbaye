/* A Shell gives 4 KB of stack; game() alone keeps the map and the enemy
 * tables (about 80 KB) in locals. libnix's startup sets this much up.
 * The AmiCradle Team. GPL 3, like Abbaye des Morts. */
unsigned long __stack = 256 * 1024;
static const char version[] __attribute__((used)) = "$VER: Abbaye 2.0.5 Amiga (05.11.2026)";
