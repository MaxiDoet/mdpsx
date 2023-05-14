#ifndef _r3000_opcodes_h
#define _r3000_opcodes_h

#include <stdint.h>

#include "cpu/r3000.h"
#include "mem/mem.h"
#include "log.h"

/* Load/Store instructions */

void opcode_lb(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint8_t value = (int8_t) mem_read(mem_state, MEM_SIZE_BYTE, r3000_state->regs[rs] + (int16_t) imm);

    r3000_enqueue_load(r3000_state, mem_state, rt, value);
}

void opcode_lbu(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint8_t value = (uint8_t) mem_read(mem_state, MEM_SIZE_BYTE, r3000_state->regs[rs] + (int16_t) imm);

    r3000_enqueue_load(r3000_state, mem_state, rt, value);
}

void opcode_lh(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint16_t value = (int16_t) mem_read(mem_state, MEM_SIZE_WORD, r3000_state->regs[rs] + (int16_t) imm);

    r3000_enqueue_load(r3000_state, mem_state, rt, value);
}

void opcode_lhu(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint16_t value = (uint16_t) mem_read(mem_state, MEM_SIZE_WORD, r3000_state->regs[rs] + (int16_t) imm);

    r3000_enqueue_load(r3000_state, mem_state, rt, value);
}

void opcode_lw(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t value = mem_read(mem_state, MEM_SIZE_DWORD, r3000_state->regs[rs] + (int16_t) imm);

    r3000_enqueue_load(r3000_state, mem_state, rt, value);
}

void opcode_sb(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t addr = r3000_state->regs[rs] + (int16_t) imm;
    
    if (r3000_state->cop0_state.regs[COP0_REG_SR] & COP0_SR_ISC) {
        log_debug("R3000", "Ignored store due to ISC bit in COP0 SR\n");
        return;
    }

    mem_write(mem_state, MEM_SIZE_BYTE, addr, r3000_state->regs[rt] & 0xFF);
}

void opcode_sh(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t addr = r3000_state->regs[rs] + (int16_t) imm;

    if (r3000_state->cop0_state.regs[COP0_REG_SR] & COP0_SR_ISC) {
        log_debug("R3000", "Ignored store due to ISC bit in COP0 SR\n");
        return;
    }

    mem_write(mem_state, MEM_SIZE_WORD, addr, r3000_state->regs[rt] & 0xFFFF);
}

void opcode_sw(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t addr = r3000_state->regs[rs] + (int16_t) imm;

    if (r3000_state->cop0_state.regs[COP0_REG_SR] & COP0_SR_ISC) {
        log_debug("R3000", "Ignored store due to ISC bit in COP0 SR\n");
        return;
    }

    mem_write(mem_state, MEM_SIZE_DWORD, addr, r3000_state->regs[rt]);
}

/* ALU Instructions */

void opcode_add(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] + r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_addu(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] + r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_sub(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] - r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_subu(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] - r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_addi(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = r3000_state->regs[rs] + (int16_t) imm;

    r3000_state->regs[rt] = result;
}

void opcode_addiu(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = r3000_state->regs[rs] + (int16_t) imm;

    r3000_state->regs[rt] = result;
}

void opcode_slt(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = ((int32_t) r3000_state->regs[rs] < (int32_t) r3000_state->regs[rt]) ? 1 : 0;

    r3000_state->regs[rd] = result;
}

void opcode_sltu(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = (r3000_state->regs[rs] < r3000_state->regs[rt]) ? 1 : 0;

    r3000_state->regs[rd] = result;
}

void opcode_slti(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = ((int32_t) r3000_state->regs[rs] < (int16_t) imm) ? 1 : 0;

    r3000_state->regs[rt] = result;
}

void opcode_sltiu(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = (r3000_state->regs[rs] < (int16_t) imm) ? 1 : 0;

    r3000_state->regs[rt] = result;
}

void opcode_and(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] & r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_or(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] | r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_xor(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rs] ^ r3000_state->regs[rt];

    r3000_state->regs[rd] = result;
}

void opcode_nor(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = ~(r3000_state->regs[rs] & r3000_state->regs[rt]);

    r3000_state->regs[rd] = result;
}

void opcode_andi(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = r3000_state->regs[rs] & imm;

    r3000_state->regs[rt] = result;
}

void opcode_ori(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = r3000_state->regs[rs] | imm;

    r3000_state->regs[rt] = result;
}

void opcode_xori(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = r3000_state->regs[rs] ^ imm;

    r3000_state->regs[rt] = result;
}

void opcode_sllv(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rt] << (r3000_state->regs[rs] & 0x1F);

    r3000_state->regs[rd] = result;
}

void opcode_srlv(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = r3000_state->regs[rt] >> (r3000_state->regs[rs] & 0x1F);

    r3000_state->regs[rd] = result;
}

void opcode_srav(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint32_t result = (int32_t) r3000_state->regs[rt] >> (r3000_state->regs[rs] & 0x1F);

    r3000_state->regs[rd] = result;
}

void opcode_sll(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt)
{
    uint32_t result = r3000_state->regs[rt] << shamt;

    r3000_state->regs[rd] = result;
}

void opcode_srl(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt)
{
    uint32_t result = r3000_state->regs[rt] >> shamt;

    r3000_state->regs[rd] = result;
}

void opcode_sra(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt)
{
    uint32_t result = (int32_t) r3000_state->regs[rt] >> shamt;

    r3000_state->regs[rd] = result;
}

void opcode_lui(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    uint32_t result = imm << 16;

    r3000_state->regs[rt] = result;
}

void opcode_mult(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt)
{
    uint64_t result = (int64_t) (int32_t) r3000_state->regs[rs] * (int64_t) (int32_t) r3000_state->regs[rt];

    r3000_state->hi = (result >> 32);
    r3000_state->lo = result;
}

