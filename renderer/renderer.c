#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "renderer/renderer.h"
#include "log.h"

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

/*
void renderer_push_vertex(renderer_t *renderer, GLshort x, GLshort y, uint8_t r, uint8_t g, uint8_t b)
{
    renderer->vertex_buffer[renderer->vertex_buffer_index].x = x;
    renderer->vertex_buffer[renderer->vertex_buffer_index].y = y;
    renderer->vertex_buffer[renderer->vertex_buffer_index].r = r;
    renderer->vertex_buffer[renderer->vertex_buffer_index].g = g;
    renderer->vertex_buffer[renderer->vertex_buffer_index++].b = b;
}
*/

void renderer_push(renderer_t *renderer, renderer_entry_data_t data)
{
    renderer_entry_t *entry = &renderer->entries[renderer->entries_index++];

    entry->data = data;

    // Create and bind VBO
    glGenBuffers(1, &entry->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, entry->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderer_entry_data_t), &entry->data, GL_STATIC_DRAW);

    // Create and bind VAO and VAs
    glGenVertexArrays(1, &entry->vao);
    glBindVertexArray(entry->vao);

    // v_pos
    glEnableVertexAttribArray(0);
    GLint v_pos = glGetAttribLocation(renderer->program, "v_pos");
    glVertexAttribIPointer(v_pos, 2, GL_SHORT, 0, (void*) offsetof(renderer_entry_data_t, positions));

    // v_col
    //glEnableVertexAttribArray(1);
    //GLint v_col = glGetAttribLocation(renderer->program, "v_color");
    //glVertexAttribPointer(v_col, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*) offsetof(renderer_entry_data_t, colors));

    glEnableVertexAttribArray(1);
    GLint v_col = glGetAttribLocation(renderer->program, "v_color");
    glVertexAttribPointer(v_col, 3, GL_FLOAT, GL_FALSE, 0, (void*) offsetof(renderer_entry_data_t, colors));

    // v_uv
    //glEnableVertexAttribArray(2);
    //glVertexAttribIPointer(2, 2, GL_SHORT, 0, (void*) offsetof(renderer_entry_data_t, uvs));

    glBindVertexArray(0);
}

void renderer_init(renderer_t *renderer, SDL_Window *window, SDL_Renderer *sdl_renderer, SDL_GLContext *gl_context)
{
    renderer->window = window;
    renderer->sdl_renderer = sdl_renderer;
    renderer->gl_context = gl_context;

    glewInit();

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "OpenGL Version: %s | Vendor: %s\n", glGetString(GL_VERSION), glGetString(GL_VENDOR));
    #endif

    renderer->program = renderer_create_program("renderer/vertex.glsl", "renderer/fragment.glsl");
    glUseProgram(renderer->program);
}

