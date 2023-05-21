#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem/mem.h"
#include "cpu/r3000.h"
#include "log.h"

const char *mem_size_names[] = {
    "byte", "word", "dword"
};

const uint32_t mem_segment_map[] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, // USEG
    0x7FFFFFFF,                                     // KSEG0
    0x1FFFFFFF,                                     // KSEG1
    0xFFFFFFFF, 0xFFFFFFFF                          // KSEG2
};

uint32_t mem_read(mem_state_t *state, uint8_t size, uint32_t addr)
{
    uint32_t result = 0;

    uint32_t phy_addr = addr & mem_segment_map[addr >> 29];

    if (phy_addr >= 0x00000000 && phy_addr <= 0x007FFFFF) {
        if (size == MEM_SIZE_BYTE) {
            result = *((uint8_t *) &state->ram[phy_addr]);
        } else if (size == MEM_SIZE_WORD) {
            result = *((uint16_t *) &state->ram[phy_addr]);
        } else {
            result = *((uint32_t *) &state->ram[phy_addr]);
        }
    } else if (phy_addr >= 0x1F800000 && phy_addr <= 0x1F8003FF) {
        if (size == MEM_SIZE_BYTE) {
            result = *((uint8_t *) &state->scratchpad[phy_addr & 0x3FF]);
        } else if (size == MEM_SIZE_WORD) {
            result = *((uint16_t *) &state->scratchpad[phy_addr & 0x3FF]);
        } else {
            result = *((uint32_t *) &state->scratchpad[phy_addr & 0x3FF]);
        } 
    } else if (phy_addr == 0x1F801070) {
        result = state->i_stat;
    } else if (phy_addr == 0x1F801074) {
        result = state->i_mask;
    } else if (phy_addr >= 0x1FC00000 && phy_addr <= 0x1FC7FFFF) {
        result = *((uint32_t *) &state->bios[phy_addr & 0x7FFFF]);
    } else {
        log_debug("MEM", "read addr: %x\n", addr);
    }

    if (size == MEM_SIZE_BYTE) {
        result &= 0xFF;
    } else if (size == MEM_SIZE_WORD) {
        result &= 0xFFFF;
    }

    #ifdef LOG_DEBUG_MEM_READ
    log_debug("MEM", "%08x <- %08x (%08x)\n", result, addr, phy_addr);
    #endif

    return result;
}

void mem_write(mem_state_t *state, uint8_t size, uint32_t addr, uint32_t value)
{
    uint32_t phy_addr = addr & mem_segment_map[addr >> 29];

    if (size == MEM_SIZE_BYTE) {
        value &= 0xFF;
    } else if (size == MEM_SIZE_WORD) {
        value &= 0xFFFF;
    }

    if (phy_addr >= 0x00000000 && phy_addr <= 0x007FFFFF) {
        if (size == MEM_SIZE_BYTE) {
            *((uint8_t *) &state->ram[phy_addr]) = value;
        } else if (size == MEM_SIZE_WORD) {
            *((uint16_t *) &state->ram[phy_addr]) = value;
        } else {
            *((uint32_t *) &state->ram[phy_addr]) = value;
        }
    } else if (phy_addr >= 0x00000000 && phy_addr <= 0x007FFFFF) {
        if (size == MEM_SIZE_BYTE) {
            *((uint8_t *) &state->scratchpad[phy_addr]) = value;
        } else if (size == MEM_SIZE_WORD) {
            *((uint16_t *) &state->scratchpad[phy_addr]) = value;
        } else {
            *((uint32_t *) &state->scratchpad[phy_addr]) = value;
        }
    } else if (phy_addr == 0x1F801070) {
        #ifdef LOG_DEBUG_MEM_WRITE_IO
        state->i_stat = value;

        log_debug("MEM", "%x -> I_STAT | ", value);

        for (uint8_t i=0; i < 10; i++) {
            if (value & (1 << i)) {
                printf("%s ", r3000_irq_names[i]);
            }
        }

        printf("\n");
        #endif
    } else if (phy_addr == 0x1F801074) {
        #ifdef LOG_DEBUG_MEM_WRITE_IO
        state->i_mask = value;

        log_debug("MEM", "%x -> I_MASK | ", value);

        for (uint8_t i=0; i < 10; i++) {
            if (value & (1 << i)) {
                printf("%s ", r3000_irq_names[i]);
            }
        }

        printf("\n");
        #endif
    } else if (phy_addr == 0x1F802041) {
        #ifdef LOG_DEBUG_MEM_WRITE_IO
        log_debug("MEM", "POST %x\n", value);
        #endif
    } else {
        log_debug("MEM", "write addr: %x\n", addr);
    }

    #ifdef LOG_DEBUG_MEM_WRITE
    log_debug("MEM", "%08x -> %08x (%08x) | test: %08x\n", value, addr, phy_addr, mem_read(state, size, addr));
    #endif
}