#include "cpuZ80.h"
#include<iostream>

uint64_t cpuZ80::calc(uint64_t cycles_to_run) {
    cycles_remaining += cycles_to_run;
    while(cycles_remaining > 0) {
        uint8_t opcode = memory->readByte(pc++);
        std::printf("%04X: %02x", pc-1, opcode);
        uint64_t inst_cycles = CALL_MEMBER_FN(this, op_table[opcode])(opcode);
        std::printf("\n");

        if(inst_cycles == uint64_t(-1)) {
            return 0;
        }
        cycles_remaining -= inst_cycles;
    }

    return 0;
}

cpuZ80::cpuZ80(std::shared_ptr<memmapZ80Console> memmap): memory(memmap), cycles_remaining(0), pc(0), iff1(false), iff2(false)  {

}

//CPU status flags
#define SIGN_FLAG        0x80 //Set when result is <0x80, cleared otherwise
#define ZERO_FLAG        0x40 //Set when result is zero, cleared otherwise
//                       0x20 //unused
#define HALF_CARRY_FLAG  0x10 //Set upon carry from bit3
//                       0x08 //unused
#define PARITY_FLAG      0x04 //Set when parity is even, cleared when odd / sometimes signifies overflow
#define OVERFLOW_FLAG    0x04 //Set when parity is even, cleared when odd / sometimes signifies overflow
#define SUB_FLAG         0x02 //Set when instruction was subtraction, cleared otherwise
#define CARRY_FLAG       0x01 //Set upon carry from bit7

#define set(f) (af.low |= f)
#define clear(f) (af.low &= (~f))
#define sign() ((af.low & SIGN_FLAG)>>(7))
#define zero() ((af.low & ZERO_FLAG)>>(6))
#define hc() ((af.low & HALF_CARRY_FLAG)>>(4))
#define parity() ((af.low & PAR_OVER_FLAG)>>(2))
#define overflow() ((af.low & PAR_OVER_FLAG)>>(2))
#define sub() ((af.low & SUB_FLAG)>>(1))
#define carry() ((af.low & CARRY_FLAG))

std::array<z80OpPtr, 256> cpuZ80::op_table = {
  &cpuZ80::op_nop<0x00>,    &cpuZ80::op_ld16<0x01>,     &cpuZ80::op_unimpl<0x02>, &cpuZ80::op_incr16<0x03>,
  &cpuZ80::op_incr8<0x04>,  &cpuZ80::op_decr8<0x05>,    &cpuZ80::op_ld8ri<0x06>, &cpuZ80::op_unimpl<0x07>,
  &cpuZ80::op_exx<0x08>,    &cpuZ80::op_unimpl<0x09>,   &cpuZ80::op_unimpl<0x0a>, &cpuZ80::op_decr16<0x0b>,
  &cpuZ80::op_incr8<0x0c>,  &cpuZ80::op_decr8<0x0d>,    &cpuZ80::op_ld8ri<0x0e>, &cpuZ80::op_unimpl<0x0f>,
  &cpuZ80::op_jr<0x10>,     &cpuZ80::op_ld16<0x11>,     &cpuZ80::op_unimpl<0x12>, &cpuZ80::op_incr16<0x13>,
  &cpuZ80::op_incr8<0x14>,  &cpuZ80::op_decr8<0x15>,    &cpuZ80::op_ld8ri<0x16>, &cpuZ80::op_unimpl<0x17>,
  &cpuZ80::op_jr<0x18>,     &cpuZ80::op_unimpl<0x19>,   &cpuZ80::op_unimpl<0x1a>, &cpuZ80::op_decr16<0x1b>,
  &cpuZ80::op_incr8<0x1c>,  &cpuZ80::op_decr8<0x1d>,    &cpuZ80::op_ld8ri<0x1e>, &cpuZ80::op_unimpl<0x1f>,
  &cpuZ80::op_jr<0x20>,     &cpuZ80::op_ld16<0x21>,     &cpuZ80::op_unimpl<0x22>, &cpuZ80::op_incr16<0x23>,
  &cpuZ80::op_incr8<0x24>,  &cpuZ80::op_decr8<0x25>,    &cpuZ80::op_ld8ri<0x26>, &cpuZ80::op_daa<0x27>,
  &cpuZ80::op_jr<0x28>,     &cpuZ80::op_unimpl<0x29>,   &cpuZ80::op_unimpl<0x2a>, &cpuZ80::op_decr16<0x2b>,
  &cpuZ80::op_incr8<0x2c>,  &cpuZ80::op_decr8<0x2d>,    &cpuZ80::op_ld8ri<0x2e>, &cpuZ80::op_cpl<0x2f>,
  &cpuZ80::op_jr<0x30>,     &cpuZ80::op_ld16<0x31>,     &cpuZ80::op_unimpl<0x32>, &cpuZ80::op_incr16<0x33>,
  &cpuZ80::op_unimpl<0x34>, &cpuZ80::op_unimpl<0x35>,   &cpuZ80::op_unimpl<0x36>, &cpuZ80::op_scf<0x37>,
  &cpuZ80::op_jr<0x38>,     &cpuZ80::op_unimpl<0x39>,   &cpuZ80::op_unimpl<0x3a>, &cpuZ80::op_decr16<0x3b>,
  &cpuZ80::op_incr8<0x3c>,  &cpuZ80::op_decr8<0x3d>,    &cpuZ80::op_ld8ri<0x3e>, &cpuZ80::op_ccf<0x3f>,
  &cpuZ80::op_ld8rr<0x40>,  &cpuZ80::op_ld8rr<0x41>,    &cpuZ80::op_ld8rr<0x42>, &cpuZ80::op_ld8rr<0x43>,
  &cpuZ80::op_ld8rr<0x44>,  &cpuZ80::op_ld8rr<0x45>,    &cpuZ80::op_unimpl<0x46>, &cpuZ80::op_ld8rr<0x47>,
  &cpuZ80::op_ld8rr<0x48>,  &cpuZ80::op_ld8rr<0x49>,    &cpuZ80::op_ld8rr<0x4a>, &cpuZ80::op_ld8rr<0x4b>,
  &cpuZ80::op_ld8rr<0x4c>,  &cpuZ80::op_ld8rr<0x4d>,    &cpuZ80::op_unimpl<0x4e>, &cpuZ80::op_ld8rr<0x4f>,
  &cpuZ80::op_ld8rr<0x50>,  &cpuZ80::op_ld8rr<0x51>,    &cpuZ80::op_ld8rr<0x52>, &cpuZ80::op_ld8rr<0x53>,
  &cpuZ80::op_ld8rr<0x54>,  &cpuZ80::op_ld8rr<0x55>,    &cpuZ80::op_unimpl<0x56>, &cpuZ80::op_ld8rr<0x57>,
  &cpuZ80::op_ld8rr<0x58>,  &cpuZ80::op_ld8rr<0x59>,    &cpuZ80::op_ld8rr<0x5a>, &cpuZ80::op_ld8rr<0x5b>,
  &cpuZ80::op_ld8rr<0x5c>,  &cpuZ80::op_ld8rr<0x5d>,    &cpuZ80::op_unimpl<0x5e>, &cpuZ80::op_ld8rr<0x5f>,
  &cpuZ80::op_ld8rr<0x60>,  &cpuZ80::op_ld8rr<0x61>,    &cpuZ80::op_ld8rr<0x62>, &cpuZ80::op_ld8rr<0x63>,
  &cpuZ80::op_ld8rr<0x64>,  &cpuZ80::op_ld8rr<0x65>,    &cpuZ80::op_unimpl<0x66>, &cpuZ80::op_ld8rr<0x67>,
  &cpuZ80::op_ld8rr<0x68>,  &cpuZ80::op_ld8rr<0x69>,    &cpuZ80::op_ld8rr<0x6a>, &cpuZ80::op_ld8rr<0x6b>,
  &cpuZ80::op_ld8rr<0x6c>,  &cpuZ80::op_ld8rr<0x6d>,    &cpuZ80::op_unimpl<0x6e>, &cpuZ80::op_ld8rr<0x6f>,
  &cpuZ80::op_unimpl<0x70>, &cpuZ80::op_unimpl<0x71>,   &cpuZ80::op_unimpl<0x72>, &cpuZ80::op_unimpl<0x73>,
  &cpuZ80::op_unimpl<0x74>, &cpuZ80::op_unimpl<0x75>,   &cpuZ80::op_unimpl<0x76>, &cpuZ80::op_unimpl<0x77>,
  &cpuZ80::op_ld8rr<0x78>,  &cpuZ80::op_ld8rr<0x79>,    &cpuZ80::op_ld8rr<0x7a>, &cpuZ80::op_ld8rr<0x7b>,
  &cpuZ80::op_ld8rr<0x7c>,  &cpuZ80::op_ld8rr<0x7d>,    &cpuZ80::op_unimpl<0x7e>, &cpuZ80::op_ld8rr<0x7f>,
  &cpuZ80::op_unimpl<0x80>, &cpuZ80::op_unimpl<0x81>,   &cpuZ80::op_unimpl<0x82>, &cpuZ80::op_unimpl<0x83>,
  &cpuZ80::op_unimpl<0x84>, &cpuZ80::op_unimpl<0x85>,   &cpuZ80::op_unimpl<0x86>, &cpuZ80::op_unimpl<0x87>,
  &cpuZ80::op_unimpl<0x88>, &cpuZ80::op_unimpl<0x89>,   &cpuZ80::op_unimpl<0x8a>, &cpuZ80::op_unimpl<0x8b>,
  &cpuZ80::op_unimpl<0x8c>, &cpuZ80::op_unimpl<0x8d>,   &cpuZ80::op_unimpl<0x8e>, &cpuZ80::op_unimpl<0x8f>,
  &cpuZ80::op_unimpl<0x90>, &cpuZ80::op_unimpl<0x91>,   &cpuZ80::op_unimpl<0x92>, &cpuZ80::op_unimpl<0x93>,
  &cpuZ80::op_unimpl<0x94>, &cpuZ80::op_unimpl<0x95>,   &cpuZ80::op_unimpl<0x96>, &cpuZ80::op_unimpl<0x97>,
  &cpuZ80::op_unimpl<0x98>, &cpuZ80::op_unimpl<0x99>,   &cpuZ80::op_unimpl<0x9a>, &cpuZ80::op_unimpl<0x9b>,
  &cpuZ80::op_unimpl<0x9c>, &cpuZ80::op_unimpl<0x9d>,   &cpuZ80::op_unimpl<0x9e>, &cpuZ80::op_unimpl<0x9f>,
  &cpuZ80::op_unimpl<0xa0>, &cpuZ80::op_unimpl<0xa1>,   &cpuZ80::op_unimpl<0xa2>, &cpuZ80::op_unimpl<0xa3>,
  &cpuZ80::op_unimpl<0xa4>, &cpuZ80::op_unimpl<0xa5>,   &cpuZ80::op_unimpl<0xa6>, &cpuZ80::op_unimpl<0xa7>,
  &cpuZ80::op_unimpl<0xa8>, &cpuZ80::op_unimpl<0xa9>,   &cpuZ80::op_unimpl<0xaa>, &cpuZ80::op_unimpl<0xab>,
  &cpuZ80::op_unimpl<0xac>, &cpuZ80::op_unimpl<0xad>,   &cpuZ80::op_unimpl<0xae>, &cpuZ80::op_unimpl<0xaf>,
  &cpuZ80::op_unimpl<0xb0>, &cpuZ80::op_unimpl<0xb1>,   &cpuZ80::op_unimpl<0xb2>, &cpuZ80::op_unimpl<0xb3>,
  &cpuZ80::op_unimpl<0xb4>, &cpuZ80::op_unimpl<0xb5>,   &cpuZ80::op_unimpl<0xb6>, &cpuZ80::op_unimpl<0xb7>,
  &cpuZ80::op_unimpl<0xb8>, &cpuZ80::op_unimpl<0xb9>,   &cpuZ80::op_unimpl<0xba>, &cpuZ80::op_unimpl<0xbb>,
  &cpuZ80::op_unimpl<0xbc>, &cpuZ80::op_unimpl<0xbd>,   &cpuZ80::op_unimpl<0xbe>, &cpuZ80::op_unimpl<0xbf>,
  &cpuZ80::op_unimpl<0xc0>, &cpuZ80::op_pop<0xc1>,      &cpuZ80::op_unimpl<0xc2>, &cpuZ80::op_jp<0xc3>,
  &cpuZ80::op_unimpl<0xc4>, &cpuZ80::op_push<0xc5>,     &cpuZ80::op_unimpl<0xc6>, &cpuZ80::op_unimpl<0xc7>,
  &cpuZ80::op_unimpl<0xc8>, &cpuZ80::op_ret<0xc9>,      &cpuZ80::op_unimpl<0xca>, &cpuZ80::cb_op_prefix<0xcb>,
  &cpuZ80::op_unimpl<0xcc>, &cpuZ80::op_call<0xcd>,     &cpuZ80::op_unimpl<0xce>, &cpuZ80::op_unimpl<0xcf>,
  &cpuZ80::op_unimpl<0xd0>, &cpuZ80::op_pop<0xd1>,      &cpuZ80::op_unimpl<0xd2>, &cpuZ80::op_unimpl<0xd3>,
  &cpuZ80::op_unimpl<0xd4>, &cpuZ80::op_push<0xd5>,     &cpuZ80::op_unimpl<0xd6>, &cpuZ80::op_unimpl<0xd7>,
  &cpuZ80::op_unimpl<0xd8>, &cpuZ80::op_exx<0xd9>,      &cpuZ80::op_unimpl<0xda>, &cpuZ80::op_unimpl<0xdb>,
  &cpuZ80::op_unimpl<0xdc>, &cpuZ80::dd_op_prefix<0xdd>, &cpuZ80::op_unimpl<0xde>, &cpuZ80::op_unimpl<0xdf>,
  &cpuZ80::op_unimpl<0xe0>, &cpuZ80::op_pop<0xe1>,       &cpuZ80::op_unimpl<0xe2>, &cpuZ80::op_unimpl<0xe3>,
  &cpuZ80::op_unimpl<0xe4>, &cpuZ80::op_push<0xe5>,      &cpuZ80::op_unimpl<0xe6>, &cpuZ80::op_unimpl<0xe7>,
  &cpuZ80::op_unimpl<0xe8>, &cpuZ80::op_unimpl<0xe9>,    &cpuZ80::op_unimpl<0xea>, &cpuZ80::op_unimpl<0xeb>,
  &cpuZ80::op_unimpl<0xec>, &cpuZ80::ed_op_prefix<0xed>, &cpuZ80::op_unimpl<0xee>, &cpuZ80::op_unimpl<0xef>,
  &cpuZ80::op_unimpl<0xf0>, &cpuZ80::op_pop<0xf1>,       &cpuZ80::op_unimpl<0xf2>, &cpuZ80::op_di<0xf3>,
  &cpuZ80::op_unimpl<0xf4>, &cpuZ80::op_push<0xf5>,      &cpuZ80::op_unimpl<0xf6>, &cpuZ80::op_unimpl<0xf7>,
  &cpuZ80::op_unimpl<0xf8>, &cpuZ80::op_unimpl<0xf9>,    &cpuZ80::op_unimpl<0xfa>, &cpuZ80::op_ei<0xfb>,
  &cpuZ80::op_unimpl<0xfc>, &cpuZ80::fd_op_prefix<0xfd>, &cpuZ80::op_unimpl<0xfe>, &cpuZ80::op_unimpl<0xff>
};

