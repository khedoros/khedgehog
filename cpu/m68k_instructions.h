#pragma once

#include<cstddef>
#include<string>
#include<array>
#include<functional>

#include "cpu_m68k.h"

class cpu_m68k;

enum addr_modes {
    data_reg,      //000 Reg Argument from data register
    addr_reg,      //001 Reg Argument from address register
    addr_reg_ind,  //010 Reg Argument loaded from memory, with address in register
    auto_post_inc, //011 Reg Same, but increment address after fetch
    auto_pre_dec,  //100 Reg Same, but decrement address before fetch
    index_basic,   //101 Reg Addr indirect with 16-bit signed displacement
    index_full,    //110 Reg Addr indirect with 8-bit signed offset and 16 or 32-bit index register
    abs_short,     //111 000 Absolute 15-bit sign-extended address
    abs_long,      //111 001 Absolute 24-bit address
    rel_basic,     //111 010 Relative to program counter with 16-bit signed displacement
    rel_full,      //111 011 Relative to program counter with 8-bit signed offset and 16 or 32-bit index register
    immed          //111 100 Immediate value
};

enum ops {
        ori,
        andi,
        subi,
        addi,
        eori,
        cmpi,
        btst,
        bchg,
        bclr,
        bset,
        movep,
        movea,
        move_from_sr,
        move_to_ccr,
        move_to_sr,
        move,
        negx,
        clr,
        neg,
        notop,
        ext,
        nbcd,
        swap,
        pea,
        illegal,
        tas,
        tst,
        trap,
        link,
        unlk,
        move_usp,
        special,
        jsr,
        jmp,
        movem,
        lea,
        chk,
        dbcc,
        scc,
        addq,
        subq,
        bra,
        bsr,
        bcc,
        moveq,
        divu,
        divs,
        sbcd,
        orop,
        suba,
        subx,
        sub,
        cmpa,
        cmpm,
        eor,
        cmp,
        mulu,
        muls,
        abcd,
        exg,
        andop,
        adda,
        addx,
        add,
        asd,
        lsd,
        roxd,
        rod,
        none
};

enum op_size {
    s_byte, // One byte
    s_word, // Two bytes
    s_long  // Four bytes
};

struct instr_id {
        std::string op_name;            // Display name for opcode
        uint16_t id_bits;               // Bits to identify the instruction
        uint16_t mask_bits;             // Bits to mask off for ID purposes
        ops func;
};

