/* skystreets.cpp
 * Main game stuff
 * Licensed under the Open Software License version 2.0
 * Coded by ReKleSS [rekless@fastmail.fm]
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "SDL.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <memory.h>
#include <stdlib.h>
#include "ss.h"
#include "config.h"
#include "levels.h"
#include "explosion.h"
#include "SDL_image.h"
#include "glm.h"
#include "audio.h"
#include "text.h"
#include "game.h"
#include "menu.h"

#define TEXTURES 15


// {{{ Global vars
int state;

extern int lastCycle; //this should probably be renamed
extern Text text;
extern int g_time;

FILE *debug;

int g_frames;
GLuint textures[TEXTURES];
GLuint level; //it's a GL list
bool keys[256]; //probably too many, but oh well... 256 bytes.
extern Level lvlLevel;
extern struct actor player;
int fps = 0;
audioSample explosion; //sounds - should really be in another file...
audioSample engine;
Menu menu;


GLuint *shiplists;
bool quickrestart = false;
bool drawShadow = true;

int fontLists[65];

int shipList;

char basePath[64] = { DATAPATH };

bool fullscreen = true;
int nlvl = 0;
SDL_Surface *screen;
// }}}

Uint32 AddFrames(Uint32 interval, void *param)
{
	fps = g_frames;
	g_frames = 0;

	return interval; //to ensure we get called again - SDL quirk.
}



/* main: Entry point for Skystreets
 */

int system_handleCommandLine(int argc, char *argv[], int *lvlno)
{
	// {{{ ugly command line handling stuff
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {	//single character flags...
			case 'q':
				quickrestart = true;
				break;

			case 'w':	//windowed mode
				fullscreen = false;
				break;
			case 'n':
				drawShadow = false;
				break;
#ifndef WIN32
			case 'a':	//antialiasing!!!
				if (argv[i][2])
					setenv("__GL_FSAA_MODE",
					       &argv[i][2], 0);
				else
					setenv("__GL_FSAA_MODE", "4", 0);
				break;

#endif
			case 's':	//silent
				audioSample::disabled = 1;
				break;
			case 'l':
				*lvlno = i;
				break;
			default:
				printf("Invalid argument: %s\n", argv[i]);
				printf("Arguments are:\n");
				printf
				    ("-q: Quickrestart (no explosions, no fade-in)\n");
				printf("-w: Windowed mode\n");
				printf("-s: Silent mode (no audio)\n");
				printf("-n: Don't draw shadows\n");
#ifndef WIN32
				printf
				    ("-a: Antialiasing (only for nvidia cards using the nvidia driver)\n");
#endif
				return -1;
			}
		} else
			nlvl = i;
	}

// }}}
	return 0;
}

/* InitGL: initialize OpenGL with useful values */

void InitGL()
{
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(100.0f, 800 / 600, 0.1f, 1000.0f);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_DEPTH_TEST);	// Enables Depth Testing
	glDepthFunc(GL_LESS);	// The Type Of Depth Test To Do
	glMatrixMode(GL_MODELVIEW);

}

/* LoadTexture(filename, i): Load a texture <filename> into
 * slot i. It will append the system path onto the filename.
 */

