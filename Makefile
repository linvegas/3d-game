CFLAGS = -Wall -Wextra

ifeq ($(GRAPHICS),OPENGL_21)
    CFLAGS += -DGRAPHICS_API_OPENGL_21
endif

main: main.c shader.c linalg.h
	cc $(CFLAGS) -o main main.c shader.c -lSDL3 -lm

test: test.c test.cpp linalg.h
	cc $(CFLAGS) -Wextra -o test-c test.c -lm
	g++ -o test-cpp test.cpp
