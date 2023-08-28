#ifndef _gpu_h
#define _gpu_h

#include <stdbool.h>

#include "renderer/renderer.h"

#define GPU_COMMAND_BUFFER_SIZE     10

#define GPU_STATE_WAITING_FOR_CMD       0
#define GPU_STATE_WAITING_FOR_ARG       1
#define GPU_STATE_WAITING_FOR_VRAM_DATA 2

typedef struct gpu_state_t {
    renderer_t *renderer;

    uint8_t *vram;

    uint32_t vram_transfer_loc_x;
    uint32_t vram_transfer_loc_y;

    uint16_t horizontal_resolution;
    uint16_t vertical_resolution;
    bool pal;
    uint8_t depth_24bit;
    bool display_enable;
    uint8_t dma_direction;

    uint32_t state;
    uint32_t command_buf[10];
    uint8_t command_buf_index;
    uint32_t command_buf_left;
} gpu_state_t;

void gpu_write(gpu_state_t *gpu_state, uint32_t addr, uint32_t value);
uint32_t gpu_read(gpu_state_t *gpu_state, uint32_t addr);

void gpu_send_gp0_command(gpu_state_t *gpu_state, uint32_t command);
void gpu_send_gp1_command(gpu_state_t *gpu_state, uint32_t command);

#endif