std::array<z80OpPtr, 256> cpuZ80::cb_op_table = {
  &cpuZ80::op_unimpl<0xcb00>, &cpuZ80::op_unimpl<0xcb01>, &cpuZ80::op_unimpl<0xcb02>, &cpuZ80::op_unimpl<0xcb03>,
  &cpuZ80::op_unimpl<0xcb04>, &cpuZ80::op_unimpl<0xcb05>, &cpuZ80::op_unimpl<0xcb06>, &cpuZ80::op_unimpl<0xcb07>,
  &cpuZ80::op_unimpl<0xcb08>, &cpuZ80::op_unimpl<0xcb09>, &cpuZ80::op_unimpl<0xcb0a>, &cpuZ80::op_unimpl<0xcb0b>,
  &cpuZ80::op_unimpl<0xcb0c>, &cpuZ80::op_unimpl<0xcb0d>, &cpuZ80::op_unimpl<0xcb0e>, &cpuZ80::op_unimpl<0xcb0f>,
  &cpuZ80::op_unimpl<0xcb10>, &cpuZ80::op_unimpl<0xcb11>, &cpuZ80::op_unimpl<0xcb12>, &cpuZ80::op_unimpl<0xcb13>,
  &cpuZ80::op_unimpl<0xcb14>, &cpuZ80::op_unimpl<0xcb15>, &cpuZ80::op_unimpl<0xcb16>, &cpuZ80::op_unimpl<0xcb17>,
  &cpuZ80::op_unimpl<0xcb18>, &cpuZ80::op_unimpl<0xcb19>, &cpuZ80::op_unimpl<0xcb1a>, &cpuZ80::op_unimpl<0xcb1b>,
  &cpuZ80::op_unimpl<0xcb1c>, &cpuZ80::op_unimpl<0xcb1d>, &cpuZ80::op_unimpl<0xcb1e>, &cpuZ80::op_unimpl<0xcb1f>,
  &cpuZ80::op_unimpl<0xcb20>, &cpuZ80::op_unimpl<0xcb21>, &cpuZ80::op_unimpl<0xcb22>, &cpuZ80::op_unimpl<0xcb23>,
  &cpuZ80::op_unimpl<0xcb24>, &cpuZ80::op_unimpl<0xcb25>, &cpuZ80::op_unimpl<0xcb26>, &cpuZ80::op_unimpl<0xcb27>,
  &cpuZ80::op_unimpl<0xcb28>, &cpuZ80::op_unimpl<0xcb29>, &cpuZ80::op_unimpl<0xcb2a>, &cpuZ80::op_unimpl<0xcb2b>,
  &cpuZ80::op_unimpl<0xcb2c>, &cpuZ80::op_unimpl<0xcb2d>, &cpuZ80::op_unimpl<0xcb2e>, &cpuZ80::op_unimpl<0xcb2f>,
  &cpuZ80::op_unimpl<0xcb30>, &cpuZ80::op_unimpl<0xcb31>, &cpuZ80::op_unimpl<0xcb32>, &cpuZ80::op_unimpl<0xcb33>,
  &cpuZ80::op_unimpl<0xcb34>, &cpuZ80::op_unimpl<0xcb35>, &cpuZ80::op_unimpl<0xcb36>, &cpuZ80::op_unimpl<0xcb37>,
  &cpuZ80::op_unimpl<0xcb38>, &cpuZ80::op_unimpl<0xcb39>, &cpuZ80::op_unimpl<0xcb3a>, &cpuZ80::op_unimpl<0xcb3b>,
  &cpuZ80::op_unimpl<0xcb3c>, &cpuZ80::op_unimpl<0xcb3d>, &cpuZ80::op_unimpl<0xcb3e>, &cpuZ80::op_unimpl<0xcb3f>,
  &cpuZ80::op_unimpl<0xcb40>, &cpuZ80::op_unimpl<0xcb41>, &cpuZ80::op_unimpl<0xcb42>, &cpuZ80::op_unimpl<0xcb43>,
  &cpuZ80::op_unimpl<0xcb44>, &cpuZ80::op_unimpl<0xcb45>, &cpuZ80::op_unimpl<0xcb46>, &cpuZ80::op_unimpl<0xcb47>,
  &cpuZ80::op_unimpl<0xcb48>, &cpuZ80::op_unimpl<0xcb49>, &cpuZ80::op_unimpl<0xcb4a>, &cpuZ80::op_unimpl<0xcb4b>,
  &cpuZ80::op_unimpl<0xcb4c>, &cpuZ80::op_unimpl<0xcb4d>, &cpuZ80::op_unimpl<0xcb4e>, &cpuZ80::op_unimpl<0xcb4f>,
  &cpuZ80::op_unimpl<0xcb50>, &cpuZ80::op_unimpl<0xcb51>, &cpuZ80::op_unimpl<0xcb52>, &cpuZ80::op_unimpl<0xcb53>,
  &cpuZ80::op_unimpl<0xcb54>, &cpuZ80::op_unimpl<0xcb55>, &cpuZ80::op_unimpl<0xcb56>, &cpuZ80::op_unimpl<0xcb57>,
  &cpuZ80::op_unimpl<0xcb58>, &cpuZ80::op_unimpl<0xcb59>, &cpuZ80::op_unimpl<0xcb5a>, &cpuZ80::op_unimpl<0xcb5b>,
  &cpuZ80::op_unimpl<0xcb5c>, &cpuZ80::op_unimpl<0xcb5d>, &cpuZ80::op_unimpl<0xcb5e>, &cpuZ80::op_unimpl<0xcb5f>,
  &cpuZ80::op_unimpl<0xcb60>, &cpuZ80::op_unimpl<0xcb61>, &cpuZ80::op_unimpl<0xcb62>, &cpuZ80::op_unimpl<0xcb63>,
  &cpuZ80::op_unimpl<0xcb64>, &cpuZ80::op_unimpl<0xcb65>, &cpuZ80::op_unimpl<0xcb66>, &cpuZ80::op_unimpl<0xcb67>,
  &cpuZ80::op_unimpl<0xcb68>, &cpuZ80::op_unimpl<0xcb69>, &cpuZ80::op_unimpl<0xcb6a>, &cpuZ80::op_unimpl<0xcb6b>,
  &cpuZ80::op_unimpl<0xcb6c>, &cpuZ80::op_unimpl<0xcb6d>, &cpuZ80::op_unimpl<0xcb6e>, &cpuZ80::op_unimpl<0xcb6f>,
  &cpuZ80::op_unimpl<0xcb70>, &cpuZ80::op_unimpl<0xcb71>, &cpuZ80::op_unimpl<0xcb72>, &cpuZ80::op_unimpl<0xcb73>,
  &cpuZ80::op_unimpl<0xcb74>, &cpuZ80::op_unimpl<0xcb75>, &cpuZ80::op_unimpl<0xcb76>, &cpuZ80::op_unimpl<0xcb77>,
  &cpuZ80::op_unimpl<0xcb78>, &cpuZ80::op_unimpl<0xcb79>, &cpuZ80::op_unimpl<0xcb7a>, &cpuZ80::op_unimpl<0xcb7b>,
  &cpuZ80::op_unimpl<0xcb7c>, &cpuZ80::op_unimpl<0xcb7d>, &cpuZ80::op_unimpl<0xcb7e>, &cpuZ80::op_unimpl<0xcb7f>,
  &cpuZ80::op_unimpl<0xcb80>, &cpuZ80::op_unimpl<0xcb81>, &cpuZ80::op_unimpl<0xcb82>, &cpuZ80::op_unimpl<0xcb83>,
  &cpuZ80::op_unimpl<0xcb84>, &cpuZ80::op_unimpl<0xcb85>, &cpuZ80::op_unimpl<0xcb86>, &cpuZ80::op_unimpl<0xcb87>,
  &cpuZ80::op_unimpl<0xcb88>, &cpuZ80::op_unimpl<0xcb89>, &cpuZ80::op_unimpl<0xcb8a>, &cpuZ80::op_unimpl<0xcb8b>,
  &cpuZ80::op_unimpl<0xcb8c>, &cpuZ80::op_unimpl<0xcb8d>, &cpuZ80::op_unimpl<0xcb8e>, &cpuZ80::op_unimpl<0xcb8f>,
  &cpuZ80::op_unimpl<0xcb90>, &cpuZ80::op_unimpl<0xcb91>, &cpuZ80::op_unimpl<0xcb92>, &cpuZ80::op_unimpl<0xcb93>,
  &cpuZ80::op_unimpl<0xcb94>, &cpuZ80::op_unimpl<0xcb95>, &cpuZ80::op_unimpl<0xcb96>, &cpuZ80::op_unimpl<0xcb97>,
  &cpuZ80::op_unimpl<0xcb98>, &cpuZ80::op_unimpl<0xcb99>, &cpuZ80::op_unimpl<0xcb9a>, &cpuZ80::op_unimpl<0xcb9b>,
  &cpuZ80::op_unimpl<0xcb9c>, &cpuZ80::op_unimpl<0xcb9d>, &cpuZ80::op_unimpl<0xcb9e>, &cpuZ80::op_unimpl<0xcb9f>,
  &cpuZ80::op_unimpl<0xcba0>, &cpuZ80::op_unimpl<0xcba1>, &cpuZ80::op_unimpl<0xcba2>, &cpuZ80::op_unimpl<0xcba3>,
  &cpuZ80::op_unimpl<0xcba4>, &cpuZ80::op_unimpl<0xcba5>, &cpuZ80::op_unimpl<0xcba6>, &cpuZ80::op_unimpl<0xcba7>,
  &cpuZ80::op_unimpl<0xcba8>, &cpuZ80::op_unimpl<0xcba9>, &cpuZ80::op_unimpl<0xcbaa>, &cpuZ80::op_unimpl<0xcbab>,
  &cpuZ80::op_unimpl<0xcbac>, &cpuZ80::op_unimpl<0xcbad>, &cpuZ80::op_unimpl<0xcbae>, &cpuZ80::op_unimpl<0xcbaf>,
  &cpuZ80::op_unimpl<0xcbb0>, &cpuZ80::op_unimpl<0xcbb1>, &cpuZ80::op_unimpl<0xcbb2>, &cpuZ80::op_unimpl<0xcbb3>,
  &cpuZ80::op_unimpl<0xcbb4>, &cpuZ80::op_unimpl<0xcbb5>, &cpuZ80::op_unimpl<0xcbb6>, &cpuZ80::op_unimpl<0xcbb7>,
  &cpuZ80::op_unimpl<0xcbb8>, &cpuZ80::op_unimpl<0xcbb9>, &cpuZ80::op_unimpl<0xcbba>, &cpuZ80::op_unimpl<0xcbbb>,
  &cpuZ80::op_unimpl<0xcbbc>, &cpuZ80::op_unimpl<0xcbbd>, &cpuZ80::op_unimpl<0xcbbe>, &cpuZ80::op_unimpl<0xcbbf>,
  &cpuZ80::op_unimpl<0xcbc0>, &cpuZ80::op_unimpl<0xcbc1>, &cpuZ80::op_unimpl<0xcbc2>, &cpuZ80::op_unimpl<0xcbc3>,
  &cpuZ80::op_unimpl<0xcbc4>, &cpuZ80::op_unimpl<0xcbc5>, &cpuZ80::op_unimpl<0xcbc6>, &cpuZ80::op_unimpl<0xcbc7>,
  &cpuZ80::op_unimpl<0xcbc8>, &cpuZ80::op_unimpl<0xcbc9>, &cpuZ80::op_unimpl<0xcbca>, &cpuZ80::op_unimpl<0xcbcb>,
  &cpuZ80::op_unimpl<0xcbcc>, &cpuZ80::op_unimpl<0xcbcd>, &cpuZ80::op_unimpl<0xcbce>, &cpuZ80::op_unimpl<0xcbcf>,
  &cpuZ80::op_unimpl<0xcbd0>, &cpuZ80::op_unimpl<0xcbd1>, &cpuZ80::op_unimpl<0xcbd2>, &cpuZ80::op_unimpl<0xcbd3>,
  &cpuZ80::op_unimpl<0xcbd4>, &cpuZ80::op_unimpl<0xcbd5>, &cpuZ80::op_unimpl<0xcbd6>, &cpuZ80::op_unimpl<0xcbd7>,
  &cpuZ80::op_unimpl<0xcbd8>, &cpuZ80::op_unimpl<0xcbd9>, &cpuZ80::op_unimpl<0xcbda>, &cpuZ80::op_unimpl<0xcbdb>,
  &cpuZ80::op_unimpl<0xcbdc>, &cpuZ80::op_unimpl<0xcbdd>, &cpuZ80::op_unimpl<0xcbde>, &cpuZ80::op_unimpl<0xcbdf>,
  &cpuZ80::op_unimpl<0xcbe0>, &cpuZ80::op_unimpl<0xcbe1>, &cpuZ80::op_unimpl<0xcbe2>, &cpuZ80::op_unimpl<0xcbe3>,
  &cpuZ80::op_unimpl<0xcbe4>, &cpuZ80::op_unimpl<0xcbe5>, &cpuZ80::op_unimpl<0xcbe6>, &cpuZ80::op_unimpl<0xcbe7>,
  &cpuZ80::op_unimpl<0xcbe8>, &cpuZ80::op_unimpl<0xcbe9>, &cpuZ80::op_unimpl<0xcbea>, &cpuZ80::op_unimpl<0xcbeb>,
  &cpuZ80::op_unimpl<0xcbec>, &cpuZ80::op_unimpl<0xcbed>, &cpuZ80::op_unimpl<0xcbee>, &cpuZ80::op_unimpl<0xcbef>,
  &cpuZ80::op_unimpl<0xcbf0>, &cpuZ80::op_unimpl<0xcbf1>, &cpuZ80::op_unimpl<0xcbf2>, &cpuZ80::op_unimpl<0xcbf3>,
  &cpuZ80::op_unimpl<0xcbf4>, &cpuZ80::op_unimpl<0xcbf5>, &cpuZ80::op_unimpl<0xcbf6>, &cpuZ80::op_unimpl<0xcbf7>,
  &cpuZ80::op_unimpl<0xcbf8>, &cpuZ80::op_unimpl<0xcbf9>, &cpuZ80::op_unimpl<0xcbfa>, &cpuZ80::op_unimpl<0xcbfb>,
  &cpuZ80::op_unimpl<0xcbfc>, &cpuZ80::op_unimpl<0xcbfd>, &cpuZ80::op_unimpl<0xcbfe>, &cpuZ80::op_unimpl<0xcbff>
};

