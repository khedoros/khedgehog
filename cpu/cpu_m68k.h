#pragma once

#include<functional>
#include<unordered_map>
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

    std::array<std::function<uint64_t(uint16_t)>, 8192> op_table;
    std::unordered_map<ops, std::function<uint64_t(uint16_t)>>    op_map;

    uint64_t op_ABCD(uint16_t opcode);
    uint64_t op_ADD(uint16_t opcode);
    uint64_t op_ADDA(uint16_t opcode);
    uint64_t op_ADDI(uint16_t opcode);
    uint64_t op_ADDQ(uint16_t opcode);
    uint64_t op_ADDX(uint16_t opcode);
    uint64_t op_AND(uint16_t opcode);
    uint64_t op_ANDI(uint16_t opcode);
    uint64_t op_ASd(uint16_t opcode);
    uint64_t op_Bcc(uint16_t opcode);
    uint64_t op_BCHG(uint16_t opcode);
    uint64_t op_BCLR(uint16_t opcode);
    uint64_t op_BRA(uint16_t opcode);
    uint64_t op_BSET(uint16_t opcode);
    uint64_t op_BSR(uint16_t opcode);
    uint64_t op_BTST(uint16_t opcode);
    uint64_t op_CHK(uint16_t opcode);
    uint64_t op_CLR(uint16_t opcode);
    uint64_t op_CMP(uint16_t opcode);
    uint64_t op_CMPA(uint16_t opcode);
    uint64_t op_CMPI(uint16_t opcode);
    uint64_t op_CMPM(uint16_t opcode);
    uint64_t op_DBcc(uint16_t opcode);
    uint64_t op_DIVS(uint16_t opcode);
    uint64_t op_DIVU(uint16_t opcode);
    uint64_t op_EOR(uint16_t opcode);
    uint64_t op_EORI(uint16_t opcode);
    uint64_t op_EXG(uint16_t opcode);
    uint64_t op_EXT(uint16_t opcode);
    uint64_t op_ILLEGAL(uint16_t opcode);
    uint64_t op_JMP(uint16_t opcode);
    uint64_t op_JSR(uint16_t opcode);
    uint64_t op_LEA(uint16_t opcode);
    uint64_t op_LINK(uint16_t opcode);
    uint64_t op_LSd(uint16_t opcode);
    uint64_t op_MOVE(uint16_t opcode);
    uint64_t op_MOVEA(uint16_t opcode);
    uint64_t op_MOVE_from_SR(uint16_t opcode);
    uint64_t op_MOVEM(uint16_t opcode);
    uint64_t op_MOVEP(uint16_t opcode);
    uint64_t op_MOVEQ(uint16_t opcode);
    uint64_t op_MOVE_to_CCR(uint16_t opcode);
    uint64_t op_MOVE_to_SR(uint16_t opcode);
    uint64_t op_MOVE_USP(uint16_t opcode);
    uint64_t op_MULS(uint16_t opcode);
    uint64_t op_MULU(uint16_t opcode);
    uint64_t op_NBCD(uint16_t opcode);
    uint64_t op_NEG(uint16_t opcode);
    uint64_t op_NEGX(uint16_t opcode);
    uint64_t op_NONE(uint16_t opcode);
    uint64_t op_NOT(uint16_t opcode);
    uint64_t op_OR(uint16_t opcode);
    uint64_t op_ORI(uint16_t opcode);
    uint64_t op_PEA(uint16_t opcode);
    uint64_t op_ROd(uint16_t opcode);
    uint64_t op_ROXd(uint16_t opcode);
    uint64_t op_SBCD(uint16_t opcode);
    uint64_t op_Scc(uint16_t opcode);
    uint64_t op_SPECIAL(uint16_t opcode);
    uint64_t op_SUB(uint16_t opcode);
    uint64_t op_SUBA(uint16_t opcode);
    uint64_t op_SUBI(uint16_t opcode);
    uint64_t op_SUBQ(uint16_t opcode);
    uint64_t op_SUBX(uint16_t opcode);
    uint64_t op_SWAP(uint16_t opcode);
    uint64_t op_TAS(uint16_t opcode);
    uint64_t op_TRAP(uint16_t opcode);

public:
    uint64_t calc(uint64_t cycles);
    cpu_m68k();
};
