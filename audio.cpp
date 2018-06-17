/* audio.cpp
 * Handles the audio stuff
 * Licensed under the Open Software License version 2.0
 * coded by ReKleSS (rekless@fastmail.fm)
 */

#include "SDL.h"
#include "audio.h"
#include <string.h>

char *audioSample::actBuffer;	//er... ?
bool audioSample::disabled = 0;

audioSample::audioSample()
{
	position = 0;
	state = 0;
	return;
}

audioSample::~audioSample()
{
	SDL_FreeWAV((Uint8 *) buffer);
	return;
}

int audioSample::load(char *filename)
{
	if (SDL_LoadWAV(filename, &spec, (Uint8 **) & buffer, &length) ==
	    NULL) {
		return -1;
	}
	return 0;
}

int audioSample::play(bool loop)
{
	if (disabled) {
		return 1;
	}

	if (state == 1) {
		audioSample::actBuffer = this->buffer;
		position = 0;
		SDL_PauseAudio(1);
		SDL_PauseAudio(0);
		return 0;
	}
	position = 0;
	loopSample = loop;
	SDL_AudioSpec obtained;
	spec.callback = &audioSample::callback;
	spec.samples = 256;	//yeah, nice and short
	spec.userdata = this;	//well, it's the first element
	audioSample::actBuffer = this->buffer;
	if (SDL_OpenAudio(&spec, &obtained) != 0) {
//              return -1; //look! a distraction!
	}
	SDL_PauseAudio(0);
	state = 1;
	return 0;
}

int audioSample::pause()
{
	SDL_PauseAudio(1);
	state = 0;
	return 0;
}

int audioSample::stop()
{
	SDL_PauseAudio(1);
	state = 0;
	position = 0;
	return 0;
}

void audioSample::callback(void *userdata, Uint8 * stream, int len)
{
	audioSample *sample = (audioSample *) userdata;
	if (sample->position + len > sample->length) {
		memcpy(stream, actBuffer + sample->position,
		       sample->length - sample->position);
		if (!sample->loopSample) {
			SDL_PauseAudio(1);
			sample->position = 0;
			sample->state = 0;
			return;
		}
		len -= (sample->length - sample->position);
		sample->position = 0;
		memcpy(stream, actBuffer, len);
		sample->position += len;
		return;
	}

	memcpy(stream, actBuffer + sample->position, len);
	sample->position += len;
	return;
}