std::array<instr_id, 78> instrs{
    instr_id{"ORI",          0b0000000000000000, 0b1111111100000000, ori},
    instr_id{"ANDI",         0b0000001000000000, 0b1111111100000000, andi},
    instr_id{"SUBI",         0b0000010000000000, 0b1111111100000000, subi},
    instr_id{"ADDI",         0b0000011000000000, 0b1111111100000000, addi},
    instr_id{"EORI",         0b0000101000000000, 0b1111111100000000, eori},
    instr_id{"CMPI",         0b0000110000000000, 0b1111111100000000, cmpi},
    instr_id{"MOVEP",        0b0000000100001000, 0b1111000100111000, movep},
    instr_id{"BTST",         0b0000100000000000, 0b1111111111000000, btst},
    instr_id{"BCHG",         0b0000100001000000, 0b1111111111000000, bchg},
    instr_id{"BCLR",         0b0000100010000000, 0b1111111111000000, bclr},
    instr_id{"BSET",         0b0000100011000000, 0b1111111111000000, bset},
    instr_id{"BTST",         0b0000000100000000, 0b1111000111000000, btst},
    instr_id{"BCHG",         0b0000000101000000, 0b1111000111000000, bchg},
    instr_id{"BCLR",         0b0000000110000000, 0b1111000111000000, bclr},
    instr_id{"BSET",         0b0000000111000000, 0b1111000111000000, bset},
    instr_id{"MOVEA",        0b0010000001000000, 0b1110000111000000, movea},
    instr_id{"MOVE",         0b0010000000000000, 0b1110000000000000, move},
    instr_id{"MOVE",         0b0001000000000000, 0b1101000000000000, move},
    instr_id{"MOVE from SR", 0b0100000011000000, 0b1111111111000000, move_from_sr},
    instr_id{"MOVE to CCR",  0b0100010011000000, 0b1111111111000000, move_to_ccr},
    instr_id{"MOVE to SR",   0b0100011011000000, 0b1111111111000000, move_to_sr},
    instr_id{"NEGX",         0b0100000000000000, 0b1111111100000000, negx},
    instr_id{"CLR",          0b0100001000000000, 0b1111111100000000, clr},
    instr_id{"NEG",          0b0100010000000000, 0b1111111100000000, neg},
    instr_id{"NOT",          0b0100011000000000, 0b1111111100000000, notop},
    instr_id{"EXT",          0b0100100010000000, 0b1111111110111000, ext},
    instr_id{"NBCD",         0b0100100000000000, 0b1111111111000000, nbcd},
    instr_id{"SWAP",         0b0100100001000000, 0b1111111111111000, swap},
    instr_id{"PEA",          0b0100100001000000, 0b1111111111000000, pea},
    instr_id{"ILLEGAL",      0b0100101011111100, 0b1111111111111111, illegal},
    instr_id{"TAS",          0b0100101011000000, 0b1111111111000000, tas},
    instr_id{"TST",          0b0100101000000000, 0b1111111100000000, tst},
    instr_id{"TRAP",         0b0100111001000000, 0b1111111111110000, trap},
    instr_id{"LINK",         0b0100111001010000, 0b1111111111111000, link},
    instr_id{"UNLK",         0b0100111001011000, 0b1111111111111000, unlk},
    instr_id{"MOVE USP",     0b0100111001100000, 0b1111111111110000, move_usp},
    instr_id{"SPECIAL",      0b0100111001110000, 0b1111111111111000, special},
    instr_id{"JSR",          0b0100111010000000, 0b1111111111000000, jsr},
    instr_id{"JMP",          0b0100111011000000, 0b1111111111000000, jmp},
    instr_id{"MOVEM",        0b0100100010000000, 0b1111101110000000, movem},
    instr_id{"LEA",          0b0100000111000000, 0b1111000111000000, lea},
    instr_id{"CHK",          0b0100000110000000, 0b1111000111000000, chk},
    instr_id{"DBcc",         0b0101000011001000, 0b1111000011111000, dbcc},
    instr_id{"Scc",          0b0101000011000000, 0b1111000011000000, scc},
    instr_id{"ADDQ",         0b0101000000000000, 0b1111000100000000, addq},
    instr_id{"SUBQ",         0b0101000100000000, 0b1111000100000000, subq},
    instr_id{"BRA",          0b0110000000000000, 0b1111111100000000, bra},
    instr_id{"BSR",          0b0110000100000000, 0b1111111100000000, bsr},
    instr_id{"Bcc",          0b0110000000000000, 0b1111000000000000, bcc},
    instr_id{"MOVEQ",        0b0111000000000000, 0b1111000100000000, moveq},
    instr_id{"DIVU",         0b1000000011000000, 0b1111000111000000, divu},
    instr_id{"DIVS",         0b1000000111000000, 0b1111000111000000, divs},
    instr_id{"SBCD",         0b1000000100000000, 0b1111000111110000, sbcd},
    instr_id{"OR",           0b1000000000000000, 0b1111000000000000, orop},
    instr_id{"SUBA",         0b1001000011000000, 0b1111000011000000, suba},
    instr_id{"SUBX",         0b1001000100000000, 0b1111000100110000, subx},
    instr_id{"SUB",          0b1001000000000000, 0b1111000000000000, sub},
    instr_id{"CMPA",         0b1011000011000000, 0b1111000011000000, cmpa},
    instr_id{"CMPM",         0b1011000100001000, 0b1111000100111000, cmpm},
    instr_id{"EOR",          0b1011000100000000, 0b1111000100000000, eor},
    instr_id{"CMP",          0b1011000000000000, 0b1111000100000000, cmp},
    instr_id{"MULU",         0b1100000011000000, 0b1111000111000000, mulu},
    instr_id{"MULS",         0b1100000111000000, 0b1111000111000000, muls},
    instr_id{"ABCD",         0b1100000100000000, 0b1111000111110000, abcd},
    instr_id{"EXG",          0b1100000100000000, 0b1111000100110000, exg},
    instr_id{"AND",          0b1100000000000000, 0b1111000000000000, andop},
    instr_id{"ADDA",         0b1101000011000000, 0b1111000011000000, adda},
    instr_id{"ADDX",         0b1101000100000000, 0b1111000100110000, addx},
    instr_id{"ADD",          0b1101000000000000, 0b1111000000000000, add},
    instr_id{"ASd",          0b1110000011000000, 0b1111111011000000, asd},
    instr_id{"LSd",          0b1110001011000000, 0b1111111011000000, lsd},
    instr_id{"ROXd",         0b1110010011000000, 0b1111111011000000, roxd},
    instr_id{"ROd",          0b1110011011000000, 0b1111111011000000, rod},
    instr_id{"ASd",          0b1110000000000000, 0b1111000000011000, asd},
    instr_id{"LSd",          0b1110000000001000, 0b1111000000011000, lsd},
    instr_id{"ROXd",         0b1110000000010000, 0b1111000000011000, roxd},
    instr_id{"ROd",          0b1110000000011000, 0b1111000000011000, rod},
    instr_id{"NONE",         0b0000000000000000, 0b0000000000000000, none},
};