std::array<z80OpPtr, 256> cpuZ80::dd_op_table = {
  &cpuZ80::op_unimpl<0xdd00>, &cpuZ80::op_unimpl<0xdd01>, &cpuZ80::op_unimpl<0xdd02>, &cpuZ80::op_unimpl<0xdd03>,
  &cpuZ80::op_unimpl<0xdd04>, &cpuZ80::op_unimpl<0xdd05>, &cpuZ80::op_unimpl<0xdd06>, &cpuZ80::op_unimpl<0xdd07>,
  &cpuZ80::op_unimpl<0xdd08>, &cpuZ80::op_unimpl<0xdd09>, &cpuZ80::op_unimpl<0xdd0a>, &cpuZ80::op_unimpl<0xdd0b>,
  &cpuZ80::op_unimpl<0xdd0c>, &cpuZ80::op_unimpl<0xdd0d>, &cpuZ80::op_unimpl<0xdd0e>, &cpuZ80::op_unimpl<0xdd0f>,
  &cpuZ80::op_unimpl<0xdd10>, &cpuZ80::op_unimpl<0xdd11>, &cpuZ80::op_unimpl<0xdd12>, &cpuZ80::op_unimpl<0xdd13>,
  &cpuZ80::op_unimpl<0xdd14>, &cpuZ80::op_unimpl<0xdd15>, &cpuZ80::op_unimpl<0xdd16>, &cpuZ80::op_unimpl<0xdd17>,
  &cpuZ80::op_unimpl<0xdd18>, &cpuZ80::op_unimpl<0xdd19>, &cpuZ80::op_unimpl<0xdd1a>, &cpuZ80::op_unimpl<0xdd1b>,
  &cpuZ80::op_unimpl<0xdd1c>, &cpuZ80::op_unimpl<0xdd1d>, &cpuZ80::op_unimpl<0xdd1e>, &cpuZ80::op_unimpl<0xdd1f>,
  &cpuZ80::op_unimpl<0xdd20>, &cpuZ80::op_unimpl<0xdd21>, &cpuZ80::op_unimpl<0xdd22>, &cpuZ80::op_unimpl<0xdd23>,
  &cpuZ80::op_unimpl<0xdd24>, &cpuZ80::op_unimpl<0xdd25>, &cpuZ80::op_unimpl<0xdd26>, &cpuZ80::op_unimpl<0xdd27>,
  &cpuZ80::op_unimpl<0xdd28>, &cpuZ80::op_unimpl<0xdd29>, &cpuZ80::op_unimpl<0xdd2a>, &cpuZ80::op_unimpl<0xdd2b>,
  &cpuZ80::op_unimpl<0xdd2c>, &cpuZ80::op_unimpl<0xdd2d>, &cpuZ80::op_unimpl<0xdd2e>, &cpuZ80::op_unimpl<0xdd2f>,
  &cpuZ80::op_unimpl<0xdd30>, &cpuZ80::op_unimpl<0xdd31>, &cpuZ80::op_unimpl<0xdd32>, &cpuZ80::op_unimpl<0xdd33>,
  &cpuZ80::op_unimpl<0xdd34>, &cpuZ80::op_unimpl<0xdd35>, &cpuZ80::op_unimpl<0xdd36>, &cpuZ80::op_unimpl<0xdd37>,
  &cpuZ80::op_unimpl<0xdd38>, &cpuZ80::op_unimpl<0xdd39>, &cpuZ80::op_unimpl<0xdd3a>, &cpuZ80::op_unimpl<0xdd3b>,
  &cpuZ80::op_unimpl<0xdd3c>, &cpuZ80::op_unimpl<0xdd3d>, &cpuZ80::op_unimpl<0xdd3e>, &cpuZ80::op_unimpl<0xdd3f>,
  &cpuZ80::op_unimpl<0xdd40>, &cpuZ80::op_unimpl<0xdd41>, &cpuZ80::op_unimpl<0xdd42>, &cpuZ80::op_unimpl<0xdd43>,
  &cpuZ80::op_unimpl<0xdd44>, &cpuZ80::op_unimpl<0xdd45>, &cpuZ80::op_unimpl<0xdd46>, &cpuZ80::op_unimpl<0xdd47>,
  &cpuZ80::op_unimpl<0xdd48>, &cpuZ80::op_unimpl<0xdd49>, &cpuZ80::op_unimpl<0xdd4a>, &cpuZ80::op_unimpl<0xdd4b>,
  &cpuZ80::op_unimpl<0xdd4c>, &cpuZ80::op_unimpl<0xdd4d>, &cpuZ80::op_unimpl<0xdd4e>, &cpuZ80::op_unimpl<0xdd4f>,
  &cpuZ80::op_unimpl<0xdd50>, &cpuZ80::op_unimpl<0xdd51>, &cpuZ80::op_unimpl<0xdd52>, &cpuZ80::op_unimpl<0xdd53>,
  &cpuZ80::op_unimpl<0xdd54>, &cpuZ80::op_unimpl<0xdd55>, &cpuZ80::op_unimpl<0xdd56>, &cpuZ80::op_unimpl<0xdd57>,
  &cpuZ80::op_unimpl<0xdd58>, &cpuZ80::op_unimpl<0xdd59>, &cpuZ80::op_unimpl<0xdd5a>, &cpuZ80::op_unimpl<0xdd5b>,
  &cpuZ80::op_unimpl<0xdd5c>, &cpuZ80::op_unimpl<0xdd5d>, &cpuZ80::op_unimpl<0xdd5e>, &cpuZ80::op_unimpl<0xdd5f>,
  &cpuZ80::op_unimpl<0xdd60>, &cpuZ80::op_unimpl<0xdd61>, &cpuZ80::op_unimpl<0xdd62>, &cpuZ80::op_unimpl<0xdd63>,
  &cpuZ80::op_unimpl<0xdd64>, &cpuZ80::op_unimpl<0xdd65>, &cpuZ80::op_unimpl<0xdd66>, &cpuZ80::op_unimpl<0xdd67>,
  &cpuZ80::op_unimpl<0xdd68>, &cpuZ80::op_unimpl<0xdd69>, &cpuZ80::op_unimpl<0xdd6a>, &cpuZ80::op_unimpl<0xdd6b>,
  &cpuZ80::op_unimpl<0xdd6c>, &cpuZ80::op_unimpl<0xdd6d>, &cpuZ80::op_unimpl<0xdd6e>, &cpuZ80::op_unimpl<0xdd6f>,
  &cpuZ80::op_unimpl<0xdd70>, &cpuZ80::op_unimpl<0xdd71>, &cpuZ80::op_unimpl<0xdd72>, &cpuZ80::op_unimpl<0xdd73>,
  &cpuZ80::op_unimpl<0xdd74>, &cpuZ80::op_unimpl<0xdd75>, &cpuZ80::op_unimpl<0xdd76>, &cpuZ80::op_unimpl<0xdd77>,
  &cpuZ80::op_unimpl<0xdd78>, &cpuZ80::op_unimpl<0xdd79>, &cpuZ80::op_unimpl<0xdd7a>, &cpuZ80::op_unimpl<0xdd7b>,
  &cpuZ80::op_unimpl<0xdd7c>, &cpuZ80::op_unimpl<0xdd7d>, &cpuZ80::op_unimpl<0xdd7e>, &cpuZ80::op_unimpl<0xdd7f>,
  &cpuZ80::op_unimpl<0xdd80>, &cpuZ80::op_unimpl<0xdd81>, &cpuZ80::op_unimpl<0xdd82>, &cpuZ80::op_unimpl<0xdd83>,
  &cpuZ80::op_unimpl<0xdd84>, &cpuZ80::op_unimpl<0xdd85>, &cpuZ80::op_unimpl<0xdd86>, &cpuZ80::op_unimpl<0xdd87>,
  &cpuZ80::op_unimpl<0xdd88>, &cpuZ80::op_unimpl<0xdd89>, &cpuZ80::op_unimpl<0xdd8a>, &cpuZ80::op_unimpl<0xdd8b>,
  &cpuZ80::op_unimpl<0xdd8c>, &cpuZ80::op_unimpl<0xdd8d>, &cpuZ80::op_unimpl<0xdd8e>, &cpuZ80::op_unimpl<0xdd8f>,
  &cpuZ80::op_unimpl<0xdd90>, &cpuZ80::op_unimpl<0xdd91>, &cpuZ80::op_unimpl<0xdd92>, &cpuZ80::op_unimpl<0xdd93>,
  &cpuZ80::op_unimpl<0xdd94>, &cpuZ80::op_unimpl<0xdd95>, &cpuZ80::op_unimpl<0xdd96>, &cpuZ80::op_unimpl<0xdd97>,
  &cpuZ80::op_unimpl<0xdd98>, &cpuZ80::op_unimpl<0xdd99>, &cpuZ80::op_unimpl<0xdd9a>, &cpuZ80::op_unimpl<0xdd9b>,
  &cpuZ80::op_unimpl<0xdd9c>, &cpuZ80::op_unimpl<0xdd9d>, &cpuZ80::op_unimpl<0xdd9e>, &cpuZ80::op_unimpl<0xdd9f>,
  &cpuZ80::op_unimpl<0xdda0>, &cpuZ80::op_unimpl<0xdda1>, &cpuZ80::op_unimpl<0xdda2>, &cpuZ80::op_unimpl<0xdda3>,
  &cpuZ80::op_unimpl<0xdda4>, &cpuZ80::op_unimpl<0xdda5>, &cpuZ80::op_unimpl<0xdda6>, &cpuZ80::op_unimpl<0xdda7>,
  &cpuZ80::op_unimpl<0xdda8>, &cpuZ80::op_unimpl<0xdda9>, &cpuZ80::op_unimpl<0xddaa>, &cpuZ80::op_unimpl<0xddab>,
  &cpuZ80::op_unimpl<0xddac>, &cpuZ80::op_unimpl<0xddad>, &cpuZ80::op_unimpl<0xddae>, &cpuZ80::op_unimpl<0xddaf>,
  &cpuZ80::op_unimpl<0xddb0>, &cpuZ80::op_unimpl<0xddb1>, &cpuZ80::op_unimpl<0xddb2>, &cpuZ80::op_unimpl<0xddb3>,
  &cpuZ80::op_unimpl<0xddb4>, &cpuZ80::op_unimpl<0xddb5>, &cpuZ80::op_unimpl<0xddb6>, &cpuZ80::op_unimpl<0xddb7>,
  &cpuZ80::op_unimpl<0xddb8>, &cpuZ80::op_unimpl<0xddb9>, &cpuZ80::op_unimpl<0xddba>, &cpuZ80::op_unimpl<0xddbb>,
  &cpuZ80::op_unimpl<0xddbc>, &cpuZ80::op_unimpl<0xddbd>, &cpuZ80::op_unimpl<0xddbe>, &cpuZ80::op_unimpl<0xddbf>,
  &cpuZ80::op_unimpl<0xddc0>, &cpuZ80::op_unimpl<0xddc1>, &cpuZ80::op_unimpl<0xddc2>, &cpuZ80::op_unimpl<0xddc3>,
  &cpuZ80::op_unimpl<0xddc4>, &cpuZ80::op_unimpl<0xddc5>, &cpuZ80::op_unimpl<0xddc6>, &cpuZ80::op_unimpl<0xddc7>,
  &cpuZ80::op_unimpl<0xddc8>, &cpuZ80::op_unimpl<0xddc9>, &cpuZ80::op_unimpl<0xddca>, &cpuZ80::ddcb_op_prefix<0xddcb>,
  &cpuZ80::op_unimpl<0xddcc>, &cpuZ80::op_unimpl<0xddcd>, &cpuZ80::op_unimpl<0xddce>, &cpuZ80::op_unimpl<0xddcf>,
  &cpuZ80::op_unimpl<0xddd0>, &cpuZ80::op_unimpl<0xddd1>, &cpuZ80::op_unimpl<0xddd2>, &cpuZ80::op_unimpl<0xddd3>,
  &cpuZ80::op_unimpl<0xddd4>, &cpuZ80::op_unimpl<0xddd5>, &cpuZ80::op_unimpl<0xddd6>, &cpuZ80::op_unimpl<0xddd7>,
  &cpuZ80::op_unimpl<0xddd8>, &cpuZ80::op_unimpl<0xddd9>, &cpuZ80::op_unimpl<0xddda>, &cpuZ80::op_unimpl<0xdddb>,
  &cpuZ80::op_unimpl<0xdddc>, &cpuZ80::op_unimpl<0xdddd>, &cpuZ80::op_unimpl<0xddde>, &cpuZ80::op_unimpl<0xdddf>,
  &cpuZ80::op_unimpl<0xdde0>, &cpuZ80::op_pop<0xdde1>,    &cpuZ80::op_unimpl<0xdde2>, &cpuZ80::op_unimpl<0xdde3>,
  &cpuZ80::op_unimpl<0xdde4>, &cpuZ80::op_push<0xdde5>,   &cpuZ80::op_unimpl<0xdde6>, &cpuZ80::op_unimpl<0xdde7>,
  &cpuZ80::op_unimpl<0xdde8>, &cpuZ80::op_unimpl<0xdde9>, &cpuZ80::op_unimpl<0xddea>, &cpuZ80::op_unimpl<0xddeb>,
  &cpuZ80::op_unimpl<0xddec>, &cpuZ80::op_unimpl<0xdded>, &cpuZ80::op_unimpl<0xddee>, &cpuZ80::op_unimpl<0xddef>,
  &cpuZ80::op_unimpl<0xddf0>, &cpuZ80::op_unimpl<0xddf1>, &cpuZ80::op_unimpl<0xddf2>, &cpuZ80::op_unimpl<0xddf3>,
  &cpuZ80::op_unimpl<0xddf4>, &cpuZ80::op_unimpl<0xddf5>, &cpuZ80::op_unimpl<0xddf6>, &cpuZ80::op_unimpl<0xddf7>,
  &cpuZ80::op_unimpl<0xddf8>, &cpuZ80::op_unimpl<0xddf9>, &cpuZ80::op_unimpl<0xddfa>, &cpuZ80::op_unimpl<0xddfb>,
  &cpuZ80::op_unimpl<0xddfc>, &cpuZ80::op_unimpl<0xddfd>, &cpuZ80::op_unimpl<0xddfe>, &cpuZ80::op_unimpl<0xddff>
};

