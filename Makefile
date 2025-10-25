CFLAGS = -Wall -Wextra -ggdb

FT_CFLAGS = $(shell pkg-config --cflags freetype2)
FT_LIBS   = $(shell pkg-config --libs freetype2)

LIBS = $(FT_LIBS) -lSDL3 -lm
CFLAGS += $(FT_CFLAGS)

main: main.c shader.c renderer.c linalg.c
	cc $(CFLAGS) -o main main.c renderer.c linalg.c shader.c $(LIBS)
