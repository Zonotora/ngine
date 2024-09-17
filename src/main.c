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
#include "linmath.h"
#include "window.h"

typedef struct Vertex {
    vec3 pos;
    vec3 col;
} Vertex;

static float x = 0.0;

static const Vertex vertices[8] = {
    {{-0.5f, 0.5f, 0.5f}, {1.f, 0.f, 0.f}},
    {{-0.5f, -0.5f, 0.5f}, {1.f, 0.f, 0.f}},
    {{0.5f, -0.5f, 0.5f}, {1.f, 0.f, 0.f}},
    {{0.5f, 0.5f, 0.5f}, {1.f, 0.f, 0.f}},
    {{-0.5f, 0.5f, -0.5f}, {1.f, 0.f, 0.f}},
    {{-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f}},
    {{0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f}},
    {{0.5f, 0.5f, -0.5f}, {1.f, 0.f, 0.f}},
};

static const char *vertex_shader_text =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;      // Vertex position\n"
    "layout (location = 1) in vec3 aColor;    // Color\n"
    "layout (location = 2) in vec2 aTexCoord; // Texture coordinate\n"
    "\n"
    "out vec3 ourColor;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "    ourColor = aColor;"
    "    TexCoord = aTexCoord;\n"
    "}\n";

static const char *fragment_shader_text =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "\n"
    "in vec3 ourColor;\n"
    "in vec2 TexCoord;\n"
    "\n"
    "uniform sampler2D texture1;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(texture1, TexCoord) * vec4(ourColor, 1.0);\n"
    "}\n";

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // Input handling
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        // Move player left
        x -= 0.01f;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        // Move player right
        x += 0.01f;
    }
}

static void mouse_callback(GLFWwindow *window, int button, int action,
                           int mods) {
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        printf("clicked\n");
        fflush(stdout);
    }
}

float last_x = 400 / 2;
float last_y = 300 / 2;
float yaw = 0.0f, pitch = 0.0f;
static void cursor_callback(GLFWwindow *window, double xpos, double ypos)
{
    float offset_x = xpos - last_x;
    float offset_y = ypos - last_y;
    last_x = xpos;
    last_y = ypos;

    const float sensitivity = 0.001f;
    offset_x += sensitivity;
    offset_y += sensitivity;

    yaw += offset_x;
    pitch += offset_y;

    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;
}

void processInput(GLFWwindow *window, float delta_time, vec3 pos, vec3 front, vec3 up) {
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

int main(void) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = window_init();

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetCursorPosCallback(window, cursor_callback);

    // NOTE: OpenGL error checks have been omitted for brevity

    int width, height, nrChannels;
    unsigned char *data =
        stbi_load("container.jpg", &width, &height, &nrChannels, 0);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    // float vertices[] = {
    // // positions        // colors          // texture coords
    //  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, // top right
    //  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom right
    // -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // bottom left
    // -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f,  0.0f, 1.0f  // top left
    // };
    vec3 cubePositions[] = {{0.0f, 0.0f, 0.0f},    {2.0f, 5.0f, -15.0f},
                            {-1.5f, -2.2f, -2.5f}, {-3.8f, -2.0f, -12.3f},
                            {2.4f, -0.4f, -3.5f},  {-1.7f, 3.0f, -7.5f},
                            {1.3f, -2.0f, -2.5f},  {1.5f, 2.0f, -2.5f},
                            {1.5f, 0.2f, -1.5f},   {-1.3f, 1.0f, -1.5f}};

    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f,
        -0.5f, 1.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,
        0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  -0.5f, -0.5f,
        0.5f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  1.0f,
        0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f, 0.5f,  0.5f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  -0.5f, -0.5f,
        0.5f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  1.0f,
        0.0f,  1.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 1.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  -0.5f, -0.5f,
        0.5f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  1.0f,
        0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  1.0f,
        1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.5f,  -0.5f,
        -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,
        0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  1.0f,
        1.0f,  0.0f,  -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  -0.5f,
        0.5f,  1.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  1.0f,
        0.0f,  1.0f,  1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  1.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, 0.5f,  -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.5f,  0.5f,
        -0.5f, 1.0f,  0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  1.0f,
        0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  1.0f,
        1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        -0.5f, 0.5f,  -0.5f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f};
    // unsigned int indices[] = {  // Note that we start from 0!
    //     0, 1, 3,   // First triangle
    //     1, 2, 3    // Second triangle
    //     0, 1, 3,   // First triangle
    //     1, 2, 3    // Second triangle
    // };

    // Generate buffers and arrays
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind and set vertex buffers and attributes
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
    // GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    // Position attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

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

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    mat4x4 model, view, projection;
    mat4x4_identity(model);
    mat4x4_identity(view);
    mat4x4_identity(projection);
    // mat4x4_translate(trans, 0.5f, -0.5f, 0.0f);
    mat4x4_rotate_X(model, model, -20.0f);
    mat4x4_translate(view, 0.0f, 0.0f, -3.0f);
    mat4x4_perspective(projection, 1.57f, 640.0f / 480.0f, 0.1f, 100.0f);
    unsigned int modelLoc = glGetUniformLocation(program, "model");
    unsigned int viewLoc = glGetUniformLocation(program, "view");
    unsigned int projectionLoc = glGetUniformLocation(program, "projection");

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Camera camera = camera_init();
    camera.position[2] = 3.0f;
    vec3 target, direction, tmp, right;
    vec3 up = {0.0f, 1.0f, 0.0f};
    vec3_sub(tmp, camera.position, target);
    vec3_norm(direction, tmp);
    vec3_mul_cross(tmp, up, direction);
    vec3_norm(right, tmp);
    vec3_mul_cross(tmp, direction, right);
    vec3 eye = {0.0f, 0.0f, 0.0f};
    float delta_time = 0.0f, last_frame = 0.0f;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float)height;

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
        vec3 front;;
        vec3_norm(front, camera.direction);

        processInput(window, delta_time, camera.position, front, up);

        glUseProgram(program);
        // const float radius = 10.0f;
        // float camX = sin(glfwGetTime()) * radius;
        // float camZ = cos(glfwGetTime()) * radius;

        vec3_add(tmp, camera.position, front);
        mat4x4_look_at(view, camera.position, tmp, up);

        for (unsigned int i = 0; i < 10; i++) {
            mat4x4 m;
            mat4x4_identity(m);
            float x = cubePositions[i][0];
            float y = cubePositions[i][1];
            float z = cubePositions[i][2];
            mat4x4_translate(m, x, y, z);

            float angle = i;
            mat4x4_rotate_Z(m, m, (float)glfwGetTime() * angle);
            mat4x4_rotate_X(m, m, (float)glfwGetTime());

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *)m);
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat *)view);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE,
                           (const GLfloat *)projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(program);

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}