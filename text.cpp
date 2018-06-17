/* text.cpp
 * Handles text drawing stuff
 * Licensed  under the Open Software License version 2.0
 * Coded by ReKleSS [rekless@fastmail.fm]
 */
#include "ss.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <GL/gl.h>
#include "text.h"
#include "string.h"

Text::Text()
{
}

void Text::init()
{
	buildLists();
	memset(table, 0, sizeof(int) * 128);
	table[(int) '-'] = fontLists[64];	// -
	table[(int) '.'] = fontLists[62];	// .
	table[(int) ':'] = fontLists[63];	// :
	for (int i = 0; i < 10; i++)
		table[i + '0'] = fontLists[i + 52];
	for (int i = 0; i < 26; i++)
		table[i + 'A'] = fontLists[i];
	for (int i = 0; i < 26; i++)
		table[i + 'a'] = fontLists[i + 26];
}

Text::~Text()
{
	glDeleteLists(fontLists[0], 65);
}

void Text::drawNo(int number)
{
	char text[32];		//no reason for text to be any longer than this
	sprintf(text, "%i", number);
	for (int i = 0; text[i] != 0; i++) {	//we'll install a full font + hash table later
		glCallList(fontLists[text[i] - '0']);
		glTranslatef(1.0f, 0.0f, 0.0f);
	}
	return;
}

void Text::buildLists()
{
	fontLists[0] = glGenLists(65);	//A-Za-z0-9.:-
	if (fontLists[0] == 0)
		printf("Couldn't generate lists\n");
	for (int i = 1; i < 65; i++)
		fontLists[i] = fontLists[0] + i;
	//each glyph is 11x20
	int level = 0;

	for (int i = 0; i < 65; i++) {
		if (i < 26)
			level = 2;
		else if (i < 52)
			level = 1;
		else
			level = 0;
		glNewList(fontLists[i], GL_COMPILE);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f / 26 * (i % 26), 1.0f / 3 * level);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f((i + 1) % 26 ==
			     0 ? 1.0f : 1.0f / 26 * ((i + 1) % 26),
			     1.0f / 3 * level);
		glVertex2f(1.0f, 0.0f);
		glTexCoord2f((i + 1) % 26 == 0 ? 1.0f : 1.0f / 26 * ((i + 1) % 26), 1.0f / 3 * (level + 1));	//yes, I like inlining stuff.
		glVertex2f(1.0f, 1.0f);
		glTexCoord2f(1.0f / 26 * (i % 26), 1.0f / 3 * (level + 1));
		glVertex2f(0.0f, 1.0f);
		glEnd();
		glEndList();
	}
}

int Text::drawf(char *fmt, ...)
{
	glPushMatrix();
	va_list ap;
	va_start(ap, fmt);
	char *text = (char *) malloc(256);	//there's no reason for text to be this long, therefore it's safe :p
	vsnprintf(text, 256, fmt, ap);
	char *p = text;
	while (*p) {
		if (table[(int) *p]) {
			glCallList(table[(int) *p]);
		}
		glTranslatef(1.0f, 0.0f, 0.0f);
		p++;
	}

	free(text);
	glPopMatrix();
	return 0;
}
