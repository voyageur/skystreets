/* game.cpp:
 * Main game loop and helper functions
 * Licensed under the Open Software License version 2.0
 * Coded by ReKleSS [rekless@fastmail.fm]
 */

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "SDL.h"
#include <string.h>
#include <math.h>

#include "ss.h"
#include "levels.h"
#include "audio.h"
#include "explosion.h"
#include "text.h"

float g_fuel;
float g_oxygen;
struct actor player;
Level lvlLevel;
long trackTime;
bool finished;
long lastCycle;
long currentTime;
Uint32 g_time;
Text text;
int impacted;

extern int frames;
extern GLuint textures[TEXTURES];
extern bool keys[256];
extern audioSample explosion;
extern audioSample engine;
extern bool quickrestart;
extern bool drawShadow;
extern int state;
extern int shipList;
extern int fps;
extern GLuint *shiplists;

/* Reset: Reset everything to the start of the level */

#ifdef WIN32
void glBlendEquation(GLenum mode){
	return;
}
#endif

void game_Reset()
{
	memset(&player, 0, sizeof(actor));
	player.z = 0.5f;
	player.y = 8.0f;
	player.spdy = -1.0f;
	lvlLevel.curBlock = 0;
	g_fuel = lvlLevel.maxFuel;
	g_oxygen = lvlLevel.maxOxygen;
	trackTime = quickrestart ? 0 : -1500;	//the countdown timer
	finished = 0;
	engine.stop();
	explosion.stop();
}

/* HandleKeys: handle all user input. SPF is the timing facor, collisions
 * is an array of floats, each element being the distance to the nearest
 * obstacle in that direction. Floor is the height of the floor
 */

void HandleKeys(double SPF, int surface, float *collisions, float floor)
{

	float strafemul = player.spdz > 100 ? 100 : player.spdz;	//because if it's allowed to run too high, the ship moves stupidly fast
	if (strafemul == 0)
		strafemul = 1;
	strafemul = fabs(strafemul);

	if (keys[KEY_UP]) {
		player.spdz += SPF * 30.0f;
		if (engine.state != 1)
			engine.play(1);
	}
	if (keys[KEY_DOWN])
		player.spdz -= (SPF * 30.0f);

	if (keys[KEY_RIGHT] && !(surface & SURF_SLIPPERY) && (player.spdy == 0 || player.offGroundTime < 0.3f)) {	//only have control if on the ground
		player.spdx += strafemul * SPF * 100;
		if (player.spdx > strafemul)
			player.spdx = strafemul;
	}
	if (keys[KEY_LEFT] && !(surface & SURF_SLIPPERY) && (player.spdy == 0 || player.offGroundTime < 0.3f)) {
		player.spdx -= strafemul * SPF * 100;
		if (player.spdx < -strafemul)
			player.spdx = -strafemul;
	}

	if (keys[KEY_JUMP] && player.spdy == 0) {
		player.spdy = 60;
		player.y += 0.001f;
//		player.onground = false;
		player.offGroundTime = 0;
	}

	if (player.spdz < 0) {
		player.spdz = 0;
		engine.pause();
	}
	if (player.spdz > 30)
		player.spdz = 30;
	
}

/* UpdatePlayer: Change the player's position, taking into account
 * collisions, speeds, and other stuff. Arguments are the same as
 * the previous function
 */

void updatePlayer(double SPF, int surface, float floor, float *collisions)
{
	if (collisions[FORWARD] != -1 && collisions[FORWARD] < 0.2f) {
		if (player.spdz > 10.0f) {	//we crashed fast
			engine.stop();
			explosion.play(0);
			explode(player);
			player.exploding = 11;
			player.spdy = 0;
		}
		player.spdz = 0;
	}

	g_oxygen -= SPF;
	g_fuel -= (player.spdz * SPF) / 10;

	if (g_oxygen < 0 || g_fuel < 0) {
		explode(player);
		player.exploding = 11;
	}
	
	if (!(surface & SURF_SLIPPERY) && player.spdy == 0){	//friction
		player.spdx *= 50 * SPF;
	}

	player.z += SPF * player.spdz;
	if (!player.spdy && player.y > floor)
		player.spdy = -0.0001f;

	if (player.spdy)
		player.spdy -= SPF * 200.0f / (500.0f / lvlLevel.gravity);
	if (collisions[UP] == -1 || SPF * player.spdy < collisions[UP])
		player.y += SPF * player.spdy;
	else {
		player.y += collisions[UP] - 0.1f;
		player.spdy = 0;
	}
	if (player.y < floor) {
		impacted = 1;
		if (!(surface & SURF_SLIPPERY))
			player.spdx = 0;
		if (player.spdy > -10.0f){
			player.y = floor;
			player.spdy = 0;
		}else if (player.spdy){
			player.y = floor + (floor - player.y);
			player.spdy *= -0.25;
			player.offGroundTime = 0;
		} else
			player.y = floor;
	}

	if (!(surface & SURF_SLIPPERY) && player.spdy == 0){	//friction
		player.spdx *= 50* SPF;
	}

	if (player.y == -15) {
		explode(player);
		player.exploding = 11;
		engine.stop();
		explosion.play(false);
	}

	if (player.spdx > 0 && collisions[RIGHT] != -1.0f)
		if (player.spdx * SPF >= collisions[RIGHT]) {
			player.x += collisions[RIGHT] - 0.01f;
			player.spdx = 0;
		}

	if (player.spdx < 0 && collisions[LEFT] != -1.0f)
		if (-(player.spdx * SPF) >= collisions[LEFT]) {
			player.x -= collisions[LEFT] - 0.01f;
			player.spdx = 0;
		}


	player.x += player.spdx * SPF;

	if (fabs(player.spdx) < 0.1f)
		player.spdx = 0;
}

