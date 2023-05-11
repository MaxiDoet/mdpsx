#ifndef _renderer_h
#define _renderer_h

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct renderer_t {
    GLFWwindow *window;
} renderer_t;

void renderer_init(renderer_t *renderer, GLFWwindow *window);
void renderer_render(renderer_t *renderer);

#endif