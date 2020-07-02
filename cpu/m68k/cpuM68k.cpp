#include<iostream>

#include "cpuM68k.h"
#include "m68kInstructions.h"
#include "../../util.h"

cpuM68k::cpuM68k(std::shared_ptr<memmapM68k> memmap) : memory(memmap) {

    sp[cur_stack] = bswap(memory->readLong(INIT_SSP_VECTOR));
    pc = bswap(memory->readLong(INIT_PC_VECTOR));

    op_map.insert({
        {ops::abcd, &cpuM68k::op_ABCD}, {ops::add, &cpuM68k::op_ADD}, {ops::adda, &cpuM68k::op_ADDA},
        {ops::addi, &cpuM68k::op_ADDI}, {ops::addq, &cpuM68k::op_ADDQ}, {ops::addx, &cpuM68k::op_ADDX},
        {ops::andop, &cpuM68k::op_AND}, {ops::andi, &cpuM68k::op_ANDI}, {ops::asd, &cpuM68k::op_ASd},
        {ops::bcc, &cpuM68k::op_Bcc}, {ops::bchg, &cpuM68k::op_BCHG}, {ops::bclr, &cpuM68k::op_BCLR},
        {ops::bra, &cpuM68k::op_BRA}, {ops::bset, &cpuM68k::op_BSET}, {ops::bsr, &cpuM68k::op_BSR},
        {ops::btst, &cpuM68k::op_BTST}, {ops::chk, &cpuM68k::op_CHK}, {ops::clr, &cpuM68k::op_CLR},
        {ops::cmp, &cpuM68k::op_CMP}, {ops::cmpa, &cpuM68k::op_CMPA}, {ops::cmpi, &cpuM68k::op_CMPI},
        {ops::cmpm, &cpuM68k::op_CMPM}, {ops::dbcc, &cpuM68k::op_DBcc}, {ops::divs, &cpuM68k::op_DIVS},
        {ops::divu, &cpuM68k::op_DIVU}, {ops::eor, &cpuM68k::op_EOR}, {ops::eori, &cpuM68k::op_EORI},
        {ops::exg, &cpuM68k::op_EXG}, {ops::ext, &cpuM68k::op_EXT}, {ops::illegal, &cpuM68k::op_ILLEGAL},
        {ops::jmp, &cpuM68k::op_JMP}, {ops::jsr, &cpuM68k::op_JSR}, {ops::lea, &cpuM68k::op_LEA},
        {ops::link, &cpuM68k::op_LINK}, {ops::lsd, &cpuM68k::op_LSd}, {ops::move, &cpuM68k::op_MOVE},
        {ops::movea, &cpuM68k::op_MOVEA}, {ops::move_from_sr, &cpuM68k::op_MOVE_from_SR}, {ops::movem, &cpuM68k::op_MOVEM},
        {ops::movep, &cpuM68k::op_MOVEP}, {ops::moveq, &cpuM68k::op_MOVEQ}, {ops::move_to_ccr, &cpuM68k::op_MOVE_to_CCR},
        {ops::move_to_sr, &cpuM68k::op_MOVE_to_SR}, {ops::move_usp, &cpuM68k::op_MOVE_USP}, {ops::muls, &cpuM68k::op_MULS},
        {ops::mulu, &cpuM68k::op_MULU}, {ops::nbcd, &cpuM68k::op_NBCD}, {ops::neg, &cpuM68k::op_NEG},
        {ops::negx, &cpuM68k::op_NEGX}, {ops::none, &cpuM68k::op_NONE}, {ops::notop, &cpuM68k::op_NOT},
        {ops::orop, &cpuM68k::op_OR}, {ops::ori, &cpuM68k::op_ORI}, {ops::pea, &cpuM68k::op_PEA},
        {ops::rod, &cpuM68k::op_ROd}, {ops::roxd, &cpuM68k::op_ROXd}, {ops::sbcd, &cpuM68k::op_SBCD},
        {ops::scc, &cpuM68k::op_Scc}, {ops::special, &cpuM68k::op_SPECIAL}, {ops::sub, &cpuM68k::op_SUB},
        {ops::suba, &cpuM68k::op_SUBA}, {ops::subi, &cpuM68k::op_SUBI}, {ops::subq, &cpuM68k::op_SUBQ},
        {ops::subx, &cpuM68k::op_SUBX}, {ops::swap, &cpuM68k::op_SWAP}, {ops::tas, &cpuM68k::op_TAS},
        {ops::trap, &cpuM68k::op_TRAP}, {ops:tst, &cpuM68k::op_TST}, {ops:unlk, &cpuM68k::op_UNLK}
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

uint64_t cpuM68k::calc(uint64_t cycle_max) {
    uint64_t cycles = 0;
    uint64_t inst_cycles = 0;
    while(cycles < cycle_max && inst_cycles < 1000) {
        uint16_t op = bswap(memory->readWord(pc));
        inst_cycles = (this->*op_table[op>>3])(op);
        if(inst_cycles == uint64_t(-1)) {
            std::printf("%06X: %04x (%s)\n", pc, op, op_name[op>>3].c_str());
            return 0;
        }
        cycles += inst_cycles;
    }
    return cycles;
}

uint64_t cpuM68k::op_ABCD(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ADD(uint16_t opcode)  {return -1;}
uint64_t cpuM68k::op_ADDA(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ADDI(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ADDQ(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ADDX(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_AND(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ANDI(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ASd(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_Bcc(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BCHG(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BCLR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BRA(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BSET(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BSR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BTST(uint16_t opcode) {
    pc+=4;
    operandSize size = static_cast<operandSize>((opcode & 0b11000000)>>6);
    uint8_t bitNum = memory->readByte(pc-1);
    uint32_t operand = fetchArg(opcode & 0b111111, size);

    return -1;
}
uint64_t cpuM68k::op_CHK(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_CLR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_CMP(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_CMPA(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_CMPI(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_CMPM(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_DBcc(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_DIVS(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_DIVU(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_EOR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_EORI(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_EXG(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_EXT(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ILLEGAL(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_JMP(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_JSR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_LEA(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_LINK(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_LSd(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MOVE(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MOVEA(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MOVE_from_SR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MOVEM(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MOVEP(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MOVEQ(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MOVE_to_CCR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MOVE_to_SR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MOVE_USP(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MULS(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_MULU(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_NBCD(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_NEG(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_NEGX(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_NONE(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_NOT(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_OR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ORI(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_PEA(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ROd(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_ROXd(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_SBCD(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_Scc(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_SPECIAL(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_SUB(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_SUBA(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_SUBI(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_SUBQ(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_SUBX(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_SWAP(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_TAS(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_TRAP(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_TST(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_UNLK(uint16_t opcode) {return -1;}

uint32_t cpuM68k::fetchArg(uint8_t addressBlock, operandSize size) {
    switch (addressBlock & 0b00111000) {
        case 0: // Data register direct
            break;
        case 8: // Address register direct
            break;
        case 0x10: // Address register indirect
            break;
        case 0x18: // Address register indirect with postincrement
            break;
        case 0x20: // Address register indirect with predecrement
            break;
        case 0x28: // Address register indirect with basic index
            break;
        case 0x30: // Address register indirect with full index
            break;
        case 0x38: // Non-register operand
            switch(addressBlock & 0b111) {
                case 0: // Absolute short
                    break;
                case 1: // Absolute long
                    break;
                case 2: // Relative basic
                    break;
                case 3: // Relative full
                    break;
                case 4: // Immediate
                    break;
            }
            break;
    }
    return 0;
}
