#include "SDL.h"


class audioSample {
      private:

	Uint32 length;
	SDL_AudioSpec spec;
	char *buffer;
	Uint32 position;
	bool loopSample;
	static void callback(void *userdata, Uint8 * stream, int len);
	static int sample;
	static char *actBuffer;
	static void setBuffer(char *buffer);
	static char *actbuf;
      public:
	static bool disabled;
	 audioSample();
	~audioSample();
	int load(char *filename);
	int play(bool loop);
	int pause();
	int stop();
	int state;
};
