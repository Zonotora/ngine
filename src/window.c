
#include "window.h"

#include <stdlib.h>
#include <stddef.h>

GLFWwindow* window_init() {
    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);


    return window;
}