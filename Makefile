run: main.cpp
	g++ main.cpp glad/src/gl.c -Iglad/include -Iinclude -lglfw -ldl
	./a.out