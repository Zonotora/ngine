#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "cube.h"
#include "linmath.h"
#include "window.h"

static const char *vertex_shader_text =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;      // Vertex position\n"
    "\n"
    // "uniform mat4 model;\n"
    // "uniform mat4 view;\n"
    // "uniform mat4 projection;\n"
    "\n"
    "void main()\n"
    "{\n"
    // "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "}\n";

static const char *fragment_shader_text =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "\n"
    // "uniform vec3 objectColor;\n"
    // "uniform vec3 lightColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0);\n"
    "}\n";

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const GLchar* message, const void* userParam) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}


static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

float last_x = -1.0;
float last_y = -1.0;
float yaw = 280.0f, pitch = 1.0f;
static void mouse_callback(GLFWwindow *window, int button, int action,
                           int mods) {
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        printf("yaw: %f pitch: %f\n", yaw, pitch);
        fflush(stdout);
    }
}
static void cursor_callback(GLFWwindow *window, double xpos, double ypos) {
    if (last_x < 0)
        last_x = xpos;
    if (last_y < 0)
        last_y = ypos;
    float offset_x = xpos - last_x;
    float offset_y = ypos - last_y;
    last_x = xpos;
    last_y = ypos;

    const float sensitivity = 0.001f;
    offset_x += sensitivity;
    offset_y += sensitivity;

    yaw += offset_x;
    pitch += offset_y;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

float zoom = 1.0f;
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    zoom -= (float)yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

void processInput(GLFWwindow *window, float delta_time, vec3 pos, vec3 front,
                  vec3 up) {
    const float speed = 2.5f * delta_time; // adjust accordingly
    vec3 tmp_pos, tmp_front, tmp_cross, tmp_norm;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        vec3_scale(tmp_front, front, speed);
        vec3_dup(tmp_pos, pos);
        vec3_add(pos, tmp_pos, tmp_front);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        vec3_scale(tmp_front, front, -speed);
        vec3_dup(tmp_pos, pos);
        vec3_add(pos, tmp_pos, tmp_front);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3_mul_cross(tmp_cross, front, up);
        vec3_norm(tmp_norm, tmp_cross);
        vec3_scale(tmp_front, tmp_norm, -speed);
        vec3_dup(tmp_pos, pos);
        vec3_add(pos, tmp_pos, tmp_front);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3_mul_cross(tmp_cross, front, up);
        vec3_norm(tmp_norm, tmp_cross);
        vec3_scale(tmp_front, tmp_norm, speed);
        vec3_dup(tmp_pos, pos);
        vec3_add(pos, tmp_pos, tmp_front);
    }
}

void init_buffers(unsigned int VAO, unsigned int VBO) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES_POS), CUBE_VERTICES_POS,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
}

GLuint init_shaders() {
    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
    return program;
}

int main(void) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    // Enable(GL_DEBUG_OUTPUT);

    GLFWwindow *window = window_init();

    glDebugMessageCallback(debug_callback, 0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetScrollCallback(window, scroll_callback);

    vec3 cube_pos = {0.0f, 0.0f, 0.0f};
    vec3 light_pos = {1.2f, 1.0f, 2.0f};

    // Generate buffers and arrays
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    init_buffers(VAO, VBO);
    GLuint program = init_shaders();

    mat4x4 model, view, projection;
    mat4x4_identity(model);
    mat4x4_identity(view);
    mat4x4_identity(projection);
    // mat4x4_translate(trans, 0.5f, -0.5f, 0.0f);
    mat4x4_rotate_X(model, model, -20.0f);
    mat4x4_translate(view, 0.0f, 0.0f, -3.0f);
    mat4x4_perspective(projection, zoom, 640.0f / 480.0f, 0.1f, 100.0f);
    // unsigned int modelLoc = glGetUniformLocation(program, "model");
    // unsigned int viewLoc = glGetUniformLocation(program, "view");
    // unsigned int projectionLoc = glGetUniformLocation(program, "projection");
    // unsigned int lightLoc = glGetUniformLocation(program, "lightColor");
    // unsigned int colorLoc = glGetUniformLocation(program, "objectColor");

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Camera camera = camera_init();
    camera.position[2] = 6.0f;
    vec3 target, direction, tmp, right;
    vec3 up = {0.0f, 1.0f, 0.0f};
    vec3_sub(tmp, camera.position, target);
    vec3_norm(direction, tmp);
    vec3_mul_cross(tmp, up, direction);
    vec3_norm(right, tmp);
    vec3_mul_cross(tmp, direction, right);
    vec3 eye = {0.0f, 0.0f, 0.0f};
    float delta_time = 0.0f, last_frame = 0.0f;

    vec3 lightColor = {1.0f, 1.0f, 1.0f};
    vec3 toyColor = {1.0f, 0.5f, 0.31f};
    // glUniform3f(lightLoc, lightColor[0], lightColor[1], lightColor[2]);
    // glUniform3f(colorLoc, toyColor[0], toyColor[1], toyColor[2]);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        float mult = 3.1415f / 180.0f;
        float radian_yaw = yaw * mult;
        float radian_pitch = -pitch * mult;
        camera.direction[0] = cos(radian_yaw) * cos(radian_pitch);
        camera.direction[1] = sin(radian_pitch);
        camera.direction[2] = sin(radian_yaw) * cos(radian_pitch);
        vec3 front;
        vec3_norm(front, camera.direction);

        processInput(window, delta_time, camera.position, front, up);

        vec3_add(tmp, camera.position, front);
        mat4x4_look_at(view, camera.position, tmp, up);

        // glUniform3f(lightLoc, lightColor[0], lightColor[1], lightColor[2]);
        // glUniform3f(colorLoc, toyColor[0], toyColor[1], toyColor[2]);
        glUseProgram(program);
        mat4x4 m, scaled;
        mat4x4_identity(m);
        mat4x4_translate(m, cube_pos[0], cube_pos[1], cube_pos[2]);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *)m);
        // glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat *)view);
        // glUniformMatrix4fv(projectionLoc, 1, GL_FALSE,
        //                    (const GLfloat *)projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}