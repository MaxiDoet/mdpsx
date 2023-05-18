#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "cpu/r3000.h"
#include "cpu/opcodes.h"
#include "log.h"

#define R3000_OPCODE_NIU   0
#define R3000_OPCODE_RTYPE 1
#define R3000_OPCODE_ITYPE 2
#define R3000_OPCODE_JTYPE 3

const char *r3000_primary_opcode_names[] = {
    "", "BcondZ", "J", "JAL", "BEQ", "BNE", "BLEZ", "BGTZ",
    "ADDI", "ADDIU", "SLTI", "SLTIU", "ANDI", "ORI", "XORI", "LUI",
    "COP0", "COP1", "COP2", "COP3", "", "", "", "", "", "", "", "", "", "", "", "",
    "LB", "LH", "LWL", "LW", "LBU", "LHU", "LWR", "",
    "SB", "SH", "SWL", "SW", "", "", "SWR", "",
    "LWC0", "LWC1", "LWC2", "LWC3", "", "", "", "",
    "SWC0", "SWC1", "SWC2", "SWC3", "", "", "", ""
};

const char *r3000_secondary_opcode_names[] = {
    "SLL", "", "SRL", "SRA", "SLLV", "", "SRVL", "SRAV",
    "JR", "JALR", "", "", "SYSCALL", "BREAK", "", "",
    "MFHI", "MTHI", "MFLO", "MTLO", "", "", "", "",
    "MULT", "MULTU", "DIV", "DIVU", "", "", "", "",
    "ADD", "ADDU", "SUB", "SUBU", "AND", "OR", "XOR", "NOR",
    "", "", "SLT", "SLTU", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", ""
};

/*
const char *r3000_primary_opcode_types[] = {
    R3000_OPCODE_NIU, R3000_OPCODE_ITYPE, R3000_OPCODE_JTYPE, R3000_OPCODE_JTYPE, R3000_OPCODE_ITYPE, R3000_OPCODE_ITYPE, R3000_OPCODE_ITYPE, R3000_OPCODE_ITYPE,
    R3000_OPCODE_RTYPE, R3000_OPCODE_RTYPE, R3000_OPCODE_RTYPE, R3000_OPCODE_RTYPE, R3000_OPCODE_RTYPE, R3000_OPCODE_RTYPE, R3000_OPCODE_RTYPE, R3000_OPCODE_RTYPE,
    R3000_OPCODE_JTYPE, R3000_OPCODE_JTYPE, R3000_OPCODE_JTYPE, R3000_OPCODE_JTYPE, R3000_OPCODE_NIU, R3000_OPCODE_NIU, R3000_OPCODE_NIU, R3000_OPCODE_NIU,
    R3000_OPCODE_NIU, R3000_OPCODE_NIU, R3000_OPCODE_NIU, R3000_OPCODE_NIU, R3000_OPCODE_NIU, R3000_OPCODE_NIU, R3000_OPCODE_NIU, R3000_OPCODE_NIU,
};
*/

void r3000_enqueue_load(r3000_state_t *r3000_state, mem_state_t *mem_state, uint8_t rt, uint32_t value)
{
    r3000_state->load = true;

    r3000_state->load_delay_register = rt;
    r3000_state->load_delay_value = value;
}

void r3000_branch(r3000_state_t *r3000_state, uint32_t addr)
{
    r3000_state->branch = true;
    r3000_state->branch_addr = addr;
}

void r3000_add_breakpoint(r3000_state_t *r3000_state, uint32_t pc, char *name)
{
    r3000_state->breakpoints[r3000_state->breakpoint_count].pc = pc;
    r3000_state->breakpoints[r3000_state->breakpoint_count++].name = name;
}

