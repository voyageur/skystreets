/* menu.cpp
 * Draws and deals with the menus... pretty simple
 * Licensed under the Open Software License version 2.0
 * coded by ReKleSS [rekless@fastmail.fm]
 */

#ifdef WIN32
#include <windows.h>
#endif

#include <SDL.h>
#include <GL/gl.h>

#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

#include "ss.h"
#include "text.h"
#include "menu.h"
#include "levels.h"
#include "game.h"

#define DIR_DOWN 0
#define DIR_UP 1


extern GLuint textures[12];
extern bool keys[256];
extern int state;
extern GLint shipList;
extern Text text;
extern Uint32 g_time;
extern long currentTime;
extern Level lvlLevel;

void Menu::addEntry(char *name)
{
	menuEntry *current = &head;
	while (current->next != NULL){
		current = current->next;
	}

	current->next = (menuEntry *)malloc(sizeof(struct menuEntry));
	current = current->next;
	current->filename = strdup(name);
	current->next = NULL;
	elements++;
	return;
}

Menu::Menu(){
	return;
}

void Menu::init()
{
	head.next = NULL;
	selected = 0;
	memset(&whacked, 0, sizeof(bool) * 256);
	
	char fullpath[256];
	strcpy(fullpath, basePath);
	char nextfile[32];
	strcat(fullpath, "levels/level1.lvl");
	FILE *f;
	addEntry("level1.lvl");

	while ((f = fopen(fullpath, "r"))){
		fscanf(f, "%*i %*s %s", nextfile); //%\*? means discard
		printf("%s\n", nextfile);
		if (strcmp(nextfile, "end") == 0){
			fclose(f);
			break;
		}
		addEntry(nextfile);
		strcpy(fullpath, basePath);
		strcat(fullpath, "levels/");
		strcat(fullpath, nextfile);
		fclose(f);
	}
	
	return;
}

void Menu::runFrame()
{
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(0.0f, 0.0f, -0.84f);
	glBindTexture(GL_TEXTURE_2D, textures[10]);
	glBegin(GL_QUADS); //draw the background image
	glTexCoord2f(0, 1);
	glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(1, 1);
	glVertex2f(1.0f, -1.0f);
	glTexCoord2f(1, 0);
	glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0, 0);
	glVertex2f(-1.0f, 1.0f);
	glEnd();

	glTranslatef(0.0f, 0.0f, 0.1f);
	glEnable(GL_BLEND);
	
	glPushMatrix();
	glTranslatef(-0.8f, 0.4f, 0.0f);
	glScalef(0.1f, 0.1f, 0.1f);
	glBindTexture(GL_TEXTURE_2D, textures[14]);
	
	if (selected == 0)
		glColor3f(1.0f, 0.1f, 0.1f);
	else
		glColor3f(1.0f, 1.0f, 1.0f);
	
	text.drawf("Play");
	
	glTranslatef(0.0f, -1.0f, 0.0f);

	if (selected == 1)
		glColor3f(1.0f, 0.1f, 0.1f);
	else
		glColor3f(1.0f, 1.0f, 1.0f);

	
	text.drawf("Quit");

	glTranslatef(6.0f, 3.0f, 0.0f);
	int index = 2;

	if (head.next){ //iterate through the list, draw the items as necessary
				menuEntry *current = head.next;
		glScalef(0.8f, 0.8f, 1.0f);
		while (current != NULL){
			if (selected == index)
				glColor3f(1.0f, 0.1f, 0.1f);
			else
				glColor3f(1.0f, 1.0f, 1.0f);
			text.drawf(current->filename);
			glTranslatef(0.0f, -1.0f, 0.0f);
			current = current->next;
			index++;
		}
	}
	
	glPopMatrix();
	

	//handle keystrokes
	if (keys[KEY_DOWN] && !whacked[KEY_DOWN]) {
		selected++;
		if (selected > elements+1)
			selected = 0;
		whacked[KEY_DOWN] = true;
	} else if (!keys[KEY_DOWN] && whacked[KEY_DOWN])
		whacked[KEY_DOWN] = false;

	if (keys[KEY_UP] && !whacked[KEY_UP]) {
		selected--;
		if (selected < 0)
			selected = elements + 1;
		whacked[KEY_UP] = true;
	} else if (!keys[KEY_UP] && whacked[KEY_UP])
		whacked[KEY_UP] = false;
		
	if (keys[KEY_LEFT] && !whacked[KEY_LEFT]){
		selected = 0;
		whacked[KEY_LEFT] = true;
	} else if (!keys[KEY_LEFT] && whacked[KEY_LEFT])
		whacked[KEY_LEFT] = false;
		
	if (keys[KEY_RIGHT] && !whacked[KEY_RIGHT]) {
		selected = 2;
		whacked[KEY_RIGHT] = true;
	} else if (!keys[KEY_RIGHT] && whacked[KEY_RIGHT])
		whacked[KEY_RIGHT] = false;
		
	if (keys[KEY_JUMP]) {	//we got a space; figure out wtf the user wants
		switch(selected){
			case 0:
				state = GAME;
				memset(&keys, 0, sizeof(bool) * 256);	//zero the key array
				g_time = SDL_GetTicks();
				currentTime = SDL_GetTicks();
				return;
			case 1:
				SDL_Event quitEvent;
				quitEvent.type = SDL_QUIT;
				SDL_PushEvent(&quitEvent);
			default: //levels
				menuEntry *current = head.next;
				int i=0;
				for (i = 0;i<selected - 2;i++)
					current = current->next;
				char fullpath[256];
				strcpy(fullpath, basePath);
				strcat(fullpath, "levels/");
				strcat(fullpath, current->filename);
				lvlLevel.clear();
				game_Reset();
				lvlLevel.initialize(fullpath);
				state = GAME;
				currentTime = SDL_GetTicks();
				g_time = SDL_GetTicks();		
		}
	}

	glDisable(GL_BLEND);

	glColor3f(0.8f, 0.8f, 0.8f);

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTranslatef(4.0f, -4.0f, -5.0f);
	glRotatef(135.0f, 0.0f, 1.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glCallList(shipList);


	SDL_GL_SwapBuffers();
}

Menu::~Menu()
{
	return;
}

