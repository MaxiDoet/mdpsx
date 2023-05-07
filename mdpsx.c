#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cpu/r3000.h"
#include "mem/mem.h"
#include "log.h"

r3000_state_t r3000_state;
mem_state_t mem_state;

int main()
{
    /* Read BIOS */
    FILE *bios_fp = fopen("bios/bios.bin", "rb");
    mem_state.bios = (uint8_t *) malloc(524288);
    fread(mem_state.bios, 1, 524288, bios_fp);

    uint32_t bios_date = *((uint32_t *) (mem_state.bios + 0x100));
    log_info("mdpsx", "Bios | Date: %x/%x/%x\n", (bios_date & 0x0000FF00) >> 8, bios_date & 0x000000FF, (bios_date & 0xFFFF0000) >> 16);

    /* Init CPU */
    r3000_state.pc = 0xBFC00000;

    /* Init memory */
    mem_state.ram = (uint8_t *) malloc(MEM_RAM_SIZE);

    /* Init breakpoints */
    r3000_add_breakpoint(&r3000_state, 0xA0000500, "SysInitKMem");
    r3000_add_breakpoint(&r3000_state, 0xBFC042D0, "CopyA0Table");
    r3000_add_breakpoint(&r3000_state, 0xBFC042A0, "InitSyscall");
    r3000_add_breakpoint(&r3000_state, 0x00000540, "PatchA0Table");
    r3000_add_breakpoint(&r3000_state, 0x00000EB0, "InstallExceptionHandlers");
    r3000_add_breakpoint(&r3000_state, 0x00000F2C, "ResetEntryInt");
    r3000_add_breakpoint(&r3000_state, 0xBFC06FF0, "LoadRunShell");

    while(true) {
        r3000_step(&r3000_state, &mem_state);
    }
}