#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "cpu/r3000.h"
#include "cpu/opcodes.h"
#include "log.h"

const char *r3000_primary_opcode_names[] = {
    "", "BcondZ", "J", "JAL", "BEQ", "BNE", "BLEZ", "BGTZ",
    "ADDI", "ADDIU", "SLTI", "SLTIU", "ANDI", "ORI", "XORI", "LUI",
    "COP0", "COP1", "COP2", "COP3", "", "", "", "", 
    "", "", "", "", "", "", "", "",
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

const char *r3000_exception_cause_names[] = {
    "Interrupt", "", "", "",
    "Address error load/fetch", "Address error store", "Bus error on instruction fetch", "Bus error on load/store",
    "Syscall"
};

void r3000_enqueue_load(r3000_state_t *r3000_state, bus_state_t *bus_state, uint8_t rt, uint32_t value)
{
    //r3000_state->load_delay_slot_state = DELAY_SLOT_STATE_ARMED;

    r3000_state->load_delay_reg = rt;
    r3000_state->load_delay_value = value;
}

void r3000_branch(r3000_state_t *r3000_state, uint32_t addr)
{
    r3000_state->branch_delay_slot_state = DELAY_SLOT_STATE_ARMED;
    r3000_state->branch_addr = addr;
}

void r3000_exception(r3000_state_t *r3000_state, uint8_t cause) {
    // Save current pc
    r3000_state->cop0_state.regs[COP0_REG_EPC] = r3000_state->pc;

    // Check if we're in a branch delay slot
    if (r3000_state->branch_delay_slot_state == DELAY_SLOT_STATE_DELAY_CYCLE) {
        r3000_state->cop0_state.regs[COP0_REG_EPC] -= 4;
        r3000_state->cop0_state.regs[COP0_REG_CAUSE] |= (1 << 31);
    }

    r3000_state->cop0_state.regs[COP0_REG_CAUSE] = cause << 2;

    // Disable interrupts
    r3000_state->cop0_state.regs[COP0_REG_SR] &= ~COP0_SR_IEC;

    r3000_state->pc_next = (r3000_state->cop0_state.regs[COP0_REG_SR] & COP0_SR_BEV) ? 0xBFC00180 : 0x80000080;

    #ifdef LOG_DEBUG_R3000_EXCEPTIONS
    log_debug("R3000", "Exception | Cause: %s EPC: %08x\n", r3000_exception_cause_names[cause], r3000_state->cop0_state.regs[COP0_REG_EPC]);
    #endif
}

void r3000_add_breakpoint(r3000_state_t *r3000_state, uint32_t pc, char *name)
{
    r3000_state->breakpoints[r3000_state->breakpoint_count].pc = pc;
    r3000_state->breakpoints[r3000_state->breakpoint_count++].name = name;
}

void r3000_step(r3000_state_t *r3000_state, bus_state_t *bus_state)
{
    if (r3000_state->branch_delay_slot_state == DELAY_SLOT_STATE_ARMED) {
        r3000_state->branch_delay_slot_state = DELAY_SLOT_STATE_DELAY_CYCLE;
    }

    uint32_t instruction = bus_read(bus_state, BUS_SIZE_DWORD, r3000_state->pc);
    r3000_state->pc_next += 4;

    #ifdef R3000_BREAKPOINTS
    for (uint8_t i=0; i < r3000_state->breakpoint_count; i++) {
        if (r3000_state->breakpoints[i].pc == r3000_state->pc) {
            log_debug("breakpoint", "%s\n", r3000_state->breakpoints[i].name);
        }
    }
    #endif

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
    log_debug("R3000", "pc: %08x | %s (%08x) | rs: %s rt: %s rd: %s imm: %x addr: %x | at: %x v0: %x v1: %x a0: %x a1: %x a2: %x a3: %x t0: %x t1: %x t2: %x t3: %x t4: %x t5: %x t6: %x t7: %x sp: %x ra: %x s0: %x s1: %x s2: %x s3: %x s4: %x s5: %x s6: %x s7: %x\n", r3000_state->pc, opcode ? r3000_primary_opcode_names[opcode] : r3000_secondary_opcode_names[funct], instruction, r3000_register_names[rs], r3000_register_names[rt], r3000_register_names[rd], imm, target_address,
        r3000_state->regs[R3000_REG_AT], r3000_state->regs[R3000_REG_V0], r3000_state->regs[R3000_REG_V1], r3000_state->regs[R3000_REG_A0], r3000_state->regs[R3000_REG_A1], r3000_state->regs[R3000_REG_A2], r3000_state->regs[R3000_REG_A3], r3000_state->regs[R3000_REG_T0],
        r3000_state->regs[R3000_REG_T1], r3000_state->regs[R3000_REG_T2], r3000_state->regs[R3000_REG_T3], r3000_state->regs[R3000_REG_T4], r3000_state->regs[R3000_REG_T5], r3000_state->regs[R3000_REG_T6], r3000_state->regs[R3000_REG_T7], r3000_state->regs[R3000_REG_SP],
        r3000_state->regs[R3000_REG_RA], r3000_state->regs[R3000_REG_S0], r3000_state->regs[R3000_REG_S1], r3000_state->regs[R3000_REG_S2], r3000_state->regs[R3000_REG_S3], r3000_state->regs[R3000_REG_S4], r3000_state->regs[R3000_REG_S5], r3000_state->regs[R3000_REG_S6],
        r3000_state->regs[R3000_REG_S7]
    );
    #endif

    /* SPECIAL */
    if (!opcode) {
        switch(funct) {
            case 0x00: opcode_sll(r3000_state, bus_state, rs, rt, rd, shamt); break;
            case 0x02: opcode_srl(r3000_state, bus_state, rs, rt, rd, shamt); break;
            case 0x03: opcode_sra(r3000_state, bus_state, rs, rt, rd, shamt); break;
            case 0x04: opcode_sllv(r3000_state, bus_state, rs, rt, rd); break;
            case 0x06: opcode_srlv(r3000_state, bus_state, rs, rt, rd); break;
            case 0x07: opcode_srav(r3000_state, bus_state, rs, rt, rd); break;
            case 0x08: opcode_jr(r3000_state, bus_state, rs); break;
            case 0x09: opcode_jalr(r3000_state, bus_state, rs, rd); break;
            case 0x0C: opcode_syscall(r3000_state, bus_state); break;
            case 0x10: opcode_mfhi(r3000_state, bus_state, rd); break;
            case 0x11: opcode_mthi(r3000_state, bus_state, rs); break;
            case 0x12: opcode_mflo(r3000_state, bus_state, rd); break;
            case 0x13: opcode_mtlo(r3000_state, bus_state, rs); break;
            case 0x18: opcode_mult(r3000_state, bus_state, rs, rt); break;
            case 0x19: opcode_multu(r3000_state, bus_state, rs, rt); break;
            case 0x1A: opcode_div(r3000_state, bus_state, rs, rt); break;
            case 0x1B: opcode_divu(r3000_state, bus_state, rs, rt); break;
            case 0x20: opcode_add(r3000_state, bus_state, rs, rt, rd); break;
            case 0x21: opcode_addu(r3000_state, bus_state, rs, rt, rd); break;
            case 0x22: opcode_sub(r3000_state, bus_state, rs, rt, rd); break;
            case 0x23: opcode_subu(r3000_state, bus_state, rs, rt, rd); break;
            case 0x24: opcode_and(r3000_state, bus_state, rs, rt, rd); break;
            case 0x25: opcode_or(r3000_state, bus_state, rs, rt, rd); break;
            case 0x26: opcode_xor(r3000_state, bus_state, rs, rt, rd); break;
            case 0x27: opcode_nor(r3000_state, bus_state, rs, rt, rd); break;
            case 0x2A: opcode_slt(r3000_state, bus_state, rs, rt, rd); break;
            case 0x2B: opcode_sltu(r3000_state, bus_state, rs, rt, rd); break;
            default: exit(0); break;
        }
    } else {
        switch(opcode) {
            case 0x01: opcode_bcondz(r3000_state, bus_state, rs, rt, imm); break;
            case 0x02: opcode_j(r3000_state, bus_state, target_address); break;
            case 0x03: opcode_jal(r3000_state, bus_state, target_address); break;
            case 0x04: opcode_beq(r3000_state, bus_state, rs, rt, imm); break;
            case 0x05: opcode_bne(r3000_state, bus_state, rs, rt, imm); break;
            case 0x06: opcode_blez(r3000_state, bus_state, rs, imm); break;
            case 0x07: opcode_bgtz(r3000_state, bus_state, rs, imm); break;
            case 0x08: opcode_addi(r3000_state, bus_state, rs, rt, imm); break;
            case 0x09: opcode_addiu(r3000_state, bus_state, rs, rt, imm); break;
            case 0x0A: opcode_slti(r3000_state, bus_state, rs, rt, imm); break;
            case 0x0B: opcode_sltiu(r3000_state, bus_state, rs, rt, imm); break;
            case 0x0C: opcode_andi(r3000_state, bus_state, rs, rt, imm); break;
            case 0x0D: opcode_ori(r3000_state, bus_state, rs, rt, imm); break;
            case 0x0E: opcode_xori(r3000_state, bus_state, rs, rt, imm); break;
            case 0x0F: opcode_lui(r3000_state, bus_state, rs, rt, imm); break;
            case 0x10: opcode_cop0(r3000_state, bus_state, rs, rt, rd, imm); break;
            case 0x20: opcode_lb(r3000_state, bus_state, rs, rt, imm); break;
            case 0x21: opcode_lh(r3000_state, bus_state, rs, rt, imm); break;
            case 0x23: opcode_lw(r3000_state, bus_state, rs, rt, imm); break;
            case 0x24: opcode_lbu(r3000_state, bus_state, rs, rt, imm); break;
            case 0x25: opcode_lhu(r3000_state, bus_state, rs, rt, imm); break;
            case 0x28: opcode_sb(r3000_state, bus_state, rs, rt, imm); break;
            case 0x29: opcode_sh(r3000_state, bus_state, rs, rt, imm); break;
            case 0x2B: opcode_sw(r3000_state, bus_state, rs, rt, imm); break;
            default: exit(0); break;
        }
    }

    if (r3000_state->load_delay_reg != r3000_state->load_reg) {
        r3000_state->regs[r3000_state->load_reg] = r3000_state->load_value;
    }

    r3000_state->load_reg = r3000_state->load_delay_reg;
    r3000_state->load_value = r3000_state->load_delay_value;

    r3000_state->load_delay_reg = 0;

    if (r3000_state->branch_delay_slot_state == DELAY_SLOT_STATE_DELAY_CYCLE) {
        r3000_state->pc_next = r3000_state->branch_addr;

        r3000_state->branch_delay_slot_state = 0;
    }

    r3000_state->pc = r3000_state->pc_next;

    /* R0 needs to be zero */
    r3000_state->regs[0] = 0;
}