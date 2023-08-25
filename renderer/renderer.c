#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include "renderer/renderer.h"
#include "log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include "GL/glu.h"

#define SHADER_TYPE_VERTEX      0
#define SHADER_TYPE_FRAGMENT    1

uint32_t renderer_compile_shader(const char *source, const GLint *size, uint8_t shader_type)
{
    uint32_t shader;

    if (shader_type == SHADER_TYPE_VERTEX) {
        shader = glCreateShader(GL_VERTEX_SHADER);
    } else {
        shader = glCreateShader(GL_FRAGMENT_SHADER);
    }

    glShaderSource(shader, 1, &source, size);

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "Compiling %s shader (%d bytes)", (shader_type == SHADER_TYPE_VERTEX) ? "vertex" : "fragment", *size);
    #endif

    glCompileShader(shader);

    // Check if shader was compiled successfully
    int success;
    char log[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    #ifdef LOG_DEBUG_RENDERER
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);

        printf(" | %s", log);
    } else {
        printf("\n");
    }
    #endif

    return shader;
}

uint32_t renderer_create_program(char *vertex_path, char *fragment_path)
{
    // Read shader file
    FILE *vertex_fp = fopen(vertex_path, "r");
    FILE *fragment_fp = fopen(fragment_path, "r");

    fseek(vertex_fp, 0, SEEK_END);
    uint32_t vertex_source_size = ftell(vertex_fp);
    fseek(vertex_fp, 0, SEEK_SET);
    fseek(fragment_fp, 0, SEEK_END);
    uint32_t fragment_source_size = ftell(fragment_fp);
    fseek(fragment_fp, 0, SEEK_SET);

    char *vertex_source = (char *) malloc(sizeof(char *) * vertex_source_size);
    fread(vertex_source, 1, vertex_source_size, vertex_fp);
    char *fragment_source = (char *) malloc(sizeof(char *) * fragment_source_size);
    fread(fragment_source, 1, fragment_source_size, fragment_fp);

    uint32_t vertex_shader = renderer_compile_shader(vertex_source, (const GLint *) &vertex_source_size, SHADER_TYPE_VERTEX);
    uint32_t fragment_shader = renderer_compile_shader(fragment_source, (const GLint *) &fragment_source_size, SHADER_TYPE_FRAGMENT);

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "Creating shader program (%s, %s)", vertex_path, fragment_path);
    #endif

    int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int success;
    char log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    #ifdef LOG_DEBUG_RENDERER
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, log);
        printf(" | %s\n", log);
    } else {
        printf("\n");
    }
    #endif

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void renderer_init(renderer_t *renderer, SDL_Window *window, SDL_Renderer *sdl_renderer, SDL_GLContext *gl_context)
{
    renderer->window = window;
    renderer->sdl_renderer = sdl_renderer;
    renderer->gl_context = gl_context;

    glewInit();

    uint32_t program = renderer_create_program("renderer/vertex.glsl", "renderer/fragment.glsl");
    glUseProgram(program);

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
    int16_t v0_x = (int16_t) (args[1] & 0xFFFF);
    int16_t v0_y = (int16_t) (args[1] >> 16) & 0xFFFF;

    int16_t v1_x = (int16_t) (args[2] & 0xFFFF);
    int16_t v1_y = (int16_t) (args[2] >> 16) & 0xFFFF;

    int16_t v2_x = (int16_t) (args[3] & 0xFFFF);
    int16_t v2_y = (int16_t) (args[3] >> 16) & 0xFFFF;

    int16_t v3_x = (int16_t) (args[4] & 0xFFFF);
    int16_t v3_y = (int16_t) (args[4] >> 16) & 0xFFFF;

    uint32_t first_color_r = args[0] & 0xFF;
    uint32_t first_color_g = (args[0] >> 8) & 0xFF;
    uint32_t first_color_b = (args[0] >> 16) & 0xFF;

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "Monochrome Opaque Quad | v0: %d, %d v1: %d, %d v2: %d, %d v3: %d, %d\n", v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, v3_x, v3_y);
    #endif

    float vertices[] =
    {
        v3_x, v3_y, 0.0,
        v2_x, v2_y, 0.0,
        v0_x, v0_y, 0.0,
        v1_x, v1_y, 0.0
    };

    glColor3ub(first_color_r, first_color_g, first_color_b);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_QUADS, 0, 4 );
    glDisableClientState(GL_VERTEX_ARRAY);
}

