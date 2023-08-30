#include <stdint.h>

#include "bios/bios.h"
#include "log.h"

void bios_print_header(uint8_t *bios)
{
    uint32_t bios_date = *((uint32_t *) (bios + 0x100));

    uint16_t bios_date_year = (bios_date & 0xFFFF0000) >> 16;
    uint8_t bios_date_month = (bios_date & 0x0000FF00) >> 8;
    uint8_t bios_date_day = (bios_date & 0x000000FF);

    #ifdef LOG_DEBUG_BIOS
    log_debug("BIOS", "Date: %02x/%02x/%04x\n", bios_date_month, bios_date_day, bios_date_year);
    #endif
}