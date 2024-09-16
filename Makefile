C_SRC = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJ = $(C_SRC:.c=.o)

run: src/main.c
	g++ src/*.c glad/src/gl.c -Iglad/include -Iinclude -lglfw -ldl
	./a.out