/* checkTerrain: perform whatever actions are necessary for the terrain the
 * player is currently moving over
 */

void checkTerrain(double SPF, float floor, int surface)
{
	if (player.y == floor || impacted) {
		if (surface & SURF_SUPPLY) {
			g_fuel = lvlLevel.maxFuel;
			g_oxygen = lvlLevel.maxOxygen;
		}
		if (surface & SURF_EXPLODE) {
			player.exploding = 11;
			player.spdz = 0;
			player.spdy = 0;
			engine.stop();
			explosion.play(false);
			explode(player);
			return;
		}
		if (surface & SURF_ACCEL)
			player.spdz *= 1.1;
		if (surface & SURF_STICKY)
			player.spdz *= 0.9;
		if (surface & SURF_SLIPPERY)
			player.animframe = 1;
		if (surface & SURF_GOAL) {
			player.spdz = 0;
			player.spdy = 0;
			player.exploding = 12;
			explode(player);
			engine.pause();
			finished = 1;
			return;
		}
	}
	return;
}

/* checkCollisions: The collision detection function. collisions is a pointer
 * to a series of floats that are going to be filled.
 */

void checkCollisions(float collisions[], float *floor, int *surface)
{

	float tmpfront = player.z + 0.1f, tmpleft =
	    player.x - 1.9f, tmpright = player.x + 1.9f, tmpback =
	    player.z - 5.0f, tmptop = player.y + 1.0f, dist = 0;
	// {{{ Collision detection
	
	lvlpoint *curBlock = lvlLevel.head;
	while (curBlock != NULL) {
		if (collisions[FORWARD] != -1
		    && curBlock->blz > player.z + collisions[FORWARD]) {
			curBlock = curBlock->next;
			continue;
		}
		if (curBlock->trz < tmpback) {
			curBlock = curBlock->next;
			continue;
		}
		//right
		if (curBlock->blx > tmpright
		    && curBlock->blz < tmpfront
		    && curBlock->trz > tmpback
		    && curBlock->trY > player.y
		    && curBlock->bly <= player.y) {
			dist = curBlock->blx - tmpright;
			if (collisions[RIGHT] == -1
			    || dist < collisions[RIGHT]){
				collisions[RIGHT] = dist;
			}
		}
		//to the left
		if (curBlock->trz > tmpback
		    && curBlock->blz < tmpfront
		    && curBlock->trx < tmpleft
		    && curBlock->trY > player.y
		    && curBlock->bly <= player.y) {
			dist = tmpleft - curBlock->trx;
			if (collisions[LEFT] == -1
			    || dist < collisions[LEFT]){
				collisions[LEFT] = dist;
			}
		}
		//front
		if (curBlock->blx < tmpright && curBlock->trx > tmpleft
		    && curBlock->blz > tmpfront
		    && curBlock->trY >= player.y + 0.01f
		    && curBlock->bly <= player.y) {
			dist = curBlock->blz - tmpfront;
			if (collisions[FORWARD] == -1
			    || dist < collisions[FORWARD]){
				collisions[FORWARD] = dist;
			}
		}

		if (curBlock->trx > tmpleft && curBlock->blx < tmpright && curBlock->trz > tmpback && curBlock->blz < tmpfront - 0.1f) {	//the 0.1 prevents the ship from 'lifting' itself
			if (curBlock->trY <= player.y) {
				if (*floor < curBlock->trY)
					*floor = curBlock->trY;
//				if (player.y == curBlock->trY){
					*surface |= 1 << (curBlock->material - 1);
					if (curBlock->material == 8)	/*slippy */
						player.animframe = 1;
//				}
			} else {	//above
				if (collisions[UP] == -1
				    || curBlock->bly - tmptop <
				    collisions[UP]){
					collisions[UP] =
					    curBlock->bly - tmptop;
				}
			}
		}
		curBlock = curBlock->next;
	}
	// }}}
}

