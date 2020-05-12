#include "cpu_m68k.h"
#include "m68_instructions.h"

cpu_m68k::cpu() {
    for(int inst=0;inst<8192;inst++) {
	for(auto &cand: instrs) {
	    int c = (inst & (cand.mask_bits));
	    if(c == cand.id_bits) {
		// store instruction in lookup table
	}
    }
}

int cpu_m68k::op_ABCD(uint16_t opcode) {return -1;}
int cpu_m68k::op_ADD(uint16_t opcode)  {return -1;}
int cpu_m68k::op_ADDA(uint16_t opcode) {return -1;}
int cpu_m68k::op_ADDI(uint16_t opcode) {return -1;}
int cpu_m68k::op_ADDQ(uint16_t opcode) {return -1;}
int cpu_m68k::op_ADDX(uint16_t opcode) {return -1;}
int cpu_m68k::op_AND(uint16_t opcode) {return -1;}
int cpu_m68k::op_ANDI(uint16_t opcode) {return -1;}
int cpu_m68k::op_ASd(uint16_t opcode) {return -1;}
int cpu_m68k::op_ASd(uint16_t opcode) {return -1;}
int cpu_m68k::op_Bcc(uint16_t opcode) {return -1;}
int cpu_m68k::op_BCHG(uint16_t opcode) {return -1;}
int cpu_m68k::op_BCHG(uint16_t opcode) {return -1;}
int cpu_m68k::op_BCLR(uint16_t opcode) {return -1;}
int cpu_m68k::op_BCLR(uint16_t opcode) {return -1;}
int cpu_m68k::op_BRA(uint16_t opcode) {return -1;}
int cpu_m68k::op_BSET(uint16_t opcode) {return -1;}
int cpu_m68k::op_BSET(uint16_t opcode) {return -1;}
int cpu_m68k::op_BSR(uint16_t opcode) {return -1;}
int cpu_m68k::op_BTST(uint16_t opcode) {return -1;}
int cpu_m68k::op_BTST(uint16_t opcode) {return -1;}
int cpu_m68k::op_CHK(uint16_t opcode) {return -1;}
int cpu_m68k::op_CLR(uint16_t opcode) {return -1;}
int cpu_m68k::op_CMP(uint16_t opcode) {return -1;}
int cpu_m68k::op_CMPA(uint16_t opcode) {return -1;}
int cpu_m68k::op_CMPI(uint16_t opcode) {return -1;}
int cpu_m68k::op_CMPM(uint16_t opcode) {return -1;}
int cpu_m68k::op_DBcc(uint16_t opcode) {return -1;}
int cpu_m68k::op_DIVS(uint16_t opcode) {return -1;}
int cpu_m68k::op_DIVU(uint16_t opcode) {return -1;}
int cpu_m68k::op_EOR(uint16_t opcode) {return -1;}
int cpu_m68k::op_EORI(uint16_t opcode) {return -1;}
int cpu_m68k::op_EXG(uint16_t opcode) {return -1;}
int cpu_m68k::op_EXT(uint16_t opcode) {return -1;}
int cpu_m68k::op_ILLEGAL(uint16_t opcode) {return -1;}
int cpu_m68k::op_JMP(uint16_t opcode) {return -1;}
int cpu_m68k::op_JSR(uint16_t opcode) {return -1;}
int cpu_m68k::op_LEA(uint16_t opcode) {return -1;}
int cpu_m68k::op_LINK(uint16_t opcode) {return -1;}
int cpu_m68k::op_LSd(uint16_t opcode) {return -1;}
int cpu_m68k::op_LSd(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVE(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVE(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVEA(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVE_from_SR(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVEM(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVEP(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVEQ(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVE_to_CCR(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVE_to_SR(uint16_t opcode) {return -1;}
int cpu_m68k::op_MOVE_USP(uint16_t opcode) {return -1;}
int cpu_m68k::op_MULS(uint16_t opcode) {return -1;}
int cpu_m68k::op_MULU(uint16_t opcode) {return -1;}
int cpu_m68k::op_NBCD(uint16_t opcode) {return -1;}
int cpu_m68k::op_NEG(uint16_t opcode) {return -1;}
int cpu_m68k::op_NEGX(uint16_t opcode) {return -1;}
int cpu_m68k::op_NONE(uint16_t opcode) {return -1;}
int cpu_m68k::op_NOT(uint16_t opcode) {return -1;}
int cpu_m68k::op_OR(uint16_t opcode) {return -1;}
int cpu_m68k::op_ORI(uint16_t opcode) {return -1;}
int cpu_m68k::op_PEA(uint16_t opcode) {return -1;}
int cpu_m68k::op_ROd(uint16_t opcode) {return -1;}
int cpu_m68k::op_ROd(uint16_t opcode) {return -1;}
int cpu_m68k::op_ROXd(uint16_t opcode) {return -1;}
int cpu_m68k::op_ROXd(uint16_t opcode) {return -1;}
int cpu_m68k::op_SBCD(uint16_t opcode) {return -1;}
int cpu_m68k::op_Scc(uint16_t opcode) {return -1;}
int cpu_m68k::op_SPECIAL(uint16_t opcode) {return -1;}
int cpu_m68k::op_SUB(uint16_t opcode) {return -1;}
int cpu_m68k::op_SUBA(uint16_t opcode) {return -1;}
int cpu_m68k::op_SUBI(uint16_t opcode) {return -1;}
int cpu_m68k::op_SUBQ(uint16_t opcode) {return -1;}
int cpu_m68k::op_SUBX(uint16_t opcode) {return -1;}
int cpu_m68k::op_SWAP(uint16_t opcode) {return -1;}
int cpu_m68k::op_TAS(uint16_t opcode) {return -1;}
int cpu_m68k::op_TRAP(uint16_t opcode) {return -1;}

int cpu_m68k::calc(int cycles) { return -1; }
