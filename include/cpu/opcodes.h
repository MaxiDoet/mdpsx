#ifndef _r3000_opcodes_h
#define _r3000_opcodes_h

#include <stdint.h>

#include "cpu/r3000.h"
#include "bus/bus.h"
#include "log.h"

/* Load/Store instructions */

void opcode_lb(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint8_t value =  bus_read(bus_state, BUS_SIZE_BYTE, r3000_state->regs[rs] + (uint32_t) (int16_t) imm);

    r3000_enqueue_load(r3000_state, bus_state, rt, (int8_t) value);
}

void opcode_lbu(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint8_t value = bus_read(bus_state, BUS_SIZE_BYTE, r3000_state->regs[rs] + (uint32_t) (int16_t) imm);

    r3000_enqueue_load(r3000_state, bus_state, rt, value);
}

void opcode_lh(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint16_t value = bus_read(bus_state, BUS_SIZE_WORD, r3000_state->regs[rs] + (uint32_t) (int16_t) imm);

    r3000_enqueue_load(r3000_state, bus_state, rt, (int16_t) value);
}

void opcode_lhu(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint16_t value = bus_read(bus_state, BUS_SIZE_WORD, r3000_state->regs[rs] + (uint32_t) (int16_t) imm);

    r3000_enqueue_load(r3000_state, bus_state, rt, value);
}

void opcode_lw(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t value = bus_read(bus_state, BUS_SIZE_DWORD, r3000_state->regs[rs] + (uint32_t) (int16_t) imm);

    r3000_enqueue_load(r3000_state, bus_state, rt, value);
}

void opcode_sb(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    if (r3000_state->cop0_state.regs[COP0_REG_SR] & COP0_SR_ISC) {
        return;
    }

    uint32_t addr = r3000_state->regs[rs] + (uint32_t) (int16_t) imm;

    bus_write(bus_state, BUS_SIZE_BYTE, addr, r3000_state->regs[rt] & 0xFF);
}

void opcode_sh(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    if (r3000_state->cop0_state.regs[COP0_REG_SR] & COP0_SR_ISC) {
        return;
    }

    uint32_t addr = r3000_state->regs[rs] + (uint32_t) (int16_t) imm;

    bus_write(bus_state, BUS_SIZE_WORD, addr, r3000_state->regs[rt] & 0xFFFF);
}

void opcode_sw(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    if (r3000_state->cop0_state.regs[COP0_REG_SR] & COP0_SR_ISC) {
        return;
    }

    uint32_t addr = r3000_state->regs[rs] + (uint32_t) (int16_t) imm;

    bus_write(bus_state, BUS_SIZE_DWORD, addr, r3000_state->regs[rt]);
}