std::array<z80OpPtr, 256> cpuZ80::ed_op_table = {
  &cpuZ80::op_unimpl<0xed40>, &cpuZ80::op_unimpl<0xed41>, &cpuZ80::op_unimpl<0xed42>, &cpuZ80::op_unimpl<0xed43>,
  &cpuZ80::op_unimpl<0xed44>, &cpuZ80::op_unimpl<0xed45>, &cpuZ80::op_im<0xed46>, &cpuZ80::op_unimpl<0xed47>,
  &cpuZ80::op_unimpl<0xed48>, &cpuZ80::op_unimpl<0xed49>, &cpuZ80::op_unimpl<0xed4a>, &cpuZ80::op_unimpl<0xed4b>,
  &cpuZ80::op_unimpl<0xed4c>, &cpuZ80::op_unimpl<0xed4d>, &cpuZ80::op_unimpl<0xed4e>, &cpuZ80::op_unimpl<0xed4f>,
  &cpuZ80::op_unimpl<0xed50>, &cpuZ80::op_unimpl<0xed51>, &cpuZ80::op_unimpl<0xed52>, &cpuZ80::op_unimpl<0xed53>,
  &cpuZ80::op_unimpl<0xed54>, &cpuZ80::op_unimpl<0xed55>, &cpuZ80::op_im<0xed56>, &cpuZ80::op_unimpl<0xed57>,
  &cpuZ80::op_unimpl<0xed58>, &cpuZ80::op_unimpl<0xed59>, &cpuZ80::op_unimpl<0xed5a>, &cpuZ80::op_unimpl<0xed5b>,
  &cpuZ80::op_unimpl<0xed5c>, &cpuZ80::op_unimpl<0xed5d>, &cpuZ80::op_im<0xed5e>, &cpuZ80::op_unimpl<0xed5f>,
  &cpuZ80::op_unimpl<0xed60>, &cpuZ80::op_unimpl<0xed61>, &cpuZ80::op_unimpl<0xed62>, &cpuZ80::op_unimpl<0xed63>,
  &cpuZ80::op_unimpl<0xed64>, &cpuZ80::op_unimpl<0xed65>, &cpuZ80::op_im<0xed66>, &cpuZ80::op_unimpl<0xed67>,
  &cpuZ80::op_unimpl<0xed68>, &cpuZ80::op_unimpl<0xed69>, &cpuZ80::op_unimpl<0xed6a>, &cpuZ80::op_unimpl<0xed6b>,
  &cpuZ80::op_unimpl<0xed6c>, &cpuZ80::op_unimpl<0xed6d>, &cpuZ80::op_unimpl<0xed6e>, &cpuZ80::op_unimpl<0xed6f>,
  &cpuZ80::op_unimpl<0xed70>, &cpuZ80::op_unimpl<0xed71>, &cpuZ80::op_unimpl<0xed72>, &cpuZ80::op_unimpl<0xed73>,
  &cpuZ80::op_unimpl<0xed74>, &cpuZ80::op_unimpl<0xed75>, &cpuZ80::op_im<0xed76>, &cpuZ80::op_unimpl<0xed77>,
  &cpuZ80::op_unimpl<0xed78>, &cpuZ80::op_unimpl<0xed79>, &cpuZ80::op_unimpl<0xed7a>, &cpuZ80::op_unimpl<0xed7b>,
  &cpuZ80::op_unimpl<0xed7c>, &cpuZ80::op_unimpl<0xed7d>, &cpuZ80::op_im<0xed7e>, &cpuZ80::op_unimpl<0xed7f>,
  &cpuZ80::op_unimpl<0xed80>, &cpuZ80::op_unimpl<0xed81>, &cpuZ80::op_unimpl<0xed82>, &cpuZ80::op_unimpl<0xed83>,
  &cpuZ80::op_unimpl<0xed84>, &cpuZ80::op_unimpl<0xed85>, &cpuZ80::op_unimpl<0xed86>, &cpuZ80::op_unimpl<0xed87>,
  &cpuZ80::op_unimpl<0xed88>, &cpuZ80::op_unimpl<0xed89>, &cpuZ80::op_unimpl<0xed8a>, &cpuZ80::op_unimpl<0xed8b>,
  &cpuZ80::op_unimpl<0xed8c>, &cpuZ80::op_unimpl<0xed8d>, &cpuZ80::op_unimpl<0xed8e>, &cpuZ80::op_unimpl<0xed8f>,
  &cpuZ80::op_unimpl<0xed90>, &cpuZ80::op_unimpl<0xed91>, &cpuZ80::op_unimpl<0xed92>, &cpuZ80::op_unimpl<0xed93>,
  &cpuZ80::op_unimpl<0xed94>, &cpuZ80::op_unimpl<0xed95>, &cpuZ80::op_unimpl<0xed96>, &cpuZ80::op_unimpl<0xed97>,
  &cpuZ80::op_unimpl<0xed98>, &cpuZ80::op_unimpl<0xed99>, &cpuZ80::op_unimpl<0xed9a>, &cpuZ80::op_unimpl<0xed9b>,
  &cpuZ80::op_unimpl<0xed9c>, &cpuZ80::op_unimpl<0xed9d>, &cpuZ80::op_unimpl<0xed9e>, &cpuZ80::op_unimpl<0xed9f>,
  &cpuZ80::op_unimpl<0xeda0>, &cpuZ80::op_unimpl<0xeda1>, &cpuZ80::op_unimpl<0xeda2>, &cpuZ80::op_out<0xeda3>,
  &cpuZ80::op_unimpl<0xeda4>, &cpuZ80::op_unimpl<0xeda5>, &cpuZ80::op_unimpl<0xeda6>, &cpuZ80::op_unimpl<0xeda7>,
  &cpuZ80::op_unimpl<0xeda8>, &cpuZ80::op_unimpl<0xeda9>, &cpuZ80::op_unimpl<0xedaa>, &cpuZ80::op_unimpl<0xedab>,
  &cpuZ80::op_unimpl<0xedac>, &cpuZ80::op_unimpl<0xedad>, &cpuZ80::op_unimpl<0xedae>, &cpuZ80::op_unimpl<0xedaf>,
  &cpuZ80::op_unimpl<0xedb0>, &cpuZ80::op_unimpl<0xedb1>, &cpuZ80::op_unimpl<0xedb2>, &cpuZ80::op_out<0xedb3>,
  &cpuZ80::op_unimpl<0xedb4>, &cpuZ80::op_unimpl<0xedb5>, &cpuZ80::op_unimpl<0xedb6>, &cpuZ80::op_unimpl<0xedb7>,
  &cpuZ80::op_unimpl<0xedb8>, &cpuZ80::op_unimpl<0xedb9>, &cpuZ80::op_unimpl<0xedba>, &cpuZ80::op_unimpl<0xedbb>
};

