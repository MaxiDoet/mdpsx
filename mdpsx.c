#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cpu/r3000.h"
#include "mem/mem.h"
#include "renderer.h"
#include "log.h"

r3000_state_t r3000_state;
mem_state_t mem_state;
renderer_t renderer;

int main()
{
    /* Read BIOS */
    FILE *bios_fp = fopen("bios/bios.bin", "rb");
    mem_state.bios = (uint8_t *) malloc(524288);
    fread(mem_state.bios, 1, 524288, bios_fp);
    fclose(bios_fp);

    uint32_t bios_date = *((uint32_t *) (mem_state.bios + 0x100));
    log_info("mdpsx", "Bios | Date: %x/%x/%x\n", (bios_date & 0x0000FF00) >> 8, bios_date & 0x000000FF, (bios_date & 0xFFFF0000) >> 16);

    /* Init CPU */
    r3000_state.pc = 0xBFC00000;
    r3000_state.pc_next = 0xBFC00004;

    /* Init memory */
    mem_state.ram = (uint8_t *) malloc(2048 * 1024);
    mem_state.scratchpad = (uint8_t *) malloc(1024);

    /* Init breakpoints */
    r3000_add_breakpoint(&r3000_state, 0xBFC067E8, "Main");
    r3000_add_breakpoint(&r3000_state, 0xA0000500, "SysInitKMem");
    r3000_add_breakpoint(&r3000_state, 0xBFC042D0, "CopyA0Table");
    r3000_add_breakpoint(&r3000_state, 0xBFC042A0, "InitSyscall");
    r3000_add_breakpoint(&r3000_state, 0x00000540, "PatchA0Table");
    r3000_add_breakpoint(&r3000_state, 0x00000EB0, "InstallExceptionHandlers");
    r3000_add_breakpoint(&r3000_state, 0x00000F2C, "ResetEntryInt");
    r3000_add_breakpoint(&r3000_state, 0xBFC06FF0, "LoadRunShell");

    /* Init GLFW */
    if (!glfwInit()) {
        log_error("mdpsx", "Failed to init GLFW");
        exit(0);
    }

    /* Set OpenGL version */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    /* Create window */
    GLFWwindow *window = glfwCreateWindow(800, 600, "mdpsx", NULL, NULL);

    if (!window) {
        log_error("mdpsx", "Failed to init window");
        exit(0);
    }

    glfwSwapInterval(1);
    glfwMakeContextCurrent(window);

    renderer_init(&renderer, window);

    while(!glfwWindowShouldClose(window)) {
        r3000_step(&r3000_state, &mem_state);

        renderer_render(&renderer);

        //glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}