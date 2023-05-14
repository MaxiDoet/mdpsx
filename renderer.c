#include <stdint.h>
#include <stdlib.h>

#include "renderer.h"
#include "log.h"

void renderer_init(renderer_t *renderer, GLFWwindow *window)
{
    renderer->window = window;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        log_error("renderer", "Failed to load glad");
        exit(0);
    }
}

void renderer_render(renderer_t *renderer)
{    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}