/* drawMeters(): draw the fuel, oxygen and throttle
 */

void drawMeters()
{

	//finally... draw g_throttle, g_fuel, O2
	float status = 0;
	glLoadIdentity();
	glEnable(GL_BLEND);
	glColor4f(0.1f, 0.39f, 0.82f, 0.5f);	//a nice electric blue
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	// {{{ Bunch of colourful quads
	glBegin(GL_QUADS);
	glVertex3f(player.spdz / 30.0f, 1.0f, -1.0f);
	glVertex3f(-player.spdz / 30.0f, 1.0f, -1.0f);
	glVertex3f(-player.spdz / 30.0f, 0.9f, -1.0f);
	glVertex3f(player.spdz / 30.0f, 0.9f, -1.0f);
	glEnd();		//g_fuel and O2 can be added in quite easily...

	//next: Oxygen - left
	//Oxygen - erm... green
	glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
	glBegin(GL_QUADS);
	status = g_oxygen/lvlLevel.maxOxygen;
	glVertex3f(-1.2f,-status, -1.0f);
	glVertex3f(-1.1f,-status, -1.0f);
	glVertex3f(-1.1f, status, -1.0f);
	glVertex3f(-1.2f, status, -1.0f);
	glEnd();
	//g_fuel, in yellow
	glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
	glBegin(GL_QUADS);
	status = g_fuel / lvlLevel.maxFuel;
	glVertex3f(1.1f,-status, -1.0f);
	glVertex3f(1.2f,-status, -1.0f);
	glVertex3f(1.2f, status, -1.0f);
	glVertex3f(1.1f, status, -1.0f);
	glEnd();

	if (!player.onground) {
		glColor4f(1.0f, 0.1f, 0.1f, 0.5f);
		glBegin(GL_QUADS);
		glVertex3f(1.1f, -1.1f, -1.0f);
		glVertex3f(1.1f, -1.2f, -1.0f);
		glVertex3f(1.2f, -1.2f, -1.0f);
		glVertex3f(1.2f, -1.1f, -1.0f);
		glEnd();
	}
	// }}}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	return;
}

/*drawBackground(): Draw the background (texture 13)
 */

void drawBackground()
{

	glDisable(GL_DEPTH_TEST);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 1.0f);

	glEnd();
	glEnable(GL_DEPTH_TEST);
	return;
}

/* gDrawScene(): Run a frame in the actual game
 */