std::array<z80OpPtr, 256> cpuZ80::fd_op_table = {
  &cpuZ80::op_unimpl<0xfd00>, &cpuZ80::op_unimpl<0xfd01>, &cpuZ80::op_unimpl<0xfd02>, &cpuZ80::op_unimpl<0xfd03>,
  &cpuZ80::op_unimpl<0xfd04>, &cpuZ80::op_unimpl<0xfd05>, &cpuZ80::op_unimpl<0xfd06>, &cpuZ80::op_unimpl<0xfd07>,
  &cpuZ80::op_unimpl<0xfd08>, &cpuZ80::op_unimpl<0xfd09>, &cpuZ80::op_unimpl<0xfd0a>, &cpuZ80::op_unimpl<0xfd0b>,
  &cpuZ80::op_unimpl<0xfd0c>, &cpuZ80::op_unimpl<0xfd0d>, &cpuZ80::op_unimpl<0xfd0e>, &cpuZ80::op_unimpl<0xfd0f>,
  &cpuZ80::op_unimpl<0xfd10>, &cpuZ80::op_unimpl<0xfd11>, &cpuZ80::op_unimpl<0xfd12>, &cpuZ80::op_unimpl<0xfd13>,
  &cpuZ80::op_unimpl<0xfd14>, &cpuZ80::op_unimpl<0xfd15>, &cpuZ80::op_unimpl<0xfd16>, &cpuZ80::op_unimpl<0xfd17>,
  &cpuZ80::op_unimpl<0xfd18>, &cpuZ80::op_unimpl<0xfd19>, &cpuZ80::op_unimpl<0xfd1a>, &cpuZ80::op_unimpl<0xfd1b>,
  &cpuZ80::op_unimpl<0xfd1c>, &cpuZ80::op_unimpl<0xfd1d>, &cpuZ80::op_unimpl<0xfd1e>, &cpuZ80::op_unimpl<0xfd1f>,
  &cpuZ80::op_unimpl<0xfd20>, &cpuZ80::op_unimpl<0xfd21>, &cpuZ80::op_unimpl<0xfd22>, &cpuZ80::op_unimpl<0xfd23>,
  &cpuZ80::op_unimpl<0xfd24>, &cpuZ80::op_unimpl<0xfd25>, &cpuZ80::op_unimpl<0xfd26>, &cpuZ80::op_unimpl<0xfd27>,
  &cpuZ80::op_unimpl<0xfd28>, &cpuZ80::op_unimpl<0xfd29>, &cpuZ80::op_unimpl<0xfd2a>, &cpuZ80::op_unimpl<0xfd2b>,
  &cpuZ80::op_unimpl<0xfd2c>, &cpuZ80::op_unimpl<0xfd2d>, &cpuZ80::op_unimpl<0xfd2e>, &cpuZ80::op_unimpl<0xfd2f>,
  &cpuZ80::op_unimpl<0xfd30>, &cpuZ80::op_unimpl<0xfd31>, &cpuZ80::op_unimpl<0xfd32>, &cpuZ80::op_unimpl<0xfd33>,
  &cpuZ80::op_unimpl<0xfd34>, &cpuZ80::op_unimpl<0xfd35>, &cpuZ80::op_unimpl<0xfd36>, &cpuZ80::op_unimpl<0xfd37>,
  &cpuZ80::op_unimpl<0xfd38>, &cpuZ80::op_unimpl<0xfd39>, &cpuZ80::op_unimpl<0xfd3a>, &cpuZ80::op_unimpl<0xfd3b>,
  &cpuZ80::op_unimpl<0xfd3c>, &cpuZ80::op_unimpl<0xfd3d>, &cpuZ80::op_unimpl<0xfd3e>, &cpuZ80::op_unimpl<0xfd3f>,
  &cpuZ80::op_unimpl<0xfd40>, &cpuZ80::op_unimpl<0xfd41>, &cpuZ80::op_unimpl<0xfd42>, &cpuZ80::op_unimpl<0xfd43>,
  &cpuZ80::op_unimpl<0xfd44>, &cpuZ80::op_unimpl<0xfd45>, &cpuZ80::op_unimpl<0xfd46>, &cpuZ80::op_unimpl<0xfd47>,
  &cpuZ80::op_unimpl<0xfd48>, &cpuZ80::op_unimpl<0xfd49>, &cpuZ80::op_unimpl<0xfd4a>, &cpuZ80::op_unimpl<0xfd4b>,
  &cpuZ80::op_unimpl<0xfd4c>, &cpuZ80::op_unimpl<0xfd4d>, &cpuZ80::op_unimpl<0xfd4e>, &cpuZ80::op_unimpl<0xfd4f>,
  &cpuZ80::op_unimpl<0xfd50>, &cpuZ80::op_unimpl<0xfd51>, &cpuZ80::op_unimpl<0xfd52>, &cpuZ80::op_unimpl<0xfd53>,
  &cpuZ80::op_unimpl<0xfd54>, &cpuZ80::op_unimpl<0xfd55>, &cpuZ80::op_unimpl<0xfd56>, &cpuZ80::op_unimpl<0xfd57>,
  &cpuZ80::op_unimpl<0xfd58>, &cpuZ80::op_unimpl<0xfd59>, &cpuZ80::op_unimpl<0xfd5a>, &cpuZ80::op_unimpl<0xfd5b>,
  &cpuZ80::op_unimpl<0xfd5c>, &cpuZ80::op_unimpl<0xfd5d>, &cpuZ80::op_unimpl<0xfd5e>, &cpuZ80::op_unimpl<0xfd5f>,
  &cpuZ80::op_unimpl<0xfd60>, &cpuZ80::op_unimpl<0xfd61>, &cpuZ80::op_unimpl<0xfd62>, &cpuZ80::op_unimpl<0xfd63>,
  &cpuZ80::op_unimpl<0xfd64>, &cpuZ80::op_unimpl<0xfd65>, &cpuZ80::op_unimpl<0xfd66>, &cpuZ80::op_unimpl<0xfd67>,
  &cpuZ80::op_unimpl<0xfd68>, &cpuZ80::op_unimpl<0xfd69>, &cpuZ80::op_unimpl<0xfd6a>, &cpuZ80::op_unimpl<0xfd6b>,
  &cpuZ80::op_unimpl<0xfd6c>, &cpuZ80::op_unimpl<0xfd6d>, &cpuZ80::op_unimpl<0xfd6e>, &cpuZ80::op_unimpl<0xfd6f>,
  &cpuZ80::op_unimpl<0xfd70>, &cpuZ80::op_unimpl<0xfd71>, &cpuZ80::op_unimpl<0xfd72>, &cpuZ80::op_unimpl<0xfd73>,
  &cpuZ80::op_unimpl<0xfd74>, &cpuZ80::op_unimpl<0xfd75>, &cpuZ80::op_unimpl<0xfd76>, &cpuZ80::op_unimpl<0xfd77>,
  &cpuZ80::op_unimpl<0xfd78>, &cpuZ80::op_unimpl<0xfd79>, &cpuZ80::op_unimpl<0xfd7a>, &cpuZ80::op_unimpl<0xfd7b>,
  &cpuZ80::op_unimpl<0xfd7c>, &cpuZ80::op_unimpl<0xfd7d>, &cpuZ80::op_unimpl<0xfd7e>, &cpuZ80::op_unimpl<0xfd7f>,
  &cpuZ80::op_unimpl<0xfd80>, &cpuZ80::op_unimpl<0xfd81>, &cpuZ80::op_unimpl<0xfd82>, &cpuZ80::op_unimpl<0xfd83>,
  &cpuZ80::op_unimpl<0xfd84>, &cpuZ80::op_unimpl<0xfd85>, &cpuZ80::op_unimpl<0xfd86>, &cpuZ80::op_unimpl<0xfd87>,
  &cpuZ80::op_unimpl<0xfd88>, &cpuZ80::op_unimpl<0xfd89>, &cpuZ80::op_unimpl<0xfd8a>, &cpuZ80::op_unimpl<0xfd8b>,
  &cpuZ80::op_unimpl<0xfd8c>, &cpuZ80::op_unimpl<0xfd8d>, &cpuZ80::op_unimpl<0xfd8e>, &cpuZ80::op_unimpl<0xfd8f>,
  &cpuZ80::op_unimpl<0xfd90>, &cpuZ80::op_unimpl<0xfd91>, &cpuZ80::op_unimpl<0xfd92>, &cpuZ80::op_unimpl<0xfd93>,
  &cpuZ80::op_unimpl<0xfd94>, &cpuZ80::op_unimpl<0xfd95>, &cpuZ80::op_unimpl<0xfd96>, &cpuZ80::op_unimpl<0xfd97>,
  &cpuZ80::op_unimpl<0xfd98>, &cpuZ80::op_unimpl<0xfd99>, &cpuZ80::op_unimpl<0xfd9a>, &cpuZ80::op_unimpl<0xfd9b>,
  &cpuZ80::op_unimpl<0xfd9c>, &cpuZ80::op_unimpl<0xfd9d>, &cpuZ80::op_unimpl<0xfd9e>, &cpuZ80::op_unimpl<0xfd9f>,
  &cpuZ80::op_unimpl<0xfda0>, &cpuZ80::op_unimpl<0xfda1>, &cpuZ80::op_unimpl<0xfda2>, &cpuZ80::op_unimpl<0xfda3>,
  &cpuZ80::op_unimpl<0xfda4>, &cpuZ80::op_unimpl<0xfda5>, &cpuZ80::op_unimpl<0xfda6>, &cpuZ80::op_unimpl<0xfda7>,
  &cpuZ80::op_unimpl<0xfda8>, &cpuZ80::op_unimpl<0xfda9>, &cpuZ80::op_unimpl<0xfdaa>, &cpuZ80::op_unimpl<0xfdab>,
  &cpuZ80::op_unimpl<0xfdac>, &cpuZ80::op_unimpl<0xfdad>, &cpuZ80::op_unimpl<0xfdae>, &cpuZ80::op_unimpl<0xfdaf>,
  &cpuZ80::op_unimpl<0xfdb0>, &cpuZ80::op_unimpl<0xfdb1>, &cpuZ80::op_unimpl<0xfdb2>, &cpuZ80::op_unimpl<0xfdb3>,
  &cpuZ80::op_unimpl<0xfdb4>, &cpuZ80::op_unimpl<0xfdb5>, &cpuZ80::op_unimpl<0xfdb6>, &cpuZ80::op_unimpl<0xfdb7>,
  &cpuZ80::op_unimpl<0xfdb8>, &cpuZ80::op_unimpl<0xfdb9>, &cpuZ80::op_unimpl<0xfdba>, &cpuZ80::op_unimpl<0xfdbb>,
  &cpuZ80::op_unimpl<0xfdbc>, &cpuZ80::op_unimpl<0xfdbd>, &cpuZ80::op_unimpl<0xfdbe>, &cpuZ80::op_unimpl<0xfdbf>,
  &cpuZ80::op_unimpl<0xfdc0>, &cpuZ80::op_unimpl<0xfdc1>, &cpuZ80::op_unimpl<0xfdc2>, &cpuZ80::op_unimpl<0xfdc3>,
  &cpuZ80::op_unimpl<0xfdc4>, &cpuZ80::op_unimpl<0xfdc5>, &cpuZ80::op_unimpl<0xfdc6>, &cpuZ80::op_unimpl<0xfdc7>,
  &cpuZ80::op_unimpl<0xfdc8>, &cpuZ80::op_unimpl<0xfdc9>, &cpuZ80::op_unimpl<0xfdca>, &cpuZ80::fdcb_op_prefix<0xfdcb>,
  &cpuZ80::op_unimpl<0xfdcc>, &cpuZ80::op_unimpl<0xfdcd>, &cpuZ80::op_unimpl<0xfdce>, &cpuZ80::op_unimpl<0xfdcf>,
  &cpuZ80::op_unimpl<0xfdd0>, &cpuZ80::op_unimpl<0xfdd1>, &cpuZ80::op_unimpl<0xfdd2>, &cpuZ80::op_unimpl<0xfdd3>,
  &cpuZ80::op_unimpl<0xfdd4>, &cpuZ80::op_unimpl<0xfdd5>, &cpuZ80::op_unimpl<0xfdd6>, &cpuZ80::op_unimpl<0xfdd7>,
  &cpuZ80::op_unimpl<0xfdd8>, &cpuZ80::op_unimpl<0xfdd9>, &cpuZ80::op_unimpl<0xfdda>, &cpuZ80::op_unimpl<0xfddb>,
  &cpuZ80::op_unimpl<0xfddc>, &cpuZ80::op_unimpl<0xfddd>, &cpuZ80::op_unimpl<0xfdde>, &cpuZ80::op_unimpl<0xfddf>,
  &cpuZ80::op_unimpl<0xfde0>, &cpuZ80::op_pop<0xfde1>,    &cpuZ80::op_unimpl<0xfde2>, &cpuZ80::op_unimpl<0xfde3>,
  &cpuZ80::op_unimpl<0xfde4>, &cpuZ80::op_push<0xfde5>,   &cpuZ80::op_unimpl<0xfde6>, &cpuZ80::op_unimpl<0xfde7>,
  &cpuZ80::op_unimpl<0xfde8>, &cpuZ80::op_unimpl<0xfde9>, &cpuZ80::op_unimpl<0xfdea>, &cpuZ80::op_unimpl<0xfdeb>,
  &cpuZ80::op_unimpl<0xfdec>, &cpuZ80::op_unimpl<0xfded>, &cpuZ80::op_unimpl<0xfdee>, &cpuZ80::op_unimpl<0xfdef>,
  &cpuZ80::op_unimpl<0xfdf0>, &cpuZ80::op_unimpl<0xfdf1>, &cpuZ80::op_unimpl<0xfdf2>, &cpuZ80::op_unimpl<0xfdf3>,
  &cpuZ80::op_unimpl<0xfdf4>, &cpuZ80::op_unimpl<0xfdf5>, &cpuZ80::op_unimpl<0xfdf6>, &cpuZ80::op_unimpl<0xfdf7>,
  &cpuZ80::op_unimpl<0xfdf8>, &cpuZ80::op_unimpl<0xfdf9>, &cpuZ80::op_unimpl<0xfdfa>, &cpuZ80::op_unimpl<0xfdfb>,
  &cpuZ80::op_unimpl<0xfdfc>, &cpuZ80::op_unimpl<0xfdfd>, &cpuZ80::op_unimpl<0xfdfe>, &cpuZ80::op_unimpl<0xfdff>
};

