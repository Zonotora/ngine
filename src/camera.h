#ifndef CAMERA_H
#define CAMERA_H

#include "linmath.h"

typedef struct Camera {
  vec3 position;
  vec3 direction;
  vec4 view_matrix;
} Camera;


Camera camera_init();
void camera_reset(Camera* camera);

#endif