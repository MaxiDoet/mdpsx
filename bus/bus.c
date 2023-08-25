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

void dma_set_irq(dma_state_t *state, dma_channel_state_t *channel_state, uint8_t channel_type)
{
    bool irq_enabled = state->dicr & (1 << (16 + channel_type));
    
    if (irq_enabled) {
        // Set irq flag
        state->dicr |= (1 << (24 + channel_type));
    }
}

void dma_transfer_words(dma_state_t *state, dma_channel_state_t *channel_state, uint8_t channel_type, bus_state_t *bus_state)
{
    uint32_t words = 0;

    uint8_t sync_mode = (channel_state->chcr >> 9) & 0x3;
    if (sync_mode == 0) {
        words = (channel_state->bcr & 0x0000FFFF);
    } else if (sync_mode == 1) {
        words = (channel_state->bcr & 0x0000FFFF) * ((channel_state->bcr >> 16) & 0x0000FFFF);
    }

    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "Transfering %d words | sync_mode: %d | madr: %08X\n", words, sync_mode, channel_state->madr);
    #endif

    uint32_t addr = channel_state->madr;
    int8_t addr_step = (channel_state->chcr & DMA_CHANNEL_CHCR_ADDR_STEP) ? -4 : 4;

    for (uint32_t i=0; i < words; i++) {
        uint32_t value = 0;

        if (channel_state->chcr & DMA_CHANNEL_CHCR_DIRECTION) {
            // From RAM
            uint32_t word = bus_read(bus_state, BUS_SIZE_DWORD, addr);

            switch(channel_type) {
                case DMA_CHANNEL_GPU:
                    gpu_send_gp0_command(&bus_state->gpu_state, word);
                    break;
            }
        } else {
            uint32_t value = 0;

            // To RAM
            switch(channel_type) {
                case DMA_CHANNEL_OTC:
                    if (i == (words - 1)) {
                        value = 0xFFFFFF;
                    } else {
                        value = (addr - 4) & 0x1FFFFF;
                    }

                    break;
            }

            bus_write(bus_state, BUS_SIZE_DWORD, addr & 0x1FFFFC, value);
        }
    
        addr = (addr + addr_step);
    }
}

void dma_transfer_linked_list(dma_state_t *state, dma_channel_state_t *channel_state, bus_state_t *bus_state)
{
    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "Transfering linked list from %08X\n", channel_state->madr);
    #endif

    uint32_t addr = channel_state->madr & 0x1FFFFC;

    while(true) {
        uint32_t node_header = bus_read(bus_state, BUS_SIZE_DWORD, addr);
        uint32_t word_count = (node_header >> 24);

        //printf("%08X | words: %d next_addr: %x\n", addr, word_count, node_header & 0x1FFFFC);

        // Read all words that belong to this node
        while (word_count > 0) {
            addr = (addr + 4) & 0x1FFFFC;

            uint32_t word = bus_read(bus_state, BUS_SIZE_DWORD, addr);
            gpu_send_gp0_command(&bus_state->gpu_state, word);

            word_count--;
        }

        if (node_header & (1 << 23)) {
            #ifdef LOG_DEBUG_DMA
            log_debug("DMA", "End of linked list\n");
            #endif

            break;
        }

        addr = (node_header & 0x1FFFFC);
    }
}

void dma_transfer(dma_state_t *state, dma_channel_state_t *channel_state, uint8_t channel_type, bus_state_t *bus_state)
{
    /* Calculate the amount of words we need to transfer */
    uint8_t sync_mode = (channel_state->chcr >> 9) & 0x3;

    switch(sync_mode) {
        case 0:
            dma_transfer_words(state, channel_state, channel_type, bus_state);
            break;

        case 1:
            dma_transfer_words(state, channel_state, channel_type, bus_state);
            break;

        case 2:
            dma_transfer_linked_list(state, channel_state, bus_state);
            break;
    }

    dma_set_irq(state, channel_state, channel_type);

    channel_state->chcr &= ~DMA_CHANNEL_CHCR_START_BUSY;
    channel_state->chcr &= ~DMA_CHANNEL_CHCR_TRIGGER;
}

void dma_channel_write(dma_state_t *state, dma_channel_state_t *channel_state, uint8_t channel_type, bus_state_t *bus_state, uint8_t reg, uint32_t value)
{
    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "%08X -> %s %s\n", value, dma_channel_names[channel_type], dma_reg_names[reg]);
    #endif

    switch(reg) {
        case DMA_CHANNEL_REG_MADR:
            channel_state->madr = value;
            break;

        case DMA_CHANNEL_REG_BCR:
            channel_state->bcr = value;
            break;

        case DMA_CHANNEL_REG_CHCR:
            channel_state->chcr = value;

            // If start bit is set, start transfer
            if (value & DMA_CHANNEL_CHCR_START_BUSY) {
                dma_transfer(state, channel_state, channel_type, bus_state);
            }

            break;
    }
}

uint32_t dma_channel_read(dma_state_t *state, dma_channel_state_t *channel_state, uint8_t channel_type, uint8_t reg)
{
    uint32_t result = 0;

    switch(reg) {
        case DMA_CHANNEL_REG_MADR:
            result = channel_state->madr;
            break;

        case DMA_CHANNEL_REG_BCR:
            result = channel_state->bcr;
            break;

        case DMA_CHANNEL_REG_CHCR:
            result = channel_state->chcr;
            break;
    }

    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "%08X <- %s %s\n", result, dma_channel_names[channel_type], dma_reg_names[reg]);
    #endif

    return result;
}

void dma_write_dpcr(dma_state_t *dma_state, uint32_t value)
{
    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "%08X -> DPCR\n", value);
    #endif

    dma_state->dpcr = value;
}

void dma_write_dicr(dma_state_t *dma_state, uint32_t value)
{
    #ifdef LOG_DEBUG_DMA
    log_debug("DMA", "%08X -> DICR\n", value);
    #endif

    dma_state->dicr = value;
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
            } else if ((addr & 0xF) == 4) {
                dma_write_dicr(dma_state, value);
            }

            break;
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
            result = dma_channel_read(dma_state, &dma_state->channels[2], DMA_CHANNEL_GPU, addr & 0xF);
            break;

        case 0xB0:
        case 0xC0:
        case 0xD0:
            break;

        case 0xE0:
            result = dma_channel_read(dma_state, &dma_state->channels[6], DMA_CHANNEL_OTC, addr & 0xF);
            break;

        case 0xF0:
            if ((addr & 0xF) == 0) {
                result = dma_state->dpcr;

                #ifdef LOG_DEBUG_DMA
                log_debug("DMA", "%08X <- DPCR\n", result);
                #endif
            } else if ((addr & 0xF) == 4) {
                result = dma_state->dicr;

                #ifdef LOG_DEBUG_DMA
                log_debug("DMA", "%08X <- DICR\n", result);
                #endif
            }

            break;
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