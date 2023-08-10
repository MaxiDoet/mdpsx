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

const char *dma_channel_names[] = {
    "MDEC IN",
    "MDEC OUT",
    "GPU",
    "CDROM",
    "SPU",
    "PIO",
    "OTC"
};

const char *dma_reg_names[] = {
    "MADR", "", "", "", "BCR", "", "", "", "CHCR"
};

void dma_transfer_all_at_once(dma_state_t *state, dma_channel_state_t *channel_state, bus_state_t *bus_state)
{
    uint32_t word_amount = channel_state->bcr & 0xFFFF;

    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "Transfering %d words from %08X\n", word_amount, channel_state->madr);
    #endif

    uint32_t addr = channel_state->madr;

    for (uint32_t i=0; i < word_amount; i++) {
        uint32_t word = 0;

        if (i == 0) {
            word = 0xFFFFFF;
        } else {
            word = (addr - 4) & 0xFFFFF;
        }

        printf("addr: %08X | %08X\n", addr, word);

        bus_write(bus_state, BUS_SIZE_DWORD, addr, word);
    
        addr -= 4;
    }
}

void dma_transfer_linked_list(dma_state_t *state, dma_channel_state_t *channel_state, bus_state_t *bus_state)
{
    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "Transfering linked list from %08X\n", channel_state->madr);
    #endif

    uint32_t node_addr = channel_state->madr;
    uint32_t node_header = bus_read(bus_state, BUS_SIZE_DWORD, node_addr);
    uint32_t node_word_count = (node_header >> 24);

    while(true) {
        printf("%08X | words: %d next_addr: %x\n", node_addr, node_word_count, node_header & 0x1FFFFC);

        //printf("000EB918: %08X\n", bus_read(bus_state, BUS_SIZE_DWORD, 0x800EB918));

        // Read all words that belong to this node
        for (uint32_t i=0; i < node_word_count; i++) {
            uint32_t word = bus_read(bus_state, BUS_SIZE_DWORD, node_addr + 4 + i * 4);

            gpu_send_gp0_command(&bus_state->gpu_state, word);
        }

        node_addr = (node_header & 0xFFFFFF);

        if (node_addr & (1 << 23)) {
            printf("End of linked list\n");
            break;
        }

        node_header = bus_read(bus_state, BUS_SIZE_DWORD, node_addr & 0x1FFFFC);
        node_word_count = (node_header >> 24);
    }
}

void dma_transfer(dma_state_t *state, dma_channel_state_t *channel_state, bus_state_t *bus_state)
{
    /* Calculate the amount of words we need to transfer */
    uint8_t sync_mode = (channel_state->chcr >> 9) & 0x3;

    switch(sync_mode) {
        case 0:
            dma_transfer_all_at_once(state, channel_state, bus_state);
            break;

        case 1:
            break;

        case 2:
            dma_transfer_linked_list(state, channel_state, bus_state);
            break;
    }

    channel_state->chcr &= ~DMA_CHANNEL_CHCR_START_BUSY;
    channel_state->chcr &= ~DMA_CHANNEL_CHCR_TRIGGER;
}

void dma_channel_write(dma_state_t *state, dma_channel_state_t *channel_state, uint8_t channel_type, bus_state_t *bus_state, uint8_t reg, uint32_t value)
{
    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "%08X -> %s %s\n", value, dma_channel_names[channel_type], dma_reg_names[reg]);
    #endif

    switch(reg) {
        case DMA_REG_MADR:
            channel_state->madr = value;
            break;

        case DMA_REG_BCR:
            channel_state->bcr = value;
            break;

        case DMA_REG_CHCR:
            channel_state->chcr = value;

            // If start bit is set, start transfer
            if (value & DMA_CHANNEL_CHCR_START_BUSY) {
                dma_transfer(state, channel_state, bus_state);
            }

            break;
    }
}

uint32_t dma_read_channel_2(dma_state_t *dma_state, uint32_t addr)
{
    uint32_t result = 0;

    switch(addr & 0xF) {
        // MADR
        case 0x0:
            result = dma_state->channels[2].madr;

            #ifdef LOG_DEBUG_DMA
            log_debug("DMA", "%08X <- MADR\n", result);
            #endif

            break;

        // BCR
        case 0x4:
            result = dma_state->channels[2].bcr;

            #ifdef LOG_DEBUG_DMA
            log_debug("DMA", "%08X <- BCR\n", result);
            #endif

            break;

        // CHCR
        case 0x8:
            result = dma_state->channels[2].chcr;

            #ifdef LOG_DEBUG_DMA
            log_debug("DMA", "%08X <- CHCR\n", result);
            #endif

            break;
    }

    return result;
}

void dma_write_dpcr(dma_state_t *dma_state, uint32_t value)
{
    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "%08X -> DPCR\n", value);
    #endif

    dma_state->dpcr = value;
}

void dma_write(dma_state_t *dma_state, bus_state_t *bus_state, uint32_t addr, uint32_t value)
{
    switch(addr & 0xF0) {
        case 0x80:
            break;

        case 0x90:
            break;

        case 0xA0:
            dma_channel_write(dma_state, &dma_state->channels[2], DMA_CHANNEL_GPU, bus_state, addr & 0xF, value);
            break;

        case 0xB0:
        case 0xC0:
        case 0xD0:
            break;

        case 0xE0:
            dma_channel_write(dma_state, &dma_state->channels[6], DMA_CHANNEL_OTC, bus_state, addr & 0xF, value);
            break;

        case 0xF0:
            if ((addr & 0xF) == 0) {
                dma_write_dpcr(dma_state, value);
            }
    }
}

uint32_t dma_read(dma_state_t *dma_state, uint32_t addr)
{
    uint32_t result = 0;

    switch(addr & 0xF0) {
        case 0x80:
            break;

        case 0x90:
            break;

        case 0xA0:
            result = dma_read_channel_2(dma_state, addr);
            break;

        case 0xB0:
        case 0xC0:
        case 0xD0:
        case 0xE0:
            break;

        case 0xF0:
            if ((addr & 0xF) == 0) {
                result = dma_state->dpcr;
            }
    }

    return result;
}

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
    } else if (phy_addr >= 0x1F801080 && phy_addr <= 0x1F8010FC) {
        result = dma_read(&state->dma_state, phy_addr);
    } else if (phy_addr == 0x1F801810 || phy_addr == 0x1F801814) {
        result = gpu_read(&state->gpu_state, phy_addr);
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
    } else if (phy_addr >= 0x1F801080 && phy_addr <= 0x1F8010FC) {
        dma_write(&state->dma_state, state, phy_addr, value);
    } else if (phy_addr == 0x1F801810 || phy_addr == 0x1F801814) {
        gpu_write(&state->gpu_state, phy_addr, value);
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