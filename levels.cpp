/* levels.cpp
 * Loads and renders levels
 * Licensed under the Open Software License version 2.0
 * coded by ReKleSS [rekless@fastmail.fm]
 */

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <math.h>
#include <string.h>

#include "levels.h"
#include "ss.h"

bool LoadTexture(const char *filename, int i);

extern bool sysPath;

extern GLuint textures[11];


Level::Level()
{
	curBlock = 0;
	head = NULL;
	tail = NULL;
	lvlList = glGenLists(1);
}

Level::~Level()
{
	lvlpoint *current = head;
	lvlpoint *next;
	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}
}

bool Level::initialize(const char *filename)
{

	strcpy(fname, basePath);
	strcat(fname, filename);
	data = fopen(fname, "r");
//      printf("Attempting to load %s\n", fname
	if (data == NULL) {	//try the other path
		printf("File not found; creating new\n");
		maxFuel = 10000;
		maxOxygen = 10000;	//better for testing
		gravity = 500;
		LoadTexture("gfx/sunscene.png", 13);
		return false;
	}

	char background[32];
	char fullname[64];
	fscanf(data, "%i %s %s %i %i %i\n", &blocks, background, next,
	       &maxFuel, &maxOxygen, &gravity);
	lvlpoint *current;
	for (int i = 0; i < blocks; i++) {
		int c = fgetc(data);
		if (c != '{')	//to support comments and shit
			i--;
		else {
			current = addPoint();
			fscanf(data, "%f %f %f}{%f %f %f} %i\n",
			       &current->blx, &current->bly,
			       &current->blz, &current->trx,
			       &current->trY, &current->trz,
			       &current->material);
			current->blx *= 3;
			current->bly *= 3;
			current->blz *= 3;
			current->trx *= 3;
			current->trY *= 3;
			current->trz *= 3;
		}
	}
	printf("%s loaded successfully\n", filename);
	fclose(data);
	printf("Data closed...\n");
	strcpy(fullname, "gfx/");
	strcat(fullname, background);
	printf("%s\n", fullname);
	if ((data = fopen(fullname, "r")) == NULL)
		printf("Couldn't open %s\n", fullname);
	else
		fclose(data);
	LoadTexture(fullname, 13);
	prepare(); //set it up for rendering
	return true;
}

