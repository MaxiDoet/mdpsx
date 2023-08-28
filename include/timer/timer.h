#ifndef _timer_h
#define _timer_h

typedef struct timer_channel_t {
    uint16_t counter;
    uint32_t mode;
    uint16_t target;

    bool irq_req;
} timer_channel_t;

typedef struct timer_state_t {
    timer_channel_t channel_0;
    timer_channel_t channel_1;
    timer_channel_t channel_2;
} timer_state_t;

uint32_t timer_read(timer_state_t *state, uint32_t addr);
void timer_write(timer_state_t *state, uint32_t addr, uint32_t value);
void timer_channel_tick(timer_channel_t *channel, uint8_t channel_num);

#endif