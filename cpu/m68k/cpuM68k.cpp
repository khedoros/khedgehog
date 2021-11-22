#include<iostream>
#include<byteswap.h>

#include "cpuM68k.h"
#include "memmapM68k.h"
#include "m68kInstructions.h"
#include "../../util.h"

cpuM68k::cpuM68k(std::shared_ptr<memmapM68k> memmap) : memory(memmap) {

    sp[curStack] = bswap_32(memory->readLong(INIT_SSP_VECTOR));
    pc = bswap_32(memory->readLong(INIT_PC_VECTOR));

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
        {ops::trap, &cpuM68k::op_TRAP}, {ops::tst, &cpuM68k::op_TST}, {ops::unlk, &cpuM68k::op_UNLK}
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
        uint16_t op = bswap_16(memory->readWord(pc));
        std::printf("%06X: %04x (%s)\n", pc, op, op_name[op>>3].c_str());
        inst_cycles = (this->*op_table[op>>3])(op);
        if(inst_cycles == uint64_t(-1)) {
            return 0;
        }
        cycles += inst_cycles;
    }
    return cycles;
}

void cpuM68k::interrupt(uint8_t vector){}

void cpuM68k::setCCRReg(ccrField f) {
    ccr |= f;
}

void cpuM68k::clearCCRReg(ccrField f) {
    ccr &= (~f);
}

uint16_t cpuM68k::getCCRReg(ccrField f) {
    return (ccr & f);
}

