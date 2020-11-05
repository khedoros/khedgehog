#include "cpuZ80.h"
#include<iostream>

uint64_t cpuZ80::calc(uint64_t) {
    return 0;
}

cpuZ80::cpuZ80(std::shared_ptr<memmapZ80> memmap): memory(memmap) {

}

//std::array<z80OpPtr, 256> cpuZ80::op_table = {&cpuZ80::op_unimplemented<0>};
//std::array<z80OpPtr, 256> cpuZ80::cb_op_table = {&cpuZ80::op_unimplemented<0>};
//std::array<z80OpPtr, 256> cpuZ80::dd_op_table = {&cpuZ80::op_unimplemented<0>};
//std::array<z80OpPtr, 256> cpuZ80::ed_op_table = {&cpuZ80::op_unimplemented<0>};
//std::array<z80OpPtr, 256> cpuZ80::fd_op_table = {&cpuZ80::op_unimplemented<0>};
//std::array<z80OpPtr, 256> cpuZ80::fdcb_op_table = {&cpuZ80::op_unimplemented<0>};


uint64_t cpuZ80::cb_op_prefix() { return -1; }
uint64_t cpuZ80::dd_op_prefix() { return -1; }
uint64_t cpuZ80::ed_op_prefix() { return -1; }
uint64_t cpuZ80::fd_op_prefix() { return -1; }
uint64_t cpuZ80::fdcb_op_prefix() { return -1; }

template <uint8_t OPCODE>
uint64_t op_unimplemented() {
    std::cout<<"Opcode "<<std::hex<<OPCODE<<" not implemented."<<std::endl;
    return -1;
}
