#include <stdint.h>
#include <stdlib.h>

#include "renderer.h"
#include "log.h"

void renderer_init(renderer_t *renderer, SDL_Window *window, SDL_Renderer *sdl_renderer, SDL_GLContext *gl_context)
{
    renderer->window = window;
    renderer->sdl_renderer = sdl_renderer;
    renderer->gl_context = gl_context;

    glViewport(0, 0, 800, 600);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderer_render(renderer_t *renderer)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer_swap(renderer_t *renderer)
{
    SDL_GL_SwapWindow(renderer->window);
}

void renderer_monochrome_opaque_quad(renderer_t *renderer, uint32_t *args)
{
    renderer_render(renderer);

    int16_t v0_x = (int16_t) (args[1] & 0xFFFF);
    int16_t v0_y = (int16_t) (args[1] >> 16) & 0xFFFF;

    int16_t v1_x = (int16_t) (args[2] & 0xFFFF);
    int16_t v1_y = (int16_t) (args[2] >> 16) & 0xFFFF;

    int16_t v2_x = (int16_t) (args[3] & 0xFFFF);
    int16_t v2_y = (int16_t) (args[3] >> 16) & 0xFFFF;

    int16_t v3_x = (int16_t) (args[4] & 0xFFFF);
    int16_t v3_y = (int16_t) (args[4] >> 16) & 0xFFFF;

    float vertices[] =
    {
        v3_x, v3_y, 0.0, // top right corner
        v2_x, v2_y, 0.0, // top left corner
        v0_x, v0_y, 0.0, // bottom left corner
        v1_x, v1_y, 0.0 // bottom right corner
    };

    printf("\nv0_x: %d v0_y: %d\n", v0_x, v0_y);
    printf("v1_x: %d v1_y: %d\n", v1_x, v1_y);
    printf("v2_x: %d v2_y: %d\n", v2_x, v2_y);
    printf("v3_x: %d v3_y: %d\n", v3_x, v3_y);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_QUADS, 0, 4 );
    glDisableClientState(GL_VERTEX_ARRAY);

    renderer_swap(renderer);
}