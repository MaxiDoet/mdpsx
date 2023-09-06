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

uint32_t gpu_read_gpustat(gpu_state_t *gpu_state)
{
    uint32_t gpustat = 0;

    // Horizontal Resolution
    if (gpu_state->horizontal_resolution == 368) {
        gpustat |= (1 << 16);
    } else {
        switch(gpu_state->horizontal_resolution) {
            case 320:
                gpustat |= (1 << 17);
                break;
            
            case 512:
                gpustat |= (2 << 17);
                break;

            case 640:
                gpustat |= (3 << 17);
                break;
        }
    }

    // Vertical Resolution
    //if (gpu_state->vertical_resolution == 480) gpustat |= (1 << 19);

    // Video Mode
    if (gpu_state->pal) gpustat |= (1 << 20);

    // Color Depth
    if (gpu_state->depth_24bit) gpustat |= (1 << 21);

    // Display enable
    if (gpu_state->display_enable) gpustat |= (1 << 23);

    // Ready to receive cmds
    gpustat |= (1 << 26);

    // Ready to send vram data to cpu
    gpustat |= (1 << 27);

    // Ready to receive dma block
    gpustat |= (1 << 28);

    // DMA Direction
    switch(gpu_state->dma_direction) {
        case 0:
            gpustat |= (0 << 25);
            break;

        case 1:
            gpustat |= (1 << 25);
            break;

        case 2:
            gpustat |= ((gpustat & (1 << 28)) << 25);
            break;

        case 3:
            gpustat |= ((gpustat & (1 << 27)) << 25);
            break;
    }

    return gpustat;
}

uint32_t gpu_read(gpu_state_t *gpu_state, uint32_t addr)
{
    uint32_t result = 0;

    // GPUSTAT
    if (addr == 0x1F801814) {
        result = gpu_read_gpustat(gpu_state);

        #ifdef LOG_DEBUG_GPU_READ
        log_debug("GPU", "%08x <- GPUSTAT\n", result);
        #endif
    }

    return result;
}

void gpu_send_gp0_command(gpu_state_t *gpu_state, uint32_t command)
{
    if (gpu_state->state == GPU_STATE_WAITING_FOR_CMD){
        #ifdef LOG_DEBUG_GPU_COMMANDS
        log_debug("GPU", "GP0 CMD (%08X) | ", command);
        #endif

        gpu_state->command_buf[0] = command;

        uint8_t type = (command >> 24) & 0xFF;

        switch(type) {
            // NOP
            case 0x00:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("NOP");
                #endif

                break;

            // Clear cache
            case 0x01:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Clear cache");
                #endif

                break;

            // Monochrome Opaque Quad
            case 0x28:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Monochrome Opaque Quad");
                #endif

                gpu_state->command_buf_left = 4;
                gpu_state->state = GPU_STATE_WAITING_FOR_ARG;

                break;

            // Textured Blend Quad
            case 0x2C:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Textured Blend Quad");
                #endif

                gpu_state->command_buf_left = 8;
                gpu_state->state = GPU_STATE_WAITING_FOR_ARG;

                break;

            // Gouraud Triangle
            case 0x30:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Gouraud Triangle");
                #endif

                gpu_state->command_buf_left = 5;
                gpu_state->state = GPU_STATE_WAITING_FOR_ARG;

                break;

            // Gouraud Quad
            case 0x38:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Gouraud Quad");
                #endif

                gpu_state->command_buf_left = 7;
                gpu_state->state = GPU_STATE_WAITING_FOR_ARG;

                break;

            // CPU->VRAM
            case 0xA0:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("CPU->VRAM");
                #endif

                // These are just the first two arguments we need to determine how much data we wait for
                gpu_state->command_buf_left = 2;
                gpu_state->state = GPU_STATE_WAITING_FOR_ARG;

                break;

            // VRAM->CPU
            case 0xC0:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("VRAM->CPU");
                #endif
                
                break;

            // Texpage
            case 0xE1:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Texpage");
                #endif
                
                break;

            // Texture Window
            case 0xE2:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Texture Window");
                #endif
                
                break;

            // Drawing Area top left
            case 0xE3:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Drawing Area top left");
                #endif
                
                break;

            // Drawing Area bottom right
            case 0xE4:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Drawing Area bottom right");
                #endif

                break;

            // Set Drawing Offset
            case 0xE5:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Render");
                #endif
                
                renderer_render(gpu_state->renderer);

                break;

            // Masking Bit
            case 0xE6:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Masking bit");
                #endif
                
                break;

            default:
                #ifdef LOG_DEBUG_GPU_COMMANDS
                printf("Unknown command");
                #endif

                break;
        }

        #ifdef LOG_DEBUG_GPU_COMMANDS
        printf("\n");
        #endif
    } else if (gpu_state->state == GPU_STATE_WAITING_FOR_ARG) {
        #ifdef LOG_DEBUG_GPU_COMMANDS
        log_debug("GPU", "GP0 ARG (%08X)\n", command);
        #endif

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

                // Textured Blend Quad
                case 0x2C:
                    renderer_textured_blend_quad(gpu_state->renderer, gpu_state->command_buf);
                    break;

                // Gouraud Triangle
                case 0x30:
                    renderer_gouraud_triangle(gpu_state->renderer, gpu_state->command_buf);
                    break;

                // Gouraud Quad
                case 0x38:
                    renderer_gouraud_quad(gpu_state->renderer, gpu_state->command_buf);
                    break;

                // CPU->VRAM
                case 0xA0:
                    // Now that we have all arguments we can calculate how much data arguments we need
                    uint16_t width = gpu_state->command_buf[2] & 0x0000FFFF;
                    uint16_t height = (gpu_state->command_buf[2] >> 16) & 0x0000FFFF;

                    width = ((width - 1) & 0x3FF) + 1;
                    height = ((height - 1) & 0x3FF) + 1;

                    gpu_state->vram_transfer_loc_x = gpu_state->command_buf[1] & 0x0000FFFF;
                    gpu_state->vram_transfer_loc_y = (gpu_state->command_buf[1] >> 16) & 0x0000FFFF;

                    gpu_state->command_buf_left = width * height / 2;
                    gpu_state->state = GPU_STATE_WAITING_FOR_VRAM_DATA;

                    break;
            }
        }
    } else if (gpu_state->state == GPU_STATE_WAITING_FOR_VRAM_DATA) {
        gpu_state->command_buf_left--;

        #ifdef LOG_DEBUG_GPU_COMMANDS
        log_debug("GPU", "GP0 VRAM_DATA (%08X)\n", command);
        #endif

        gpu_state->renderer->vram[gpu_state->vram_transfer_loc_x][gpu_state->vram_transfer_loc_y] = command;

        if (gpu_state->command_buf_left == 0) {
            gpu_state->command_buf_index = 0;
            gpu_state->state = GPU_STATE_WAITING_FOR_CMD;
        }
    }
}