void renderer_render(renderer_t *renderer)
{
    /*
    glUseProgram(renderer->program);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->vertex_buffer), renderer->vertex_buffer, GL_STATIC_DRAW);

    // v_pos
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 2, GL_SHORT, sizeof(vertex_t), (void*) offsetof(vertex_t, x));

    // v_col
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex_t), (void*) offsetof(vertex_t, r));

    // v_uv
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 2, GL_SHORT, sizeof(vertex_t), (void*) offsetof(vertex_t, u));

    glDrawArrays(GL_TRIANGLES, 0, renderer->vertex_buffer_index);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    */

    glUseProgram(renderer->program);

    for (uint32_t i=0; i < renderer->entries_index; i++) {
        renderer_entry_t *entry = &renderer->entries[i];

        glBindVertexArray(entry->vao);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    SDL_GL_SwapWindow(renderer->window);

    renderer->entries_index = 0;
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

    uint8_t first_color_r = args[0] & 0xFF;
    uint8_t first_color_g = (args[0] >> 8) & 0xFF;
    uint8_t first_color_b = (args[0] >> 16) & 0xFF;

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "Monochrome Opaque Quad | v0: %d, %d v1: %d, %d v2: %d, %d v3: %d, %d\n", v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, v3_x, v3_y);
    #endif

    /*
    renderer_push_vertex(renderer, v0_x, v0_y, first_color_r, first_color_g, first_color_b);
    renderer_push_vertex(renderer, v1_x, v1_y, first_color_r, first_color_g, first_color_b);
    renderer_push_vertex(renderer, v2_x, v2_y, first_color_r, first_color_g, first_color_b);

    renderer_push_vertex(renderer, v1_x, v1_y, first_color_r, first_color_g, first_color_b);
    renderer_push_vertex(renderer, v2_x, v2_y, first_color_r, first_color_g, first_color_b);
    renderer_push_vertex(renderer, v3_x, v3_y, first_color_r, first_color_g, first_color_b);
    */

    renderer_entry_data_t data0;
    renderer_entry_data_t data1;

    data0.positions[0] = v0_x;
    data0.positions[1] = v0_y;
    data0.positions[2] = v1_x;
    data0.positions[3] = v1_y;
    data0.positions[4] = v2_x;
    data0.positions[5] = v2_y;

    data1.positions[0] = v1_x;
    data1.positions[1] = v1_y;
    data1.positions[2] = v2_x;
    data1.positions[3] = v2_y;
    data1.positions[4] = v3_x;
    data1.positions[5] = v3_y;

    data0.colors[0] = (float) (first_color_r / 255.0f);
    data0.colors[1] = (float) (first_color_g / 255.0f);
    data0.colors[2] = (float) (first_color_b / 255.0f);
    data0.colors[3] = (float) (first_color_r / 255.0f);
    data0.colors[4] = (float) (first_color_g / 255.0f);
    data0.colors[5] = (float) (first_color_b / 255.0f);
    data0.colors[6] = (float) (first_color_r / 255.0f);
    data0.colors[7] = (float) (first_color_g / 255.0f);
    data0.colors[8] = (float) (first_color_b / 255.0f);

    data1.colors[0] = (float) (first_color_r / 255.0f);
    data1.colors[1] = (float) (first_color_g / 255.0f);
    data1.colors[2] = (float) (first_color_b / 255.0f);
    data1.colors[3] = (float) (first_color_r / 255.0f);
    data1.colors[4] = (float) (first_color_g / 255.0f);
    data1.colors[5] = (float) (first_color_b / 255.0f);
    data1.colors[6] = (float) (first_color_r / 255.0f);
    data1.colors[7] = (float) (first_color_g / 255.0f);
    data1.colors[8] = (float) (first_color_b / 255.0f);

    renderer_push(renderer, data0);
    renderer_push(renderer, data1);
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

    /*
    renderer_push_vertex(renderer, v0_x, v0_y, first_color_r, first_color_g, first_color_b);
    renderer_push_vertex(renderer, v1_x, v1_y, first_color_r, first_color_g, first_color_b);
    renderer_push_vertex(renderer, v2_x, v2_y, first_color_r, first_color_g, first_color_b);

    renderer_push_vertex(renderer, v1_x, v1_y, first_color_r, first_color_g, first_color_b);
    renderer_push_vertex(renderer, v2_x, v2_y, first_color_r, first_color_g, first_color_b);
    renderer_push_vertex(renderer, v3_x, v3_y, first_color_r, first_color_g, first_color_b);
    */

    renderer_entry_data_t data0;
    renderer_entry_data_t data1;

    data0.positions[0] = v0_x;
    data0.positions[1] = v0_y;
    data0.positions[2] = v1_x;
    data0.positions[3] = v1_y;
    data0.positions[4] = v2_x;
    data0.positions[5] = v2_y;

    data1.positions[0] = v1_x;
    data1.positions[1] = v1_y;
    data1.positions[2] = v2_x;
    data1.positions[3] = v2_y;
    data1.positions[4] = v3_x;
    data1.positions[5] = v3_y;

    data0.colors[0] = (float) (first_color_r / 255.0f);
    data0.colors[1] = (float) (first_color_g / 255.0f);
    data0.colors[2] = (float) (first_color_b / 255.0f);
    data0.colors[3] = (float) (first_color_r / 255.0f);
    data0.colors[4] = (float) (first_color_g / 255.0f);
    data0.colors[5] = (float) (first_color_b / 255.0f);
    data0.colors[6] = (float) (first_color_r / 255.0f);
    data0.colors[7] = (float) (first_color_g / 255.0f);
    data0.colors[8] = (float) (first_color_b / 255.0f);

    data1.colors[0] = (float) (first_color_r / 255.0f);
    data1.colors[1] = (float) (first_color_g / 255.0f);
    data1.colors[2] = (float) (first_color_b / 255.0f);
    data1.colors[3] = (float) (first_color_r / 255.0f);
    data1.colors[4] = (float) (first_color_g / 255.0f);
    data1.colors[5] = (float) (first_color_b / 255.0f);
    data1.colors[6] = (float) (first_color_r / 255.0f);
    data1.colors[7] = (float) (first_color_g / 255.0f);
    data1.colors[8] = (float) (first_color_b / 255.0f);

    renderer_push(renderer, data0);
    renderer_push(renderer, data1);
}

