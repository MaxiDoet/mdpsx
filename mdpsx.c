#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "cpu/r3000.h"
#include "bus/bus.h"
#include "renderer/renderer.h"
#include "log.h"

bool running = true;
r3000_state_t r3000_state;
bus_state_t bus_state;
renderer_t renderer;

/*
typedef struct compute_thread_args_t {
    r3000_state_t *r3000_state;
    bus_state_t *bus_state;
} compute_thread_args_t;

void *compute_thread(void *args) {
    compute_thread_args_t *compute_thread_args = (compute_thread_args_t *) args;
    
    while(running) {
        r3000_step(compute_thread_args->r3000_state, compute_thread_args->bus_state);
    }
}
*/

int main()
{
    /* Read BIOS */
    FILE *bios_fp = fopen("bios/bios.bin", "rb");
    bus_state.bios = (uint8_t *) malloc(524288);
    fread(bus_state.bios, 1, 524288, bios_fp);
    fclose(bios_fp);

    uint32_t bios_date = *((uint32_t *) (bus_state.bios + 0x100));
    log_info("mdpsx", "Bios | Date: %x/%x/%x\n", (bios_date & 0x0000FF00) >> 8, bios_date & 0x000000FF, (bios_date & 0xFFFF0000) >> 16);

    /* Init CPU */
    r3000_state.pc_instruction = 0xBFC00000;
    r3000_state.pc = 0xBFC00000;
    r3000_state.pc_next = 0xBFC00004;

    /* Init memory */
    bus_state.ram = (uint8_t *) malloc(2048 * 1024);
    bus_state.scratchpad = (uint8_t *) malloc(1024);

    r3000_state.debug_enabled = &bus_state.debug_enabled;

    /* Init SDL */
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        log_error("mdpsx", "Failed to init SDL");
        exit(0);
    }

    /* Create window */
    SDL_Window *window = SDL_CreateWindow("mdpsx", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);

    if (!window) {
        log_error("mdpsx", "Failed to init window");
        exit(0);
    }

    /* Set window icon */
    SDL_Surface *icon_surface = IMG_Load("./assets/icon.png");

    SDL_SetWindowIcon(window, icon_surface);

    /* Create SDL render context*/
    SDL_Renderer *sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    /* Create OpenGL context */
    SDL_GLContext *gl_context = SDL_GL_CreateContext(window);

    renderer_init(&renderer, window, sdl_renderer, gl_context);
    bus_state.gpu_state.renderer = &renderer;

    /* Init seperate compute thread*/
    //compute_thread_args_t compute_thread_args;
    //compute_thread_args.r3000_state = &r3000_state;
    //compute_thread_args.bus_state = &bus_state;
    //pthread_t compute_thread_id;
    //pthread_create(&compute_thread_id, NULL, compute_thread, &compute_thread_args);
    //pthread_join(compute_thread_id, NULL);

    SDL_Event event;
    while(running) {
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        for (uint32_t i=0; i < 10000; i++) {
            r3000_step(&r3000_state, &bus_state);
        }

        //renderer_render(&renderer);
        //renderer_swap(&renderer);
    }
}