void opcode_lwl(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t value = bus_read(bus_state, BUS_SIZE_DWORD, (r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & ~0x3);

    switch((r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & 0x3) {
        case 0: value = (r3000_state->regs[rt] & 0x00FFFFFF) | (value << 24); break;
        case 1: value = (r3000_state->regs[rt] & 0x0000FFFF) | (value << 16); break;
        case 2: value = (r3000_state->regs[rt] & 0x000000FF) | (value << 8); break;
        case 3: value = (r3000_state->regs[rt] & 0x00000000) | (value << 0); break;
    }

    r3000_enqueue_load(r3000_state, bus_state, rt, value);
}

void opcode_lwr(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t value = bus_read(bus_state, BUS_SIZE_DWORD, (r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & ~0x3);

    switch((r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & 0x3) {
        case 0: value = (r3000_state->regs[rt] & 0x00000000) | (value >> 0); break;
        case 1: value = (r3000_state->regs[rt] & 0xFF000000) | (value >> 8); break;
        case 2: value = (r3000_state->regs[rt] & 0xFFFF0000) | (value >> 16); break;
        case 3: value = (r3000_state->regs[rt] & 0xFFFFFF00) | (value >> 24); break;
    }

    r3000_enqueue_load(r3000_state, bus_state, rt, value);
}

void opcode_swl(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    if (r3000_state->cop0_state.regs[COP0_REG_SR] & COP0_SR_ISC) {
        return;
    }

    uint32_t value = bus_read(bus_state, BUS_SIZE_DWORD, (r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & ~0x3);

    switch((r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & 0x3) {
        case 0: value = (r3000_state->regs[rt] & 0xFFFFFF00) | (value >> 24); break;
        case 1: value = (r3000_state->regs[rt] & 0xFFFF0000) | (value >> 16); break;
        case 2: value = (r3000_state->regs[rt] & 0xFF000000) | (value >> 8); break;
        case 3: value = (r3000_state->regs[rt] & 0x00000000) | (value >> 0); break;
    }

    bus_write(bus_state, BUS_SIZE_DWORD, (r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & ~0x3, value);
}

void opcode_swr(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    if (r3000_state->cop0_state.regs[COP0_REG_SR] & COP0_SR_ISC) {
        return;
    }

    uint32_t value = bus_read(bus_state, BUS_SIZE_DWORD, (r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & ~0x3);

    switch((r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & 0x3) {
        case 0: value = (r3000_state->regs[rt] & 0x00000000) | (value << 0); break;
        case 1: value = (r3000_state->regs[rt] & 0x000000FF) | (value << 8); break;
        case 2: value = (r3000_state->regs[rt] & 0x0000FFFF) | (value << 16); break;
        case 3: value = (r3000_state->regs[rt] & 0x00FFFFFF) | (value << 24); break;
    }

    bus_write(bus_state, BUS_SIZE_DWORD, (r3000_state->regs[rs] + (uint32_t) (int16_t) imm) & ~0x3, value);
}

/* ALU Instructions */

void opcode_add(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = (int32_t) r3000_state->regs[rs] + (int32_t) r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_addu(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] + r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_sub(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = (int32_t) r3000_state->regs[rs] - (int32_t) r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_subu(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] - r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_addi(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = (int32_t) r3000_state->regs[rs] + (int32_t) (uint32_t) (int16_t) imm;

    r3000_state->regs[rt] = result;
}

void opcode_addiu(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = r3000_state->regs[rs] + (uint32_t) (int16_t) imm;

    r3000_state->regs[rt] = result;
}

void opcode_slt(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = ((int32_t) r3000_state->regs[rs] < (int32_t) r3000_state->regs[rt]) ? 1 : 0;

    r3000_state->regs[rd] = result;
}

void opcode_sltu(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = (r3000_state->regs[rs] < r3000_state->regs[rt]) ? 1 : 0;

    r3000_state->regs[rd] = result;
}

void opcode_slti(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = ((int32_t) r3000_state->regs[rs] < (int32_t) (uint32_t) (int16_t) imm) ? 1 : 0;

    r3000_state->regs[rt] = result;
}

void opcode_sltiu(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = (r3000_state->regs[rs] < (uint32_t) (int16_t) imm) ? 1 : 0;

    r3000_state->regs[rt] = result;
}

void opcode_and(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] & r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_or(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] | r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_xor(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] ^ r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_nor(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = ~(r3000_state->regs[rs] | r3000_state->regs[rt]);

    r3000_state->regs[rd] = result;
}

void opcode_andi(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = r3000_state->regs[rs] & imm;

    r3000_state->regs[rt] = result;
}

void opcode_ori(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = r3000_state->regs[rs] | imm;

    r3000_state->regs[rt] = result;
}

void opcode_xori(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = r3000_state->regs[rs] ^ imm;

    r3000_state->regs[rt] = result;
}

void opcode_sllv(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rt] << (r3000_state->regs[rs] & 0x1F);

    r3000_state->regs[rd] = result;
}

void opcode_srlv(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rt] >> (r3000_state->regs[rs] & 0x1F);

    r3000_state->regs[rd] = result;
}

void opcode_srav(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = ((int32_t) r3000_state->regs[rt]) >> (r3000_state->regs[rs] & 0x1F);

    r3000_state->regs[rd] = result;
}

void opcode_sll(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt)
{
    uint32_t result = r3000_state->regs[rt] << shamt;

    r3000_state->regs[rd] = result;
}

void opcode_srl(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt)
{
    uint32_t result = r3000_state->regs[rt] >> shamt;

    r3000_state->regs[rd] = result;
}

void opcode_sra(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt)
{
    uint32_t result = ((int32_t) r3000_state->regs[rt] >> shamt);

    r3000_state->regs[rd] = result;
}

void opcode_lui(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = imm << 16;

    r3000_state->regs[rt] = result;
}

void opcode_mult(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt)
{
    uint64_t result = (int64_t) (int32_t) r3000_state->regs[rs] * (int64_t) (int32_t) r3000_state->regs[rt];

    r3000_state->hi = (result >> 32);
    r3000_state->lo = result;
}

void opcode_multu(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt)
{
    uint64_t result = (uint64_t) r3000_state->regs[rs] * (uint64_t) r3000_state->regs[rt];

    r3000_state->hi = (result >> 32);
    r3000_state->lo = result;
}

void opcode_div(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt)
{
    if ((int32_t) r3000_state->regs[rt] == 0) {
        // Division by zero
        r3000_state->hi = r3000_state->regs[rs];

        if ((int32_t) r3000_state->regs[rs] >= 0) {
            r3000_state->lo = 0xFFFFFFFF; 
        } else {
            r3000_state->lo = 0x00000001;
        }
    } else if (r3000_state->regs[rs] == 0x80000000 && (int32_t) r3000_state->regs[rt] == -1) {
        r3000_state->hi = 0;
        r3000_state->lo = 0x80000000;
    } else {
        r3000_state->lo = (int32_t) r3000_state->regs[rs] / (int32_t) r3000_state->regs[rt];
        r3000_state->hi = (int32_t) r3000_state->regs[rs] % (int32_t) r3000_state->regs[rt];
    }
}

void opcode_divu(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt)
{
    if (r3000_state->regs[rt] == 0) {
        // Division by zero
        r3000_state->hi = r3000_state->regs[rs];
        r3000_state->lo = 0xFFFFFFFF;
    } else {
        r3000_state->lo = r3000_state->regs[rs] / r3000_state->regs[rt];
        r3000_state->hi = r3000_state->regs[rs] % r3000_state->regs[rt];
    }
}

void opcode_mfhi(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rd)
{
    r3000_state->regs[rd] = r3000_state->hi;
}

void opcode_mflo(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rd)
{
    r3000_state->regs[rd] = r3000_state->lo;
}

void opcode_mthi(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs)
{
    r3000_state->hi = r3000_state->regs[rs];
}

void opcode_mtlo(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs)
{
    r3000_state->lo = r3000_state->regs[rs];
}

/* Jump/Branch Instructions */

void opcode_j(r3000_state_t *r3000_state, bus_state_t *bus_state, uint32_t imm_jump)
{
    r3000_branch(r3000_state, (r3000_state->pc & 0xF0000000) | (imm_jump << 2));
}

void opcode_jal(r3000_state_t *r3000_state, bus_state_t *bus_state, uint32_t imm_jump)
{
    r3000_branch(r3000_state, (r3000_state->pc & 0xF0000000) | (imm_jump << 2));

    r3000_state->regs[R3000_REG_RA] = r3000_state->pc_next;
}

void opcode_jr(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs)
{
    r3000_branch(r3000_state, r3000_state->regs[rs]);
}

void opcode_jalr(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rd)
{
    r3000_branch(r3000_state, r3000_state->regs[rs]);

    r3000_state->regs[rd] = r3000_state->pc_next;
}

void opcode_beq(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    if (r3000_state->regs[rs] == r3000_state->regs[rt]) {
        r3000_branch(r3000_state, r3000_state->pc + (((uint32_t) (int16_t) imm) << 2));
    }
}

void opcode_bne(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    if (r3000_state->regs[rs] != r3000_state->regs[rt]) {
        r3000_branch(r3000_state, r3000_state->pc + (((uint32_t) (int16_t) imm) << 2));
    }
}

void opcode_bcondz(r3000_state_t *r3000_state, bus_state_t *bus_state, uint32_t instruction, uint8_t rs, uint16_t imm)
{
    bool is_bgez = (instruction >> 16) & 1;
    bool is_link = (instruction >> 17) & 0xF == 8;

    int32_t v = r3000_state->regs[rs];
    uint32_t test = (v < 0) ^ is_bgez;

    if (is_link) {
        r3000_state->regs[R3000_REG_RA] = r3000_state->pc_next;
    }

    if (test != 0) {
        r3000_branch(r3000_state, r3000_state->pc + (((uint32_t) (int16_t) imm) << 2));
    }
}

void opcode_bgtz(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint16_t imm)
{
    if ((int32_t) r3000_state->regs[rs] > 0) {
        r3000_branch(r3000_state, r3000_state->pc + (((uint32_t) (int16_t) imm) << 2));
    }
}

void opcode_blez(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint16_t imm)
{
    if ((int32_t) r3000_state->regs[rs] <= 0) {
        r3000_branch(r3000_state, r3000_state->pc + (((uint32_t) (int16_t) imm) << 2));
    }
}

void opcode_syscall(r3000_state_t *r3000_state, bus_state_t *bus_state)
{
    r3000_exception(r3000_state, COP0_CAUSE_SYSCALL);
}

/* Coprocessor instruction */

void opcode_mfc0(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rt, uint8_t rd)
{
    #ifdef LOG_DEBUG_R3000
    log_debug("COP0", "MFC | %s <- %s | %08x\n", r3000_register_names[rt], cop0_register_names[rd], r3000_state->cop0_state.regs[rd]);
    #endif

    r3000_enqueue_load(r3000_state, bus_state, rt, r3000_state->cop0_state.regs[rd]);
}

void opcode_mtc0(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rt, uint8_t rd)
{
    #ifdef LOG_DEBUG_R3000
    log_debug("COP0", "MTC | %s -> %s | %08x\n", r3000_register_names[rt], cop0_register_names[rd], r3000_state->regs[rt]);
    #endif

    uint32_t value = r3000_state->regs[rt];

    switch(rd) {
        case COP0_REG_SR:
            r3000_state->cop0_state.regs[COP0_REG_SR] = value;

            break;

        case COP0_REG_CAUSE:
            r3000_state->cop0_state.regs[COP0_REG_CAUSE] = r3000_state->cop0_state.regs[COP0_REG_CAUSE] | (value & 0x300);
            break;
    }
}

void opcode_rfe(r3000_state_t *r3000_state, bus_state_t *bus_state)
{
    r3000_rfe(r3000_state);
}

void opcode_cop0(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rs, uint8_t rt, uint8_t rd, uint16_t imm)
{
    if (rs == 0) {
        // MFC
        opcode_mfc0(r3000_state, bus_state, rt, rd);
    } else if (rs == 4) {
        // MTC
        opcode_mtc0(r3000_state, bus_state, rt, rd);
    } else if (rs == 16 && imm == 16) {
        // RFE
        opcode_rfe(r3000_state, bus_state);
    }
}

#endif