void r3000_step(r3000_state_t *r3000_state, mem_state_t *mem_state)
{
    if (r3000_state->branch && !r3000_state->branch_delay_slot_done) {
        r3000_state->branch_delay_slot = true;
    } else if (r3000_state->branch_delay_slot_done) {
        r3000_state->pc = r3000_state->branch_addr;

        r3000_state->branch = false;
        r3000_state->branch_addr = 0;
        r3000_state->branch_delay_slot = false;
        r3000_state->branch_delay_slot_done = false;
    }

    if (r3000_state->load && !r3000_state->load_delay_done) {
        r3000_state->load_delay = true;
    } else if (r3000_state->load_delay_done) {
        r3000_state->regs[r3000_state->load_delay_register] = r3000_state->load_delay_value;

        r3000_state->load = false;
        r3000_state->load_delay_value = 0;
        r3000_state->load_delay = false;
        r3000_state->load_delay_done = false;
    }

    if (r3000_state->pc == 0xbfc018d4) {
        r3000_state->pc = 0xBFC01910;
    }

    uint32_t instruction = mem_read(mem_state, MEM_SIZE_DWORD, r3000_state->pc);

    #ifdef R3000_BREAKPOINTS
    for (uint8_t i=0; i < r3000_state->breakpoint_count; i++) {
        if (r3000_state->breakpoints[i].pc == r3000_state->pc) {
            log_debug("breakpoint", "%s\n", r3000_state->breakpoints[i].name);
        }
    } 
    #endif

    r3000_state->pc += 4;

    uint8_t opcode = (instruction & 0x0FC000000) >> 26;
    uint8_t funct = instruction & 0x0000003F;

    uint8_t rs = (instruction & 0x03E00000) >> 21;
    uint8_t rt = (instruction & 0x001F0000) >> 16;

    /* R-Type */
    uint8_t rd = (instruction & 0x0000F800) >> 11;
    uint8_t shamt = (instruction & 0x000007C0) >> 6;

    /* I-Type */
    uint16_t imm = (instruction & 0x0000FFFF);

    /* J-Type */
    uint32_t target_address = (instruction & 0x03FFFFFF);

    #ifdef LOG_DEBUG_R3000
    log_debug("R3000", "pc: %08x | %s (%08x) | rs: %s rt: %s rd: %s imm: %x addr: %x | at: %x v0: %x v1: %x a0: %x a1: %x a2: %x a3: %x t0: %x t1: %x t2: %x t3: %x t4: %x t5: %x t6: %x t7: %x sp: %x ra: %x\n", r3000_state->pc - 4, opcode ? r3000_primary_opcode_names[opcode] : r3000_secondary_opcode_names[funct], instruction, r3000_register_names[rs], r3000_register_names[rt], r3000_register_names[rd], imm, target_address,
        r3000_state->regs[R3000_REG_AT], r3000_state->regs[R3000_REG_V0], r3000_state->regs[R3000_REG_V1], r3000_state->regs[R3000_REG_A0], r3000_state->regs[R3000_REG_A1], r3000_state->regs[R3000_REG_A2], r3000_state->regs[R3000_REG_A3], r3000_state->regs[R3000_REG_T0],
        r3000_state->regs[R3000_REG_T1], r3000_state->regs[R3000_REG_T2], r3000_state->regs[R3000_REG_T3], r3000_state->regs[R3000_REG_T4], r3000_state->regs[R3000_REG_T5], r3000_state->regs[R3000_REG_T6], r3000_state->regs[R3000_REG_T7], r3000_state->regs[R3000_REG_SP],
        r3000_state->regs[R3000_REG_RA]
    );
    #endif

    /* SPECIAL */
    if (!opcode) {
        switch(funct) {
            case 0x00: opcode_sll(r3000_state, mem_state, rs, rt, rd, shamt); break;
            case 0x02: opcode_srl(r3000_state, mem_state, rs, rt, rd, shamt); break;
            case 0x03: opcode_sra(r3000_state, mem_state, rs, rt, rd, shamt); break;
            case 0x04: opcode_sllv(r3000_state, mem_state, rs, rt, rd); break;
            case 0x06: opcode_srlv(r3000_state, mem_state, rs, rt, rd); break;
            case 0x07: opcode_srav(r3000_state, mem_state, rs, rt, rd); break;
            case 0x08: opcode_jr(r3000_state, mem_state, rs); break;
            case 0x09: opcode_jalr(r3000_state, mem_state, rs, rd); break;
            case 0x10: opcode_mfhi(r3000_state, mem_state, rd); break;
            case 0x11: opcode_mthi(r3000_state, mem_state, rs); break;
            case 0x12: opcode_mflo(r3000_state, mem_state, rd); break;
            case 0x13: opcode_mtlo(r3000_state, mem_state, rs); break;
            case 0x18: opcode_mult(r3000_state, mem_state, rs, rt); break;
            case 0x19: opcode_multu(r3000_state, mem_state, rs, rt); break;
            case 0x1A: opcode_div(r3000_state, mem_state, rs, rt); break;
            case 0x1B: opcode_divu(r3000_state, mem_state, rs, rt); break;
            case 0x20: opcode_add(r3000_state, mem_state, rs, rt, rd); break;
            case 0x21: opcode_addu(r3000_state, mem_state, rs, rt, rd); break;
            case 0x22: opcode_sub(r3000_state, mem_state, rs, rt, rd); break;
            case 0x23: opcode_subu(r3000_state, mem_state, rs, rt, rd); break;
            case 0x24: opcode_and(r3000_state, mem_state, rs, rt, rd); break;
            case 0x25: opcode_or(r3000_state, mem_state, rs, rt, rd); break;
            case 0x26: opcode_xor(r3000_state, mem_state, rs, rt, rd); break;
            case 0x27: opcode_nor(r3000_state, mem_state, rs, rt, rd); break;
            case 0x2A: opcode_slt(r3000_state, mem_state, rs, rt, rd); break;
            case 0x2B: opcode_sltu(r3000_state, mem_state, rs, rt, rd); break;
            default: exit(0); break;
        }
    } else {
        switch(opcode) {
            case 0x01: opcode_bcondz(r3000_state, mem_state, rs, rt, imm); break;
            case 0x02: opcode_j(r3000_state, mem_state, target_address); break;
            case 0x03: opcode_jal(r3000_state, mem_state, target_address); break;
            case 0x04: opcode_beq(r3000_state, mem_state, rs, rt, imm); break;
            case 0x05: opcode_bne(r3000_state, mem_state, rs, rt, imm); break;
            case 0x06: opcode_blez(r3000_state, mem_state, rs, imm); break;
            case 0x07: opcode_bgtz(r3000_state, mem_state, rs, imm); break;
            case 0x08: opcode_addi(r3000_state, mem_state, rs, rt, imm); break;
            case 0x09: opcode_addiu(r3000_state, mem_state, rs, rt, imm); break;
            case 0x0A: opcode_slti(r3000_state, mem_state, rs, rt, imm); break;
            case 0x0B: opcode_sltiu(r3000_state, mem_state, rs, rt, imm); break;
            case 0x0C: opcode_andi(r3000_state, mem_state, rs, rt, imm); break;
            case 0x0D: opcode_ori(r3000_state, mem_state, rs, rt, imm); break;
            case 0x0E: opcode_xori(r3000_state, mem_state, rs, rt, imm); break;
            case 0x0F: opcode_lui(r3000_state, mem_state, rs, rt, imm); break;
            case 0x10: opcode_cop0(r3000_state, mem_state, rs, rt, rd); break;
            case 0x20: opcode_lb(r3000_state, mem_state, rs, rt, imm); break;
            case 0x21: opcode_lh(r3000_state, mem_state, rs, rt, imm); break;
            case 0x23: opcode_lw(r3000_state, mem_state, rs, rt, imm); break;
            case 0x24: opcode_lbu(r3000_state, mem_state, rs, rt, imm); break;
            case 0x25: opcode_lhu(r3000_state, mem_state, rs, rt, imm); break;
            case 0x28: opcode_sb(r3000_state, mem_state, rs, rt, imm); break;
            case 0x29: opcode_sh(r3000_state, mem_state, rs, rt, imm); break;
            case 0x2B: opcode_sw(r3000_state, mem_state, rs, rt, imm); break;
            default: exit(0); break;
        }
    }

    if (r3000_state->branch_delay_slot) {
        r3000_state->branch_delay_slot_done = true;
    }

    if (r3000_state->load_delay) {
        r3000_state->load_delay_done = true;
    }
}