void renderer_gouraud_triangle(renderer_t *renderer, uint32_t *args)
{
    int16_t v0_x = (int16_t) (args[1] & 0xFFFF);
    int16_t v0_y = (int16_t) (args[1] >> 16) & 0xFFFF;

    int16_t v1_x = (int16_t) (args[3] & 0xFFFF);
    int16_t v1_y = (int16_t) (args[3] >> 16) & 0xFFFF;

    int16_t v2_x = (int16_t) (args[5] & 0xFFFF);
    int16_t v2_y = (int16_t) (args[5] >> 16) & 0xFFFF;

    uint8_t v0_col_r = args[0] & 0xFF;
    uint8_t v0_col_g = (args[0] >> 8) & 0xFF;
    uint8_t v0_col_b = (args[0] >> 16) & 0xFF;

    uint8_t v1_col_r = args[2] & 0xFF;
    uint8_t v1_col_g = (args[2] >> 8) & 0xFF;
    uint8_t v1_col_b = (args[2] >> 16) & 0xFF;

    uint8_t v2_col_r = args[4] & 0xFF;
    uint8_t v2_col_g = (args[4] >> 8) & 0xFF;
    uint8_t v2_col_b = (args[4] >> 16) & 0xFF;

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "Gouraud Triangle | v0: %d, %d v1: %d, %d v2: %d, %d\n", v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);
    #endif

    /*
    renderer_push_vertex(renderer, v0_x, v0_y, v0_col_r, v0_col_g, v0_col_b);
    renderer_push_vertex(renderer, v1_x, v1_y, v1_col_r, v1_col_g, v1_col_b);
    renderer_push_vertex(renderer, v2_x, v2_y, v2_col_r, v2_col_g, v2_col_b);
    */

    renderer_entry_data_t data;

    data.positions[0] = v0_x;
    data.positions[1] = v0_y;
    data.positions[2] = v1_x;
    data.positions[3] = v1_y;
    data.positions[4] = v2_x;
    data.positions[5] = v2_y;

    data.colors[0] = (float) (v0_col_r / 255.0f);
    data.colors[1] = (float) (v0_col_g / 255.0f);
    data.colors[2] = (float) (v0_col_b / 255.0f);
    data.colors[3] = (float) (v1_col_r / 255.0f);
    data.colors[4] = (float) (v1_col_g / 255.0f);
    data.colors[5] = (float) (v1_col_b / 255.0f);
    data.colors[6] = (float) (v2_col_r / 255.0f);
    data.colors[7] = (float) (v2_col_g / 255.0f);
    data.colors[8] = (float) (v2_col_b / 255.0f);

    renderer_push(renderer, data);
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

    uint8_t v0_col_r = args[0] & 0xFF;
    uint8_t v0_col_g = (args[0] >> 8) & 0xFF;
    uint8_t v0_col_b = (args[0] >> 16) & 0xFF;

    uint8_t v1_col_r = args[2] & 0xFF;
    uint8_t v1_col_g = (args[2] >> 8) & 0xFF;
    uint8_t v1_col_b = (args[2] >> 16) & 0xFF;

    uint8_t v2_col_r = args[4] & 0xFF;
    uint8_t v2_col_g = (args[4] >> 8) & 0xFF;
    uint8_t v2_col_b = (args[4] >> 16) & 0xFF;

    uint8_t v3_col_r = args[6] & 0xFF;
    uint8_t v3_col_g = (args[6] >> 8) & 0xFF;
    uint8_t v3_col_b = (args[6] >> 16) & 0xFF;

    #ifdef LOG_DEBUG_RENDERER
    log_debug("RENDERER", "Gouraud Quad | v0: %d, %d v1: %d, %d v2: %d, %d v3: %d, %d\n", v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, v3_x, v3_y);
    #endif

    /*
    renderer_push_vertex(renderer, v0_x, v0_y, v0_col_r, v0_col_g, v0_col_b);
    renderer_push_vertex(renderer, v1_x, v1_y, v1_col_r, v1_col_g, v1_col_b);
    renderer_push_vertex(renderer, v2_x, v2_y, v2_col_r, v2_col_g, v2_col_b);

    renderer_push_vertex(renderer, v1_x, v1_y, v1_col_r, v1_col_g, v1_col_b);
    renderer_push_vertex(renderer, v2_x, v2_y,  v2_col_r, v2_col_g, v2_col_b);
    renderer_push_vertex(renderer, v3_x, v3_y, v3_col_r, v3_col_g, v3_col_b);
    */

    renderer_entry_data_t data0;
    renderer_entry_data_t data1;

    data0.positions[0] = v0_x;
    data0.positions[1] = v0_y;
    data0.positions[2] = v1_x;
    data0.positions[3] = v1_y;
    data0.positions[4] = v2_x;
    data0.positions[5] = v2_y;

    data1.positions[0] = v1_x;
    data1.positions[1] = v1_y;
    data1.positions[2] = v2_x;
    data1.positions[3] = v2_y;
    data1.positions[4] = v3_x;
    data1.positions[5] = v3_y;

    data0.colors[0] = (float) (v0_col_r / 255.0f);
    data0.colors[1] = (float) (v0_col_g / 255.0f);
    data0.colors[2] = (float) (v0_col_b / 255.0f);
    data0.colors[3] = (float) (v1_col_r / 255.0f);
    data0.colors[4] = (float) (v1_col_g / 255.0f);
    data0.colors[5] = (float) (v1_col_b / 255.0f);
    data0.colors[6] = (float) (v2_col_r / 255.0f);
    data0.colors[7] = (float) (v2_col_g / 255.0f);
    data0.colors[8] = (float) (v2_col_b / 255.0f);

    data1.colors[0] = (float) (v1_col_r / 255.0f);
    data1.colors[1] = (float) (v1_col_g / 255.0f);
    data1.colors[2] = (float) (v1_col_b / 255.0f);
    data1.colors[3] = (float) (v2_col_r / 255.0f);
    data1.colors[4] = (float) (v2_col_g / 255.0f);
    data1.colors[5] = (float) (v2_col_b / 255.0f);
    data1.colors[6] = (float) (v3_col_r / 255.0f);
    data1.colors[7] = (float) (v3_col_g / 255.0f);
    data1.colors[8] = (float) (v3_col_b / 255.0f);

    renderer_push(renderer, data0);
    renderer_push(renderer, data1);
}