void gDrawScene()
{
	impacted = 0;
	float floor = -15;
	int surface = 0;
	Uint32 g_timeDiff = 0;
	double SecsperFrame = 0;
	lastCycle = currentTime;
	currentTime = SDL_GetTicks();
	GLenum retval;
	//this needs to be first
	while (!SecsperFrame) {
		g_timeDiff = SDL_GetTicks() - g_time;
		SecsperFrame = (double) g_timeDiff / 1000.0;
	}

	SecsperFrame = (SDL_GetTicks() - lastCycle) / 1000.0f;
	player.onground ? SecsperFrame *= 0.9f : SecsperFrame *= 0.86f;	//seems to give a more skyroads feel...
	if (!finished)
		trackTime += (currentTime - lastCycle);

	player.offGroundTime += SecsperFrame;

	//if one ms hasn't passed, wait until one has (effectively limiting to 1000 fps)
//	glMatrixMode(GL_MODELVIEW);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	glFrontFace(GL_CCW);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, textures[13]);

	glTranslatef(0.0f, 0.0f, -0.84f);

	drawBackground();

	retval = glGetError();
	if (retval != GL_NO_ERROR) {
		printf("OpenGL Error received!!! (setup)... %i \n",
		       retval);
	}
	glLoadIdentity();


	retval = glGetError();
	if (retval != GL_NO_ERROR) {
		printf("OpenGL Error received!!! (background)... %i \n",
		       retval);
	}

	glRotatef(6.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, player.y < 10.0f ? -14.0f : -player.y - 4.0f, player.z - 19);	//drop the camera back just enough to not lose the ship out the side...

	float collisions[4] = { -1.0f, -1.0f, -1.0f, -1.0f };
	player.animframe = 0;

	if (player.exploding == false && trackTime > 0) {
		//maybe I should standardise these arguments...
		checkCollisions(collisions, &floor, &surface);
		updatePlayer(SecsperFrame, surface, floor, collisions);
		checkTerrain(SecsperFrame, floor, surface);
		HandleKeys(SecsperFrame, surface, collisions, floor);
	} else if (player.exploding) {
		if (!updateParticles(SecsperFrame)) {
			if (player.exploding == 12) {	//load the next level
				if (!lvlLevel.loadNext())
					state = MENU;
				printf("Your time was %f seconds\n",
				       trackTime / 1000.0f);
			}
			game_Reset();
			return;
		}
	}
	//draw the level
	glColor3f(0.0f, 0.0f, 0.0f);
	//render the level
	glColor3f(1.0f, 1.0f, 1.0f);
	// current.Render(player.z + 70.0f, player.z - 15.0f);
	lvlLevel.Render();

	retval = glGetError();
	if (retval != GL_NO_ERROR) {
		printf("OpenGL Error received!!! (level)... %i \n",
		       retval);
	}
	//draw the 'player'
	if (!player.exploding) {
		glColor3f(1.0f, 1.0f, 1.0f);
		glPushMatrix();
		glTranslatef(player.x, player.y, -player.z);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glCallList(shiplists[(int)player.spdz/2]);
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
		
		if (drawShadow && floor != -15){
		
			glDisable(GL_TEXTURE_2D);
			glPushMatrix();
			glTranslatef(player.x, floor + 0.1f ,-player.z);
		
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
			glScalef(1.0f, 0.001f, 1.0f);
			glColor3f(0.1f, 0.1f, 0.1f);
			glCallList(shipList);
			glDisable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
		
			glPopMatrix();
		}
	} else {
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glTranslatef(player.x, player.y, -player.z);		

		if (player.exploding == 11){
			glBindTexture(GL_TEXTURE_2D, textures[0]);
			glPushMatrix();
			glScalef(1.0f, 1.0f, 0.1f); //flatten the ship
			glCallList(shipList);
			glPopMatrix();
		}
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBindTexture(GL_TEXTURE_2D, textures[player.exploding]);
		drawParticles(SecsperFrame);
		glDisable(GL_BLEND);
		glPopMatrix();
	}

	retval = glGetError();
	if (retval != GL_NO_ERROR) {
		printf("OpenGL Error received!!! (player)... %i \n",
		       retval);
	}

	drawMeters();

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, textures[14]);
	glLoadIdentity();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);	//the blend is just to make the black disappear
	glTranslatef(-2.0f, 10.0f, -10.0f);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	text.drawf("%2.2f", trackTime / 1000.0f);

	glColor4f(0.5f, 1.0f, 0.5f, 1.0f);
	glLoadIdentity();
	glTranslatef(7.0f, 10.0f, -10.0f);
	text.drawf("%i", fps);
	glLoadIdentity();
	glColor4f(0.5f, 0.75f, 1.0f, 1.0f);
	glTranslatef(-10.0f, -10.0f, -10.0f);
	text.drawf("%i", lvlLevel.gravity);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	if (trackTime < 0) {
		glLoadIdentity();
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		glColor4f(1.0f, 1.0f, 1.0f, trackTime/-1500.0f);
		glBegin(GL_QUADS);
		glVertex3f(-1.5f, -1.5f, -1.0f);
		glVertex3f(1.5f, -1.5f, -1.0f);
		glVertex3f(1.5f, 1.5f, -1.0f);
		glVertex3f(-1.5f, 1.5f, -1.0f);
		glEnd();
		glBlendEquation(GL_FUNC_ADD);
		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
	} else {
		glLoadIdentity();
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		
		retval = glGetError();
		if (retval != GL_NO_ERROR) {
			printf("OpenGL Error received!!! (bah)... %i \n",
		       retval);
		}
				
		glBegin(GL_QUADS);
	
	    	glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
		glVertex3f( 0.0f,-1.5f, -1.0f);
		glVertex3f( 0.0f, 1.5f, -1.0f);
		glColor4f(0.3f, 0.3f, 0.3f, 0.4f);
		glVertex3f(-1.5f, 1.5f, -1.0f);
		glVertex3f(-1.5f,-1.5f, -1.0f);
		
		glVertex3f(1.5f,-1.5f, -1.0f);
		glVertex3f(1.5f, 1.5f, -1.0f);
		glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
		glVertex3f(0.0f, 1.5f, -1.0f);
		glVertex3f(0.0f,-1.5f, -1.0f);
		
		glEnd();
		
		glDisable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glEnable(GL_TEXTURE_2D);
	}

	SDL_GL_SwapBuffers();

	retval = glGetError();
	if (retval != GL_NO_ERROR) {
		printf("OpenGL Error received!!! (counters)... %i \n",
	       retval);
	}
	
	g_time = SDL_GetTicks();
}