void opcode_multu(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt)
{
    uint64_t result = (uint64_t) r3000_state->regs[rs] * (uint64_t) r3000_state->regs[rt];

    r3000_state->hi = (result >> 32);
    r3000_state->lo = result;
}

void opcode_div(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt)
{
    r3000_state->lo = (int32_t) r3000_state->regs[rs] / (int32_t) r3000_state->regs[rt];
    r3000_state->hi = (int32_t) r3000_state->regs[rs] % (int32_t) r3000_state->regs[rt];
}

void opcode_divu(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt)
{
    r3000_state->lo = r3000_state->regs[rs] / r3000_state->regs[rt];
    r3000_state->hi = r3000_state->regs[rs] % r3000_state->regs[rt];
}

void opcode_mfhi(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rd)
{
    r3000_state->regs[rd] = r3000_state->hi;
}

void opcode_mflo(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rd)
{
    r3000_state->regs[rd] = r3000_state->lo;
}

void opcode_mthi(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs)
{
    r3000_state->hi = r3000_state->regs[rs];
}

void opcode_mtlo(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs)
{
    r3000_state->lo = r3000_state->regs[rs];
}

/* Jump/Branch Instructions */

void opcode_j(r3000_state_t *r3000_state, mem_state_t *mem_state, uint32_t target_address)
{
    r3000_branch(r3000_state, (r3000_state->pc & 0xF0000000) | (target_address << 2));
}

void opcode_jal(r3000_state_t *r3000_state, mem_state_t *mem_state, uint32_t target_address)
{
    r3000_branch(r3000_state, (r3000_state->pc & 0xF0000000) | (target_address << 2));

    r3000_state->regs[R3000_REG_RA] = r3000_state->pc + 4;
}

void opcode_jr(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs)
{
   r3000_branch(r3000_state, r3000_state->regs[rs]);
}

void opcode_jalr(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rd)
{
    r3000_branch(r3000_state, r3000_state->regs[rs]);

    r3000_state->regs[rd] = r3000_state->pc + 4;
}

void opcode_beq(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    if (r3000_state->regs[rs] == r3000_state->regs[rt]) {
        r3000_branch(r3000_state, (int32_t) r3000_state->pc + ((int16_t) imm << 2));
    }
}

void opcode_bne(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    if (r3000_state->regs[rs] != r3000_state->regs[rt]) {
        r3000_branch(r3000_state, (int32_t) r3000_state->pc + ((int16_t) imm << 2));
    }
}

void opcode_bcondz(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint16_t imm)
{
    bool branch = false;
    bool link = false;

    if (rt == 0) {
        /* BLTZ */
        branch = ((int32_t) r3000_state->regs[rs] < 0);
    } else if (rt == 1) {
        /* BGEZ */
        branch = ((int32_t) r3000_state->regs[rs] >= 0);
    } else if (rt == 16) {
        /* BLTZAL */
        branch = ((int32_t) r3000_state->regs[rs] < 0);
        link = true;
    } else if (rt == 17) {
        /* BGEZAL */
        branch = ((int32_t) r3000_state->regs[rs] >= 0);
        link = true;
    }

    if (branch) {
        r3000_branch(r3000_state, (int32_t) r3000_state->pc + ((int16_t) imm << 2));
    }

    if (link) {
        r3000_state->regs[R3000_REG_RA] = r3000_state->pc + 4;
    }

    printf("rt: %d rs: %d (rs): %d imm: %d | branch: %d link: %d\n", rt, rs, r3000_state->regs[rs], imm, branch ? 1 : 0, link ? 1 : 0);
}

void opcode_bltz(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint16_t imm)
{
    if (r3000_state->regs[rs] < 0) {
        r3000_branch(r3000_state, (int32_t) r3000_state->pc + ((int16_t) imm << 2));
    }
}

void opcode_bgez(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint16_t imm)
{
    if (r3000_state->regs[rs] >= 0) {
        r3000_branch(r3000_state, (int32_t) r3000_state->pc + ((int16_t) imm << 2));
    }
}

void opcode_bgtz(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint16_t imm)
{
    if ((int32_t) r3000_state->regs[rs] > 0) {
        r3000_branch(r3000_state, (int32_t) r3000_state->pc + ((int16_t) imm << 2));
    }
}

void opcode_blez(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint16_t imm)
{
    if ((int32_t) r3000_state->regs[rs] <= 0) {
        r3000_branch(r3000_state, (int32_t) r3000_state->pc + ((int16_t) imm << 2));
    }
}

/* Coprocessor instruction */

void opcode_mfc0(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rt, uint8_t rd)
{
    #ifdef LOG_DEBUG_R3000
    log_debug("COP0", "MFC | %s -> %s | %08x\n", cop0_register_names[rd], r3000_register_names[rt], r3000_state->cop0_state.regs[rd]);
    #endif

    r3000_enqueue_load(r3000_state, mem_state, rt, r3000_state->cop0_state.regs[rd]);
}

void opcode_mtc0(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rt, uint8_t rd)
{
    #ifdef LOG_DEBUG_R3000
    log_debug("COP0", "MTC | %s -> %s | %08x\n", r3000_register_names[rt], cop0_register_names[rd], r3000_state->regs[rt]);
    #endif

    uint8_t reg = rd;
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

void opcode_cop0(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rs, uint8_t rt, uint8_t rd)
{
    uint8_t opcode = rs;

    switch(opcode) {
        case 0x00:
            opcode_mfc0(r3000_state, mem_state, rt, rd);
            break;

        case 0x04:
            opcode_mtc0(r3000_state, mem_state, rt, rd);
            break;
    }
}

#endif