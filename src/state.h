#ifndef STATE_H
#define STATE_H

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdbool.h>

typedef struct EngineState {
  GLFWwindow* windows;
  bool is_running;
} EngineState;

#endif