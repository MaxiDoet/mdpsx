#ifndef _bus_h
#define _bus_h

#include <stdint.h>
#include <stdbool.h>

#include "gpu/gpu.h"
#include "bus/dma.h"
#include "timer/timer.h"

#define BUS_SIZE_BYTE   0
#define BUS_SIZE_WORD   1
#define BUS_SIZE_DWORD  2

typedef struct bus_state_t {
    uint8_t *ram;
    uint8_t *scratchpad;
    uint8_t *bios;

    uint32_t i_stat;
    uint32_t i_mask;

    gpu_state_t gpu_state;
    dma_state_t dma_state;
    timer_state_t timer_state;

    bool debug_enabled;
} bus_state_t;

uint32_t bus_read(bus_state_t *state, uint8_t size, uint32_t addr);
void bus_write(bus_state_t *state, uint8_t size, uint32_t addr, uint32_t value);

#endif