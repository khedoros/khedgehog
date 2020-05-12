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

    int op_ABCD(uint16_t opcode);
    int op_ADD(uint16_t opcode);
    int op_ADDA(uint16_t opcode);
    int op_ADDI(uint16_t opcode);
    int op_ADDQ(uint16_t opcode);
    int op_ADDX(uint16_t opcode);
    int op_AND(uint16_t opcode);
    int op_ANDI(uint16_t opcode);
    int op_ASd(uint16_t opcode);
    int op_ASd(uint16_t opcode);
    int op_Bcc(uint16_t opcode);
    int op_BCHG(uint16_t opcode);
    int op_BCHG(uint16_t opcode);
    int op_BCLR(uint16_t opcode);
    int op_BCLR(uint16_t opcode);
    int op_BRA(uint16_t opcode);
    int op_BSET(uint16_t opcode);
    int op_BSET(uint16_t opcode);
    int op_BSR(uint16_t opcode);
    int op_BTST(uint16_t opcode);
    int op_BTST(uint16_t opcode);
    int op_CHK(uint16_t opcode);
    int op_CLR(uint16_t opcode);
    int op_CMP(uint16_t opcode);
    int op_CMPA(uint16_t opcode);
    int op_CMPI(uint16_t opcode);
    int op_CMPM(uint16_t opcode);
    int op_DBcc(uint16_t opcode);
    int op_DIVS(uint16_t opcode);
    int op_DIVU(uint16_t opcode);
    int op_EOR(uint16_t opcode);
    int op_EORI(uint16_t opcode);
    int op_EXG(uint16_t opcode);
    int op_EXT(uint16_t opcode);
    int op_ILLEGAL(uint16_t opcode);
    int op_JMP(uint16_t opcode);
    int op_JSR(uint16_t opcode);
    int op_LEA(uint16_t opcode);
    int op_LINK(uint16_t opcode);
    int op_LSd(uint16_t opcode);
    int op_LSd(uint16_t opcode);
    int op_MOVE(uint16_t opcode);
    int op_MOVE(uint16_t opcode);
    int op_MOVEA(uint16_t opcode);
    int op_MOVE_from_SR(uint16_t opcode);
    int op_MOVEM(uint16_t opcode);
    int op_MOVEP(uint16_t opcode);
    int op_MOVEQ(uint16_t opcode);
    int op_MOVE_to_CCR(uint16_t opcode);
    int op_MOVE_to_SR(uint16_t opcode);
    int op_MOVE_USP(uint16_t opcode);
    int op_MULS(uint16_t opcode);
    int op_MULU(uint16_t opcode);
    int op_NBCD(uint16_t opcode);
    int op_NEG(uint16_t opcode);
    int op_NEGX(uint16_t opcode);
    int op_NONE(uint16_t opcode);
    int op_NOT(uint16_t opcode);
    int op_OR(uint16_t opcode);
    int op_ORI(uint16_t opcode);
    int op_PEA(uint16_t opcode);
    int op_ROd(uint16_t opcode);
    int op_ROd(uint16_t opcode);
    int op_ROXd(uint16_t opcode);
    int op_ROXd(uint16_t opcode);
    int op_SBCD(uint16_t opcode);
    int op_Scc(uint16_t opcode);
    int op_SPECIAL(uint16_t opcode);
    int op_SUB(uint16_t opcode);
    int op_SUBA(uint16_t opcode);
    int op_SUBI(uint16_t opcode);
    int op_SUBQ(uint16_t opcode);
    int op_SUBX(uint16_t opcode);
    int op_SWAP(uint16_t opcode);
    int op_TAS(uint16_t opcode);
    int op_TRAP(uint16_t opcode);

public:
    int calc(int cycles);
    cpu();
};