void Level::prepare( /*float front, float back */ )
{
    	glEnable(GL_TEXTURE_2D);
	glNewList(lvlList, GL_COMPILE);

	float width, height, length;
	lvlpoint *current = head;

	int i;
	while (current != NULL) {
/*		if (current->trz < back || current->blz > front){
			current = current->next;
			continue;
		}*/
		length = (current->trz - current->blz) / 3;
		height = (current->bly - current->trY) / 3;
		width = (fabs(current->blx - current->trx)) / 3;
		//bind the texture

		glBindTexture(GL_TEXTURE_2D, textures[current->material]);
		if (current->material == 8) {
			glEnable(GL_BLEND);
			glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
		}
		// do some shit
		glColor3f(0.5f, 0.5f, 0.5f);

		glBegin(GL_QUADS);
		//first quad: back face (visible from rear)
		glTexCoord2f(0, 0);
		glVertex3f(current->blx, current->bly, -current->blz);
		glTexCoord2f(width, 0);
		glVertex3f(current->trx, current->bly, -current->blz);
		glTexCoord2f(width, height);
		glVertex3f(current->trx, current->trY, -current->blz);
		glTexCoord2f(0, height);
		glVertex3f(current->blx, current->trY, -current->blz);

		//right face
		glTexCoord2f(0, 0);
		glVertex3f(current->trx, current->bly, -current->blz);
		glTexCoord2f(length, 0);
		glVertex3f(current->trx, current->bly, -current->trz);
		glTexCoord2f(length, height);
		glVertex3f(current->trx, current->trY, -current->trz);
		glTexCoord2f(0, height);
		glVertex3f(current->trx, current->trY, -current->blz);

		//left face
		glTexCoord2f(0, 0);
		glVertex3f(current->blx, current->bly, -current->blz);
		glTexCoord2f(0, height);
		glVertex3f(current->blx, current->trY, -current->blz);
		glTexCoord2f(length, height);
		glVertex3f(current->blx, current->trY, -current->trz);
		glTexCoord2f(length, 0);
		glVertex3f(current->blx, current->bly, -current->trz);
		
		glColor3f(1.0f, 1.0f, 1.0f);
		//top face
		glTexCoord2f(0, length);
		glVertex3f(current->blx, current->trY, -current->blz);
		glTexCoord2f(width, length);
		glVertex3f(current->trx, current->trY, -current->blz);
		glTexCoord2f(width, 0);
		glVertex3f(current->trx, current->trY, -current->trz);
		glTexCoord2f(0, 0);
		glVertex3f(current->blx, current->trY, -current->trz);
		glEnd();
		if (current->material == 8) {
			glDisable(GL_BLEND);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
/*
		glEnable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);

		for (i = current->blx;i < current->trx;i+=3){
			glColor4f(0.4f, 0.4f, 0.4f,fabs(i)/12.0f);
			glBegin(GL_QUADS);
			glVertex3f(i, current->trY + 0.01f, -current->blz);
			glVertex3f(i + 3.0f, current->trY + 0.01f, -current->blz);
			glVertex3f(i + 3.0f, current->trY + 0.01f, -current->trz);
			glVertex3f(i, current->trY + 0.01f, -current->trz);
			glEnd();
		}
		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glBlendEquation(GL_FUNC_ADD);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
*/

		current = current->next;
	}

	glEndList();

#if 0
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	glDrawArrays(GL_QUADS, 0, blocks);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
#endif
	return;
}

void Level::Render()
{
/*	vertices = (float *)malloc(blocks * sizeof(struct block) * 4); // 4 faces per block
	texcoords = (float *)malloc(blocks * sizeof(struct texcoord) * 4); //texcoords
	struct block *curblock = (struct block *)vertices;
	struct texcoord *curtexcoord = (struct texcoord *)texcoords;
	struct lvlpoint *current = head;
	
	while (current != NULL){
		//top face
		
		curblock->x1 = current->blx;
		curblock->y1 = current->trY;
		curblock->z1 = current->blz;
		
		curblock->x2 = current->trx;
		curblock->y2 = current->trY;
		curblock->z3 = current->blz;
		
		curblock->x3 = current->trx;
		curblock->y3 = current->trY;
		curblock->z3 = current->trz;
		
		curblock->x4 = current->blx;
		curblock->y4 = current->trY;
		curblock->z4 = current->trz;
		
		curtexcoord->x1 = 0;
		curtexcoord->y1 = 0;

		curtexcoord->x2 = 1;
		curtexcoord->y2 = 0;
		
		curtexcoord->x3 = 1;
		curtexcoord->x3 = 1;
		
		curtexcoord->x4 = 0;
		curtexcoord->x4 = 1;
		
		curblock++;
		curtexcoord++;
		current = current->next;
	}
	
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	printf("Level prepared\n");
	
	GLuint retval = glGetError();
	if (retval != GL_NO_ERROR)
		printf("GL error preparing level: %i\n", retval);
		
	return;*/
	
//	prepare();
	if (!glIsList(lvlList)){
		printf("List is invalid\n");
		lvlList = glGenLists(1);
		prepare();
		return;
	}
	glCallList(lvlList);
}
		

void Level::clear()
{
	lvlpoint *current = head, *next;

	while (current != NULL) {
		next = current;
		current = current->next;
		free(next);
	}
	head = tail = NULL;
}

bool Level::loadNext()
{
	free(vertices);
	free(texcoords);
	clear();
	char filename[30];
	strcpy(filename, "levels/");
	strcat(filename, next);
	if (strcmp("end", next) != 0) {
		initialize(filename);
		return true;
	} else {
		printf
		    ("You've finished, dropping back to the main menu.\n");
		return false;
	}
}

void Level::Add(lvlpoint * newPoint)
{
	//we can't use memcpy because it would overwrite the pointers
	lvlpoint *tmp = addPoint();
	tmp->blx = newPoint->blx;
	tmp->bly = newPoint->bly;
	tmp->blz = newPoint->blz;
	tmp->trx = newPoint->trx;
	tmp->trY = newPoint->trY;
	tmp->trz = newPoint->trz;
	tmp->material = newPoint->material;
	blocks++;
	return;
}

void Level::Delete(lvlpoint * point)	//just unlink it
{
	//just link around it, then free it
	if (point->next != NULL)
		point->next->prev = point->prev;
	else {			//we're the last point in the list
		point->prev->next = NULL;
		tail = point->prev;
	}
	if (point->prev != NULL)
		point->prev->next = point->next;
	else {			//we're the first point in the list
		head = point->next;
		head->prev = NULL;
	}

	free(point);
	blocks--;
}

void Level::Sort()
{
	if (blocks == 1)	//nothing to sort, it would just screw up the algorithm
		return;

	lvlpoint *base = head, *lpLowest = head, *current = head;	//that is NOT long pointer...
	float floor = 0, lowest = head->blz;
	int passes = 0;
	//use a sort somewhere between an insertion sort and a selection sort (seems effective on a linked list)
	//Perform a first pass to get the first element into place
	while (current != NULL) {
		if (current->blz < lowest) {
			lowest = current->blz;
			lpLowest = current;
		}
		current = current->next;
	}
	if (lpLowest != head) {
		if (lpLowest->prev != NULL)
			lpLowest->prev->next = lpLowest->next;	//link around it
		else if (lpLowest->next != NULL);
		lpLowest->next->prev = NULL;

		if (lpLowest->next != NULL)
			lpLowest->next->prev = lpLowest->prev;
		else if (lpLowest->prev != NULL)
			lpLowest->prev->next = NULL;

		lpLowest->next = head;
		head->prev = lpLowest;
		head = lpLowest;
		floor = head->blz;
		lowest = base->blz;
		base = head->next;
	}
	printf("Done first element - %i blocks\n", blocks);
	base = head->next;
	floor = head->blz;

	while (base->next != NULL) {
		current = base->next;
		lowest = base->next->blz;
		lpLowest = base->next;
		while (current != NULL) {
			if (current->blz < lowest && current->blz > floor) {
				lowest = current->blz;
				lpLowest = current;
			}
			current = current->next;
		}

		// move lpLowest to just after base, then prepare for the next run

		lpLowest->prev->next = lpLowest->next;
		if (lpLowest->next)
			lpLowest->next->prev = lpLowest->prev;	//take lpLowest out of the chain

		//forward links
		lpLowest->next = base->next;
		base->next = lpLowest;
		//backwards links
		lpLowest->prev = base;
		if (lpLowest->next)
			lpLowest->next->prev = lpLowest;

		floor = lpLowest->blz;
		base = lpLowest;

		passes++;
	}

}

void Level::Save()
{
	FILE *save = fopen(fname, "w");
	lvlpoint *current = head;
	if (save == NULL) {
		printf("Can't save file!\n");
		return;
	}
//      Sort(); //just in case...
	fseek(save, 0, SEEK_SET);
	fprintf(save, "%i fill.me.in next.lvl 30 30 500\n", blocks);
	fprintf(save, "  bg image  next lvl oxy g_fuel grav\n");	//just a data block, to make editing by hand easier
	printf("Header written, now writing %i blocks\n", blocks);
	while (current != NULL) {
		fprintf(save, "{%f %f %f}{%f %f %f} %i\n",
			current->blx / 3, current->bly / 3,
			current->blz / 3, current->trx / 3,
			current->trY / 3, current->trz / 3,
			current->material);
		current = current->next;
	}
	return;
}

lvlpoint *Level::addPoint()
{
	if (tail == NULL) {
		tail = (lvlpoint *) malloc(sizeof(lvlpoint));
		head = tail;
		tail->next = NULL;
		head->prev = NULL;	//erm... yeah...
		return tail;
	}
	tail->next = (lvlpoint *) malloc(sizeof(lvlpoint));
	tail->next->prev = tail;
	tail = tail->next;
	tail->next = NULL;
	return tail;
}