std::array<z80OpPtr, 256> cpuZ80::ddcb_op_table = {
  &cpuZ80::op_unimpl<0xddcb00>, &cpuZ80::op_unimpl<0xddcb01>, &cpuZ80::op_unimpl<0xddcb02>, &cpuZ80::op_unimpl<0xddcb03>,
  &cpuZ80::op_unimpl<0xddcb04>, &cpuZ80::op_unimpl<0xddcb05>, &cpuZ80::op_unimpl<0xddcb06>, &cpuZ80::op_unimpl<0xddcb07>,
  &cpuZ80::op_unimpl<0xddcb08>, &cpuZ80::op_unimpl<0xddcb09>, &cpuZ80::op_unimpl<0xddcb0a>, &cpuZ80::op_unimpl<0xddcb0b>,
  &cpuZ80::op_unimpl<0xddcb0c>, &cpuZ80::op_unimpl<0xddcb0d>, &cpuZ80::op_unimpl<0xddcb0e>, &cpuZ80::op_unimpl<0xddcb0f>,
  &cpuZ80::op_unimpl<0xddcb10>, &cpuZ80::op_unimpl<0xddcb11>, &cpuZ80::op_unimpl<0xddcb12>, &cpuZ80::op_unimpl<0xddcb13>,
  &cpuZ80::op_unimpl<0xddcb14>, &cpuZ80::op_unimpl<0xddcb15>, &cpuZ80::op_unimpl<0xddcb16>, &cpuZ80::op_unimpl<0xddcb17>,
  &cpuZ80::op_unimpl<0xddcb18>, &cpuZ80::op_unimpl<0xddcb19>, &cpuZ80::op_unimpl<0xddcb1a>, &cpuZ80::op_unimpl<0xddcb1b>,
  &cpuZ80::op_unimpl<0xddcb1c>, &cpuZ80::op_unimpl<0xddcb1d>, &cpuZ80::op_unimpl<0xddcb1e>, &cpuZ80::op_unimpl<0xddcb1f>,
  &cpuZ80::op_unimpl<0xddcb20>, &cpuZ80::op_unimpl<0xddcb21>, &cpuZ80::op_unimpl<0xddcb22>, &cpuZ80::op_unimpl<0xddcb23>,
  &cpuZ80::op_unimpl<0xddcb24>, &cpuZ80::op_unimpl<0xddcb25>, &cpuZ80::op_unimpl<0xddcb26>, &cpuZ80::op_unimpl<0xddcb27>,
  &cpuZ80::op_unimpl<0xddcb28>, &cpuZ80::op_unimpl<0xddcb29>, &cpuZ80::op_unimpl<0xddcb2a>, &cpuZ80::op_unimpl<0xddcb2b>,
  &cpuZ80::op_unimpl<0xddcb2c>, &cpuZ80::op_unimpl<0xddcb2d>, &cpuZ80::op_unimpl<0xddcb2e>, &cpuZ80::op_unimpl<0xddcb2f>,
  &cpuZ80::op_unimpl<0xddcb30>, &cpuZ80::op_unimpl<0xddcb31>, &cpuZ80::op_unimpl<0xddcb32>, &cpuZ80::op_unimpl<0xddcb33>,
  &cpuZ80::op_unimpl<0xddcb34>, &cpuZ80::op_unimpl<0xddcb35>, &cpuZ80::op_unimpl<0xddcb36>, &cpuZ80::op_unimpl<0xddcb37>,
  &cpuZ80::op_unimpl<0xddcb38>, &cpuZ80::op_unimpl<0xddcb39>, &cpuZ80::op_unimpl<0xddcb3a>, &cpuZ80::op_unimpl<0xddcb3b>,
  &cpuZ80::op_unimpl<0xddcb3c>, &cpuZ80::op_unimpl<0xddcb3d>, &cpuZ80::op_unimpl<0xddcb3e>, &cpuZ80::op_unimpl<0xddcb3f>,
  &cpuZ80::op_unimpl<0xddcb40>, &cpuZ80::op_unimpl<0xddcb41>, &cpuZ80::op_unimpl<0xddcb42>, &cpuZ80::op_unimpl<0xddcb43>,
  &cpuZ80::op_unimpl<0xddcb44>, &cpuZ80::op_unimpl<0xddcb45>, &cpuZ80::op_unimpl<0xddcb46>, &cpuZ80::op_unimpl<0xddcb47>,
  &cpuZ80::op_unimpl<0xddcb48>, &cpuZ80::op_unimpl<0xddcb49>, &cpuZ80::op_unimpl<0xddcb4a>, &cpuZ80::op_unimpl<0xddcb4b>,
  &cpuZ80::op_unimpl<0xddcb4c>, &cpuZ80::op_unimpl<0xddcb4d>, &cpuZ80::op_unimpl<0xddcb4e>, &cpuZ80::op_unimpl<0xddcb4f>,
  &cpuZ80::op_unimpl<0xddcb50>, &cpuZ80::op_unimpl<0xddcb51>, &cpuZ80::op_unimpl<0xddcb52>, &cpuZ80::op_unimpl<0xddcb53>,
  &cpuZ80::op_unimpl<0xddcb54>, &cpuZ80::op_unimpl<0xddcb55>, &cpuZ80::op_unimpl<0xddcb56>, &cpuZ80::op_unimpl<0xddcb57>,
  &cpuZ80::op_unimpl<0xddcb58>, &cpuZ80::op_unimpl<0xddcb59>, &cpuZ80::op_unimpl<0xddcb5a>, &cpuZ80::op_unimpl<0xddcb5b>,
  &cpuZ80::op_unimpl<0xddcb5c>, &cpuZ80::op_unimpl<0xddcb5d>, &cpuZ80::op_unimpl<0xddcb5e>, &cpuZ80::op_unimpl<0xddcb5f>,
  &cpuZ80::op_unimpl<0xddcb60>, &cpuZ80::op_unimpl<0xddcb61>, &cpuZ80::op_unimpl<0xddcb62>, &cpuZ80::op_unimpl<0xddcb63>,
  &cpuZ80::op_unimpl<0xddcb64>, &cpuZ80::op_unimpl<0xddcb65>, &cpuZ80::op_unimpl<0xddcb66>, &cpuZ80::op_unimpl<0xddcb67>,
  &cpuZ80::op_unimpl<0xddcb68>, &cpuZ80::op_unimpl<0xddcb69>, &cpuZ80::op_unimpl<0xddcb6a>, &cpuZ80::op_unimpl<0xddcb6b>,
  &cpuZ80::op_unimpl<0xddcb6c>, &cpuZ80::op_unimpl<0xddcb6d>, &cpuZ80::op_unimpl<0xddcb6e>, &cpuZ80::op_unimpl<0xddcb6f>,
  &cpuZ80::op_unimpl<0xddcb70>, &cpuZ80::op_unimpl<0xddcb71>, &cpuZ80::op_unimpl<0xddcb72>, &cpuZ80::op_unimpl<0xddcb73>,
  &cpuZ80::op_unimpl<0xddcb74>, &cpuZ80::op_unimpl<0xddcb75>, &cpuZ80::op_unimpl<0xddcb76>, &cpuZ80::op_unimpl<0xddcb77>,
  &cpuZ80::op_unimpl<0xddcb78>, &cpuZ80::op_unimpl<0xddcb79>, &cpuZ80::op_unimpl<0xddcb7a>, &cpuZ80::op_unimpl<0xddcb7b>,
  &cpuZ80::op_unimpl<0xddcb7c>, &cpuZ80::op_unimpl<0xddcb7d>, &cpuZ80::op_unimpl<0xddcb7e>, &cpuZ80::op_unimpl<0xddcb7f>,
  &cpuZ80::op_unimpl<0xddcb80>, &cpuZ80::op_unimpl<0xddcb81>, &cpuZ80::op_unimpl<0xddcb82>, &cpuZ80::op_unimpl<0xddcb83>,
  &cpuZ80::op_unimpl<0xddcb84>, &cpuZ80::op_unimpl<0xddcb85>, &cpuZ80::op_unimpl<0xddcb86>, &cpuZ80::op_unimpl<0xddcb87>,
  &cpuZ80::op_unimpl<0xddcb88>, &cpuZ80::op_unimpl<0xddcb89>, &cpuZ80::op_unimpl<0xddcb8a>, &cpuZ80::op_unimpl<0xddcb8b>,
  &cpuZ80::op_unimpl<0xddcb8c>, &cpuZ80::op_unimpl<0xddcb8d>, &cpuZ80::op_unimpl<0xddcb8e>, &cpuZ80::op_unimpl<0xddcb8f>,
  &cpuZ80::op_unimpl<0xddcb90>, &cpuZ80::op_unimpl<0xddcb91>, &cpuZ80::op_unimpl<0xddcb92>, &cpuZ80::op_unimpl<0xddcb93>,
  &cpuZ80::op_unimpl<0xddcb94>, &cpuZ80::op_unimpl<0xddcb95>, &cpuZ80::op_unimpl<0xddcb96>, &cpuZ80::op_unimpl<0xddcb97>,
  &cpuZ80::op_unimpl<0xddcb98>, &cpuZ80::op_unimpl<0xddcb99>, &cpuZ80::op_unimpl<0xddcb9a>, &cpuZ80::op_unimpl<0xddcb9b>,
  &cpuZ80::op_unimpl<0xddcb9c>, &cpuZ80::op_unimpl<0xddcb9d>, &cpuZ80::op_unimpl<0xddcb9e>, &cpuZ80::op_unimpl<0xddcb9f>,
  &cpuZ80::op_unimpl<0xddcba0>, &cpuZ80::op_unimpl<0xddcba1>, &cpuZ80::op_unimpl<0xddcba2>, &cpuZ80::op_unimpl<0xddcba3>,
  &cpuZ80::op_unimpl<0xddcba4>, &cpuZ80::op_unimpl<0xddcba5>, &cpuZ80::op_unimpl<0xddcba6>, &cpuZ80::op_unimpl<0xddcba7>,
  &cpuZ80::op_unimpl<0xddcba8>, &cpuZ80::op_unimpl<0xddcba9>, &cpuZ80::op_unimpl<0xddcbaa>, &cpuZ80::op_unimpl<0xddcbab>,
  &cpuZ80::op_unimpl<0xddcbac>, &cpuZ80::op_unimpl<0xddcbad>, &cpuZ80::op_unimpl<0xddcbae>, &cpuZ80::op_unimpl<0xddcbaf>,
  &cpuZ80::op_unimpl<0xddcbb0>, &cpuZ80::op_unimpl<0xddcbb1>, &cpuZ80::op_unimpl<0xddcbb2>, &cpuZ80::op_unimpl<0xddcbb3>,
  &cpuZ80::op_unimpl<0xddcbb4>, &cpuZ80::op_unimpl<0xddcbb5>, &cpuZ80::op_unimpl<0xddcbb6>, &cpuZ80::op_unimpl<0xddcbb7>,
  &cpuZ80::op_unimpl<0xddcbb8>, &cpuZ80::op_unimpl<0xddcbb9>, &cpuZ80::op_unimpl<0xddcbba>, &cpuZ80::op_unimpl<0xddcbbb>,
  &cpuZ80::op_unimpl<0xddcbbc>, &cpuZ80::op_unimpl<0xddcbbd>, &cpuZ80::op_unimpl<0xddcbbe>, &cpuZ80::op_unimpl<0xddcbbf>,
  &cpuZ80::op_unimpl<0xddcbc0>, &cpuZ80::op_unimpl<0xddcbc1>, &cpuZ80::op_unimpl<0xddcbc2>, &cpuZ80::op_unimpl<0xddcbc3>,
  &cpuZ80::op_unimpl<0xddcbc4>, &cpuZ80::op_unimpl<0xddcbc5>, &cpuZ80::op_unimpl<0xddcbc6>, &cpuZ80::op_unimpl<0xddcbc7>,
  &cpuZ80::op_unimpl<0xddcbc8>, &cpuZ80::op_unimpl<0xddcbc9>, &cpuZ80::op_unimpl<0xddcbca>, &cpuZ80::op_unimpl<0xddcbcb>,
  &cpuZ80::op_unimpl<0xddcbcc>, &cpuZ80::op_unimpl<0xddcbcd>, &cpuZ80::op_unimpl<0xddcbce>, &cpuZ80::op_unimpl<0xddcbcf>,
  &cpuZ80::op_unimpl<0xddcbd0>, &cpuZ80::op_unimpl<0xddcbd1>, &cpuZ80::op_unimpl<0xddcbd2>, &cpuZ80::op_unimpl<0xddcbd3>,
  &cpuZ80::op_unimpl<0xddcbd4>, &cpuZ80::op_unimpl<0xddcbd5>, &cpuZ80::op_unimpl<0xddcbd6>, &cpuZ80::op_unimpl<0xddcbd7>,
  &cpuZ80::op_unimpl<0xddcbd8>, &cpuZ80::op_unimpl<0xddcbd9>, &cpuZ80::op_unimpl<0xddcbda>, &cpuZ80::op_unimpl<0xddcbdb>,
  &cpuZ80::op_unimpl<0xddcbdc>, &cpuZ80::op_unimpl<0xddcbdd>, &cpuZ80::op_unimpl<0xddcbde>, &cpuZ80::op_unimpl<0xddcbdf>,
  &cpuZ80::op_unimpl<0xddcbe0>, &cpuZ80::op_unimpl<0xddcbe1>, &cpuZ80::op_unimpl<0xddcbe2>, &cpuZ80::op_unimpl<0xddcbe3>,
  &cpuZ80::op_unimpl<0xddcbe4>, &cpuZ80::op_unimpl<0xddcbe5>, &cpuZ80::op_unimpl<0xddcbe6>, &cpuZ80::op_unimpl<0xddcbe7>,
  &cpuZ80::op_unimpl<0xddcbe8>, &cpuZ80::op_unimpl<0xddcbe9>, &cpuZ80::op_unimpl<0xddcbea>, &cpuZ80::op_unimpl<0xddcbeb>,
  &cpuZ80::op_unimpl<0xddcbec>, &cpuZ80::op_unimpl<0xddcbed>, &cpuZ80::op_unimpl<0xddcbee>, &cpuZ80::op_unimpl<0xddcbef>,
  &cpuZ80::op_unimpl<0xddcbf0>, &cpuZ80::op_unimpl<0xddcbf1>, &cpuZ80::op_unimpl<0xddcbf2>, &cpuZ80::op_unimpl<0xddcbf3>,
  &cpuZ80::op_unimpl<0xddcbf4>, &cpuZ80::op_unimpl<0xddcbf5>, &cpuZ80::op_unimpl<0xddcbf6>, &cpuZ80::op_unimpl<0xddcbf7>,
  &cpuZ80::op_unimpl<0xddcbf8>, &cpuZ80::op_unimpl<0xddcbf9>, &cpuZ80::op_unimpl<0xddcbfa>, &cpuZ80::op_unimpl<0xddcbfb>,
  &cpuZ80::op_unimpl<0xddcbfc>, &cpuZ80::op_unimpl<0xddcbfd>, &cpuZ80::op_unimpl<0xddcbfe>, &cpuZ80::op_unimpl<0xddcbff>
};

