#include<iostream>

#include "cpu_m68k.h"
#include "m68k_instructions.h"

cpu_m68k::cpu_m68k(std::shared_ptr<memmap_m68k> memmap) : memory(memmap) {

    sp[cur_stack] = memory->readLong(INIT_SSP_VECTOR);
    pc = memory->readLong(INIT_PC_VECTOR);

    op_map.insert({
        {ops::abcd, &cpu_m68k::op_ABCD}, {ops::add, &cpu_m68k::op_ADD}, {ops::adda, &cpu_m68k::op_ADDA},
        {ops::addi, &cpu_m68k::op_ADDI}, {ops::addq, &cpu_m68k::op_ADDQ}, {ops::addx, &cpu_m68k::op_ADDX},
        {ops::andop, &cpu_m68k::op_AND}, {ops::andi, &cpu_m68k::op_ANDI}, {ops::asd, &cpu_m68k::op_ASd},
        {ops::bcc, &cpu_m68k::op_Bcc}, {ops::bchg, &cpu_m68k::op_BCHG}, {ops::bclr, &cpu_m68k::op_BCLR},
        {ops::bra, &cpu_m68k::op_BRA}, {ops::bset, &cpu_m68k::op_BSET}, {ops::bsr, &cpu_m68k::op_BSR},
        {ops::btst, &cpu_m68k::op_BTST}, {ops::chk, &cpu_m68k::op_CHK}, {ops::clr, &cpu_m68k::op_CLR},
        {ops::cmp, &cpu_m68k::op_CMP}, {ops::cmpa, &cpu_m68k::op_CMPA}, {ops::cmpi, &cpu_m68k::op_CMPI},
        {ops::cmpm, &cpu_m68k::op_CMPM}, {ops::dbcc, &cpu_m68k::op_DBcc}, {ops::divs, &cpu_m68k::op_DIVS},
        {ops::divu, &cpu_m68k::op_DIVU}, {ops::eor, &cpu_m68k::op_EOR}, {ops::eori, &cpu_m68k::op_EORI},
        {ops::exg, &cpu_m68k::op_EXG}, {ops::ext, &cpu_m68k::op_EXT}, {ops::illegal, &cpu_m68k::op_ILLEGAL},
        {ops::jmp, &cpu_m68k::op_JMP}, {ops::jsr, &cpu_m68k::op_JSR}, {ops::lea, &cpu_m68k::op_LEA},
        {ops::link, &cpu_m68k::op_LINK}, {ops::lsd, &cpu_m68k::op_LSd}, {ops::move, &cpu_m68k::op_MOVE},
        {ops::movea, &cpu_m68k::op_MOVEA}, {ops::move_from_sr, &cpu_m68k::op_MOVE_from_SR}, {ops::movem, &cpu_m68k::op_MOVEM},
        {ops::movep, &cpu_m68k::op_MOVEP}, {ops::moveq, &cpu_m68k::op_MOVEQ}, {ops::move_to_ccr, &cpu_m68k::op_MOVE_to_CCR},
        {ops::move_to_sr, &cpu_m68k::op_MOVE_to_SR}, {ops::move_usp, &cpu_m68k::op_MOVE_USP}, {ops::muls, &cpu_m68k::op_MULS},
        {ops::mulu, &cpu_m68k::op_MULU}, {ops::nbcd, &cpu_m68k::op_NBCD}, {ops::neg, &cpu_m68k::op_NEG},
        {ops::negx, &cpu_m68k::op_NEGX}, {ops::none, &cpu_m68k::op_NONE}, {ops::notop, &cpu_m68k::op_NOT},
        {ops::orop, &cpu_m68k::op_OR}, {ops::ori, &cpu_m68k::op_ORI}, {ops::pea, &cpu_m68k::op_PEA},
        {ops::rod, &cpu_m68k::op_ROd}, {ops::roxd, &cpu_m68k::op_ROXd}, {ops::sbcd, &cpu_m68k::op_SBCD},
        {ops::scc, &cpu_m68k::op_Scc}, {ops::special, &cpu_m68k::op_SPECIAL}, {ops::sub, &cpu_m68k::op_SUB},
        {ops::suba, &cpu_m68k::op_SUBA}, {ops::subi, &cpu_m68k::op_SUBI}, {ops::subq, &cpu_m68k::op_SUBQ},
        {ops::subx, &cpu_m68k::op_SUBX}, {ops::swap, &cpu_m68k::op_SWAP}, {ops::tas, &cpu_m68k::op_TAS},
        {ops::trap, &cpu_m68k::op_TRAP}, {ops:tst, &cpu_m68k::op_TST}, {ops:unlk, &cpu_m68k::op_UNLK}
    });

    for(int inst=0;inst<8192;inst++) {
        for(auto &cand: instrs) {
            int c = ((inst<<3) & (cand.mask_bits));
            if(c == cand.id_bits) {
                // store instruction in lookup table
                op_table[inst] = op_map[cand.func];
                op_name[inst] = cand.op_name;
                break;
            }
        }
    }
}

uint64_t cpu_m68k::calc(uint64_t cycle_max) {
    uint64_t cycles = 0;
    uint64_t inst_cycles = 0;
    while(cycles < cycle_max && inst_cycles < 1000) {
        uint16_t op = memory->readWord(pc);
        inst_cycles = (this->*op_table[op>>3])(op);
        if(inst_cycles == uint64_t(-1)) {
            std::printf("%06X: %04x (%s)\n", pc, op, op_name[op>>3].c_str());
            return 0;
        }
        cycles += inst_cycles;
    }
    return cycles;
}

uint64_t cpu_m68k::op_ABCD(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ADD(uint16_t opcode)  {return -1;}
uint64_t cpu_m68k::op_ADDA(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ADDI(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ADDQ(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ADDX(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_AND(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ANDI(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ASd(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_Bcc(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_BCHG(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_BCLR(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_BRA(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_BSET(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_BSR(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_BTST(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_CHK(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_CLR(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_CMP(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_CMPA(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_CMPI(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_CMPM(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_DBcc(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_DIVS(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_DIVU(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_EOR(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_EORI(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_EXG(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_EXT(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ILLEGAL(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_JMP(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_JSR(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_LEA(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_LINK(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_LSd(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MOVE(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MOVEA(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MOVE_from_SR(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MOVEM(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MOVEP(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MOVEQ(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MOVE_to_CCR(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MOVE_to_SR(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MOVE_USP(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MULS(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_MULU(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_NBCD(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_NEG(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_NEGX(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_NONE(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_NOT(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_OR(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ORI(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_PEA(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ROd(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_ROXd(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_SBCD(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_Scc(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_SPECIAL(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_SUB(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_SUBA(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_SUBI(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_SUBQ(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_SUBX(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_SWAP(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_TAS(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_TRAP(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_TST(uint16_t opcode) {return -1;}
uint64_t cpu_m68k::op_UNLK(uint16_t opcode) {return -1;}
