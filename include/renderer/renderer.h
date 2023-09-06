#ifndef _renderer_h
#define _renderer_h

#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include "GL/glu.h"

typedef struct vertex_t {
    GLshort x;
    GLshort y;

    GLubyte r;
    GLubyte g;
    GLubyte b;

    GLshort u;
    GLshort v;
} vertex_t;

typedef struct renderer_entry_data_t {
    GLshort positions[6];
    GLfloat uvs[6];
    GLfloat colors[9];
} renderer_entry_data_t;

typedef struct renderer_entry_t {    
    renderer_entry_data_t data;

    GLuint vbo;
    GLuint vao;
    GLuint texture;
} renderer_entry_t;

typedef struct renderer_t {
    SDL_Window *window;
    SDL_Renderer *sdl_renderer;
    SDL_GLContext *gl_context;

    GLuint vbo;
    GLuint vao;

    uint16_t vram[1024][512];

    uint32_t program;
    //vertex_t vertex_buffer[1000];
    renderer_entry_t entries[1000];
    uint32_t entries_index;
    GLuint texture_id;
} renderer_t;

void renderer_init(renderer_t *renderer, SDL_Window *window, SDL_Renderer *sdl_renderer, SDL_GLContext *gl_context);
void renderer_render(renderer_t *renderer);

void renderer_monochrome_opaque_quad(renderer_t *renderer, uint32_t *args);
void renderer_textured_blend_quad(renderer_t *renderer, uint32_t *args);
void renderer_gouraud_triangle(renderer_t *renderer, uint32_t *args);
void renderer_gouraud_quad(renderer_t *renderer, uint32_t *args);

#endif