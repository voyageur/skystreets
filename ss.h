#ifdef WIN32
#include <windows.h>
#endif

#define deg2rad(x) (((x)*180)/PI)

#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_LEFT 2
#define KEY_RIGHT 3
#define KEY_JUMP 4
#define KEY_RAISE 5
#define KEY_LOWER 6

#define MENU 1
#define GAME 2
#define EDITOR 3

#define SURF_TILE1	0x1
#define SURF_TILE2	0x2
#define SURF_TILE3	0x4
#define SURF_SUPPLY	0x8
#define SURF_EXPLODE	0x10
#define SURF_ACCEL	0x20
#define SURF_STICKY	0x40
#define SURF_SLIPPERY	0x80
#define SURF_GOAL	0x100

#define TEXTURES 15

#define FORWARD 0
#define LEFT 1
#define RIGHT 2
#define UP 3
#define FLOOR 5

extern char basePath[];

//for holding bullets
typedef struct bullet {
      public:
	float x;
	float spdx;
	float y;
	float spdy;
	float z;
	float spdz;
	float angle;
	int bounces;
	float exploding;
	bullet *next;
	bullet *previous;
};

//just a vector
typedef struct vector {
	float x;
	float y;
	float z;
} vector;

//actors - character, monsters, other shit.
typedef struct actor {
	float x;
	float y;
	float z;
	float spdx, spdy, spdz;
	float angle;
	float g_fuel;		//health remaining
	float g_oxygen;		//standard skyroads stuff
	int animframe;		//frame for animation
	int exploding;
	int flags;
	bool onground;
	float offGroundTime;
	actor *next;
};
