#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus/bus.h"
#include "cpu/r3000.h"
#include "log.h"

const char *BUS_SIZE_names[] = {
    "byte", "word", "dword"
};

const uint32_t bus_segment_map[] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, // USEG
    0x7FFFFFFF,                                     // KSEG0
    0x1FFFFFFF,                                     // KSEG1
    0xFFFFFFFF, 0xFFFFFFFF                          // KSEG2
};

uint32_t bus_read(bus_state_t *state, uint8_t size, uint32_t addr)
{
    uint32_t result = 0;

    uint32_t phy_addr = addr & bus_segment_map[addr >> 29];

    if (phy_addr >= 0x00000000 && phy_addr <= 0x001FFFFF) {
        if (size == BUS_SIZE_BYTE) {
            result = *((uint8_t *) &state->ram[phy_addr & 0x1FFFFF]);
        } else if (size == BUS_SIZE_WORD) {
            result = *((uint16_t *) &state->ram[phy_addr & 0x1FFFFF]);
        } else {
            result = *((uint32_t *) &state->ram[phy_addr & 0x1FFFFF]);
        }
    } else if (phy_addr >= 0x1F800000 && phy_addr <= 0x1F8003FF) {
        if (size == BUS_SIZE_BYTE) {
            result = *((uint8_t *) &state->scratchpad[phy_addr & 0x3FF]);
        } else if (size == BUS_SIZE_WORD) {
            result = *((uint16_t *) &state->scratchpad[phy_addr & 0x3FF]);
        } else {
            result = *((uint32_t *) &state->scratchpad[phy_addr & 0x3FF]);
        }
    } else if (phy_addr == 0x1F801070) {
        result = state->i_stat;
    } else if (phy_addr == 0x1F801074) {
        result = state->i_mask;
    } else if (phy_addr >= 0x1FC00000 && phy_addr <= 0x1FC7FFFF) {
        if (size == BUS_SIZE_BYTE) {
            result = *((uint8_t *) &state->bios[phy_addr & 0x7FFFF]);
        } else if (size == BUS_SIZE_WORD) {
            result = *((uint16_t *) &state->bios[phy_addr & 0x7FFFF]);
        } else {
            result = *((uint32_t *) &state->bios[phy_addr & 0x7FFFF]);
        }
    }

    #ifdef LOG_DEBUG_BUS_READ
    log_debug("BUS", "%08x <- %08x (%08x)\n", result, addr, phy_addr);
    #endif

    return result;
}

void bus_write(bus_state_t *state, uint8_t size, uint32_t addr, uint32_t value)
{
    uint32_t phy_addr = addr & bus_segment_map[addr >> 29];

    if (phy_addr >= 0x00000000 && phy_addr <= 0x001FFFFF) {
        if (size == BUS_SIZE_BYTE) {
            *((uint8_t *) &state->ram[phy_addr & 0x1FFFFF]) = value;
        } else if (size == BUS_SIZE_WORD) {
            *((uint16_t *) &state->ram[phy_addr & 0x1FFFFF]) = value;
        } else {
            *((uint32_t *) &state->ram[phy_addr & 0x1FFFFF]) = value;
        }
    } else if (phy_addr >= 0x1F800000 && phy_addr <= 0x1F8003FF) {
        if (size == BUS_SIZE_BYTE) {
            *((uint8_t *) &state->scratchpad[phy_addr & 0x400]) = value;
        } else if (size == BUS_SIZE_WORD) {
            *((uint16_t *) &state->scratchpad[phy_addr & 0x400]) = value;
        } else {
            *((uint32_t *) &state->scratchpad[phy_addr & 0x400]) = value;
        }
    } else if (phy_addr == 0x1F801008) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        log_debug("BUS", "%x -> EXP1_DELAY (unused)\n", value);
        #endif
    } else if (phy_addr == 0x1F801010) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        log_debug("BUS", "%x -> BIOS_DELAY (unused)\n", value);
        #endif
    } else if (phy_addr == 0x1F801014) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        log_debug("BUS", "%x -> SPU_DELAY (unused)\n", value);
        #endif
    } else if (phy_addr == 0x1F801018) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        log_debug("BUS", "%x -> CDROM_DELAY (unused)\n", value);
        #endif
    } else if (phy_addr == 0x1F80101C) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        log_debug("BUS", "%x -> EXP2_DELAY (unused)\n", value);
        #endif
    } else if (phy_addr == 0x1F801020) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        log_debug("BUS", "%x -> COM_DELAY (unused)\n", value);
        #endif
    } else if (phy_addr == 0x1F801070) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        state->i_stat = value;

        log_debug("BUS", "%x -> I_STAT\n", value);
        #endif
    } else if (phy_addr == 0x1F801074) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        state->i_mask = value;

        log_debug("BUS", "%x -> I_MASK\n", value);
        #endif
    } else if (phy_addr == 0x1F802041) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        log_debug("BUS", "POST %x\n", value);
        #endif
    } else if (phy_addr == 0xFFFE0130) {
        #ifdef LOG_DEBUG_BUS_WRITE_IO
        log_debug("BUS", "%x -> Cache Control\n", value);
        #endif
    }

    #ifdef LOG_DEBUG_BUS_WRITE
    log_debug("BUS", "%08x -> %08x (%08x) | test: %08x\n", value, addr, phy_addr, bus_read(state, size, addr));
    #endif
}