#pragma once
#include "../cpu.h"
//#include "../../memmap.h"
#include "memmapZ80Console.h"
#include<memory>
#include<array>
#include<cstdint>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

class cpuZ80;

using z80OpPtr = uint64_t (cpuZ80::*)(uint8_t);

class cpuZ80 : public cpu {
private:
    std::shared_ptr<memmapZ80Console> memory;

    struct regpair {
        union {
            struct {
                uint8_t low;
                uint8_t hi;
            };
            uint16_t pair;
        };
    };

    regpair af;
    regpair af_1;
    regpair bc;
    regpair bc_1;
    regpair de;
    regpair de_1;
    regpair hl;
    regpair hl_1;
    regpair ix;
    regpair iy;

    uint8_t int_vect;
    uint8_t mem_refresh;
    uint16_t sp, pc;
    bool iff1, iff2; //interrupt enable registers

    enum intMode {
        mode0,
        mode1,
        mode2
    } int_mode;

    //NMI does call to 0066h
    //interrupt mode 0: interrupting device puts an instruction on data bus, CPU executes it
    //interrupt mode 1: interrupt calls to 0038h
    //interrupt mode 2: I (int_vect) contains high byte, device provides low byte, CPU executes indirect call

    int64_t cycles_remaining;

    static std::array<z80OpPtr, 256> op_table;
    static std::array<z80OpPtr, 256> cb_op_table;
    static std::array<z80OpPtr, 256> dd_op_table;
    static std::array<z80OpPtr, 256> ed_op_table;
    static std::array<z80OpPtr, 256> fd_op_table;
    static std::array<z80OpPtr, 256> ddcb_op_table;
    static std::array<z80OpPtr, 256> fdcb_op_table;

    template <uint32_t OPCODE> uint64_t cb_op_prefix(uint8_t);
    template <uint32_t OPCODE> uint64_t dd_op_prefix(uint8_t);
    template <uint32_t OPCODE> uint64_t ed_op_prefix(uint8_t);
    template <uint32_t OPCODE> uint64_t fd_op_prefix(uint8_t);
    template <uint32_t OPCODE> uint64_t ddcb_op_prefix(uint8_t);
    template <uint32_t OPCODE> uint64_t fdcb_op_prefix(uint8_t);
    template <uint32_t OPCODE> uint64_t op_call(uint8_t);
    template <uint32_t OPCODE> uint64_t op_di(uint8_t);
    template <uint32_t OPCODE> uint64_t op_ei(uint8_t);
    template <uint32_t OPCODE> uint64_t op_exx(uint8_t);
    template <uint32_t OPCODE> uint64_t op_im(uint8_t);
    template <uint32_t OPCODE> uint64_t op_jp(uint8_t);
    template <uint32_t OPCODE> uint64_t op_ld16(uint8_t);
    template <uint32_t OPCODE> uint64_t op_ld8ri(uint8_t);
    template <uint32_t OPCODE> uint64_t op_ld8rr(uint8_t);
    template <uint32_t OPCODE> uint64_t op_nop(uint8_t);
    template <uint32_t OPCODE> uint64_t op_out(uint8_t);
    template <uint32_t OPCODE> uint64_t op_pop(uint8_t);
    template <uint32_t OPCODE> uint64_t op_push(uint8_t);
    template <uint32_t OPCODE> uint64_t op_ret(uint8_t);

    uint64_t decode(uint8_t opcode);
    void push(uint16_t);
    uint16_t pop();

    template <uint32_t OPCODE> uint64_t op_unimpl(uint8_t);

    uint8_t dummy8;
    uint16_t dummy16;


public:
    cpuZ80(std::shared_ptr<memmapZ80Console>);
    uint64_t calc(uint64_t);
};
