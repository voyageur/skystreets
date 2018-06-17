bin_PROGRAMS = skystreets
skystreets_SOURCES = skystreets.cpp game.cpp levels.cpp explosion.cpp menu.cpp glm.cpp audio.cpp text.cpp
EXTRA_DIST = *.h CODE BUGS
AM_CPPFLAGS = `sdl-config --cflags` -DDATAPATH=\"$(pkgdatadir)\"
AM_LDFLAGS = `sdl-config --libs` -lSDL_image -lGL -lGLU -lm
SUBDIRS=audio gfx levels
AUTOMAKE_OPTIONS = dist-bzip2