bool LoadTexture(const char *filename, int i)
{
	SDL_Surface *bgrText;
	SDL_Rect area;
	Uint32 saved_flags;
	Uint8 saved_alpha;
	SDL_Surface *image;
	char fullpath[64];
	GLenum retval;

	strcpy(fullpath, basePath);
	strcat(fullpath, filename);


	image = IMG_Load(fullpath);

	if (image == NULL) {
		printf("Here comes a segfault... IMG_Load %i failed: %s\n",
		       i, SDL_GetError());
		return false;
	}

	printf("Loading %s into place %i\n", fullpath, i);

	bgrText =
	    SDL_CreateRGBSurface(SDL_SWSURFACE, image->w, image->h,
				 image->format->BitsPerPixel, 0x000000FF,
				 0x0000FF00, 0x00FF0000, 0xFF000000);
	area.x = 0;
	area.y = 0;
	area.w = image->w;
	area.h = image->h;
	saved_flags = image->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
	saved_alpha = image->format->alpha;
	if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
		SDL_SetAlpha(image, 0, 0);
	SDL_BlitSurface(image, &area, bgrText, &area);
	if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
		SDL_SetAlpha(image, saved_flags, saved_alpha);
	glBindTexture(GL_TEXTURE_2D, textures[i]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image->w, image->h, 0,
		     GL_RGB, GL_UNSIGNED_BYTE, bgrText->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	retval = glGetError();
	if (retval != GL_NO_ERROR) {
		printf
		    ("OpenGL Error received!!! (loading texture %i)... 0x%X \n",
		     i, retval);
	}

	SDL_FreeSurface(image);
	SDL_FreeSurface(bgrText);
	return true;
}

/* LoadTextures(): Loads the textures into their slots
 */
void system_loadTextures()
{

	char files[TEXTURES][32] = { {"gfx/skin.png"}, {"gfx/rtile1.png"},
	{"gfx/rtile2.png"},
	{"gfx/rtile3.png"}, {"gfx/supply.png"}, {"gfx/burning.png"},
	{"gfx/speedup.png"}, {"gfx/sticky.png"}, {"gfx/slippery.png"},
	{"gfx/chequered.png"}, {"gfx/menubg.png"}, {"gfx/boom.png"},
	{"gfx/goal.png"}, {"gfx/menubg.png"}, {"gfx/font.png"}
	};
	printf("Loading textures...\n");
	glEnable(GL_TEXTURE_2D);
	glGenTextures(TEXTURES, &textures[0]);
	printf("%i textures generated\n", TEXTURES);
	int i;
	for (i = 0; i < TEXTURES; i++)
		LoadTexture(files[i], i);
}

int main(int argc, char *argv[])
{

	static long levelTime;
	int lvlno = 0; //level name can't be arg 0 because that's filename
	if (system_handleCommandLine(argc, argv, &lvlno))
		return 0;
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	memset(&player, 0, sizeof(actor));
	SDLKey sym;
	srand(SDL_GetTicks());
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	screen =
	    SDL_SetVideoMode(800, 600, 16,
			     SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL |
			     (fullscreen ? SDL_FULLSCREEN : 0));
	if (fullscreen)
		SDL_ShowCursor(0);
	SDL_WM_SetCaption(PACKAGE_STRING, NULL);
	printf("Starting %s\n", PACKAGE_STRING);

	state = MENU;
	player.exploding = false;

	fps = 0;

	lastCycle = 0;
	levelTime = SDL_GetTicks();

	char fullpath[64];
	strcat(basePath, "/");
#ifndef WIN32
	FILE *file = fopen("gfx/ship.obj", "r");
	if (file == NULL) {	//try the other path
		strcpy(fullpath, basePath);
		strcat(fullpath, "gfx/ship.obj");
		file = fopen(fullpath, "r");
		if (file == NULL) {
			printf("Can't find data files; giving up. Are you sure the data is installed?\n");
			SDL_Quit();
			return 1;
		}
	} else
#endif
		strcpy(basePath, "./");	//current dir

	initParticles();
	InitGL();
	menu.init();
	text.init();

	strcpy(fullpath, basePath);
	strcat(fullpath, "audio/explosion.wav");
	if (explosion.load(fullpath) == -1)
		printf("Couldn't load explosion.wav\n");
	strcpy(fullpath, basePath);
	strcat(fullpath, "audio/engine.wav");
	if (engine.load(fullpath) == -1)
		printf("Couldn't load engine.wav");


	GLMmodel *model, *model2;

	strcpy(fullpath, basePath);
	strcat(fullpath, "gfx/ship.obj");
	model = glmReadOBJ(fullpath);
	strcpy(fullpath, basePath);
	strcat(fullpath, "gfx/ship2.obj");
	model2 = glmReadOBJ(fullpath);

	shipList = glmList(model, GLM_TEXTURE | GLM_SMOOTH); //shipList is used to render the "Shadow"
	shiplists = glmInterpolate(model, model2, 16);
	
	glmDelete(model);
	glmDelete(model2);

	system_loadTextures();

	if (nlvl) //are we building a level?
		lvlLevel.initialize(argv[nlvl]);
	else if (lvlno) //was a level specified?
		lvlLevel.initialize(argv[lvlno]);
	else
		lvlLevel.initialize("levels/level1.lvl");

	//go into the drawing loop
	SDL_Event evt;
	SDL_TimerID frameg_timer = SDL_AddTimer(1000, AddFrames, NULL); //for the frame counter
	SDL_EnableKeyRepeat(0, 0);
	printf("Entering message loop\n");
	game_Reset();
	g_time = SDL_GetTicks();
	// {{{ Ugly giant switch statements
	while (1) {
		if (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_KEYDOWN) {
				sym = evt.key.keysym.sym;
				if (sym == SDLK_LEFT) //can't use switch() because it's not an int
					keys[KEY_LEFT] = true;
				else if (sym == SDLK_RIGHT)
					keys[KEY_RIGHT] = true;
				else if (sym == SDLK_UP)
					keys[KEY_UP] = true;
				else if (sym == SDLK_DOWN)
					keys[KEY_DOWN] = true;
				else if (sym == SDLK_SPACE)
					keys[KEY_JUMP] = true;
				else if (sym == SDLK_ESCAPE) {
					game_Reset();
					state = MENU;
				}
			} else if (evt.type == SDL_KEYUP) {
				sym = evt.key.keysym.sym;
				if (sym == SDLK_LEFT)
					keys[KEY_LEFT] = false;
				else if (sym == SDLK_RIGHT)
					keys[KEY_RIGHT] = false;
				else if (sym == SDLK_UP)
					keys[KEY_UP] = false;
				else if (sym == SDLK_DOWN)
					keys[KEY_DOWN] = false;
				else if (sym == SDLK_SPACE)
					keys[KEY_JUMP] = false;
			} else if (evt.type == SDL_QUIT)
				break;
		}
		switch (state) {
		case MENU:
			menu.runFrame();
			break;
		case GAME:
			gDrawScene();
			break;
		default:
			printf("Weird state; dying (memory corruption?)\n");
			return 0;
		}

		g_frames++; //fps counter
		SDL_Delay(0);	//surrender the rest of the g_timeslice
	}
	// }}}
//	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(true);
	printf("Quitting SDL\n");
	SDL_RemoveTimer(frameg_timer);
	printf("Quitting g_timer subsystem\n");
	SDL_QuitSubSystem(SDL_INIT_TIMER);
	printf("Leaving Video subsystem\n");
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	printf("Leaving SDL\n");
	SDL_Quit();
	printf("All done\n");
	return 0;
}