std::array<z80OpPtr, 256> cpuZ80::fdcb_op_table = {
  &cpuZ80::op_unimpl<0xfdcb00>, &cpuZ80::op_unimpl<0xfdcb01>, &cpuZ80::op_unimpl<0xfdcb02>, &cpuZ80::op_unimpl<0xfdcb03>,
  &cpuZ80::op_unimpl<0xfdcb04>, &cpuZ80::op_unimpl<0xfdcb05>, &cpuZ80::op_unimpl<0xfdcb06>, &cpuZ80::op_unimpl<0xfdcb07>,
  &cpuZ80::op_unimpl<0xfdcb08>, &cpuZ80::op_unimpl<0xfdcb09>, &cpuZ80::op_unimpl<0xfdcb0a>, &cpuZ80::op_unimpl<0xfdcb0b>,
  &cpuZ80::op_unimpl<0xfdcb0c>, &cpuZ80::op_unimpl<0xfdcb0d>, &cpuZ80::op_unimpl<0xfdcb0e>, &cpuZ80::op_unimpl<0xfdcb0f>,
  &cpuZ80::op_unimpl<0xfdcb10>, &cpuZ80::op_unimpl<0xfdcb11>, &cpuZ80::op_unimpl<0xfdcb12>, &cpuZ80::op_unimpl<0xfdcb13>,
  &cpuZ80::op_unimpl<0xfdcb14>, &cpuZ80::op_unimpl<0xfdcb15>, &cpuZ80::op_unimpl<0xfdcb16>, &cpuZ80::op_unimpl<0xfdcb17>,
  &cpuZ80::op_unimpl<0xfdcb18>, &cpuZ80::op_unimpl<0xfdcb19>, &cpuZ80::op_unimpl<0xfdcb1a>, &cpuZ80::op_unimpl<0xfdcb1b>,
  &cpuZ80::op_unimpl<0xfdcb1c>, &cpuZ80::op_unimpl<0xfdcb1d>, &cpuZ80::op_unimpl<0xfdcb1e>, &cpuZ80::op_unimpl<0xfdcb1f>,
  &cpuZ80::op_unimpl<0xfdcb20>, &cpuZ80::op_unimpl<0xfdcb21>, &cpuZ80::op_unimpl<0xfdcb22>, &cpuZ80::op_unimpl<0xfdcb23>,
  &cpuZ80::op_unimpl<0xfdcb24>, &cpuZ80::op_unimpl<0xfdcb25>, &cpuZ80::op_unimpl<0xfdcb26>, &cpuZ80::op_unimpl<0xfdcb27>,
  &cpuZ80::op_unimpl<0xfdcb28>, &cpuZ80::op_unimpl<0xfdcb29>, &cpuZ80::op_unimpl<0xfdcb2a>, &cpuZ80::op_unimpl<0xfdcb2b>,
  &cpuZ80::op_unimpl<0xfdcb2c>, &cpuZ80::op_unimpl<0xfdcb2d>, &cpuZ80::op_unimpl<0xfdcb2e>, &cpuZ80::op_unimpl<0xfdcb2f>,
  &cpuZ80::op_unimpl<0xfdcb30>, &cpuZ80::op_unimpl<0xfdcb31>, &cpuZ80::op_unimpl<0xfdcb32>, &cpuZ80::op_unimpl<0xfdcb33>,
  &cpuZ80::op_unimpl<0xfdcb34>, &cpuZ80::op_unimpl<0xfdcb35>, &cpuZ80::op_unimpl<0xfdcb36>, &cpuZ80::op_unimpl<0xfdcb37>,
  &cpuZ80::op_unimpl<0xfdcb38>, &cpuZ80::op_unimpl<0xfdcb39>, &cpuZ80::op_unimpl<0xfdcb3a>, &cpuZ80::op_unimpl<0xfdcb3b>,
  &cpuZ80::op_unimpl<0xfdcb3c>, &cpuZ80::op_unimpl<0xfdcb3d>, &cpuZ80::op_unimpl<0xfdcb3e>, &cpuZ80::op_unimpl<0xfdcb3f>,
  &cpuZ80::op_unimpl<0xfdcb40>, &cpuZ80::op_unimpl<0xfdcb41>, &cpuZ80::op_unimpl<0xfdcb42>, &cpuZ80::op_unimpl<0xfdcb43>,
  &cpuZ80::op_unimpl<0xfdcb44>, &cpuZ80::op_unimpl<0xfdcb45>, &cpuZ80::op_unimpl<0xfdcb46>, &cpuZ80::op_unimpl<0xfdcb47>,
  &cpuZ80::op_unimpl<0xfdcb48>, &cpuZ80::op_unimpl<0xfdcb49>, &cpuZ80::op_unimpl<0xfdcb4a>, &cpuZ80::op_unimpl<0xfdcb4b>,
  &cpuZ80::op_unimpl<0xfdcb4c>, &cpuZ80::op_unimpl<0xfdcb4d>, &cpuZ80::op_unimpl<0xfdcb4e>, &cpuZ80::op_unimpl<0xfdcb4f>,
  &cpuZ80::op_unimpl<0xfdcb50>, &cpuZ80::op_unimpl<0xfdcb51>, &cpuZ80::op_unimpl<0xfdcb52>, &cpuZ80::op_unimpl<0xfdcb53>,
  &cpuZ80::op_unimpl<0xfdcb54>, &cpuZ80::op_unimpl<0xfdcb55>, &cpuZ80::op_unimpl<0xfdcb56>, &cpuZ80::op_unimpl<0xfdcb57>,
  &cpuZ80::op_unimpl<0xfdcb58>, &cpuZ80::op_unimpl<0xfdcb59>, &cpuZ80::op_unimpl<0xfdcb5a>, &cpuZ80::op_unimpl<0xfdcb5b>,
  &cpuZ80::op_unimpl<0xfdcb5c>, &cpuZ80::op_unimpl<0xfdcb5d>, &cpuZ80::op_unimpl<0xfdcb5e>, &cpuZ80::op_unimpl<0xfdcb5f>,
  &cpuZ80::op_unimpl<0xfdcb60>, &cpuZ80::op_unimpl<0xfdcb61>, &cpuZ80::op_unimpl<0xfdcb62>, &cpuZ80::op_unimpl<0xfdcb63>,
  &cpuZ80::op_unimpl<0xfdcb64>, &cpuZ80::op_unimpl<0xfdcb65>, &cpuZ80::op_unimpl<0xfdcb66>, &cpuZ80::op_unimpl<0xfdcb67>,
  &cpuZ80::op_unimpl<0xfdcb68>, &cpuZ80::op_unimpl<0xfdcb69>, &cpuZ80::op_unimpl<0xfdcb6a>, &cpuZ80::op_unimpl<0xfdcb6b>,
  &cpuZ80::op_unimpl<0xfdcb6c>, &cpuZ80::op_unimpl<0xfdcb6d>, &cpuZ80::op_unimpl<0xfdcb6e>, &cpuZ80::op_unimpl<0xfdcb6f>,
  &cpuZ80::op_unimpl<0xfdcb70>, &cpuZ80::op_unimpl<0xfdcb71>, &cpuZ80::op_unimpl<0xfdcb72>, &cpuZ80::op_unimpl<0xfdcb73>,
  &cpuZ80::op_unimpl<0xfdcb74>, &cpuZ80::op_unimpl<0xfdcb75>, &cpuZ80::op_unimpl<0xfdcb76>, &cpuZ80::op_unimpl<0xfdcb77>,
  &cpuZ80::op_unimpl<0xfdcb78>, &cpuZ80::op_unimpl<0xfdcb79>, &cpuZ80::op_unimpl<0xfdcb7a>, &cpuZ80::op_unimpl<0xfdcb7b>,
  &cpuZ80::op_unimpl<0xfdcb7c>, &cpuZ80::op_unimpl<0xfdcb7d>, &cpuZ80::op_unimpl<0xfdcb7e>, &cpuZ80::op_unimpl<0xfdcb7f>,
  &cpuZ80::op_unimpl<0xfdcb80>, &cpuZ80::op_unimpl<0xfdcb81>, &cpuZ80::op_unimpl<0xfdcb82>, &cpuZ80::op_unimpl<0xfdcb83>,
  &cpuZ80::op_unimpl<0xfdcb84>, &cpuZ80::op_unimpl<0xfdcb85>, &cpuZ80::op_unimpl<0xfdcb86>, &cpuZ80::op_unimpl<0xfdcb87>,
  &cpuZ80::op_unimpl<0xfdcb88>, &cpuZ80::op_unimpl<0xfdcb89>, &cpuZ80::op_unimpl<0xfdcb8a>, &cpuZ80::op_unimpl<0xfdcb8b>,
  &cpuZ80::op_unimpl<0xfdcb8c>, &cpuZ80::op_unimpl<0xfdcb8d>, &cpuZ80::op_unimpl<0xfdcb8e>, &cpuZ80::op_unimpl<0xfdcb8f>,
  &cpuZ80::op_unimpl<0xfdcb90>, &cpuZ80::op_unimpl<0xfdcb91>, &cpuZ80::op_unimpl<0xfdcb92>, &cpuZ80::op_unimpl<0xfdcb93>,
  &cpuZ80::op_unimpl<0xfdcb94>, &cpuZ80::op_unimpl<0xfdcb95>, &cpuZ80::op_unimpl<0xfdcb96>, &cpuZ80::op_unimpl<0xfdcb97>,
  &cpuZ80::op_unimpl<0xfdcb98>, &cpuZ80::op_unimpl<0xfdcb99>, &cpuZ80::op_unimpl<0xfdcb9a>, &cpuZ80::op_unimpl<0xfdcb9b>,
  &cpuZ80::op_unimpl<0xfdcb9c>, &cpuZ80::op_unimpl<0xfdcb9d>, &cpuZ80::op_unimpl<0xfdcb9e>, &cpuZ80::op_unimpl<0xfdcb9f>,
  &cpuZ80::op_unimpl<0xfdcba0>, &cpuZ80::op_unimpl<0xfdcba1>, &cpuZ80::op_unimpl<0xfdcba2>, &cpuZ80::op_unimpl<0xfdcba3>,
  &cpuZ80::op_unimpl<0xfdcba4>, &cpuZ80::op_unimpl<0xfdcba5>, &cpuZ80::op_unimpl<0xfdcba6>, &cpuZ80::op_unimpl<0xfdcba7>,
  &cpuZ80::op_unimpl<0xfdcba8>, &cpuZ80::op_unimpl<0xfdcba9>, &cpuZ80::op_unimpl<0xfdcbaa>, &cpuZ80::op_unimpl<0xfdcbab>,
  &cpuZ80::op_unimpl<0xfdcbac>, &cpuZ80::op_unimpl<0xfdcbad>, &cpuZ80::op_unimpl<0xfdcbae>, &cpuZ80::op_unimpl<0xfdcbaf>,
  &cpuZ80::op_unimpl<0xfdcbb0>, &cpuZ80::op_unimpl<0xfdcbb1>, &cpuZ80::op_unimpl<0xfdcbb2>, &cpuZ80::op_unimpl<0xfdcbb3>,
  &cpuZ80::op_unimpl<0xfdcbb4>, &cpuZ80::op_unimpl<0xfdcbb5>, &cpuZ80::op_unimpl<0xfdcbb6>, &cpuZ80::op_unimpl<0xfdcbb7>,
  &cpuZ80::op_unimpl<0xfdcbb8>, &cpuZ80::op_unimpl<0xfdcbb9>, &cpuZ80::op_unimpl<0xfdcbba>, &cpuZ80::op_unimpl<0xfdcbbb>,
  &cpuZ80::op_unimpl<0xfdcbbc>, &cpuZ80::op_unimpl<0xfdcbbd>, &cpuZ80::op_unimpl<0xfdcbbe>, &cpuZ80::op_unimpl<0xfdcbbf>,
  &cpuZ80::op_unimpl<0xfdcbc0>, &cpuZ80::op_unimpl<0xfdcbc1>, &cpuZ80::op_unimpl<0xfdcbc2>, &cpuZ80::op_unimpl<0xfdcbc3>,
  &cpuZ80::op_unimpl<0xfdcbc4>, &cpuZ80::op_unimpl<0xfdcbc5>, &cpuZ80::op_unimpl<0xfdcbc6>, &cpuZ80::op_unimpl<0xfdcbc7>,
  &cpuZ80::op_unimpl<0xfdcbc8>, &cpuZ80::op_unimpl<0xfdcbc9>, &cpuZ80::op_unimpl<0xfdcbca>, &cpuZ80::op_unimpl<0xfdcbcb>,
  &cpuZ80::op_unimpl<0xfdcbcc>, &cpuZ80::op_unimpl<0xfdcbcd>, &cpuZ80::op_unimpl<0xfdcbce>, &cpuZ80::op_unimpl<0xfdcbcf>,
  &cpuZ80::op_unimpl<0xfdcbd0>, &cpuZ80::op_unimpl<0xfdcbd1>, &cpuZ80::op_unimpl<0xfdcbd2>, &cpuZ80::op_unimpl<0xfdcbd3>,
  &cpuZ80::op_unimpl<0xfdcbd4>, &cpuZ80::op_unimpl<0xfdcbd5>, &cpuZ80::op_unimpl<0xfdcbd6>, &cpuZ80::op_unimpl<0xfdcbd7>,
  &cpuZ80::op_unimpl<0xfdcbd8>, &cpuZ80::op_unimpl<0xfdcbd9>, &cpuZ80::op_unimpl<0xfdcbda>, &cpuZ80::op_unimpl<0xfdcbdb>,
  &cpuZ80::op_unimpl<0xfdcbdc>, &cpuZ80::op_unimpl<0xfdcbdd>, &cpuZ80::op_unimpl<0xfdcbde>, &cpuZ80::op_unimpl<0xfdcbdf>,
  &cpuZ80::op_unimpl<0xfdcbe0>, &cpuZ80::op_unimpl<0xfdcbe1>, &cpuZ80::op_unimpl<0xfdcbe2>, &cpuZ80::op_unimpl<0xfdcbe3>,
  &cpuZ80::op_unimpl<0xfdcbe4>, &cpuZ80::op_unimpl<0xfdcbe5>, &cpuZ80::op_unimpl<0xfdcbe6>, &cpuZ80::op_unimpl<0xfdcbe7>,
  &cpuZ80::op_unimpl<0xfdcbe8>, &cpuZ80::op_unimpl<0xfdcbe9>, &cpuZ80::op_unimpl<0xfdcbea>, &cpuZ80::op_unimpl<0xfdcbeb>,
  &cpuZ80::op_unimpl<0xfdcbec>, &cpuZ80::op_unimpl<0xfdcbed>, &cpuZ80::op_unimpl<0xfdcbee>, &cpuZ80::op_unimpl<0xfdcbef>,
  &cpuZ80::op_unimpl<0xfdcbf0>, &cpuZ80::op_unimpl<0xfdcbf1>, &cpuZ80::op_unimpl<0xfdcbf2>, &cpuZ80::op_unimpl<0xfdcbf3>,
  &cpuZ80::op_unimpl<0xfdcbf4>, &cpuZ80::op_unimpl<0xfdcbf5>, &cpuZ80::op_unimpl<0xfdcbf6>, &cpuZ80::op_unimpl<0xfdcbf7>,
  &cpuZ80::op_unimpl<0xfdcbf8>, &cpuZ80::op_unimpl<0xfdcbf9>, &cpuZ80::op_unimpl<0xfdcbfa>, &cpuZ80::op_unimpl<0xfdcbfb>,
  &cpuZ80::op_unimpl<0xfdcbfc>, &cpuZ80::op_unimpl<0xfdcbfd>, &cpuZ80::op_unimpl<0xfdcbfe>, &cpuZ80::op_unimpl<0xfdcbff>
};

