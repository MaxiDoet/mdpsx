#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "timer/timer.h"
#include "log.h"

#define TIMER_CHANNEL_MODE_RESET_COUNTER                (1 << 3)
#define TIMER_CHANNEL_MODE_IRQ_COUNTER_EQUALS_TARGET    (1 << 4)
#define TIMER_CHANNEL_MODE_IRQ_COUNTER_EQUALS_FFFF      (1 << 5)
#define TIMER_CHANNEL_MODE_IRQ_REPEAT                   (1 << 6)
#define TIMER_CHANNEL_MODE_REACHED_TARGET               (1 << 11)
#define TIMER_CHANNEL_MODE_REACHED_FFFF                 (1 << 12)

uint32_t timer_channel_read(timer_channel_t *channel, uint8_t channel_num, uint8_t reg)
{
    uint32_t result = 0;

    #ifdef LOG_DEBUG_TIMER
    log_debug("TIMER", "Channel %d | ", channel_num);
    #endif

    switch(reg) {
        case 0x0:
            result = channel->counter;

            // Counter
            #ifdef LOG_DEBUG_TIMER
            printf("%08X <- Counter\n", result);
            #endif

            break;

        case 0x4:
            result = channel->mode;

            // Mode
            #ifdef LOG_DEBUG_TIMER
            printf("%08X <- Mode\n", result);
            #endif

            channel->mode &= ~TIMER_CHANNEL_MODE_REACHED_TARGET;
            channel->mode &= ~TIMER_CHANNEL_MODE_REACHED_FFFF;

            break;

        case 0x8:
            result = channel->target;

            // Target
            #ifdef LOG_DEBUG_TIMER
            printf("%08X <- Target\n", result);
            #endif

            break;     
    }

    return result;
}

void timer_channel_write(timer_channel_t *channel, uint8_t channel_num, uint8_t reg, uint32_t value)
{
    #ifdef LOG_DEBUG_TIMER
    log_debug("TIMER", "Channel %d | ", channel_num);
    #endif

    switch(reg) {
        case 0x0:
            // Counter
            #ifdef LOG_DEBUG_TIMER
            printf("%08X -> Counter\n", value);
            #endif

            channel->counter = value;

            break;

        case 0x4:
            // Mode
            #ifdef LOG_DEBUG_TIMER
            printf("%08X -> Mode\n", value);
            #endif

            channel->mode = value;

            break;

        case 0x8:
            // Target
            #ifdef LOG_DEBUG_TIMER
            printf("%08X -> Target\n", value);
            #endif

            channel->target = value;

            break;       
    }
}

uint32_t timer_read(timer_state_t *state, uint32_t addr)
{
    uint32_t result = 0;

    switch(addr & 0xF0) {
        case 0x00:
            // Channel 0
            result = timer_channel_read(&state->channel_0, 0, addr & 0xF);
            break;

        case 0x10:
            // Channel 1
            result = timer_channel_read(&state->channel_1, 0, addr & 0xF);
            break;

        case 0x20:
            // Channel 2
            result = timer_channel_read(&state->channel_2, 0, addr & 0xF);
            break;
    }

    return result;
}

void timer_write(timer_state_t *state, uint32_t addr, uint32_t value)
{
    switch(addr & 0xF0) {
        case 0x00:
            // Channel 0
            timer_channel_write(&state->channel_0, 0, addr & 0xF, value);
            break;

        case 0x10:
            // Channel 1
            timer_channel_write(&state->channel_1, 0, addr & 0xF, value);
            break;

        case 0x20:
            // Channel 2
            timer_channel_write(&state->channel_2, 0, addr & 0xF, value);
            break;
    }
}

void timer_channel_tick(timer_channel_t *channel, uint8_t channel_num)
{
    if (channel->counter == 0xFFFF) {
        #ifdef LOG_DEBUG_TIMER
        log_debug("TIMER", "Channel %d | Counter reached FFFF\n", channel_num);
        #endif
 
        if (!(channel->mode & TIMER_CHANNEL_MODE_RESET_COUNTER)) {
            channel->counter = 0x0000;
        }

        if (channel->mode & TIMER_CHANNEL_MODE_IRQ_COUNTER_EQUALS_FFFF) {
            channel->irq_req = true;
        }

        channel->mode |= TIMER_CHANNEL_MODE_REACHED_FFFF;
    }

    if (channel->counter == channel->target) {
        #ifdef LOG_DEBUG_TIMER
        log_debug("TIMER", "Channel %d | Counter reached target value\n", channel_num);
        #endif

        if (channel->mode & TIMER_CHANNEL_MODE_IRQ_COUNTER_EQUALS_TARGET) {
            channel->irq_req = true;
        }

        channel->counter = 0x0000;
        channel->mode |= TIMER_CHANNEL_MODE_REACHED_TARGET;
    }

    channel->counter++;
}