#ifndef _renderer_h
#define _renderer_h

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "GL/glu.h"

typedef struct renderer_t {
    SDL_Window *window;
    SDL_Renderer *sdl_renderer;
    SDL_GLContext *gl_context;
} renderer_t;

void renderer_init(renderer_t *renderer, SDL_Window *window, SDL_Renderer *sdl_renderer, SDL_GLContext *gl_context);
void renderer_render(renderer_t *renderer);
void renderer_swap(renderer_t *renderer);

void renderer_monochrome_opaque_quad(renderer_t *renderer, uint32_t *args);

#endif