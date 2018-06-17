/* explosion.cpp
 * Handles particle explosions and related stuff
 * Licensed under the Open Software License version 2.0
 * Coded by ReKleSS [rekless@fastmail.fm]
 */


#include "ss.h"
#include <stdlib.h>
#include <GL/gl.h>
#include "explosion.h"
#include <stdio.h>
#define PARTICLES 300

actor particles[PARTICLES];
float life; //there's no reason to use a seperate life counter for each particle
extern bool quickrestart;
int particleList;

void initParticles()
{				//this has to be done seperately becaues it takes ~1 second
	for (int i = 0; i < PARTICLES; i++) {
		particles[i].spdz = (rand() % 50) - 25;
		particles[i].spdx = (rand() % 50) - 25;
		particles[i].spdy = (rand() % 50) - 25;
	}
	particleList = glGenLists(1);
/*	glNewList(particleList, GL_COMPILE);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1, 1);
	glVertex3f(0.2f, 0.2f, 0.0f);
	glTexCoord2f(0, 1);
	glVertex3f(-0.2f, 0.2f, 0.0f);
	glTexCoord2f(1, 0);
	glVertex3f(0.2f, -0.2f, 0.0f);
	glTexCoord2f(0, 0);
	glVertex3f(-0.2f, -0.2f, 0.0f);
	glEnd();
	glEndList();*/
	
	glNewList(particleList, GL_COMPILE);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1, 0);
	glVertex3f(0.2f, 0.2f, 0.0f);
	glTexCoord2f(0, 0);
	glVertex3f(-0.2f, 0.2f, 0.0f);
	glTexCoord2f(1, 1);
	glVertex3f(0.2f, -0.2f, 0.0f);
	glTexCoord2f(0, 1);
	glVertex3f(-0.2f, -0.2f, 0.0f);
	glEnd();
	glEndList();

}

void explode(actor player)
{
	for (int i = 0; i < PARTICLES; i++) {
		particles[i].x = 0;
		particles[i].y = 0;
		particles[i].z = 0;
	}
	
	life = 2;		//2 secs
}

bool updateParticles(float SPF)
{
	if (quickrestart)
		return 0;
	
	life -= SPF;
	if (life < 0) { //do this test first so we don't waste time updating particles if they're not going to be seen
		initParticles();
		return 0;
	}
	
	for (int i = 0; i < PARTICLES; i++) {
		particles[i].x += particles[i].spdx * SPF;
		particles[i].y += particles[i].spdy * SPF;
		particles[i].z += particles[i].spdz * SPF;
	}

	return 1;
}

void drawParticles(float SPF)
{
	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);
	glColor4f(1.0f, 1.0f, 1.0f, (life / 2)); //draw the giant one
	glPushMatrix();
	glTranslatef(0.0f, 10.0f, 0.0f);
	glScalef(50.0f, 50.0f, 50.0f);
	glCallList(particleList);
	glPopMatrix();
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	for (int i = 0; i < PARTICLES; i++) {
		glPushMatrix();
		glTranslatef(particles[i].x, particles[i].y,
			     particles[i].z);
		glCallList(particleList);
		glPopMatrix();
	}
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
}