template <uint32_t OPCODE>
uint64_t cpuZ80::cb_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    printf(" %x", opcode);
    return CALL_MEMBER_FN(this, cb_op_table[opcode])(opcode);
}

template <uint32_t OPCODE>
uint64_t cpuZ80::dd_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    printf(" %x", opcode);
    return CALL_MEMBER_FN(this, dd_op_table[opcode])(opcode);
}

template <uint32_t OPCODE>
uint64_t cpuZ80::ed_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    printf(" %x", opcode);
    return CALL_MEMBER_FN(this, ed_op_table[opcode - 0x40])(opcode);
}

template <uint32_t OPCODE>
uint64_t cpuZ80::fd_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    printf(" %x", opcode);
    return CALL_MEMBER_FN(this, fd_op_table[opcode])(opcode);
}

template <uint32_t OPCODE>
uint64_t cpuZ80::ddcb_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    printf(" %x", opcode);
    return CALL_MEMBER_FN(this, ddcb_op_table[opcode])(opcode);
}


template <uint32_t OPCODE>
uint64_t cpuZ80::fdcb_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    printf(" %x", opcode);
    return CALL_MEMBER_FN(this, fdcb_op_table[opcode])(opcode);
}

uint64_t cpuZ80::decode(uint8_t opcode) {
    return CALL_MEMBER_FN(this, op_table[opcode])(opcode);
}

void cpuZ80::push(uint16_t val) {
    std::printf("\tPush %04x to %04x", val, sp-2);
    memory->writeWord(sp - 2, val);
    sp -= 2;
}

uint16_t cpuZ80::pop() {
    uint16_t val = memory->readWord(sp);
    std::printf("\tPop %04x from %04x", val, sp);
    sp += 2;
    return val;
}

std::array<bool,256> cpuZ80::parity = cpuZ80::setParityArray(); // Lookup table for parity.

constexpr std::array<bool,256> cpuZ80::setParityArray() { // Calculate number of set bits in given 8-bit value. Parity is true is bit count is even.
    std::array<bool,256> parArray{};
    uint8_t mask1 = 0b01010101;
    uint8_t mask2 = 0b00110011;
    uint8_t mask3 = 0b00001111;
    for(int i=0;i<256;i++) {
        int res1 = (i&mask1);
        res1 += ((i>>1)&mask1);
        int res2 = (res1&mask2);
        res2 += ((res1>>2)&mask2);
        int res3  = (res2&mask3);
        res3 += ((res2>>4)&mask3);
        parArray[i] = ((res3 % 2) == 0);
    }
    return parArray;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_unimpl(uint8_t opcode) {
    std::cout<<"\nOpcode "<<std::hex<<OPCODE<<" not implemented.\n";
    return -1;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_call(uint8_t opcode) { // CALL 17
    uint16_t address = memory->readWord(pc);
    std::printf(" %04x", address);
    pc+=2;
    push(pc);
    pc = address;
    return 1;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ccf(uint8_t opcode) { // CCF 4
    if(carry()) clear(CARRY_FLAG);
    else set(CARRY_FLAG);
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_cpl(uint8_t opcode) { // CPL 4
    af.hi = ~(af.hi);
    set(HALF_CARRY_FLAG);
    set(SUB_FLAG);
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_daa(uint8_t opcode) { // DAA 4
   int t = 0;

   if(hc() || ((af.hi & 0xF) > 9) ) {
         t++;
   }

   if(carry() || (af.hi > 0x99) ) {
         t += 2;
         set(CARRY_FLAG);
   }

   // builds final H flag
   if (sub() && !hc()) {
      clear(HALF_CARRY_FLAG);
   }
   else if (sub() && hc()) {
      if(((af.hi & 0x0F)) < 6) set(HALF_CARRY_FLAG);
      else clear(HALF_CARRY_FLAG);
   }
   else {
        if(hc()) {
            if((af.hi & 0x0F) >= 0x0A) set(HALF_CARRY_FLAG);
            else clear(HALF_CARRY_FLAG);
        }
   }

   switch(t)
   {
        case 1:
            af.hi += (sub())?0xFA:0x06; // -6:6
            break;
        case 2:
            af.hi += (sub())?0xA0:0x60; // -0x60:0x60
            break;
        case 3:
            af.hi += (sub())?0x9A:0x66; // -0x66:0x66
            break;
   }

   if((af.hi & 0x80) != 0) set(SIGN_FLAG);
   else clear(SIGN_FLAG);
   if(af.hi == 0) set(ZERO_FLAG);
   else clear(ZERO_FLAG);
   if(parity[af.hi]) set(PARITY_FLAG);
   else clear(PARITY_FLAG);

   af.low &= 0b11010111;
   af.low |= (af.hi & 0b00101000);

   return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_decr16(uint8_t opcode) {
    uint16_t* const regset[] = {&(bc.pair), &(de.pair), &(hl.pair), &(sp)};
    (*regset[OPCODE>>4])--;
    return 6;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_decr8(uint8_t opcode) {
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    (*regset[OPCODE>>3])--;
    if(*regset[OPCODE>>3] >= 128) { set(SIGN_FLAG); } else { clear(SIGN_FLAG); }
    if(*regset[OPCODE>>3] == 0)   { set(ZERO_FLAG); } else { clear(ZERO_FLAG); }
    if(*regset[OPCODE>>3] == 15)  { set(HALF_CARRY_FLAG);} else { clear(HALF_CARRY_FLAG); }
    if(*regset[OPCODE>>3] == 255 || *regset[OPCODE>>3] == 127) { set(OVERFLOW_FLAG); } else { clear(OVERFLOW_FLAG); }
    set(SUB_FLAG);
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_di(uint8_t opcode) { // DI 4
    iff1 = false;
    iff2 = false;
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ei(uint8_t opcode) { // EI 4
    iff1 = true;
    iff2 = true;
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_exx(uint8_t opcode) { // EXX 4
    if(OPCODE == 0x08) { // EX AF, AF'
        std::swap(af.pair, af_1.pair);
    }
    else { // EXX
        std::swap(bc.pair, bc_1.pair);
        std::swap(de.pair, de_1.pair);
        std::swap(hl.pair, hl_1.pair);
    }
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_im(uint8_t opcode) { // IM0 IM1 IM2 8
    switch(OPCODE) {
    case 0xed46:
    case 0xed66:
        int_mode = mode0;
        break;
    case 0xed56:
    case 0xed76:
        int_mode = mode1;
        break;
    case 0xed5e:
    case 0xed7e:
        int_mode = mode2;
        break;
    }
    return 1;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_incr16(uint8_t opcode) {
    uint16_t* const regset[] = {&(bc.pair), &(de.pair), &(hl.pair), &(sp)};
    (*regset[OPCODE>>4])++;
    return 6;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_incr8(uint8_t opcode) {
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    (*regset[OPCODE>>3])++;
    if(*regset[OPCODE>>3] >= 128) { set(SIGN_FLAG); } else { clear(SIGN_FLAG); }
    if(*regset[OPCODE>>3] == 0)   { set(ZERO_FLAG); } else { clear(ZERO_FLAG); }
    if(*regset[OPCODE>>3] == 16)  { set(HALF_CARRY_FLAG);} else { clear(HALF_CARRY_FLAG); }
    if(*regset[OPCODE>>3] == 0 || *regset[OPCODE>>3] == 128) { set(OVERFLOW_FLAG); } else { clear(OVERFLOW_FLAG); }
    clear(SUB_FLAG);
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_jp(uint8_t opcode) {
    switch(OPCODE) {
    case 0xc3: { //JP nn 4,3,3
            uint16_t jump_addr = memory->readWord(pc);
            std::printf(" %04x", jump_addr);
            pc = jump_addr;
            return 1;
        }
    }
    return -1;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_jr(uint8_t opcode) { //DJNZ and various JR instructions
    bool branch = false;
    uint64_t cycles = 12;
    switch(OPCODE) {
    case 0x10: //DJNZ
        bc.hi--;
        if(bc.hi) {
            branch = true;
            cycles = 13;
        }
        else cycles = 8;
        break;
    case 0x18: //JR *
        branch = true;
        break;
    case 0x20: //JR NZ, *
        if(!zero()) branch = true;
        else cycles = 7;
        break;
    case 0x28: //JR Z, *
        if(zero()) branch = true;
        else cycles = 7;
        break;
    case 0x30: //JR NC, *
        if(!carry()) branch = true;
        else cycles = 7;
        break;
    case 0x38: //JR C, *
        if(carry()) branch = true;
        else cycles = 7;
    }
    if(branch) {
        int8_t offset = memory->readByte(pc);
        pc += (offset - 1);
    }
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8ri(uint8_t opcode) { //LD r,immed 7
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    constexpr uint8_t dest_index = ((OPCODE>>3) & 0x07);
    *regset[dest_index] = memory->readByte(pc++);
    std::printf(" %02x", *regset[dest_index]);

    return 1;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8rr(uint8_t opcode) { //LD r,r 4
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    constexpr uint8_t src_index = (OPCODE & 0x07);
    constexpr uint8_t dest_index = ((OPCODE>>3) & 0x07);

    *regset[dest_index] = *regset[src_index];
    return 1;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld16(uint8_t opcode) {
    uint16_t immediate = memory->readWord(pc);
    std::printf(" %04x", immediate);
    pc+=2;
    switch(OPCODE) {
    case 0x01: //LD BC, nn 4,3,3
            bc.pair = immediate;
            break;
    case 0x11: //LD DE, nn 4,3,3
            de.pair = immediate;
            break;
    case 0x21: //LD HL, nn 4,3,3
            hl.pair = immediate;
            break;
    case 0x31: //LD SP, nn 4,3,3
            sp = immediate;
            break;
    }
    return 1;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_nop(uint8_t opcode) { // NOP 4
    return 1;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_out(uint8_t opcode) { // OUTI 16 OTIR 21/16
    uint8_t val = memory->readByte(hl.pair);
    memory->writePortByte(bc.low, val);
    hl.pair++;
    bc.hi--;

    if(OPCODE == 0xedb3) {
        if(bc.hi != 0) {
            pc -= 2; // repeat the same opcode
            return 21;
        }
    }

    return 16;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_pop(uint8_t opcode) { // PUSH bc/de/hl/af 10 PUSH ix/iy 14
    switch(OPCODE) {
    case 0xC1: bc.pair = pop(); break;
    case 0xD1: de.pair = pop(); break;
    case 0xE1: hl.pair = pop(); break;
    case 0xF1: af.pair = pop(); break;
    case 0xDDE1: ix.pair = pop(); return 14;
    case 0xFDE1: iy.pair = pop(); return 14;
    }
    return 10;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_push(uint8_t opcode) { // PUSH bc/de/hl/af 11 PUSH ix/iy 15
    switch(OPCODE) {
    case 0xC5: push(bc.pair); break;
    case 0xD5: push(de.pair); break;
    case 0xE5: push(hl.pair); break;
    case 0xF5: push(af.pair); break;
    case 0xDDE5: push(ix.pair); return 15;
    case 0xFDE5: push(iy.pair); return 15;
    }
    return 11;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ret(uint8_t opcode) { // RET 10
    pc = pop();
    return 10;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_scf(uint8_t opcode) { // SCF 4
    set(CARRY_FLAG);
    return 4;
}
