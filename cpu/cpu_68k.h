#pragma once

#include "m68k_instructions.h"

class cpu_m68k {
    class m68k_dreg {
        uint32_t d;
    };
    class m68k_areg {
        uint32_t a;
    };
    class m68k_ccr {
//      system   user
//     TSMOIII|000XNZVC
        uint16_t c;
    };

    m68k_dreg dreg[8];
    m68k_areg areg[7];
    m68k_areg sp[2]; //A7, USP is sp[0], SSP is sp[1]
    uint32_t pc;
    m68k_ccr ccr;

    enum stack_type {
        user,
        supervisor
    };

    stack_type cur_stack = supervisor;
};