void renderer_textured_blend_quad(renderer_t *renderer, uint32_t *args)
{
    int16_t v0_x = (int16_t) (args[1] & 0xFFFF);
    int16_t v0_y = (int16_t) (args[1] >> 16) & 0xFFFF;

    int16_t v1_x = (int16_t) (args[3] & 0xFFFF);
    int16_t v1_y = (int16_t) (args[3] >> 16) & 0xFFFF;

    int16_t v2_x = (int16_t) (args[5] & 0xFFFF);
    int16_t v2_y = (int16_t) (args[5] >> 16) & 0xFFFF;

    int16_t v3_x = (int16_t) (args[7] & 0xFFFF);
    int16_t v3_y = (int16_t) (args[7] >> 16) & 0xFFFF;

    uint32_t first_color_r = args[0] & 0xFF;
    uint32_t first_color_g = (args[0] >> 8) & 0xFF;
    uint32_t first_color_b = (args[0] >> 16) & 0xFF;

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "Textured Blend Quad | v0: %d, %d v1: %d, %d v2: %d, %d v3: %d, %d\n", v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, v3_x, v3_y);
    #endif

    float vertices[] =
    {
        v3_x, v3_y, 0.0,
        v2_x, v2_y, 0.0,
        v0_x, v0_y, 0.0,
        v1_x, v1_y, 0.0
    };

    glColor3ub(first_color_r, first_color_g, first_color_b);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_QUADS, 0, 4 );
    glDisableClientState(GL_VERTEX_ARRAY);
}

void renderer_gouraud_triangle(renderer_t *renderer, uint32_t *args)
{
    int16_t v0_x = (int16_t) (args[1] & 0xFFFF);
    int16_t v0_y = (int16_t) (args[1] >> 16) & 0xFFFF;

    int16_t v1_x = (int16_t) (args[3] & 0xFFFF);
    int16_t v1_y = (int16_t) (args[3] >> 16) & 0xFFFF;

    int16_t v2_x = (int16_t) (args[5] & 0xFFFF);
    int16_t v2_y = (int16_t) (args[5] >> 16) & 0xFFFF;

    uint32_t first_color_r = args[0] & 0xFF + 10;
    uint32_t first_color_g = (args[0] >> 8) & 0xFF + 10;
    uint32_t first_color_b = (args[0] >> 16) & 0xFF + 10;

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "Gouraud Triangle | v0: %d, %d v1: %d, %d v2: %d, %d\n", v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);
    #endif

    float vertices[] =
    {
        v2_x, v2_y, 0.0,
        v0_x, v0_y, 0.0,
        v1_x, v1_y, 0.0
    };

    glColor3ub(first_color_r, first_color_g, first_color_b);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void renderer_gouraud_quad(renderer_t *renderer, uint32_t *args)
{
    int16_t v0_x = (int16_t) (args[1] & 0xFFFF);
    int16_t v0_y = (int16_t) (args[1] >> 16) & 0xFFFF;

    int16_t v1_x = (int16_t) (args[3] & 0xFFFF);
    int16_t v1_y = (int16_t) (args[3] >> 16) & 0xFFFF;

    int16_t v2_x = (int16_t) (args[5] & 0xFFFF);
    int16_t v2_y = (int16_t) (args[5] >> 16) & 0xFFFF;

    int16_t v3_x = (int16_t) (args[7] & 0xFFFF);
    int16_t v3_y = (int16_t) (args[7] >> 16) & 0xFFFF;

    uint32_t first_color_r = args[0] & 0xFF;
    uint32_t first_color_g = (args[0] >> 8) & 0xFF;
    uint32_t first_color_b = (args[0] >> 16) & 0xFF;

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "Gouraud Quad | v0: %d, %d v1: %d, %d v2: %d, %d v3: %d, %d\n", v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, v3_x, v3_y);
    #endif

    float vertices[] =
    {
        v3_x, v3_y, 0.0,
        v2_x, v2_y, 0.0,
        v0_x, v0_y, 0.0,
        v1_x, v1_y, 0.0
    };

    glColor3ub(first_color_r, first_color_g, first_color_b);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}