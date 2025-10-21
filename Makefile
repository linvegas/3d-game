CFLAGS = -Wall -Wextra -ggdb

ifeq ($(GRAPHICS),OPENGL_21)
    CFLAGS += -DGRAPHICS_API_OPENGL_21
endif

main: main.c shader.c renderer.c linalg.c
	cc $(CFLAGS) -o main main.c renderer.c linalg.c shader.c -lSDL3 -lm
