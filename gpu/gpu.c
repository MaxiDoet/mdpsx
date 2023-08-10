#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpu/gpu.h"
#include "log.h"

void gpu_write(gpu_state_t *gpu_state, uint32_t addr, uint32_t value)
{
    if (addr == 0x1F801810) {
        // GP0
        gpu_send_gp0_command(gpu_state, value);
    } else if (addr == 0x1F801814) {
        // GP1
        gpu_send_gp1_command(gpu_state, value);
    }
}

uint32_t gpu_read(gpu_state_t *gpu_state, uint32_t addr)
{
    // GPUSTAT
    if (addr == 0x1F801814) {
        #ifdef LOG_DEBUG_BUS_READ
        log_debug("GPU", "%08x <- GPUSTAT\n", 0x1C000000);
        #endif
 
        return 0x1C000000;
    }
}

void gpu_send_gp0_command(gpu_state_t *gpu_state, uint32_t command)
{
    #ifdef LOG_DEBUG_GPU_COMMANDS
    log_debug("GPU", "GP0 %s (%08X) | ", (gpu_state->state == GPU_STATE_WAITING_FOR_CMD) ? "CMD" : "ARG", command);
    #endif    

    if (gpu_state->state == GPU_STATE_WAITING_FOR_CMD){
        gpu_state->command_buf[0] = command;

        uint8_t type = (command >> 24) & 0xFF;

        switch(type) {
            // NOP
            case 0x00:
                printf("NOP");
                break;

            // Clear cache
            case 0x01:
                printf("Clear cache");
                break;

            // Monochrome Opaque Quad
            case 0x28:
                printf("Monochrome Opaque Quad");

                gpu_state->command_buf_left = 4;
                gpu_state->state = GPU_STATE_WAITING_FOR_ARG;

                break;

            // CPU->VRAM
            case 0xA0:
                printf("CPU->VRAM");

                // These are just the first two arguments we need to determine how much data we wait for
                gpu_state->command_buf_left = 2;
                gpu_state->state = GPU_STATE_WAITING_FOR_ARG;

                break;

            // VRAM->CPU
            case 0xC0:
                printf("VRAM->CPU");
                break;

            // Texpage
            case 0xE1:
                printf("Texpage");
                break;

            // Set Drawing Offset
            case 0xE5:
                printf("Render");

                
                break;
        }
    } else if (gpu_state->state == GPU_STATE_WAITING_FOR_ARG) {
        gpu_state->command_buf[++gpu_state->command_buf_index] = command;

        gpu_state->command_buf_left--;

        // If all arguments are in the buffer we can switch back to receiving the next command
        if (gpu_state->command_buf_left == 0) {
            gpu_state->state = GPU_STATE_WAITING_FOR_CMD;
            gpu_state->command_buf_index = 0;

            switch((gpu_state->command_buf[0] >> 24) & 0xFF) {
                // Monochrome Opaque Quad
                case 0x28:
                    renderer_monochrome_opaque_quad(gpu_state->renderer, gpu_state->command_buf);
                    break;

                // CPU->VRAM
                case 0xA0:
                    // Now that we have all arguments we can calculate how much data arguments we need
                    uint16_t width = gpu_state->command_buf[2] & 0x0000FFFF;
                    uint16_t height = gpu_state->command_buf[2] & 0xFFFF0000;

                    gpu_state->command_buf_left = width * height / 2;
                    gpu_state->state = GPU_STATE_WAITING_FOR_VRAM_DATA;

                    break;
            }
        }
    } else if (gpu_state->state == GPU_STATE_WAITING_FOR_VRAM_DATA) {
        if (gpu_state->command_buf_left == 0) {
            // Done Loading
            printf("Done!!!!\n");

            gpu_state->state == GPU_STATE_WAITING_FOR_CMD;
        }
    }

    printf("\n");
}

void gpu_gp1_display_mode(gpu_state_t *gpu_state, uint32_t command)
{
    uint16_t hor_resolutions[] = {256, 320, 512, 640};
    uint16_t hor_resolution = hor_resolutions[command & 0x3];

    #ifdef LOG_DEBUG_GPU_COMMANDS
    printf("Display mode | Horizontal Resolution: %d\n", hor_resolution);
    #endif
}

void gpu_gp1_dma_direction(gpu_state_t *gpu_state, uint32_t command)
{
    char *directions[] = {"Off", "FIFO", "CPU -> GP0", "GPU -> CPU"};
    char *direction = directions[command & 0x3];

    #ifdef LOG_DEBUG_GPU_COMMANDS
    printf("DMA Direction | %s\n", direction);
    #endif
}

void gpu_send_gp1_command(gpu_state_t *gpu_state, uint32_t command)
{
    uint8_t type = (command >> 24);

    #ifdef LOG_DEBUG_GPU_COMMANDS
    log_debug("GPU", "GP1 CMD (%08X) | ", command);
    #endif

    switch(type) {
        // Reset
        case 0x00:
            printf("Reset\n");
            break;

        // DMA Direction
        case 0x04:
            gpu_gp1_dma_direction(gpu_state, command);
            break;

        // Display mode
        case 0x08:
            gpu_gp1_display_mode(gpu_state, command);
            break;

        default:
            printf("\n");
            break;
    }
}

