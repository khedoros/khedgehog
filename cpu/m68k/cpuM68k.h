#pragma once

#include<functional>
#include<unordered_map>
#include<cstdint>
#include<stdexcept>
#include<memory>
#include "memmapM68k.h"
#include "../cpu.h"

class cpuM68k;
enum ops : unsigned int;

//typedef uint64_t (cpu_m68k::*m68kOpPtr)(uint16_t);
using m68kOpPtr = uint64_t (cpuM68k::*)(uint16_t);
using m68k_dreg = uint32_t;
using m68k_areg = uint32_t;
using m68k_ccr = uint16_t;

class cpuM68k : public cpu {
private:

    m68k_dreg dreg[8];
    m68k_areg areg[7];
    m68k_areg sp[2]; //A7, USP is sp[0], SSP is sp[1]
    uint32_t pc;

    //      system   user
    //     TSMOIII|000XNZVC
    m68k_ccr ccr;

    enum stackType {
        userMode,
        supervisorMode
    };

    enum operandSize {
        byteSize = 1,
        wordSize = 2,
        longSize = 4
    };

    enum ccrField {
        carry = 1<<0,
        overflow = 1<<1,
        zero = 1<<2,
        negative = 1<<3,
        extend = 1<<4,
        interrupts = 7<<8,
        supervisor = 1<<13,
        trace = 1<<15
    };

    enum conditions {
        always = 0,
        never = 1,
        higher = 2,
        lowerEqual = 3,
        carryClear = 4,
        carrySet = 5,
        notEqual = 6,
        equal = 7,
        overflowClear = 8,
        overflowSet = 9,
        plus = 10,
        minus = 11,
        greaterEqual = 12,
        lessThan = 13,
        greaterThan = 14,
        lessEqual = 15
    };



    void setCCRReg(ccrField);
    void clearCCRReg(ccrField);
    uint16_t getCCRReg(ccrField);
    void adjustCCRReg(uint8_t, bool);
    bool evalCond(uint8_t);

    stackType curStack = supervisorMode;

    std::shared_ptr<memmapM68k> memory;

    std::array<m68kOpPtr, 8192> op_table;
    std::array<std::string, 8192> op_name;
    std::unordered_map<ops, m68kOpPtr> op_map;

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
    uint64_t op_TST(uint16_t opcode);
    uint64_t op_UNLK(uint16_t opcode);

    bool isAddrEA(uint8_t addressBlock);

    template <class retType>
    retType fetchArg(uint8_t addressBlock);

    template <class argType>
    void stashArg(uint8_t addressBlock, argType value);

    const static uint32_t INIT_SSP_VECTOR = 0;
    const static uint32_t INIT_PC_VECTOR = 4;

    class memoryMapException: public std::exception {
    public:
        memoryMapException(const char* srcName, uint8_t mode, uint8_t reg, uint8_t size){
            std::snprintf(buffer, 100, "Mapping failed in %s with mode: %x, register: %x, operandSize: %x\n", srcName, mode, reg, size);
        }
        virtual const char* what() const throw() {
            return const_cast<const char*>(buffer);
        }

    private:
        char buffer[100];
    };

public:
    uint64_t calc(uint64_t cycles);
    void interrupt(uint8_t vector);
    cpuM68k(std::shared_ptr<memmapM68k>);
};
