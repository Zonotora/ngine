C_SRC = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJ = $(C_SRC:.c=.o)

main: main.c
	g++ main.c src/*.c glad/src/gl.c -Isrc -Iglad/include -Iinclude -lglfw -ldl
	./a.out

light: light.c
	g++ light.c src/*.c glad/src/gl.c -Isrc -Iglad/include -Iinclude -lglfw -ldl
	./a.out