void gpu_gp1_dma_direction(gpu_state_t *gpu_state, uint32_t command)
{
    char *directions[] = {"Off", "FIFO", "CPU -> GP0", "GPU -> CPU"};
    uint8_t direction = (command & 0x3);
    char *direction_name = directions[direction];

    #ifdef LOG_DEBUG_GPU_COMMANDS
    printf("DMA Direction | %s\n", direction_name);
    #endif

    gpu_state->dma_direction = direction;
}

void gpu_gp1_display_area(gpu_state_t *gpu_state, uint32_t command)
{
    uint16_t x = command & 0x000003FF;
    uint16_t y = (command >> 10) & 0x000001FF;

    #ifdef LOG_DEBUG_GPU_COMMANDS
    printf("Start of Display area | %d, %d\n", x, y);
    #endif
}

void gpu_gp1_display_mode(gpu_state_t *gpu_state, uint32_t command)
{
    uint16_t hor_resolutions[] = {256, 320, 512, 640};

    if (command & (1 << 6)) {
        gpu_state->horizontal_resolution = 368;
    } else {
        gpu_state->horizontal_resolution = hor_resolutions[command & 0x3];
    }

    gpu_state->vertical_resolution = (command & (1 << 2)) ? 480 : 240;

    gpu_state->pal = (command & (1 << 3));
    gpu_state->depth_24bit = (command & (1 << 4));

    #ifdef LOG_DEBUG_GPU_COMMANDS
    printf("Display mode | Horizontal Resolution: %d | Vertical Resolution: %d | Video Mode: %s | Color Depth: %s\n", gpu_state->horizontal_resolution, gpu_state->vertical_resolution, gpu_state->pal ? "PAL" : "NTSC", gpu_state->depth_24bit ? "24bit" : "15bit");
    #endif
}

void gpu_send_gp1_command(gpu_state_t *gpu_state, uint32_t command)
{
    uint8_t type = (command >> 24);

    #ifdef LOG_DEBUG_GPU_COMMANDS
    log_debug("GPU", "GP1 CMD (%08X) | ", command);
    #endif

    switch(type) {
        // DMA Direction
        case 0x04:
            gpu_gp1_dma_direction(gpu_state, command);
            break;

        // Start of Display area
        case 0x05:
            gpu_gp1_display_area(gpu_state, command);
            break;

        // Display mode
        case 0x08:
            gpu_gp1_display_mode(gpu_state, command);
            break;

        default:
            #ifdef LOG_DEBUG_GPU_COMMANDS
            printf("\n");
            #endif

            break;
    }
}

