#include "debug_console.h"
#include "util.h"

void write_control(uint8_t val) {
    dbg_printf(" wrote %02x to console control", val);
}

void write_data(uint8_t val) {
    dbg_printf(" console data: %c", val);
}