bool cpuM68k::evalCond(uint8_t c) {
    switch(c) {
        case always:
            std::cout<<"Always";
            return true;
        case higher:
            std::cout<<"Higher";
            return !getCCRReg(cpuM68k::carry) && !getCCRReg(cpuM68k::zero);
        case lowerEqual:
            std::cout<<"LowerEqual";
            return getCCRReg(cpuM68k::carry) || getCCRReg(cpuM68k::zero);
        case carryClear:
            std::cout<<"CarryClear";
            return !getCCRReg(cpuM68k::carry);
        case carrySet:
            std::cout<<"CarrySet";
            return getCCRReg(cpuM68k::carry);
        case notEqual:
            std::cout<<"NotEqual";
            return !getCCRReg(cpuM68k::zero);
        case equal:
            std::cout<<"Equal";
            return getCCRReg(cpuM68k::zero);
        case overflowClear:
            std::cout<<"OverflowClear";
            return !getCCRReg(cpuM68k::overflow);
        case overflowSet:
            std::cout<<"OverflowSet";
            return getCCRReg(cpuM68k::overflow);
        case plus:
            std::cout<<"Positive";
            return !getCCRReg(cpuM68k::negative);
        case minus:
            std::cout<<"Negative";
            return getCCRReg(cpuM68k::negative);
        case greaterEqual:
            std::cout<<"greaterEqual";
            return (getCCRReg(cpuM68k::negative))>>(3) == (getCCRReg(cpuM68k::overflow))>>(1);
        case lessThan:
            std::cout<<"lessThan";
            return getCCRReg(zero) && ((!getCCRReg(negative) && getCCRReg(overflow)) || (getCCRReg(negative) && !getCCRReg(overflow)));
        case greaterThan:
            std::cout<<"greaterThan";
            return (!getCCRReg(zero) && !getCCRReg(negative) && !getCCRReg(overflow)) || (!getCCRReg(zero) && getCCRReg(negative) && getCCRReg(overflow));
        case lessEqual:
            std::cout<<"lessEqual";
            return getCCRReg(zero) || (getCCRReg(negative) && !getCCRReg(overflow)) || (!getCCRReg(negative) && getCCRReg(overflow));
        default:
            std::cout<<"Invalid! "<<int(c);
            break;
    }
    return false;
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
uint64_t cpuM68k::op_Bcc(uint16_t opcode) {
    uint8_t condition = (opcode>>8) & 0b1111;
    int32_t displacement = static_cast<int8_t>(opcode & 0b1111'1111);
    bool trigger = false;

    //printf("Displacement: %d ", displacement);

    if(evalCond(condition)) {
        if(displacement == 0) { // 16-bit displacement
            displacement = static_cast<int16_t>(bswap(memory->readWord(pc+2))) + 2;
            //printf("Displacing by 16-bit: %d", displacement);
        }
        else if(displacement == -1) { // 32-bit displacement
            displacement = static_cast<int32_t>(bswap(memory->readLong(pc+2))) + 2;
            //printf("Displacing by 32-bit: %d", displacement);
        }
        else { // 8-bit displacement
            //printf("Displacing by 8-bit: %d", displacement);
            displacement += 2;
        }
        //std::cout<<" Taking the branch. Displacing by "<<displacement<<"\n";
        pc += displacement;
    }
    else {
        if(displacement == 0) { // 16-bit displacement
            pc +=4;
        }
        else if(displacement == -1) { // 32-bit displacement
            pc += 6;
        }
        else { // 8-bit displacement
            pc += 2;
        }
        //std::cout<<" Not taking the branch.\n";
    }
    //printf("\n");

    return 1; // TODO: Fix timing
}
uint64_t cpuM68k::op_BCHG(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BCLR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BRA(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BSET(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BSR(uint16_t opcode) {return -1;}
uint64_t cpuM68k::op_BTST(uint16_t opcode) {
    // f e d c b a 9    8 7 6 543  2 1 0    f e d c b a 9 8 76543210
    // ----------------------------------|--------------------------
    // 0 0 0 0 1 0 0    0 0 0 mode register 0 0 0 0 0 0 0 0 bit-number   Bit-number specified as immediate data
    // 0 0 0 0 register 1 0 0 mode register                              Bit-number specified in data register
    pc+=2;
    operandSize size = static_cast<operandSize>((opcode & 0b11000000)>>6);
    uint8_t bitNum;
    if (opcode & 0b1'0000'0000) { // bit number in register
        int regNum = (opcode & 0b1110'0000'0000)>>9;
        bitNum = dreg[regNum] & 0x1f;
    }
    else { // bit number in immediate value
        bitNum = memory->readByte(pc+1) & 0x1f;
        pc+=2;
    }
    uint32_t operand = fetchArg<uint32_t>(opcode & 0b111111);
    if(operand & (1<<bitNum)) clearCCRReg(cpuM68k::zero);
    else                      setCCRReg(cpuM68k::zero);

    return 1; // TODO: Fix timing
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
uint64_t cpuM68k::op_LEA(uint16_t opcode) {
    // 000396: 41f9 (LEA)    396:   41f9 00ff 8000  lea 0xff8000,%a0
    // 00023C: 4bfa (LEA)    23c:   4bfa 0088       lea %pc@(0x2c6),%a5
    // 15 14 13 12    11 10 9   8 7 6   5  4  3  2  1  0
    //  0  1  0  0   REGISTER   1 1 1   EFFECTIVE ADDRESS
    pc += 2;
    uint8_t regnum = ((opcode>>9) & 0b111);
    if(regnum < 7) {
        areg[regnum] = fetchArg<uint32_t>(opcode & 0b111111);
    }
    else {
        sp[curStack] = fetchArg<uint32_t>(opcode & 0b111111);
    }

    return 1; // TODO: Fix timing
}
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
uint64_t cpuM68k::op_TST(uint16_t opcode) {
    // 15 14 13 12 11 10 9 8   7 6     5 4 3   2 1 0
    //  0  1  0  0  1  0 1 0   SIZE   EFFECTIVE ADDRESS
    //                                  MODE   REGISTER
    pc+=2;
    operandSize size = static_cast<operandSize>((opcode & 0b11000000)>>6);
    uint32_t operand;
    switch(size) {
        case byteSize:
            operand = fetchArg<uint8_t>(opcode & 0b111111);
            if(operand & 1<<7) setCCRReg(negative); else clearCCRReg(negative);
            break;
        case wordSize:
            operand = fetchArg<uint16_t>(opcode & 0b111111);
            if(operand & 1<<15) setCCRReg(negative); else clearCCRReg(negative);
            break;
        case longSize:
            operand = fetchArg<uint32_t>(opcode & 0b111111);
            if(operand & 1<<31) setCCRReg(negative); else clearCCRReg(negative);
            break;
    }
    if(operand == 0) setCCRReg(zero);
    else             clearCCRReg(zero);
    clearCCRReg(overflow);
    clearCCRReg(carry);
    return 1;     // TODO: Fix timing
}
uint64_t cpuM68k::op_UNLK(uint16_t opcode) {return -1;}


template <class retType>
retType cpuM68k::fetchArg(uint8_t addressBlock) {
    uint8_t mode = addressBlock & 0b00'111'000;
    uint8_t reg = addressBlock & 0b00'000'111;
    switch (mode) {
        case 0: // Data register direct
            return dreg[reg];
            break;
        case 8: // Address register direct
            if(reg < 7) return areg[reg];
            else return sp[curStack];
            break;
        case 0x10: // Address register indirect
            if(reg < 7) return memory->readLong(areg[reg]);
            else        return memory->readLong(sp[curStack]);
            break;
        case 0x18: // Address register indirect with postincrement
            if(reg < 7) {
                uint32_t& val = memory->readLong(areg[reg]);
                areg[reg] += sizeof(retType);
                return val;
            }
            else {
                uint32_t& val = memory->readLong(sp[curStack]);
                sp[curStack] += sizeof(retType);
                if(sizeof(retType) == byteSize) { sp[curStack]++; }
                return val;
            }
            break;
        case 0x20: // Address register indirect with predecrement
            if(reg < 7) {
                areg[reg] -= sizeof(retType);
                return memory->readLong(areg[reg]);
            }
            else {
                sp[curStack] -= sizeof(retType);
                if(sizeof(retType) == byteSize) { sp[curStack]--;}
                return memory->readLong(sp[curStack]);
            }
            break;
        case 0x28: // Address register indirect with basic index
            {
                int16_t offset = memory->readWord(pc);
                pc+=2;
                uint32_t regval = 0;
                if(reg < 7) {
                    regval = areg[reg] + offset;
                }
                else {
                    regval = sp[curStack] + offset;
                }
                return memory->readLong(regval);
            }
            break;
        case 0x30: // Address register indirect with full index
            {
                union basicDisplacement {
                    uint16_t val;
                    #pragma pack(push, 1)
                    struct {
                        unsigned unused:1;
                        unsigned scale:2;
                        unsigned size:1;
                        unsigned indexRegNum:3;
                        unsigned da:1;
                        signed displacement:8;
                    } indexWord;
                    #pragma pack(pop)
                } word;
                word.val = memory->readWord(pc);
                pc += 2;

            }
            break;
        case 0x38: // Non-register operand
            switch(addressBlock & 0b00'000'111) {
                case 0: // Absolute short
                    {
                        uint32_t ptr = bswap_16(memory->readWord(pc));
                        pc += 2;
                        return memory->readLong(ptr);
                    }
                    break;
                case 1: // Absolute long
                    {
                        uint32_t ptr = bswap_32(memory->readLong(pc));
                        pc += 4;
                        return memory->readLong(ptr);
                    }
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

    throw memoryMapException(mode, reg, sizeof(retType));
}

    template <class argType>
    void cpuM68k::stashArg(uint8_t addressBlock, argType value) {
        uint8_t mode = addressBlock & 0b00'111'000;
        uint8_t reg = addressBlock & 0b00'000'111;
        throw memoryMapException(mode, reg, sizeof(argType));
    }
