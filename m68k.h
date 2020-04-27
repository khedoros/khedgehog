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

enum op_size {
    s_byte, // One byte
    s_word, // Two bytes
    s_long  // Four bytes
};

struct instr_info {
    string op_name;                 // Display name for opcode
    std::array<bool, 13> addr_mode; // Supported addressing modes
    std::array<bool, 3> op_size;    // Byte, word, and long-sized arguments
    int op_length;                  // Size of the instruction in words
};

//Potentially 132 different formats?
std:array<27+23+28+20+20+14, instr_info> instrs;
