#include <stdio.h>
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

    if (phy_addr >= 0x00000000 && phy_addr <= 0x001FFFFF) {
        result = *((uint32_t *) &state->ram[phy_addr]);
    } else if (phy_addr >= 0x1FC00000 && phy_addr <= 0x1FC80000) {
        result = *((uint32_t *) &state->bios[phy_addr - 0x1FC00000]);
    }

    if (size == MEM_SIZE_BYTE) {
        result &= 0xFF;
    } else if (size == MEM_SIZE_WORD) {
        result &= 0xFFFF;
    }

    #ifdef LOG_DEBUG_MEM_READ
    log_debug("MEM", "%08x <- %08x\n", result, addr);
    #endif

    return result;
}

uint32_t mem_write(mem_state_t *state, uint8_t size, uint32_t addr, uint32_t value)
{
    uint32_t result = 0;

    uint32_t phy_addr = addr & mem_segment_map[addr >> 29];

    if (size == MEM_SIZE_BYTE) {
        value &= 0xFF;
    } else if (size == MEM_SIZE_WORD) {
        value &= 0xFFFF;
    }

    if (phy_addr >= 0x00000000 && phy_addr <= 0x001FFFFF) {
        *((uint32_t *) &state->ram[phy_addr]) = value; // Todo: Only write the bits that need to be written
    } else if (phy_addr == 0x1F801070) {
        #ifdef LOG_DEBUG_MEM_WRITE_IO
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
        log_debug("MEM", "%x -> I_MASK | ", value);

        for (uint8_t i=0; i < 10; i++) {
            if (value & (1 << i)) {
                printf("%s ", r3000_irq_names[i]);
            }
        }

        printf("\n");
        #endif
    }

    #ifdef LOG_DEBUG_MEM_WRITE
    log_debug("MEM", "%08x -> %08x\n", value, addr);
    #endif

    return result;
}