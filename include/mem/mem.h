#ifndef _mem_h
#define _mem_h

#include <stdint.h>

#define MEM_SIZE_BYTE   0
#define MEM_SIZE_WORD   1
#define MEM_SIZE_DWORD  2

#define MEM_RAM_SIZE        0x200000
#define MEM_SCRATCHPAD_SIZE 0x400

typedef struct mem_state_t {
    uint8_t *ram;
    uint8_t *bios;

    uint32_t i_stat;
    uint32_t i_mask;
} mem_state_t;

uint32_t mem_read(mem_state_t *state, uint8_t size, uint32_t addr);
void mem_write(mem_state_t *state, uint8_t size, uint32_t addr, uint32_t value);

#endif