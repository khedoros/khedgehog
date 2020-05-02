#pragma once

#include<string>
#include<array>

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

enum op_size {
    s_byte, // One byte
    s_word, // Two bytes
    s_long  // Four bytes
};

struct instr_id {
        std::string op_name;            // Display name for opcode
        uint16_t id_bits;               // Bits to identify the instruction
        uint16_t mask_bits;             // Bits to mask off for ID purposes
};

//Potentially 132 different formats?
std::array<instr_id, 75> instrs{instr_id{"ORI",          0b0000000000000000, 0b1111111100000000},
                                instr_id{"ANDI",         0b0000001000000000, 0b1111111100000000},
                                instr_id{"SUBI",         0b0000010000000000, 0b1111111100000000},
                                instr_id{"ADDI",         0b0000011000000000, 0b1111111100000000},
                                instr_id{"EORI",         0b0000101000000000, 0b1111111100000000},
                                instr_id{"CMPI",         0b0000110000000000, 0b1111111100000000},
                                instr_id{"BTST",         0b0000100000000000, 0b1111111111000000},
                                instr_id{"BCHG",         0b0000100001000000, 0b1111111111000000},
                                instr_id{"BCLR",         0b0000100010000000, 0b1111111111000000},
                                instr_id{"BSET",         0b0000100011000000, 0b1111111111000000},
                                instr_id{"BTST",         0b0000000100000000, 0b1111000111000000},
                                instr_id{"BCHG",         0b0000000101000000, 0b1111000111000000},
                                instr_id{"BCLR",         0b0000000110000000, 0b1111000111000000},
                                instr_id{"BSET",         0b0000000111000000, 0b1111000111000000},
                                instr_id{"MOVEP",        0b0000000100001000, 0b1111000100111000},
                                instr_id{"MOVEA",        0b0000000001000000, 0b1100000111000000},
                                instr_id{"MOVE from SR", 0b0100000011000000, 0b1111111111000000},
                                instr_id{"MOVE to CCR",  0b0100010011000000, 0b1111111111000000},
                                instr_id{"MOVE to SR",   0b0100011011000000, 0b1111111111000000},
                                instr_id{"NEGX", 0b, 0b},
                                instr_id{"CLR", 0b, 0b},
                                instr_id{"NEG", 0b, 0b},
                                instr_id{"NOT", 0b, 0b},
                                instr_id{"EXT", 0b, 0b},
                                instr_id{"NBCD", 0b, 0b},
                                instr_id{"SWAP", 0b, 0b},
                                instr_id{"PEA", 0b, 0b},
                                instr_id{"ILLEGAL", 0b, 0b},
                                instr_id{"TAS", 0b, 0b},
                                instr_id{"TST", 0b, 0b},
                                instr_id{"TRAP", 0b, 0b},
                                instr_id{"LINK", 0b, 0b},
                                instr_id{"UNLK", 0b, 0b},
                                instr_id{"MOVE USP", 0b, 0b},
                                instr_id{"SPECIAL", 0b, 0b}, //reset, nop, stop, rte, rts, trapv, rtr
                                instr_id{"JSR", 0b, 0b},
                                instr_id{"JMP", 0b, 0b},
                                instr_id{"MOVEM", 0b, 0b},
                                instr_id{"LEA", 0b, 0b},
                                instr_id{"CHK", 0b, 0b},
                                instr_id{"ADDQ", 0b, 0b},
                                instr_id{"SUBQ", 0b, 0b},
                                instr_id{"Scc", 0b, 0b},
                                instr_id{"DBcc", 0b, 0b},
                                instr_id{"BRA", 0b, 0b},
                                instr_id{"BSR", 0b, 0b},
                                instr_id{"Bcc", 0b, 0b},
                                instr_id{"MOVEQ", 0b, 0b},
                                instr_id{"DIVU", 0b, 0b},
                                instr_id{"DIVS", 0b, 0b},
                                instr_id{"SBCD", 0b, 0b},
                                instr_id{"OR", 0b, 0b},
                                instr_id{"SUB", 0b, 0b},
                                instr_id{"SUBX", 0b, 0b},
                                instr_id{"SUBA", 0b, 0b},
                                instr_id{"EOR", 0b, 0b},
                                instr_id{"CMPM", 0b, 0b},
                                instr_id{"CMP", 0b, 0b},
                                instr_id{"CMPA", 0b, 0b},
                                instr_id{"MULU", 0b, 0b},
                                instr_id{"MULS", 0b, 0b},
                                instr_id{"ABCD", 0b, 0b},
                                instr_id{"EXG", 0b, 0b},
                                instr_id{"AND", 0b, 0b},
                                instr_id{"ADD", 0b, 0b},
                                instr_id{"ADDX", 0b, 0b},
                                instr_id{"ADDA", 0b, 0b},
                                instr_id{"ASd", 0b, 0b},
                                instr_id{"LSd", 0b, 0b},
                                instr_id{"ROXd", 0b, 0b},
                                instr_id{"ROd", 0b, 0b},
                                instr_id{"ASd", 0b, 0b},
                                instr_id{"LSd", 0b, 0b},
                                instr_id{"ROXd", 0b, 0b},
                                instr_id{"ROd", 0b, 0b},
                               };

