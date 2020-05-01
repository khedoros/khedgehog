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
	std::string op_name;            // Display name for opcode
	std::string op_format;          // Format of the bits in the operation
    std::array<bool, 13> addr_mode; // Supported addressing modes
    std::array<bool, 3> op_size;    // Byte, word, and long-sized arguments
    int op_length;                  // Size of the instruction in words
};

//Potentially 132 different formats?
std::array<instr_info, 2> instrs{instr_info{"test",  "0000000000000000", {0,0,0,0,0,0,0,0,0,0,0,0,0}, {0,0,0}, 1},
		                         instr_info{"test2", "0000000000000001", {0,0,0,0,0,0,0,0,0,0,0,0,0}, {0,0,0}, 1}
		                        };

/*
0000:
ORI:     0000 0000 SS EEEEEE
ANDI:    0000 0010 SS EEEEEE
SUBI:    0000 0100 SS EEEEEE
ADDI:    0000 0110 SS EEEEEE
EORI:    0000 1010 SS EEEEEE
CMPI:    0000 1100 SS EEEEEE

MOVEP:   0000 DDD1 1000 1AAA
MOVEP:   0000 DDD1 1100 1AAA
MOVEP:   0000 DDD1 0000 1AAA
MOVEP:   0000 DDD1 0100 1AAA

BSET.b   0000 rrr1 11EE EEEE
BSET.b   0000 1000 11EE EEEE
BSET.l   0000 rrr1 11EE EEEE
BSET.l   0000 1000 11EE EEEE

BTST.b   0000 rrr1 00EE EEEE
BTST.b   0000 1000 00EE EEEE
BTST.l   0000 rrr1 00EE EEEE
BTST.l   0000 1000 00EE EEEE

BCHG.b   0000 rrr1 01EE EEEE
BCHG.b   0000 1000 01EE EEEE
BCHG.l   0000 rrr1 01EE EEEE
BCHG.l   0000 1000 01EE EEEE

BCLR.b   0000 rrr1 10EE EEEE
BCLR.b   0000 1000 10EE EEEE
BCLR.l   0000 rrr1 10EE EEEE
BCLR.l   0000 1000 10EE EEEE

0001:
MOVE.b   0001 RRRM MMee eeee

0010:
MOVE.l   0010 RRRM MMee eeee
MOVEA.l  0010 AAA0 01ee eeee

0011:
MOVE.w   0011 RRRM MMee eeee
MOVEA.w  0011 AAA0 01ee eeee

0100:
CHK.w    0100 DDD1 10ee eeee
CLR.b    0100 0010 00EE EEEE
CLR.w    0100 0010 01EE EEEE
CLR.l    0100 0010 10EE EEEE
EXT.w    0100 1000 1000 0DDD
EXT.l    0100 1000 1100 0DDD
JMP      0100 1110 11EE EEEE
JSR      0100 1110 10EE EEEE
LEA      0100 AAA1 11ee eeee
LINK     0100 1110 0101 0AAA
MOV.w    0100 0100 11ee eeee
MOV.w    0100 0110 11ee eeee
MOV.w    0100 0000 11EE EEEE
MOV.l    0100 1110 0110 1AAA
MOV.l    0100 1110 0110 0AAA
MOVEM.w  0100 1000 10EE EEEE
MOVEM.w  0100 1100 10ee eeee
MOVEM.l  0100 1000 11EE EEEE
MOVEM.l  0100 1100 11ee eeee
NBCD     0100 1000 00EE EEEE
NEG.b    0100 0100 00EE EEEE
NEG.w    0100 0100 01EE EEEE
NEG.l    0100 0100 10EE EEEE
NEGX.b   0100 0000 00EE EEEE
NEGX.w   0100 0000 01EE EEEE
NEGX.l   0100 0000 10EE EEEE
NOP      0100 1110 0111 0001
NOT.b    0100 0110 00EE EEEE
NOT.w    0100 0110 01EE EEEE
NOT.l    0100 0110 10EE EEEE
PEA      0100 1000 01ee eeee
RESET    0100 1110 0111 0000
ETE      0100 1110 0111 0011
RTR      0100 1110 0111 0111
RTS      0100 1110 0111 0101
STOP     0100 1110 0111 0010
SWAP     0100 1000 0100 0DDD
TAS      0100 1010 11EE EEEE
TRAP     0100 1110 0100 VVVV
TRAPV    0100 1110 0111 0110
TST.b    0100 1010 00EE EEEE
TST.w    0100 1010 01EE EEEE
TST.l    0100 1010 10EE EEEE
UNLK     0100 1110 0101 1AAA
BRA      0100 0000 PPPP PPPP

0101:
0110:
0111:
1000:
1001:
1011:
1100:
1101:
1110:

not seen: 1010 and 1111
*/
