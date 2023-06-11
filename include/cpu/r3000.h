#ifndef _r3000_h
#define _r3000_h

#include <stdbool.h>
#include "bus/bus.h"

#define R3000_BREAKPOINTS

#define R3000_REG_AT        1
#define R3000_REG_V0        2
#define R3000_REG_V1        3
#define R3000_REG_A0        4
#define R3000_REG_A1        5
#define R3000_REG_A2        6
#define R3000_REG_A3        7
#define R3000_REG_T0        8
#define R3000_REG_T1        9
#define R3000_REG_T2        10
#define R3000_REG_T3        11
#define R3000_REG_T4        12
#define R3000_REG_T5        13
#define R3000_REG_T6        14
#define R3000_REG_T7        15
#define R3000_REG_S0        16
#define R3000_REG_S1        17
#define R3000_REG_S2        18
#define R3000_REG_S3        19
#define R3000_REG_S4        20
#define R3000_REG_S5        21
#define R3000_REG_S6        22
#define R3000_REG_S7        23
#define R3000_REG_T8        24
#define R3000_REG_T9        25
#define R3000_REG_K0        26
#define R3000_REG_K1        27
#define R3000_REG_GP        28
#define R3000_REG_SP        29
#define R3000_REG_FP        30
#define R3000_REG_RA        31

#define COP0_REG_BPC      3
#define COP0_REG_BDA      5
#define COP0_REG_JUMPDEST 6
#define COP0_REG_DCIC     7
#define COP0_REG_BADVADDR 8
#define COP0_REG_BDAM     9
#define COP0_REG_BPCM     11
#define COP0_REG_SR       12
#define COP0_REG_CAUSE    13
#define COP0_REG_EPC      14
#define COP0_REG_PRID     15

#define COP0_SR_IEC (1 << 0)
#define COP0_SR_IEP (1 << 2)
#define COP0_SR_IEO (1 << 4)
#define COP0_SR_ISC (1 << 16)
#define COP0_SR_BEV (1 << 22)

#define COP0_CAUSE_INT      0x00
#define COP0_CAUSE_ADEL     0x04
#define COP0_CAUSE_ADES     0x05
#define COP0_CAUSE_IBE      0x06
#define COP0_CAUSE_DBE      0x07
#define COP0_CAUSE_SYSCALL  0x08

#define DELAY_SLOT_STATE_ARMED          1
#define DELAY_SLOT_STATE_DELAY_CYCLE    2
#define DELAY_SLOT_STATE_DONE           3

static const char *r3000_register_names[] = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "v2",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

static const char *cop0_register_names[] = {
    "", "", "", "BPC", "", "BDA", "JUMPDEST", "DCIC",
    "BadVaddr", "BDAM", "", "BPCM", "SR", "CAUSE", "EPC", "PRID",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", ""
};

static const char *r3000_irq_names[] = {
    "VBLANK",
    "GPU",
    "CDROM",
    "DMA",
    "TMR0",
    "TMR1",
    "TMR2",
    "",
    "SIO",
    "SPU",
    "IRQ10"
};

#define R3000_IRQ_VBLANK    (1 << 0)
#define R3000_IRQ_GPU       (1 << 1)
#define R3000_IRQ_CDROM     (1 << 2)
#define R3000_IRQ_DMA       (1 << 3)
#define R3000_IRQ_TMR0      (1 << 4)
#define R3000_IRQ_TMR1      (1 << 5)
#define R3000_IRQ_TMR2      (1 << 6)

typedef struct cop0_state_t {
    uint32_t regs[32];
} cop0_state_t;

#ifdef R3000_BREAKPOINTS
typedef struct r3000_breakpoint_t {
    uint32_t pc;
    char *name;
} r3000_breakpoint_t;
#endif

typedef struct r3000_state_t {
    uint32_t regs[32];
    uint32_t hi, lo;

    uint32_t pc;
    uint32_t pc_next;

    uint8_t branch_delay_slot_state;
    uint32_t branch_addr;

    uint8_t load_delay_slot_state;

    uint8_t load_reg;
    uint32_t load_value;

    uint8_t load_delay_reg;
    uint32_t load_delay_value;

    uint32_t i_stat;
    uint32_t i_mask;

    cop0_state_t cop0_state;

    #ifdef R3000_BREAKPOINTS
    r3000_breakpoint_t breakpoints[100];
    uint8_t breakpoint_count;
    #endif
} r3000_state_t; 

void r3000_enqueue_load(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rt, uint32_t value);
void r3000_branch(r3000_state_t *r3000_state, uint32_t addr);
void r3000_exception(r3000_state_t *r3000_state, uint8_t cause);
void r3000_add_breakpoint(r3000_state_t *r3000_state, uint32_t pc, char *name);
void r3000_init(r3000_state_t *state);
void r3000_step(r3000_state_t *r3000_state, bus_state_t *bus_state);

#endif