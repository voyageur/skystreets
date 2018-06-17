bin_PROGRAMS = skystreets
skystreets_SOURCES = skystreets.cpp game.cpp levels.cpp explosion.cpp menu.cpp glm.cpp audio.cpp text.cpp
EXTRA_DIST = *.h CODE BUGS
INCLUDES = `sdl-config --cflags` -Wall -O2 -DDATAPATH=\"$(pkgdatadir)\"
#LDFLAGS = -lSDL_image -lm
 LDFLAGS = -lSDL_image -lGL -lGLU -lm
# LDFLAGS = `sdl-config --libs` -lSDL_image -lm
SUBDIRS=audio gfx levels
AUTOMAKE_OPTIONS = dist-bzip2
