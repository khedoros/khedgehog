#include "cpuZ80.h"
#include<iostream>
#include<limits>
#include<cassert>
#include "../../util.h"

uint64_t cpuZ80::calc(const uint64_t cycles_to_run) {

    cycles_remaining += cycles_to_run;
    while(cycles_remaining > 0) {
        const uint8_t opcode = memory->readByte(pc++);
        dbg_printf("%04X: %02x", pc-1, opcode);
        const uint64_t inst_cycles = CALL_MEMBER_FN(this, op_table[opcode])(opcode);
        print_registers();
        dbg_printf("\n");

        if(inst_cycles == uint64_t(-1)) {
            return 0;
        }

        cycles_remaining -= inst_cycles;
        total_cycles+=inst_cycles;
    }
    // std::printf("total_cycles: %lu\n", total_cycles);

    return cycles_to_run - cycles_remaining;
}

cpuZ80::cpuZ80(std::shared_ptr<memmapZ80Console> memmap): memory(memmap), cycles_remaining(0), pc(0), iff1(false), iff2(false), total_cycles(0), halted(false), sp(0xffff), int_mode(cpuZ80::mode0) {
    af.pair = 0xffff;
    reset();
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
#define parity() ((af.low & PARITY_FLAG)>>(2))
#define overflow() ((af.low & OVERFLOW_FLAG)>>(2))
#define sub() ((af.low & SUB_FLAG)>>(1))
#define carry() ((af.low & CARRY_FLAG))

uint64_t cpuZ80::test_daa() {
    std::ifstream results("daaoutput.txt");
    if(!results) { std::cerr<<"Couldn't open daaoutput.txt"<<std::endl;return 0;}
    while(results) {
        char reg;
        int n_start, n_end, c_start, c_end, h_start, h_end, a_start, a_end;
        bool c_m = false, h_m = false, n_m = false, a_m = false;
        results>>std::hex;
        //N 0 C 0 H 0 00 N 0 C 0 H 0 00
        results>>reg>>n_start>>reg>>c_start>>reg>>h_start>>a_start>>reg>>n_end>>reg>>c_end>>reg>>h_end>>a_end;

        std::cout<<"Test case "<<n_start<<" "<<c_start<<" "<<h_start<<" "<<a_start<<": ";

        af.hi = a_start;
        if(n_start) set(SUB_FLAG);
        else clear(SUB_FLAG);
        if(c_start) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        if(h_start) set(HALF_CARRY_FLAG);
        else clear(HALF_CARRY_FLAG);

        op_daa<0>(0);

        if((carry()) != c_end) c_m = true;
        if((hc())    != h_end) h_m = true;
        if((sub())   != n_end) n_m = true;
        if((af.hi)   != a_end) a_m = true;

        if(c_m || h_m || n_m || a_m) {
            std::cout<<std::hex<<"Mismatch for a: "<<uint32_t(a_start)<<
            " expected: "<<n_end<<" "<<c_end<<" "<<h_end<<" "<<a_end<<
            " got: "<<sub()<<" "<<carry()<<" "<<hc()<<" "<<int(af.hi)<<"\n";
        }
        else {
            std::cout<<std::hex<<"Result matched for a: "<<uint32_t(a_start)<<std::endl;
        }
    }
    results.close();
    return 0;
}


void cpuZ80::reset() { // Jump to 0

}

void cpuZ80::nmi() { // Pause button, jump to 66h
    push(pc);
    pc = 0x0066;
    iff1 = false;
    //TODO: interrupt timing
}

void cpuZ80::interrupt(uint8_t vector) { // Maskable interrupts, no vector provided on data bus, so it jumps to 38h
    if(iff1) {
        push(pc);
        uint16_t temp_addr;
        switch(int_mode) {
        case mode0: decode(vector);
        case mode1: pc = 0x0038; break;
        case mode2:
            temp_addr = int_vect * 256 + vector;
            pc = memory->readWord(temp_addr);
            break;
        }
    }
}

cpuZ80::int_type_t cpuZ80::check_interrupts() {
    // TODO: implement real interrupt checking
    return int_type_t::irq_int;
}

const std::array<z80OpPtr, 256> cpuZ80::op_table = {
  &cpuZ80::op_nop<0x00>,     &cpuZ80::op_ld16<0x01>,      &cpuZ80::op_ld8rm<0x02>,  &cpuZ80::op_incr16<0x03>,
  &cpuZ80::op_incr8<0x04>,   &cpuZ80::op_decr8<0x05>,     &cpuZ80::op_ld8ri<0x06>,  &cpuZ80::op_rot_a<0x07>,
  &cpuZ80::op_exx<0x08>,     &cpuZ80::op_add16<0x09>,     &cpuZ80::op_ld8rm<0x0a>,  &cpuZ80::op_decr16<0x0b>,
  &cpuZ80::op_incr8<0x0c>,   &cpuZ80::op_decr8<0x0d>,     &cpuZ80::op_ld8ri<0x0e>,  &cpuZ80::op_rot_a<0x0f>,
  &cpuZ80::op_jr<0x10>,      &cpuZ80::op_ld16<0x11>,      &cpuZ80::op_ld8rm<0x12>,  &cpuZ80::op_incr16<0x13>,
  &cpuZ80::op_incr8<0x14>,   &cpuZ80::op_decr8<0x15>,     &cpuZ80::op_ld8ri<0x16>,  &cpuZ80::op_rot_a<0x17>,
  &cpuZ80::op_jr<0x18>,      &cpuZ80::op_add16<0x19>,     &cpuZ80::op_ld8rm<0x1a>,  &cpuZ80::op_decr16<0x1b>,
  &cpuZ80::op_incr8<0x1c>,   &cpuZ80::op_decr8<0x1d>,     &cpuZ80::op_ld8ri<0x1e>,  &cpuZ80::op_rot_a<0x1f>,
  &cpuZ80::op_jr<0x20>,      &cpuZ80::op_ld16<0x21>,      &cpuZ80::op_ld16rm<0x22>, &cpuZ80::op_incr16<0x23>,
  &cpuZ80::op_incr8<0x24>,   &cpuZ80::op_decr8<0x25>,     &cpuZ80::op_ld8ri<0x26>,  &cpuZ80::op_daa<0x27>,
  &cpuZ80::op_jr<0x28>,      &cpuZ80::op_add16<0x29>,     &cpuZ80::op_ld16rm<0x2a>, &cpuZ80::op_decr16<0x2b>,
  &cpuZ80::op_incr8<0x2c>,   &cpuZ80::op_decr8<0x2d>,     &cpuZ80::op_ld8ri<0x2e>,  &cpuZ80::op_cpl<0x2f>,
  &cpuZ80::op_jr<0x30>,      &cpuZ80::op_ld16<0x31>,      &cpuZ80::op_ld8rm<0x32>,  &cpuZ80::op_incr16<0x33>,
  &cpuZ80::op_incr8<0x34>,   &cpuZ80::op_decr8<0x35>,     &cpuZ80::op_ld8ri<0x36>,  &cpuZ80::op_scf<0x37>,
  &cpuZ80::op_jr<0x38>,      &cpuZ80::op_add16<0x39>,     &cpuZ80::op_ld8rm<0x3a>,  &cpuZ80::op_decr16<0x3b>,
  &cpuZ80::op_incr8<0x3c>,   &cpuZ80::op_decr8<0x3d>,     &cpuZ80::op_ld8ri<0x3e>,  &cpuZ80::op_ccf<0x3f>,
  &cpuZ80::op_ld8rr<0x40>,   &cpuZ80::op_ld8rr<0x41>,     &cpuZ80::op_ld8rr<0x42>,  &cpuZ80::op_ld8rr<0x43>,
  &cpuZ80::op_ld8rr<0x44>,   &cpuZ80::op_ld8rr<0x45>,     &cpuZ80::op_ld8rr<0x46>,  &cpuZ80::op_ld8rr<0x47>,
  &cpuZ80::op_ld8rr<0x48>,   &cpuZ80::op_ld8rr<0x49>,     &cpuZ80::op_ld8rr<0x4a>,  &cpuZ80::op_ld8rr<0x4b>,
  &cpuZ80::op_ld8rr<0x4c>,   &cpuZ80::op_ld8rr<0x4d>,     &cpuZ80::op_ld8rr<0x4e>,  &cpuZ80::op_ld8rr<0x4f>,
  &cpuZ80::op_ld8rr<0x50>,   &cpuZ80::op_ld8rr<0x51>,     &cpuZ80::op_ld8rr<0x52>,  &cpuZ80::op_ld8rr<0x53>,
  &cpuZ80::op_ld8rr<0x54>,   &cpuZ80::op_ld8rr<0x55>,     &cpuZ80::op_ld8rr<0x56>,  &cpuZ80::op_ld8rr<0x57>,
  &cpuZ80::op_ld8rr<0x58>,   &cpuZ80::op_ld8rr<0x59>,     &cpuZ80::op_ld8rr<0x5a>,  &cpuZ80::op_ld8rr<0x5b>,
  &cpuZ80::op_ld8rr<0x5c>,   &cpuZ80::op_ld8rr<0x5d>,     &cpuZ80::op_ld8rr<0x5e>,  &cpuZ80::op_ld8rr<0x5f>,
  &cpuZ80::op_ld8rr<0x60>,   &cpuZ80::op_ld8rr<0x61>,     &cpuZ80::op_ld8rr<0x62>,  &cpuZ80::op_ld8rr<0x63>,
  &cpuZ80::op_ld8rr<0x64>,   &cpuZ80::op_ld8rr<0x65>,     &cpuZ80::op_ld8rr<0x66>,  &cpuZ80::op_ld8rr<0x67>,
  &cpuZ80::op_ld8rr<0x68>,   &cpuZ80::op_ld8rr<0x69>,     &cpuZ80::op_ld8rr<0x6a>,  &cpuZ80::op_ld8rr<0x6b>,
  &cpuZ80::op_ld8rr<0x6c>,   &cpuZ80::op_ld8rr<0x6d>,     &cpuZ80::op_ld8rr<0x6e>,  &cpuZ80::op_ld8rr<0x6f>,
  &cpuZ80::op_ld8rr<0x70>,   &cpuZ80::op_ld8rr<0x71>,     &cpuZ80::op_ld8rr<0x72>,  &cpuZ80::op_ld8rr<0x73>,
  &cpuZ80::op_ld8rr<0x74>,   &cpuZ80::op_ld8rr<0x75>,     &cpuZ80::op_halt<0x76>,   &cpuZ80::op_ld8rr<0x77>,
  &cpuZ80::op_ld8rr<0x78>,   &cpuZ80::op_ld8rr<0x79>,     &cpuZ80::op_ld8rr<0x7a>,  &cpuZ80::op_ld8rr<0x7b>,
  &cpuZ80::op_ld8rr<0x7c>,   &cpuZ80::op_ld8rr<0x7d>,     &cpuZ80::op_ld8rr<0x7e>,  &cpuZ80::op_ld8rr<0x7f>,
  &cpuZ80::op_alu<0x80>,     &cpuZ80::op_alu<0x81>,       &cpuZ80::op_alu<0x82>,    &cpuZ80::op_alu<0x83>,
  &cpuZ80::op_alu<0x84>,     &cpuZ80::op_alu<0x85>,       &cpuZ80::op_alu<0x86>,    &cpuZ80::op_alu<0x87>,
  &cpuZ80::op_alu<0x88>,     &cpuZ80::op_alu<0x89>,       &cpuZ80::op_alu<0x8a>,    &cpuZ80::op_alu<0x8b>,
  &cpuZ80::op_alu<0x8c>,     &cpuZ80::op_alu<0x8d>,       &cpuZ80::op_alu<0x8e>,    &cpuZ80::op_alu<0x8f>,
  &cpuZ80::op_alu<0x90>,     &cpuZ80::op_alu<0x91>,       &cpuZ80::op_alu<0x92>,    &cpuZ80::op_alu<0x93>,
  &cpuZ80::op_alu<0x94>,     &cpuZ80::op_alu<0x95>,       &cpuZ80::op_alu<0x96>,    &cpuZ80::op_alu<0x97>,
  &cpuZ80::op_alu<0x98>,     &cpuZ80::op_alu<0x99>,       &cpuZ80::op_alu<0x9a>,    &cpuZ80::op_alu<0x9b>,
  &cpuZ80::op_alu<0x9c>,     &cpuZ80::op_alu<0x9d>,       &cpuZ80::op_alu<0x9e>,    &cpuZ80::op_alu<0x9f>,
  &cpuZ80::op_alu<0xa0>,     &cpuZ80::op_alu<0xa1>,       &cpuZ80::op_alu<0xa2>,    &cpuZ80::op_alu<0xa3>,
  &cpuZ80::op_alu<0xa4>,     &cpuZ80::op_alu<0xa5>,       &cpuZ80::op_alu<0xa6>,    &cpuZ80::op_alu<0xa7>,
  &cpuZ80::op_alu<0xa8>,     &cpuZ80::op_alu<0xa9>,       &cpuZ80::op_alu<0xaa>,    &cpuZ80::op_alu<0xab>,
  &cpuZ80::op_alu<0xac>,     &cpuZ80::op_alu<0xad>,       &cpuZ80::op_alu<0xae>,    &cpuZ80::op_alu<0xaf>,
  &cpuZ80::op_alu<0xb0>,     &cpuZ80::op_alu<0xb1>,       &cpuZ80::op_alu<0xb2>,    &cpuZ80::op_alu<0xb3>,
  &cpuZ80::op_alu<0xb4>,     &cpuZ80::op_alu<0xb5>,       &cpuZ80::op_alu<0xb6>,    &cpuZ80::op_alu<0xb7>,
  &cpuZ80::op_alu<0xb8>,     &cpuZ80::op_alu<0xb9>,       &cpuZ80::op_alu<0xba>,    &cpuZ80::op_alu<0xbb>,
  &cpuZ80::op_alu<0xbc>,     &cpuZ80::op_alu<0xbd>,       &cpuZ80::op_alu<0xbe>,    &cpuZ80::op_alu<0xbf>,
  &cpuZ80::op_ret<0xc0>,     &cpuZ80::op_pop<0xc1>,       &cpuZ80::op_jp<0xc2>,     &cpuZ80::op_jp<0xc3>,
  &cpuZ80::op_call_cc<0xc4>, &cpuZ80::op_push<0xc5>,      &cpuZ80::op_alu<0xc6>,    &cpuZ80::op_call<0xc7>,
  &cpuZ80::op_ret<0xc8>,     &cpuZ80::op_ret<0xc9>,       &cpuZ80::op_jp<0xca>,     &cpuZ80::cb_op_prefix<0xcb>,
  &cpuZ80::op_call_cc<0xcc>, &cpuZ80::op_call<0xcd>,      &cpuZ80::op_alu<0xce>,    &cpuZ80::op_call<0xcf>,
  &cpuZ80::op_ret<0xd0>,     &cpuZ80::op_pop<0xd1>,       &cpuZ80::op_jp<0xd2>,     &cpuZ80::op_out<0xd3>,
  &cpuZ80::op_call_cc<0xd4>, &cpuZ80::op_push<0xd5>,      &cpuZ80::op_alu<0xd6>,    &cpuZ80::op_call<0xd7>,
  &cpuZ80::op_ret<0xd8>,     &cpuZ80::op_exx<0xd9>,       &cpuZ80::op_jp<0xda>,     &cpuZ80::op_in<0xdb>,
  &cpuZ80::op_call_cc<0xdc>, &cpuZ80::dd_op_prefix<0xdd>, &cpuZ80::op_alu<0xde>,    &cpuZ80::op_call<0xdf>,
  &cpuZ80::op_ret<0xe0>,     &cpuZ80::op_pop<0xe1>,       &cpuZ80::op_jp<0xe2>,     &cpuZ80::op_ex16<0xe3>,
  &cpuZ80::op_call_cc<0xe4>, &cpuZ80::op_push<0xe5>,      &cpuZ80::op_alu<0xe6>,    &cpuZ80::op_call<0xe7>,
  &cpuZ80::op_ret<0xe8>,     &cpuZ80::op_jp<0xe9>,        &cpuZ80::op_jp<0xea>,     &cpuZ80::op_ex16<0xeb>,
  &cpuZ80::op_call_cc<0xec>, &cpuZ80::ed_op_prefix<0xed>, &cpuZ80::op_alu<0xee>,    &cpuZ80::op_call<0xef>,
  &cpuZ80::op_ret<0xf0>,     &cpuZ80::op_pop<0xf1>,       &cpuZ80::op_jp<0xf2>,     &cpuZ80::op_di<0xf3>,
  &cpuZ80::op_call_cc<0xf4>, &cpuZ80::op_push<0xf5>,      &cpuZ80::op_alu<0xf6>,    &cpuZ80::op_call<0xf7>,
  &cpuZ80::op_ret<0xf8>,     &cpuZ80::op_unimpl<0xf9>,    &cpuZ80::op_jp<0xfa>,     &cpuZ80::op_ei<0xfb>,
  &cpuZ80::op_call_cc<0xfc>, &cpuZ80::fd_op_prefix<0xfd>, &cpuZ80::op_alu<0xfe>,    &cpuZ80::op_call<0xff>
};

const std::array<z80OpPtr, 256> cpuZ80::cb_op_table = {
  &cpuZ80::op_cbrot<0xcb00>, &cpuZ80::op_cbrot<0xcb01>, &cpuZ80::op_cbrot<0xcb02>, &cpuZ80::op_cbrot<0xcb03>,
  &cpuZ80::op_cbrot<0xcb04>, &cpuZ80::op_cbrot<0xcb05>, &cpuZ80::op_cbrot<0xcb06>, &cpuZ80::op_cbrot<0xcb07>,
  &cpuZ80::op_cbrot<0xcb08>, &cpuZ80::op_cbrot<0xcb09>, &cpuZ80::op_cbrot<0xcb0a>, &cpuZ80::op_cbrot<0xcb0b>,
  &cpuZ80::op_cbrot<0xcb0c>, &cpuZ80::op_cbrot<0xcb0d>, &cpuZ80::op_cbrot<0xcb0e>, &cpuZ80::op_cbrot<0xcb0f>,
  &cpuZ80::op_cbrot<0xcb10>, &cpuZ80::op_cbrot<0xcb11>, &cpuZ80::op_cbrot<0xcb12>, &cpuZ80::op_cbrot<0xcb13>,
  &cpuZ80::op_cbrot<0xcb14>, &cpuZ80::op_cbrot<0xcb15>, &cpuZ80::op_cbrot<0xcb16>, &cpuZ80::op_cbrot<0xcb17>,
  &cpuZ80::op_cbrot<0xcb18>, &cpuZ80::op_cbrot<0xcb19>, &cpuZ80::op_cbrot<0xcb1a>, &cpuZ80::op_cbrot<0xcb1b>,
  &cpuZ80::op_cbrot<0xcb1c>, &cpuZ80::op_cbrot<0xcb1d>, &cpuZ80::op_cbrot<0xcb1e>, &cpuZ80::op_cbrot<0xcb1f>,
  &cpuZ80::op_cbrot<0xcb20>, &cpuZ80::op_cbrot<0xcb21>, &cpuZ80::op_cbrot<0xcb22>, &cpuZ80::op_cbrot<0xcb23>,
  &cpuZ80::op_cbrot<0xcb24>, &cpuZ80::op_cbrot<0xcb25>, &cpuZ80::op_cbrot<0xcb26>, &cpuZ80::op_cbrot<0xcb27>,
  &cpuZ80::op_cbrot<0xcb28>, &cpuZ80::op_cbrot<0xcb29>, &cpuZ80::op_cbrot<0xcb2a>, &cpuZ80::op_cbrot<0xcb2b>,
  &cpuZ80::op_cbrot<0xcb2c>, &cpuZ80::op_cbrot<0xcb2d>, &cpuZ80::op_cbrot<0xcb2e>, &cpuZ80::op_cbrot<0xcb2f>,
  &cpuZ80::op_cbrot<0xcb30>, &cpuZ80::op_cbrot<0xcb31>, &cpuZ80::op_cbrot<0xcb32>, &cpuZ80::op_cbrot<0xcb33>,
  &cpuZ80::op_cbrot<0xcb34>, &cpuZ80::op_cbrot<0xcb35>, &cpuZ80::op_cbrot<0xcb36>, &cpuZ80::op_cbrot<0xcb37>,
  &cpuZ80::op_cbrot<0xcb38>, &cpuZ80::op_cbrot<0xcb39>, &cpuZ80::op_cbrot<0xcb3a>, &cpuZ80::op_cbrot<0xcb3b>,
  &cpuZ80::op_cbrot<0xcb3c>, &cpuZ80::op_cbrot<0xcb3d>, &cpuZ80::op_cbrot<0xcb3e>, &cpuZ80::op_cbrot<0xcb3f>,
  &cpuZ80::op_cbbit<0xcb40>, &cpuZ80::op_cbbit<0xcb41>, &cpuZ80::op_cbbit<0xcb42>, &cpuZ80::op_cbbit<0xcb43>,
  &cpuZ80::op_cbbit<0xcb44>, &cpuZ80::op_cbbit<0xcb45>, &cpuZ80::op_cbbit<0xcb46>, &cpuZ80::op_cbbit<0xcb47>,
  &cpuZ80::op_cbbit<0xcb48>, &cpuZ80::op_cbbit<0xcb49>, &cpuZ80::op_cbbit<0xcb4a>, &cpuZ80::op_cbbit<0xcb4b>,
  &cpuZ80::op_cbbit<0xcb4c>, &cpuZ80::op_cbbit<0xcb4d>, &cpuZ80::op_cbbit<0xcb4e>, &cpuZ80::op_cbbit<0xcb4f>,
  &cpuZ80::op_cbbit<0xcb50>, &cpuZ80::op_cbbit<0xcb51>, &cpuZ80::op_cbbit<0xcb52>, &cpuZ80::op_cbbit<0xcb53>,
  &cpuZ80::op_cbbit<0xcb54>, &cpuZ80::op_cbbit<0xcb55>, &cpuZ80::op_cbbit<0xcb56>, &cpuZ80::op_cbbit<0xcb57>,
  &cpuZ80::op_cbbit<0xcb58>, &cpuZ80::op_cbbit<0xcb59>, &cpuZ80::op_cbbit<0xcb5a>, &cpuZ80::op_cbbit<0xcb5b>,
  &cpuZ80::op_cbbit<0xcb5c>, &cpuZ80::op_cbbit<0xcb5d>, &cpuZ80::op_cbbit<0xcb5e>, &cpuZ80::op_cbbit<0xcb5f>,
  &cpuZ80::op_cbbit<0xcb60>, &cpuZ80::op_cbbit<0xcb61>, &cpuZ80::op_cbbit<0xcb62>, &cpuZ80::op_cbbit<0xcb63>,
  &cpuZ80::op_cbbit<0xcb64>, &cpuZ80::op_cbbit<0xcb65>, &cpuZ80::op_cbbit<0xcb66>, &cpuZ80::op_cbbit<0xcb67>,
  &cpuZ80::op_cbbit<0xcb68>, &cpuZ80::op_cbbit<0xcb69>, &cpuZ80::op_cbbit<0xcb6a>, &cpuZ80::op_cbbit<0xcb6b>,
  &cpuZ80::op_cbbit<0xcb6c>, &cpuZ80::op_cbbit<0xcb6d>, &cpuZ80::op_cbbit<0xcb6e>, &cpuZ80::op_cbbit<0xcb6f>,
  &cpuZ80::op_cbbit<0xcb70>, &cpuZ80::op_cbbit<0xcb71>, &cpuZ80::op_cbbit<0xcb72>, &cpuZ80::op_cbbit<0xcb73>,
  &cpuZ80::op_cbbit<0xcb74>, &cpuZ80::op_cbbit<0xcb75>, &cpuZ80::op_cbbit<0xcb76>, &cpuZ80::op_cbbit<0xcb77>,
  &cpuZ80::op_cbbit<0xcb78>, &cpuZ80::op_cbbit<0xcb79>, &cpuZ80::op_cbbit<0xcb7a>, &cpuZ80::op_cbbit<0xcb7b>,
  &cpuZ80::op_cbbit<0xcb7c>, &cpuZ80::op_cbbit<0xcb7d>, &cpuZ80::op_cbbit<0xcb7e>, &cpuZ80::op_cbbit<0xcb7f>,
  &cpuZ80::op_cbres<0xcb80>, &cpuZ80::op_cbres<0xcb81>, &cpuZ80::op_cbres<0xcb82>, &cpuZ80::op_cbres<0xcb83>,
  &cpuZ80::op_cbres<0xcb84>, &cpuZ80::op_cbres<0xcb85>, &cpuZ80::op_cbres<0xcb86>, &cpuZ80::op_cbres<0xcb87>,
  &cpuZ80::op_cbres<0xcb88>, &cpuZ80::op_cbres<0xcb89>, &cpuZ80::op_cbres<0xcb8a>, &cpuZ80::op_cbres<0xcb8b>,
  &cpuZ80::op_cbres<0xcb8c>, &cpuZ80::op_cbres<0xcb8d>, &cpuZ80::op_cbres<0xcb8e>, &cpuZ80::op_cbres<0xcb8f>,
  &cpuZ80::op_cbres<0xcb90>, &cpuZ80::op_cbres<0xcb91>, &cpuZ80::op_cbres<0xcb92>, &cpuZ80::op_cbres<0xcb93>,
  &cpuZ80::op_cbres<0xcb94>, &cpuZ80::op_cbres<0xcb95>, &cpuZ80::op_cbres<0xcb96>, &cpuZ80::op_cbres<0xcb97>,
  &cpuZ80::op_cbres<0xcb98>, &cpuZ80::op_cbres<0xcb99>, &cpuZ80::op_cbres<0xcb9a>, &cpuZ80::op_cbres<0xcb9b>,
  &cpuZ80::op_cbres<0xcb9c>, &cpuZ80::op_cbres<0xcb9d>, &cpuZ80::op_cbres<0xcb9e>, &cpuZ80::op_cbres<0xcb9f>,
  &cpuZ80::op_cbres<0xcba0>, &cpuZ80::op_cbres<0xcba1>, &cpuZ80::op_cbres<0xcba2>, &cpuZ80::op_cbres<0xcba3>,
  &cpuZ80::op_cbres<0xcba4>, &cpuZ80::op_cbres<0xcba5>, &cpuZ80::op_cbres<0xcba6>, &cpuZ80::op_cbres<0xcba7>,
  &cpuZ80::op_cbres<0xcba8>, &cpuZ80::op_cbres<0xcba9>, &cpuZ80::op_cbres<0xcbaa>, &cpuZ80::op_cbres<0xcbab>,
  &cpuZ80::op_cbres<0xcbac>, &cpuZ80::op_cbres<0xcbad>, &cpuZ80::op_cbres<0xcbae>, &cpuZ80::op_cbres<0xcbaf>,
  &cpuZ80::op_cbres<0xcbb0>, &cpuZ80::op_cbres<0xcbb1>, &cpuZ80::op_cbres<0xcbb2>, &cpuZ80::op_cbres<0xcbb3>,
  &cpuZ80::op_cbres<0xcbb4>, &cpuZ80::op_cbres<0xcbb5>, &cpuZ80::op_cbres<0xcbb6>, &cpuZ80::op_cbres<0xcbb7>,
  &cpuZ80::op_cbres<0xcbb8>, &cpuZ80::op_cbres<0xcbb9>, &cpuZ80::op_cbres<0xcbba>, &cpuZ80::op_cbres<0xcbbb>,
  &cpuZ80::op_cbres<0xcbbc>, &cpuZ80::op_cbres<0xcbbd>, &cpuZ80::op_cbres<0xcbbe>, &cpuZ80::op_cbres<0xcbbf>,
  &cpuZ80::op_cbset<0xcbc0>, &cpuZ80::op_cbset<0xcbc1>, &cpuZ80::op_cbset<0xcbc2>, &cpuZ80::op_cbset<0xcbc3>,
  &cpuZ80::op_cbset<0xcbc4>, &cpuZ80::op_cbset<0xcbc5>, &cpuZ80::op_cbset<0xcbc6>, &cpuZ80::op_cbset<0xcbc7>,
  &cpuZ80::op_cbset<0xcbc8>, &cpuZ80::op_cbset<0xcbc9>, &cpuZ80::op_cbset<0xcbca>, &cpuZ80::op_cbset<0xcbcb>,
  &cpuZ80::op_cbset<0xcbcc>, &cpuZ80::op_cbset<0xcbcd>, &cpuZ80::op_cbset<0xcbce>, &cpuZ80::op_cbset<0xcbcf>,
  &cpuZ80::op_cbset<0xcbd0>, &cpuZ80::op_cbset<0xcbd1>, &cpuZ80::op_cbset<0xcbd2>, &cpuZ80::op_cbset<0xcbd3>,
  &cpuZ80::op_cbset<0xcbd4>, &cpuZ80::op_cbset<0xcbd5>, &cpuZ80::op_cbset<0xcbd6>, &cpuZ80::op_cbset<0xcbd7>,
  &cpuZ80::op_cbset<0xcbd8>, &cpuZ80::op_cbset<0xcbd9>, &cpuZ80::op_cbset<0xcbda>, &cpuZ80::op_cbset<0xcbdb>,
  &cpuZ80::op_cbset<0xcbdc>, &cpuZ80::op_cbset<0xcbdd>, &cpuZ80::op_cbset<0xcbde>, &cpuZ80::op_cbset<0xcbdf>,
  &cpuZ80::op_cbset<0xcbe0>, &cpuZ80::op_cbset<0xcbe1>, &cpuZ80::op_cbset<0xcbe2>, &cpuZ80::op_cbset<0xcbe3>,
  &cpuZ80::op_cbset<0xcbe4>, &cpuZ80::op_cbset<0xcbe5>, &cpuZ80::op_cbset<0xcbe6>, &cpuZ80::op_cbset<0xcbe7>,
  &cpuZ80::op_cbset<0xcbe8>, &cpuZ80::op_cbset<0xcbe9>, &cpuZ80::op_cbset<0xcbea>, &cpuZ80::op_cbset<0xcbeb>,
  &cpuZ80::op_cbset<0xcbec>, &cpuZ80::op_cbset<0xcbed>, &cpuZ80::op_cbset<0xcbee>, &cpuZ80::op_cbset<0xcbef>,
  &cpuZ80::op_cbset<0xcbf0>, &cpuZ80::op_cbset<0xcbf1>, &cpuZ80::op_cbset<0xcbf2>, &cpuZ80::op_cbset<0xcbf3>,
  &cpuZ80::op_cbset<0xcbf4>, &cpuZ80::op_cbset<0xcbf5>, &cpuZ80::op_cbset<0xcbf6>, &cpuZ80::op_cbset<0xcbf7>,
  &cpuZ80::op_cbset<0xcbf8>, &cpuZ80::op_cbset<0xcbf9>, &cpuZ80::op_cbset<0xcbfa>, &cpuZ80::op_cbset<0xcbfb>,
  &cpuZ80::op_cbset<0xcbfc>, &cpuZ80::op_cbset<0xcbfd>, &cpuZ80::op_cbset<0xcbfe>, &cpuZ80::op_cbset<0xcbff>
};

const std::array<z80OpPtr, 256> cpuZ80::dd_op_table = {
  &cpuZ80::op_unimpl<0xdd00>, &cpuZ80::op_unimpl<0xdd01>, &cpuZ80::op_unimpl<0xdd02>, &cpuZ80::op_unimpl<0xdd03>,
  &cpuZ80::op_unimpl<0xdd04>, &cpuZ80::op_unimpl<0xdd05>, &cpuZ80::op_unimpl<0xdd06>, &cpuZ80::op_unimpl<0xdd07>,
  &cpuZ80::op_unimpl<0xdd08>, &cpuZ80::op_add16<0xdd09>,  &cpuZ80::op_unimpl<0xdd0a>, &cpuZ80::op_unimpl<0xdd0b>,
  &cpuZ80::op_unimpl<0xdd0c>, &cpuZ80::op_unimpl<0xdd0d>, &cpuZ80::op_unimpl<0xdd0e>, &cpuZ80::op_unimpl<0xdd0f>,
  &cpuZ80::op_unimpl<0xdd10>, &cpuZ80::op_unimpl<0xdd11>, &cpuZ80::op_unimpl<0xdd12>, &cpuZ80::op_unimpl<0xdd13>,
  &cpuZ80::op_unimpl<0xdd14>, &cpuZ80::op_unimpl<0xdd15>, &cpuZ80::op_unimpl<0xdd16>, &cpuZ80::op_unimpl<0xdd17>,
  &cpuZ80::op_unimpl<0xdd18>, &cpuZ80::op_add16<0xdd19>,  &cpuZ80::op_unimpl<0xdd1a>, &cpuZ80::op_unimpl<0xdd1b>,
  &cpuZ80::op_unimpl<0xdd1c>, &cpuZ80::op_unimpl<0xdd1d>, &cpuZ80::op_unimpl<0xdd1e>, &cpuZ80::op_unimpl<0xdd1f>,
  &cpuZ80::op_unimpl<0xdd20>, &cpuZ80::op_ld16<0xdd21>,   &cpuZ80::op_ld16rim<0xdd22>, &cpuZ80::op_incr16<0xdd23>,
  &cpuZ80::op_incr8<0xdd24>, &cpuZ80::op_decr8<0xdd25>,   &cpuZ80::op_ld8idxri<0xdd26>, &cpuZ80::op_unimpl<0xdd27>,
  &cpuZ80::op_unimpl<0xdd28>, &cpuZ80::op_add16<0xdd29>,  &cpuZ80::op_ld16rim<0xdd2a>, &cpuZ80::op_decr16<0xdd2b>,
  &cpuZ80::op_incr8<0xdd2c>, &cpuZ80::op_decr8<0xdd2d>,   &cpuZ80::op_ld8idxri<0xdd2e>, &cpuZ80::op_unimpl<0xdd2f>,
  &cpuZ80::op_unimpl<0xdd30>, &cpuZ80::op_unimpl<0xdd31>, &cpuZ80::op_unimpl<0xdd32>, &cpuZ80::op_unimpl<0xdd33>,
  &cpuZ80::op_incr8<0xdd34>, &cpuZ80::op_decr8<0xdd35>,   &cpuZ80::op_ld8mioff<0xdd36>, &cpuZ80::op_unimpl<0xdd37>,
  &cpuZ80::op_unimpl<0xdd38>, &cpuZ80::op_add16<0xdd39>,  &cpuZ80::op_unimpl<0xdd3a>, &cpuZ80::op_unimpl<0xdd3b>,
  &cpuZ80::op_unimpl<0xdd3c>, &cpuZ80::op_unimpl<0xdd3d>, &cpuZ80::op_unimpl<0xdd3e>, &cpuZ80::op_unimpl<0xdd3f>,
  &cpuZ80::op_ld8rrix<0xdd40>, &cpuZ80::op_ld8rrix<0xdd41>, &cpuZ80::op_ld8rrix<0xdd42>, &cpuZ80::op_ld8rrix<0xdd43>,
  &cpuZ80::op_ld8rrix<0xdd44>, &cpuZ80::op_ld8rrix<0xdd45>, &cpuZ80::op_ld8rr<0xdd46>,   &cpuZ80::op_ld8rrix<0xdd47>,
  &cpuZ80::op_ld8rrix<0xdd48>, &cpuZ80::op_ld8rrix<0xdd49>, &cpuZ80::op_ld8rrix<0xdd4a>, &cpuZ80::op_ld8rrix<0xdd4b>,
  &cpuZ80::op_ld8rrix<0xdd4c>, &cpuZ80::op_ld8rrix<0xdd4d>, &cpuZ80::op_ld8rr<0xdd4e>,   &cpuZ80::op_ld8rrix<0xdd4f>,
  &cpuZ80::op_ld8rrix<0xdd50>, &cpuZ80::op_ld8rrix<0xdd51>, &cpuZ80::op_ld8rrix<0xdd52>, &cpuZ80::op_ld8rrix<0xdd53>,
  &cpuZ80::op_ld8rrix<0xdd54>, &cpuZ80::op_ld8rrix<0xdd55>, &cpuZ80::op_ld8rr<0xdd56>,   &cpuZ80::op_ld8rrix<0xdd57>,
  &cpuZ80::op_ld8rrix<0xdd58>, &cpuZ80::op_ld8rrix<0xdd59>, &cpuZ80::op_ld8rrix<0xdd5a>, &cpuZ80::op_ld8rrix<0xdd5b>,
  &cpuZ80::op_ld8rrix<0xdd5c>, &cpuZ80::op_ld8rrix<0xdd5d>, &cpuZ80::op_ld8rr<0xdd5e>,   &cpuZ80::op_ld8rrix<0xdd5f>,
  &cpuZ80::op_ld8rrix<0xdd60>, &cpuZ80::op_ld8rrix<0xdd61>, &cpuZ80::op_ld8rrix<0xdd62>, &cpuZ80::op_ld8rrix<0xdd63>,
  &cpuZ80::op_ld8rrix<0xdd64>, &cpuZ80::op_ld8rrix<0xdd65>, &cpuZ80::op_ld8rr<0xdd66>,   &cpuZ80::op_ld8rrix<0xdd67>,
  &cpuZ80::op_ld8rrix<0xdd68>, &cpuZ80::op_ld8rrix<0xdd69>, &cpuZ80::op_ld8rrix<0xdd6a>, &cpuZ80::op_ld8rrix<0xdd6b>,
  &cpuZ80::op_ld8rrix<0xdd6c>, &cpuZ80::op_ld8rrix<0xdd6d>, &cpuZ80::op_ld8rr<0xdd6e>,   &cpuZ80::op_ld8rrix<0xdd6f>,
  &cpuZ80::op_ld8rr<0xdd70>,   &cpuZ80::op_ld8rr<0xdd71>,   &cpuZ80::op_ld8rr<0xdd72>,   &cpuZ80::op_ld8rr<0xdd73>,
  &cpuZ80::op_ld8rr<0xdd74>,   &cpuZ80::op_ld8rr<0xdd75>,   &cpuZ80::op_halt<0xdd76>,    &cpuZ80::op_ld8rr<0xdd77>,
  &cpuZ80::op_ld8rrix<0xdd78>, &cpuZ80::op_ld8rrix<0xdd79>, &cpuZ80::op_ld8rrix<0xdd7a>, &cpuZ80::op_ld8rrix<0xdd7b>,
  &cpuZ80::op_ld8rrix<0xdd7c>, &cpuZ80::op_ld8rrix<0xdd7d>, &cpuZ80::op_ld8rr<0xdd7e>,   &cpuZ80::op_ld8rrix<0xdd7f>,
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

const std::array<z80OpPtr, 256> cpuZ80::ed_op_table = {
  &cpuZ80::op_in<0xed40>,     &cpuZ80::op_out<0xed41>,    &cpuZ80::op_sbc16<0xed42>,  &cpuZ80::op_ld16rim<0xed43>,
  &cpuZ80::op_neg<0xed44>,    &cpuZ80::op_retn<0xed45>,   &cpuZ80::op_im<0xed46>,     &cpuZ80::op_unimpl<0xed47>,
  &cpuZ80::op_in<0xed48>,     &cpuZ80::op_out<0xed49>,    &cpuZ80::op_adc16<0xed4a>,  &cpuZ80::op_ld16rim<0xed4b>,
  &cpuZ80::op_unimpl<0xed4c>, &cpuZ80::op_reti<0xed4d>,   &cpuZ80::op_unimpl<0xed4e>, &cpuZ80::op_unimpl<0xed4f>,
  &cpuZ80::op_in<0xed50>,     &cpuZ80::op_out<0xed51>,    &cpuZ80::op_sbc16<0xed52>,  &cpuZ80::op_ld16rim<0xed53>,
  &cpuZ80::op_unimpl<0xed54>, &cpuZ80::op_retn<0xed55>,   &cpuZ80::op_im<0xed56>,     &cpuZ80::op_unimpl<0xed57>,
  &cpuZ80::op_in<0xed58>,     &cpuZ80::op_out<0xed59>,    &cpuZ80::op_adc16<0xed5a>,  &cpuZ80::op_ld16rim<0xed5b>,
  &cpuZ80::op_unimpl<0xed5c>, &cpuZ80::op_retn<0xed5d>,   &cpuZ80::op_im<0xed5e>,     &cpuZ80::op_unimpl<0xed5f>,
  &cpuZ80::op_in<0xed60>,     &cpuZ80::op_out<0xed61>,    &cpuZ80::op_sbc16<0xed62>,  &cpuZ80::op_ld16rim<0xed63>,
  &cpuZ80::op_unimpl<0xed64>, &cpuZ80::op_retn<0xed65>,   &cpuZ80::op_im<0xed66>,     &cpuZ80::op_rxd<0xed67>,
  &cpuZ80::op_in<0xed68>,     &cpuZ80::op_out<0xed69>,    &cpuZ80::op_adc16<0xed6a>,  &cpuZ80::op_ld16rim<0xed6b>,
  &cpuZ80::op_unimpl<0xed6c>, &cpuZ80::op_retn<0xed6d>,   &cpuZ80::op_unimpl<0xed6e>, &cpuZ80::op_rxd<0xed6f>,
  &cpuZ80::op_in<0xed70>,     &cpuZ80::op_out<0xed71>,    &cpuZ80::op_sbc16<0xed72>,  &cpuZ80::op_ld16rim<0xed73>,
  &cpuZ80::op_unimpl<0xed74>, &cpuZ80::op_retn<0xed75>,   &cpuZ80::op_im<0xed76>,     &cpuZ80::op_unimpl<0xed77>,
  &cpuZ80::op_in<0xed78>,     &cpuZ80::op_out<0xed79>,    &cpuZ80::op_adc16<0xed7a>,  &cpuZ80::op_ld16rim<0xed7b>,
  &cpuZ80::op_unimpl<0xed7c>, &cpuZ80::op_retn<0xed7d>,   &cpuZ80::op_im<0xed7e>,     &cpuZ80::op_unimpl<0xed7f>,
  &cpuZ80::op_unimpl<0xed80>, &cpuZ80::op_unimpl<0xed81>, &cpuZ80::op_unimpl<0xed82>, &cpuZ80::op_unimpl<0xed83>,
  &cpuZ80::op_unimpl<0xed84>, &cpuZ80::op_unimpl<0xed85>, &cpuZ80::op_unimpl<0xed86>, &cpuZ80::op_unimpl<0xed87>,
  &cpuZ80::op_unimpl<0xed88>, &cpuZ80::op_unimpl<0xed89>, &cpuZ80::op_unimpl<0xed8a>, &cpuZ80::op_unimpl<0xed8b>,
  &cpuZ80::op_unimpl<0xed8c>, &cpuZ80::op_unimpl<0xed8d>, &cpuZ80::op_unimpl<0xed8e>, &cpuZ80::op_unimpl<0xed8f>,
  &cpuZ80::op_unimpl<0xed90>, &cpuZ80::op_unimpl<0xed91>, &cpuZ80::op_unimpl<0xed92>, &cpuZ80::op_unimpl<0xed93>,
  &cpuZ80::op_unimpl<0xed94>, &cpuZ80::op_unimpl<0xed95>, &cpuZ80::op_unimpl<0xed96>, &cpuZ80::op_unimpl<0xed97>,
  &cpuZ80::op_unimpl<0xed98>, &cpuZ80::op_unimpl<0xed99>, &cpuZ80::op_unimpl<0xed9a>, &cpuZ80::op_unimpl<0xed9b>,
  &cpuZ80::op_unimpl<0xed9c>, &cpuZ80::op_unimpl<0xed9d>, &cpuZ80::op_unimpl<0xed9e>, &cpuZ80::op_unimpl<0xed9f>,
  &cpuZ80::op_ld8mm<0xeda0>,  &cpuZ80::op_cp<0xeda1>,     &cpuZ80::op_in<0xeda2>,     &cpuZ80::op_out<0xeda3>,
  &cpuZ80::op_unimpl<0xeda4>, &cpuZ80::op_unimpl<0xeda5>, &cpuZ80::op_unimpl<0xeda6>, &cpuZ80::op_unimpl<0xeda7>,
  &cpuZ80::op_ld8mm<0xeda8>,  &cpuZ80::op_cp<0xeda9>,     &cpuZ80::op_in<0xedaa>,     &cpuZ80::op_out<0xedab>,
  &cpuZ80::op_unimpl<0xedac>, &cpuZ80::op_unimpl<0xedad>, &cpuZ80::op_unimpl<0xedae>, &cpuZ80::op_unimpl<0xedaf>,
  &cpuZ80::op_ld8mm<0xedb0>,  &cpuZ80::op_cp<0xedb1>,     &cpuZ80::op_in<0xedb2>,     &cpuZ80::op_out<0xedb3>,
  &cpuZ80::op_unimpl<0xedb4>, &cpuZ80::op_unimpl<0xedb5>, &cpuZ80::op_unimpl<0xedb6>, &cpuZ80::op_unimpl<0xedb7>,
  &cpuZ80::op_ld8mm<0xedb8>,  &cpuZ80::op_cp<0xedb9>,     &cpuZ80::op_in<0xedba>,     &cpuZ80::op_out<0xedbb>
};

const std::array<z80OpPtr, 256> cpuZ80::fd_op_table = {
  &cpuZ80::op_unimpl<0xfd00>, &cpuZ80::op_unimpl<0xfd01>, &cpuZ80::op_unimpl<0xfd02>, &cpuZ80::op_unimpl<0xfd03>,
  &cpuZ80::op_unimpl<0xfd04>, &cpuZ80::op_unimpl<0xfd05>, &cpuZ80::op_unimpl<0xfd06>, &cpuZ80::op_unimpl<0xfd07>,
  &cpuZ80::op_unimpl<0xfd08>, &cpuZ80::op_add16<0xfd09>,  &cpuZ80::op_unimpl<0xfd0a>, &cpuZ80::op_unimpl<0xfd0b>,
  &cpuZ80::op_unimpl<0xfd0c>, &cpuZ80::op_unimpl<0xfd0d>, &cpuZ80::op_unimpl<0xfd0e>, &cpuZ80::op_unimpl<0xfd0f>,
  &cpuZ80::op_unimpl<0xfd10>, &cpuZ80::op_unimpl<0xfd11>, &cpuZ80::op_unimpl<0xfd12>, &cpuZ80::op_unimpl<0xfd13>,
  &cpuZ80::op_unimpl<0xfd14>, &cpuZ80::op_unimpl<0xfd15>, &cpuZ80::op_unimpl<0xfd16>, &cpuZ80::op_unimpl<0xfd17>,
  &cpuZ80::op_unimpl<0xfd18>, &cpuZ80::op_add16<0xfd19>,  &cpuZ80::op_unimpl<0xfd1a>, &cpuZ80::op_unimpl<0xfd1b>,
  &cpuZ80::op_unimpl<0xfd1c>, &cpuZ80::op_unimpl<0xfd1d>, &cpuZ80::op_unimpl<0xfd1e>, &cpuZ80::op_unimpl<0xfd1f>,
  &cpuZ80::op_unimpl<0xfd20>, &cpuZ80::op_ld16<0xfd21>,   &cpuZ80::op_ld16rim<0xfd22>, &cpuZ80::op_incr16<0xfd23>,
  &cpuZ80::op_incr8<0xfd24>, &cpuZ80::op_decr8<0xfd25>,   &cpuZ80::op_ld8idxri<0xfd26>, &cpuZ80::op_unimpl<0xfd27>,
  &cpuZ80::op_unimpl<0xfd28>, &cpuZ80::op_add16<0xfd29>,  &cpuZ80::op_ld16rim<0xfd2a>, &cpuZ80::op_decr16<0xfd2b>,
  &cpuZ80::op_incr8<0xfd2c>, &cpuZ80::op_decr8<0xfd2d>,   &cpuZ80::op_ld8idxri<0xfd2e>, &cpuZ80::op_unimpl<0xfd2f>,
  &cpuZ80::op_unimpl<0xfd30>, &cpuZ80::op_unimpl<0xfd31>, &cpuZ80::op_unimpl<0xfd32>, &cpuZ80::op_unimpl<0xfd33>,
  &cpuZ80::op_incr8<0xfd34>, &cpuZ80::op_decr8<0xfd35>,   &cpuZ80::op_ld8mioff<0xfd36>, &cpuZ80::op_unimpl<0xfd37>,
  &cpuZ80::op_unimpl<0xfd38>, &cpuZ80::op_add16<0xfd39>,  &cpuZ80::op_unimpl<0xfd3a>, &cpuZ80::op_unimpl<0xfd3b>,
  &cpuZ80::op_unimpl<0xfd3c>, &cpuZ80::op_unimpl<0xfd3d>, &cpuZ80::op_unimpl<0xfd3e>, &cpuZ80::op_unimpl<0xfd3f>,
  &cpuZ80::op_ld8rriy<0xfd40>, &cpuZ80::op_ld8rriy<0xfd41>, &cpuZ80::op_ld8rriy<0xfd42>, &cpuZ80::op_ld8rriy<0xfd43>,
  &cpuZ80::op_ld8rriy<0xfd44>, &cpuZ80::op_ld8rriy<0xfd45>, &cpuZ80::op_ld8rr<0xfd46>,   &cpuZ80::op_ld8rriy<0xfd47>,
  &cpuZ80::op_ld8rriy<0xfd48>, &cpuZ80::op_ld8rriy<0xfd49>, &cpuZ80::op_ld8rriy<0xfd4a>, &cpuZ80::op_ld8rriy<0xfd4b>,
  &cpuZ80::op_ld8rriy<0xfd4c>, &cpuZ80::op_ld8rriy<0xfd4d>, &cpuZ80::op_ld8rr<0xfd4e>,   &cpuZ80::op_ld8rriy<0xfd4f>,
  &cpuZ80::op_ld8rriy<0xfd50>, &cpuZ80::op_ld8rriy<0xfd51>, &cpuZ80::op_ld8rriy<0xfd52>, &cpuZ80::op_ld8rriy<0xfd53>,
  &cpuZ80::op_ld8rriy<0xfd54>, &cpuZ80::op_ld8rriy<0xfd55>, &cpuZ80::op_ld8rr<0xfd56>,   &cpuZ80::op_ld8rriy<0xfd57>,
  &cpuZ80::op_ld8rriy<0xfd58>, &cpuZ80::op_ld8rriy<0xfd59>, &cpuZ80::op_ld8rriy<0xfd5a>, &cpuZ80::op_ld8rriy<0xfd5b>,
  &cpuZ80::op_ld8rriy<0xfd5c>, &cpuZ80::op_ld8rriy<0xfd5d>, &cpuZ80::op_ld8rr<0xfd5e>,   &cpuZ80::op_ld8rriy<0xfd5f>,
  &cpuZ80::op_ld8rriy<0xfd60>, &cpuZ80::op_ld8rriy<0xfd61>, &cpuZ80::op_ld8rriy<0xfd62>, &cpuZ80::op_ld8rriy<0xfd63>,
  &cpuZ80::op_ld8rriy<0xfd64>, &cpuZ80::op_ld8rriy<0xfd65>, &cpuZ80::op_ld8rr<0xfd66>,   &cpuZ80::op_ld8rriy<0xfd67>,
  &cpuZ80::op_ld8rriy<0xfd68>, &cpuZ80::op_ld8rriy<0xfd69>, &cpuZ80::op_ld8rriy<0xfd6a>, &cpuZ80::op_ld8rriy<0xfd6b>,
  &cpuZ80::op_ld8rriy<0xfd6c>, &cpuZ80::op_ld8rriy<0xfd6d>, &cpuZ80::op_ld8rr<0xfd6e>,   &cpuZ80::op_ld8rriy<0xfd6f>,
  &cpuZ80::op_ld8rr<0xfd70>,   &cpuZ80::op_ld8rr<0xfd71>,   &cpuZ80::op_ld8rr<0xfd72>,   &cpuZ80::op_ld8rr<0xfd73>,
  &cpuZ80::op_ld8rr<0xfd74>,   &cpuZ80::op_ld8rr<0xfd75>,   &cpuZ80::op_halt<0xfd76>,    &cpuZ80::op_ld8rr<0xfd77>,
  &cpuZ80::op_ld8rriy<0xfd78>, &cpuZ80::op_ld8rriy<0xfd79>, &cpuZ80::op_ld8rriy<0xfd7a>, &cpuZ80::op_ld8rriy<0xfd7b>,
  &cpuZ80::op_ld8rriy<0xfd7c>, &cpuZ80::op_ld8rriy<0xfd7d>, &cpuZ80::op_ld8rr<0xfd7e>,   &cpuZ80::op_ld8rriy<0xfd7f>,
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

const std::array<z80OpPtr, 256> cpuZ80::ddcb_op_table = {
  &cpuZ80::op_cbrot<0xddcb00>, &cpuZ80::op_cbrot<0xddcb01>, &cpuZ80::op_cbrot<0xddcb02>, &cpuZ80::op_cbrot<0xddcb03>,
  &cpuZ80::op_cbrot<0xddcb04>, &cpuZ80::op_cbrot<0xddcb05>, &cpuZ80::op_cbrot<0xddcb06>, &cpuZ80::op_cbrot<0xddcb07>,
  &cpuZ80::op_cbrot<0xddcb08>, &cpuZ80::op_cbrot<0xddcb09>, &cpuZ80::op_cbrot<0xddcb0a>, &cpuZ80::op_cbrot<0xddcb0b>,
  &cpuZ80::op_cbrot<0xddcb0c>, &cpuZ80::op_cbrot<0xddcb0d>, &cpuZ80::op_cbrot<0xddcb0e>, &cpuZ80::op_cbrot<0xddcb0f>,
  &cpuZ80::op_cbrot<0xddcb10>, &cpuZ80::op_cbrot<0xddcb11>, &cpuZ80::op_cbrot<0xddcb12>, &cpuZ80::op_cbrot<0xddcb13>,
  &cpuZ80::op_cbrot<0xddcb14>, &cpuZ80::op_cbrot<0xddcb15>, &cpuZ80::op_cbrot<0xddcb16>, &cpuZ80::op_cbrot<0xddcb17>,
  &cpuZ80::op_cbrot<0xddcb18>, &cpuZ80::op_cbrot<0xddcb19>, &cpuZ80::op_cbrot<0xddcb1a>, &cpuZ80::op_cbrot<0xddcb1b>,
  &cpuZ80::op_cbrot<0xddcb1c>, &cpuZ80::op_cbrot<0xddcb1d>, &cpuZ80::op_cbrot<0xddcb1e>, &cpuZ80::op_cbrot<0xddcb1f>,
  &cpuZ80::op_cbrot<0xddcb20>, &cpuZ80::op_cbrot<0xddcb21>, &cpuZ80::op_cbrot<0xddcb22>, &cpuZ80::op_cbrot<0xddcb23>,
  &cpuZ80::op_cbrot<0xddcb24>, &cpuZ80::op_cbrot<0xddcb25>, &cpuZ80::op_cbrot<0xddcb26>, &cpuZ80::op_cbrot<0xddcb27>,
  &cpuZ80::op_cbrot<0xddcb28>, &cpuZ80::op_cbrot<0xddcb29>, &cpuZ80::op_cbrot<0xddcb2a>, &cpuZ80::op_cbrot<0xddcb2b>,
  &cpuZ80::op_cbrot<0xddcb2c>, &cpuZ80::op_cbrot<0xddcb2d>, &cpuZ80::op_cbrot<0xddcb2e>, &cpuZ80::op_cbrot<0xddcb2f>,
  &cpuZ80::op_cbrot<0xddcb30>, &cpuZ80::op_cbrot<0xddcb31>, &cpuZ80::op_cbrot<0xddcb32>, &cpuZ80::op_cbrot<0xddcb33>,
  &cpuZ80::op_cbrot<0xddcb34>, &cpuZ80::op_cbrot<0xddcb35>, &cpuZ80::op_cbrot<0xddcb36>, &cpuZ80::op_cbrot<0xddcb37>,
  &cpuZ80::op_cbrot<0xddcb38>, &cpuZ80::op_cbrot<0xddcb39>, &cpuZ80::op_cbrot<0xddcb3a>, &cpuZ80::op_cbrot<0xddcb3b>,
  &cpuZ80::op_cbrot<0xddcb3c>, &cpuZ80::op_cbrot<0xddcb3d>, &cpuZ80::op_cbrot<0xddcb3e>, &cpuZ80::op_cbrot<0xddcb3f>,
  &cpuZ80::op_cbbit<0xddcb40>, &cpuZ80::op_cbbit<0xddcb41>, &cpuZ80::op_cbbit<0xddcb42>, &cpuZ80::op_cbbit<0xddcb43>,
  &cpuZ80::op_cbbit<0xddcb44>, &cpuZ80::op_cbbit<0xddcb45>, &cpuZ80::op_cbbit<0xddcb46>, &cpuZ80::op_cbbit<0xddcb47>,
  &cpuZ80::op_cbbit<0xddcb48>, &cpuZ80::op_cbbit<0xddcb49>, &cpuZ80::op_cbbit<0xddcb4a>, &cpuZ80::op_cbbit<0xddcb4b>,
  &cpuZ80::op_cbbit<0xddcb4c>, &cpuZ80::op_cbbit<0xddcb4d>, &cpuZ80::op_cbbit<0xddcb4e>, &cpuZ80::op_cbbit<0xddcb4f>,
  &cpuZ80::op_cbbit<0xddcb50>, &cpuZ80::op_cbbit<0xddcb51>, &cpuZ80::op_cbbit<0xddcb52>, &cpuZ80::op_cbbit<0xddcb53>,
  &cpuZ80::op_cbbit<0xddcb54>, &cpuZ80::op_cbbit<0xddcb55>, &cpuZ80::op_cbbit<0xddcb56>, &cpuZ80::op_cbbit<0xddcb57>,
  &cpuZ80::op_cbbit<0xddcb58>, &cpuZ80::op_cbbit<0xddcb59>, &cpuZ80::op_cbbit<0xddcb5a>, &cpuZ80::op_cbbit<0xddcb5b>,
  &cpuZ80::op_cbbit<0xddcb5c>, &cpuZ80::op_cbbit<0xddcb5d>, &cpuZ80::op_cbbit<0xddcb5e>, &cpuZ80::op_cbbit<0xddcb5f>,
  &cpuZ80::op_cbbit<0xddcb60>, &cpuZ80::op_cbbit<0xddcb61>, &cpuZ80::op_cbbit<0xddcb62>, &cpuZ80::op_cbbit<0xddcb63>,
  &cpuZ80::op_cbbit<0xddcb64>, &cpuZ80::op_cbbit<0xddcb65>, &cpuZ80::op_cbbit<0xddcb66>, &cpuZ80::op_cbbit<0xddcb67>,
  &cpuZ80::op_cbbit<0xddcb68>, &cpuZ80::op_cbbit<0xddcb69>, &cpuZ80::op_cbbit<0xddcb6a>, &cpuZ80::op_cbbit<0xddcb6b>,
  &cpuZ80::op_cbbit<0xddcb6c>, &cpuZ80::op_cbbit<0xddcb6d>, &cpuZ80::op_cbbit<0xddcb6e>, &cpuZ80::op_cbbit<0xddcb6f>,
  &cpuZ80::op_cbbit<0xddcb70>, &cpuZ80::op_cbbit<0xddcb71>, &cpuZ80::op_cbbit<0xddcb72>, &cpuZ80::op_cbbit<0xddcb73>,
  &cpuZ80::op_cbbit<0xddcb74>, &cpuZ80::op_cbbit<0xddcb75>, &cpuZ80::op_cbbit<0xddcb76>, &cpuZ80::op_cbbit<0xddcb77>,
  &cpuZ80::op_cbbit<0xddcb78>, &cpuZ80::op_cbbit<0xddcb79>, &cpuZ80::op_cbbit<0xddcb7a>, &cpuZ80::op_cbbit<0xddcb7b>,
  &cpuZ80::op_cbbit<0xddcb7c>, &cpuZ80::op_cbbit<0xddcb7d>, &cpuZ80::op_cbbit<0xddcb7e>, &cpuZ80::op_cbbit<0xddcb7f>,
  &cpuZ80::op_cbres<0xddcb80>, &cpuZ80::op_cbres<0xddcb81>, &cpuZ80::op_cbres<0xddcb82>, &cpuZ80::op_cbres<0xddcb83>,
  &cpuZ80::op_cbres<0xddcb84>, &cpuZ80::op_cbres<0xddcb85>, &cpuZ80::op_cbres<0xddcb86>, &cpuZ80::op_cbres<0xddcb87>,
  &cpuZ80::op_cbres<0xddcb88>, &cpuZ80::op_cbres<0xddcb89>, &cpuZ80::op_cbres<0xddcb8a>, &cpuZ80::op_cbres<0xddcb8b>,
  &cpuZ80::op_cbres<0xddcb8c>, &cpuZ80::op_cbres<0xddcb8d>, &cpuZ80::op_cbres<0xddcb8e>, &cpuZ80::op_cbres<0xddcb8f>,
  &cpuZ80::op_cbres<0xddcb90>, &cpuZ80::op_cbres<0xddcb91>, &cpuZ80::op_cbres<0xddcb92>, &cpuZ80::op_cbres<0xddcb93>,
  &cpuZ80::op_cbres<0xddcb94>, &cpuZ80::op_cbres<0xddcb95>, &cpuZ80::op_cbres<0xddcb96>, &cpuZ80::op_cbres<0xddcb97>,
  &cpuZ80::op_cbres<0xddcb98>, &cpuZ80::op_cbres<0xddcb99>, &cpuZ80::op_cbres<0xddcb9a>, &cpuZ80::op_cbres<0xddcb9b>,
  &cpuZ80::op_cbres<0xddcb9c>, &cpuZ80::op_cbres<0xddcb9d>, &cpuZ80::op_cbres<0xddcb9e>, &cpuZ80::op_cbres<0xddcb9f>,
  &cpuZ80::op_cbres<0xddcba0>, &cpuZ80::op_cbres<0xddcba1>, &cpuZ80::op_cbres<0xddcba2>, &cpuZ80::op_cbres<0xddcba3>,
  &cpuZ80::op_cbres<0xddcba4>, &cpuZ80::op_cbres<0xddcba5>, &cpuZ80::op_cbres<0xddcba6>, &cpuZ80::op_cbres<0xddcba7>,
  &cpuZ80::op_cbres<0xddcba8>, &cpuZ80::op_cbres<0xddcba9>, &cpuZ80::op_cbres<0xddcbaa>, &cpuZ80::op_cbres<0xddcbab>,
  &cpuZ80::op_cbres<0xddcbac>, &cpuZ80::op_cbres<0xddcbad>, &cpuZ80::op_cbres<0xddcbae>, &cpuZ80::op_cbres<0xddcbaf>,
  &cpuZ80::op_cbres<0xddcbb0>, &cpuZ80::op_cbres<0xddcbb1>, &cpuZ80::op_cbres<0xddcbb2>, &cpuZ80::op_cbres<0xddcbb3>,
  &cpuZ80::op_cbres<0xddcbb4>, &cpuZ80::op_cbres<0xddcbb5>, &cpuZ80::op_cbres<0xddcbb6>, &cpuZ80::op_cbres<0xddcbb7>,
  &cpuZ80::op_cbres<0xddcbb8>, &cpuZ80::op_cbres<0xddcbb9>, &cpuZ80::op_cbres<0xddcbba>, &cpuZ80::op_cbres<0xddcbbb>,
  &cpuZ80::op_cbres<0xddcbbc>, &cpuZ80::op_cbres<0xddcbbd>, &cpuZ80::op_cbres<0xddcbbe>, &cpuZ80::op_cbres<0xddcbbf>,
  &cpuZ80::op_cbset<0xddcbc0>, &cpuZ80::op_cbset<0xddcbc1>, &cpuZ80::op_cbset<0xddcbc2>, &cpuZ80::op_cbset<0xddcbc3>,
  &cpuZ80::op_cbset<0xddcbc4>, &cpuZ80::op_cbset<0xddcbc5>, &cpuZ80::op_cbset<0xddcbc6>, &cpuZ80::op_cbset<0xddcbc7>,
  &cpuZ80::op_cbset<0xddcbc8>, &cpuZ80::op_cbset<0xddcbc9>, &cpuZ80::op_cbset<0xddcbca>, &cpuZ80::op_cbset<0xddcbcb>,
  &cpuZ80::op_cbset<0xddcbcc>, &cpuZ80::op_cbset<0xddcbcd>, &cpuZ80::op_cbset<0xddcbce>, &cpuZ80::op_cbset<0xddcbcf>,
  &cpuZ80::op_cbset<0xddcbd0>, &cpuZ80::op_cbset<0xddcbd1>, &cpuZ80::op_cbset<0xddcbd2>, &cpuZ80::op_cbset<0xddcbd3>,
  &cpuZ80::op_cbset<0xddcbd4>, &cpuZ80::op_cbset<0xddcbd5>, &cpuZ80::op_cbset<0xddcbd6>, &cpuZ80::op_cbset<0xddcbd7>,
  &cpuZ80::op_cbset<0xddcbd8>, &cpuZ80::op_cbset<0xddcbd9>, &cpuZ80::op_cbset<0xddcbda>, &cpuZ80::op_cbset<0xddcbdb>,
  &cpuZ80::op_cbset<0xddcbdc>, &cpuZ80::op_cbset<0xddcbdd>, &cpuZ80::op_cbset<0xddcbde>, &cpuZ80::op_cbset<0xddcbdf>,
  &cpuZ80::op_cbset<0xddcbe0>, &cpuZ80::op_cbset<0xddcbe1>, &cpuZ80::op_cbset<0xddcbe2>, &cpuZ80::op_cbset<0xddcbe3>,
  &cpuZ80::op_cbset<0xddcbe4>, &cpuZ80::op_cbset<0xddcbe5>, &cpuZ80::op_cbset<0xddcbe6>, &cpuZ80::op_cbset<0xddcbe7>,
  &cpuZ80::op_cbset<0xddcbe8>, &cpuZ80::op_cbset<0xddcbe9>, &cpuZ80::op_cbset<0xddcbea>, &cpuZ80::op_cbset<0xddcbeb>,
  &cpuZ80::op_cbset<0xddcbec>, &cpuZ80::op_cbset<0xddcbed>, &cpuZ80::op_cbset<0xddcbee>, &cpuZ80::op_cbset<0xddcbef>,
  &cpuZ80::op_cbset<0xddcbf0>, &cpuZ80::op_cbset<0xddcbf1>, &cpuZ80::op_cbset<0xddcbf2>, &cpuZ80::op_cbset<0xddcbf3>,
  &cpuZ80::op_cbset<0xddcbf4>, &cpuZ80::op_cbset<0xddcbf5>, &cpuZ80::op_cbset<0xddcbf6>, &cpuZ80::op_cbset<0xddcbf7>,
  &cpuZ80::op_cbset<0xddcbf8>, &cpuZ80::op_cbset<0xddcbf9>, &cpuZ80::op_cbset<0xddcbfa>, &cpuZ80::op_cbset<0xddcbfb>,
  &cpuZ80::op_cbset<0xddcbfc>, &cpuZ80::op_cbset<0xddcbfd>, &cpuZ80::op_cbset<0xddcbfe>, &cpuZ80::op_cbset<0xddcbff>
};

const std::array<z80OpPtr, 256> cpuZ80::fdcb_op_table = {
  &cpuZ80::op_cbrot<0xfdcb00>, &cpuZ80::op_cbrot<0xfdcb01>, &cpuZ80::op_cbrot<0xfdcb02>, &cpuZ80::op_cbrot<0xfdcb03>,
  &cpuZ80::op_cbrot<0xfdcb04>, &cpuZ80::op_cbrot<0xfdcb05>, &cpuZ80::op_cbrot<0xfdcb06>, &cpuZ80::op_cbrot<0xfdcb07>,
  &cpuZ80::op_cbrot<0xfdcb08>, &cpuZ80::op_cbrot<0xfdcb09>, &cpuZ80::op_cbrot<0xfdcb0a>, &cpuZ80::op_cbrot<0xfdcb0b>,
  &cpuZ80::op_cbrot<0xfdcb0c>, &cpuZ80::op_cbrot<0xfdcb0d>, &cpuZ80::op_cbrot<0xfdcb0e>, &cpuZ80::op_cbrot<0xfdcb0f>,
  &cpuZ80::op_cbrot<0xfdcb10>, &cpuZ80::op_cbrot<0xfdcb11>, &cpuZ80::op_cbrot<0xfdcb12>, &cpuZ80::op_cbrot<0xfdcb13>,
  &cpuZ80::op_cbrot<0xfdcb14>, &cpuZ80::op_cbrot<0xfdcb15>, &cpuZ80::op_cbrot<0xfdcb16>, &cpuZ80::op_cbrot<0xfdcb17>,
  &cpuZ80::op_cbrot<0xfdcb18>, &cpuZ80::op_cbrot<0xfdcb19>, &cpuZ80::op_cbrot<0xfdcb1a>, &cpuZ80::op_cbrot<0xfdcb1b>,
  &cpuZ80::op_cbrot<0xfdcb1c>, &cpuZ80::op_cbrot<0xfdcb1d>, &cpuZ80::op_cbrot<0xfdcb1e>, &cpuZ80::op_cbrot<0xfdcb1f>,
  &cpuZ80::op_cbrot<0xfdcb20>, &cpuZ80::op_cbrot<0xfdcb21>, &cpuZ80::op_cbrot<0xfdcb22>, &cpuZ80::op_cbrot<0xfdcb23>,
  &cpuZ80::op_cbrot<0xfdcb24>, &cpuZ80::op_cbrot<0xfdcb25>, &cpuZ80::op_cbrot<0xfdcb26>, &cpuZ80::op_cbrot<0xfdcb27>,
  &cpuZ80::op_cbrot<0xfdcb28>, &cpuZ80::op_cbrot<0xfdcb29>, &cpuZ80::op_cbrot<0xfdcb2a>, &cpuZ80::op_cbrot<0xfdcb2b>,
  &cpuZ80::op_cbrot<0xfdcb2c>, &cpuZ80::op_cbrot<0xfdcb2d>, &cpuZ80::op_cbrot<0xfdcb2e>, &cpuZ80::op_cbrot<0xfdcb2f>,
  &cpuZ80::op_cbrot<0xfdcb30>, &cpuZ80::op_cbrot<0xfdcb31>, &cpuZ80::op_cbrot<0xfdcb32>, &cpuZ80::op_cbrot<0xfdcb33>,
  &cpuZ80::op_cbrot<0xfdcb34>, &cpuZ80::op_cbrot<0xfdcb35>, &cpuZ80::op_cbrot<0xfdcb36>, &cpuZ80::op_cbrot<0xfdcb37>,
  &cpuZ80::op_cbrot<0xfdcb38>, &cpuZ80::op_cbrot<0xfdcb39>, &cpuZ80::op_cbrot<0xfdcb3a>, &cpuZ80::op_cbrot<0xfdcb3b>,
  &cpuZ80::op_cbrot<0xfdcb3c>, &cpuZ80::op_cbrot<0xfdcb3d>, &cpuZ80::op_cbrot<0xfdcb3e>, &cpuZ80::op_cbrot<0xfdcb3f>,
  &cpuZ80::op_cbbit<0xfdcb40>, &cpuZ80::op_cbbit<0xfdcb41>, &cpuZ80::op_cbbit<0xfdcb42>, &cpuZ80::op_cbbit<0xfdcb43>,
  &cpuZ80::op_cbbit<0xfdcb44>, &cpuZ80::op_cbbit<0xfdcb45>, &cpuZ80::op_cbbit<0xfdcb46>, &cpuZ80::op_cbbit<0xfdcb47>,
  &cpuZ80::op_cbbit<0xfdcb48>, &cpuZ80::op_cbbit<0xfdcb49>, &cpuZ80::op_cbbit<0xfdcb4a>, &cpuZ80::op_cbbit<0xfdcb4b>,
  &cpuZ80::op_cbbit<0xfdcb4c>, &cpuZ80::op_cbbit<0xfdcb4d>, &cpuZ80::op_cbbit<0xfdcb4e>, &cpuZ80::op_cbbit<0xfdcb4f>,
  &cpuZ80::op_cbbit<0xfdcb50>, &cpuZ80::op_cbbit<0xfdcb51>, &cpuZ80::op_cbbit<0xfdcb52>, &cpuZ80::op_cbbit<0xfdcb53>,
  &cpuZ80::op_cbbit<0xfdcb54>, &cpuZ80::op_cbbit<0xfdcb55>, &cpuZ80::op_cbbit<0xfdcb56>, &cpuZ80::op_cbbit<0xfdcb57>,
  &cpuZ80::op_cbbit<0xfdcb58>, &cpuZ80::op_cbbit<0xfdcb59>, &cpuZ80::op_cbbit<0xfdcb5a>, &cpuZ80::op_cbbit<0xfdcb5b>,
  &cpuZ80::op_cbbit<0xfdcb5c>, &cpuZ80::op_cbbit<0xfdcb5d>, &cpuZ80::op_cbbit<0xfdcb5e>, &cpuZ80::op_cbbit<0xfdcb5f>,
  &cpuZ80::op_cbbit<0xfdcb60>, &cpuZ80::op_cbbit<0xfdcb61>, &cpuZ80::op_cbbit<0xfdcb62>, &cpuZ80::op_cbbit<0xfdcb63>,
  &cpuZ80::op_cbbit<0xfdcb64>, &cpuZ80::op_cbbit<0xfdcb65>, &cpuZ80::op_cbbit<0xfdcb66>, &cpuZ80::op_cbbit<0xfdcb67>,
  &cpuZ80::op_cbbit<0xfdcb68>, &cpuZ80::op_cbbit<0xfdcb69>, &cpuZ80::op_cbbit<0xfdcb6a>, &cpuZ80::op_cbbit<0xfdcb6b>,
  &cpuZ80::op_cbbit<0xfdcb6c>, &cpuZ80::op_cbbit<0xfdcb6d>, &cpuZ80::op_cbbit<0xfdcb6e>, &cpuZ80::op_cbbit<0xfdcb6f>,
  &cpuZ80::op_cbbit<0xfdcb70>, &cpuZ80::op_cbbit<0xfdcb71>, &cpuZ80::op_cbbit<0xfdcb72>, &cpuZ80::op_cbbit<0xfdcb73>,
  &cpuZ80::op_cbbit<0xfdcb74>, &cpuZ80::op_cbbit<0xfdcb75>, &cpuZ80::op_cbbit<0xfdcb76>, &cpuZ80::op_cbbit<0xfdcb77>,
  &cpuZ80::op_cbbit<0xfdcb78>, &cpuZ80::op_cbbit<0xfdcb79>, &cpuZ80::op_cbbit<0xfdcb7a>, &cpuZ80::op_cbbit<0xfdcb7b>,
  &cpuZ80::op_cbbit<0xfdcb7c>, &cpuZ80::op_cbbit<0xfdcb7d>, &cpuZ80::op_cbbit<0xfdcb7e>, &cpuZ80::op_cbbit<0xfdcb7f>,
  &cpuZ80::op_cbres<0xfdcb80>, &cpuZ80::op_cbres<0xfdcb81>, &cpuZ80::op_cbres<0xfdcb82>, &cpuZ80::op_cbres<0xfdcb83>,
  &cpuZ80::op_cbres<0xfdcb84>, &cpuZ80::op_cbres<0xfdcb85>, &cpuZ80::op_cbres<0xfdcb86>, &cpuZ80::op_cbres<0xfdcb87>,
  &cpuZ80::op_cbres<0xfdcb88>, &cpuZ80::op_cbres<0xfdcb89>, &cpuZ80::op_cbres<0xfdcb8a>, &cpuZ80::op_cbres<0xfdcb8b>,
  &cpuZ80::op_cbres<0xfdcb8c>, &cpuZ80::op_cbres<0xfdcb8d>, &cpuZ80::op_cbres<0xfdcb8e>, &cpuZ80::op_cbres<0xfdcb8f>,
  &cpuZ80::op_cbres<0xfdcb90>, &cpuZ80::op_cbres<0xfdcb91>, &cpuZ80::op_cbres<0xfdcb92>, &cpuZ80::op_cbres<0xfdcb93>,
  &cpuZ80::op_cbres<0xfdcb94>, &cpuZ80::op_cbres<0xfdcb95>, &cpuZ80::op_cbres<0xfdcb96>, &cpuZ80::op_cbres<0xfdcb97>,
  &cpuZ80::op_cbres<0xfdcb98>, &cpuZ80::op_cbres<0xfdcb99>, &cpuZ80::op_cbres<0xfdcb9a>, &cpuZ80::op_cbres<0xfdcb9b>,
  &cpuZ80::op_cbres<0xfdcb9c>, &cpuZ80::op_cbres<0xfdcb9d>, &cpuZ80::op_cbres<0xfdcb9e>, &cpuZ80::op_cbres<0xfdcb9f>,
  &cpuZ80::op_cbres<0xfdcba0>, &cpuZ80::op_cbres<0xfdcba1>, &cpuZ80::op_cbres<0xfdcba2>, &cpuZ80::op_cbres<0xfdcba3>,
  &cpuZ80::op_cbres<0xfdcba4>, &cpuZ80::op_cbres<0xfdcba5>, &cpuZ80::op_cbres<0xfdcba6>, &cpuZ80::op_cbres<0xfdcba7>,
  &cpuZ80::op_cbres<0xfdcba8>, &cpuZ80::op_cbres<0xfdcba9>, &cpuZ80::op_cbres<0xfdcbaa>, &cpuZ80::op_cbres<0xfdcbab>,
  &cpuZ80::op_cbres<0xfdcbac>, &cpuZ80::op_cbres<0xfdcbad>, &cpuZ80::op_cbres<0xfdcbae>, &cpuZ80::op_cbres<0xfdcbaf>,
  &cpuZ80::op_cbres<0xfdcbb0>, &cpuZ80::op_cbres<0xfdcbb1>, &cpuZ80::op_cbres<0xfdcbb2>, &cpuZ80::op_cbres<0xfdcbb3>,
  &cpuZ80::op_cbres<0xfdcbb4>, &cpuZ80::op_cbres<0xfdcbb5>, &cpuZ80::op_cbres<0xfdcbb6>, &cpuZ80::op_cbres<0xfdcbb7>,
  &cpuZ80::op_cbres<0xfdcbb8>, &cpuZ80::op_cbres<0xfdcbb9>, &cpuZ80::op_cbres<0xfdcbba>, &cpuZ80::op_cbres<0xfdcbbb>,
  &cpuZ80::op_cbres<0xfdcbbc>, &cpuZ80::op_cbres<0xfdcbbd>, &cpuZ80::op_cbres<0xfdcbbe>, &cpuZ80::op_cbres<0xfdcbbf>,
  &cpuZ80::op_cbset<0xfdcbc0>, &cpuZ80::op_cbset<0xfdcbc1>, &cpuZ80::op_cbset<0xfdcbc2>, &cpuZ80::op_cbset<0xfdcbc3>,
  &cpuZ80::op_cbset<0xfdcbc4>, &cpuZ80::op_cbset<0xfdcbc5>, &cpuZ80::op_cbset<0xfdcbc6>, &cpuZ80::op_cbset<0xfdcbc7>,
  &cpuZ80::op_cbset<0xfdcbc8>, &cpuZ80::op_cbset<0xfdcbc9>, &cpuZ80::op_cbset<0xfdcbca>, &cpuZ80::op_cbset<0xfdcbcb>,
  &cpuZ80::op_cbset<0xfdcbcc>, &cpuZ80::op_cbset<0xfdcbcd>, &cpuZ80::op_cbset<0xfdcbce>, &cpuZ80::op_cbset<0xfdcbcf>,
  &cpuZ80::op_cbset<0xfdcbd0>, &cpuZ80::op_cbset<0xfdcbd1>, &cpuZ80::op_cbset<0xfdcbd2>, &cpuZ80::op_cbset<0xfdcbd3>,
  &cpuZ80::op_cbset<0xfdcbd4>, &cpuZ80::op_cbset<0xfdcbd5>, &cpuZ80::op_cbset<0xfdcbd6>, &cpuZ80::op_cbset<0xfdcbd7>,
  &cpuZ80::op_cbset<0xfdcbd8>, &cpuZ80::op_cbset<0xfdcbd9>, &cpuZ80::op_cbset<0xfdcbda>, &cpuZ80::op_cbset<0xfdcbdb>,
  &cpuZ80::op_cbset<0xfdcbdc>, &cpuZ80::op_cbset<0xfdcbdd>, &cpuZ80::op_cbset<0xfdcbde>, &cpuZ80::op_cbset<0xfdcbdf>,
  &cpuZ80::op_cbset<0xfdcbe0>, &cpuZ80::op_cbset<0xfdcbe1>, &cpuZ80::op_cbset<0xfdcbe2>, &cpuZ80::op_cbset<0xfdcbe3>,
  &cpuZ80::op_cbset<0xfdcbe4>, &cpuZ80::op_cbset<0xfdcbe5>, &cpuZ80::op_cbset<0xfdcbe6>, &cpuZ80::op_cbset<0xfdcbe7>,
  &cpuZ80::op_cbset<0xfdcbe8>, &cpuZ80::op_cbset<0xfdcbe9>, &cpuZ80::op_cbset<0xfdcbea>, &cpuZ80::op_cbset<0xfdcbeb>,
  &cpuZ80::op_cbset<0xfdcbec>, &cpuZ80::op_cbset<0xfdcbed>, &cpuZ80::op_cbset<0xfdcbee>, &cpuZ80::op_cbset<0xfdcbef>,
  &cpuZ80::op_cbset<0xfdcbf0>, &cpuZ80::op_cbset<0xfdcbf1>, &cpuZ80::op_cbset<0xfdcbf2>, &cpuZ80::op_cbset<0xfdcbf3>,
  &cpuZ80::op_cbset<0xfdcbf4>, &cpuZ80::op_cbset<0xfdcbf5>, &cpuZ80::op_cbset<0xfdcbf6>, &cpuZ80::op_cbset<0xfdcbf7>,
  &cpuZ80::op_cbset<0xfdcbf8>, &cpuZ80::op_cbset<0xfdcbf9>, &cpuZ80::op_cbset<0xfdcbfa>, &cpuZ80::op_cbset<0xfdcbfb>,
  &cpuZ80::op_cbset<0xfdcbfc>, &cpuZ80::op_cbset<0xfdcbfd>, &cpuZ80::op_cbset<0xfdcbfe>, &cpuZ80::op_cbset<0xfdcbff>
};

template <uint32_t OPCODE>
uint64_t cpuZ80::cb_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    dbg_printf(" %x", opcode);
    return CALL_MEMBER_FN(this, cb_op_table[opcode])(opcode);
}

template <uint32_t OPCODE>
uint64_t cpuZ80::dd_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    dbg_printf(" %x", opcode);
    return CALL_MEMBER_FN(this, dd_op_table[opcode])(opcode);
}

template <uint32_t OPCODE>
uint64_t cpuZ80::ed_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    dbg_printf(" %x", opcode);
    return CALL_MEMBER_FN(this, ed_op_table[opcode - 0x40])(opcode);
}

template <uint32_t OPCODE>
uint64_t cpuZ80::fd_op_prefix(uint8_t opcode) {
    opcode = memory->readByte(pc++);
    dbg_printf(" %x", opcode);
    return CALL_MEMBER_FN(this, fd_op_table[opcode])(opcode);
}

template <uint32_t OPCODE>
uint64_t cpuZ80::ddcb_op_prefix(uint8_t opcode) {
    uint8_t displacement = memory->readByte(pc++);
    opcode = memory->readByte(pc++);
    dbg_printf(" %x %x", displacement, opcode);
    return CALL_MEMBER_FN(this, ddcb_op_table[opcode])(displacement);
}


template <uint32_t OPCODE>
uint64_t cpuZ80::fdcb_op_prefix(uint8_t opcode) {
    uint8_t displacement = memory->readByte(pc++);
    opcode = memory->readByte(pc++);
    dbg_printf(" %x", opcode);
    return CALL_MEMBER_FN(this, fdcb_op_table[opcode])(displacement);
}

uint64_t cpuZ80::decode(uint8_t opcode) {
    return CALL_MEMBER_FN(this, op_table[opcode])(opcode);
}

void cpuZ80::push(uint16_t val) {
    //dbg_printf("\tPush %04x to %04x", val, sp-2);
    memory->writeWord(sp - 2, val);
    sp -= 2;
}

uint16_t cpuZ80::pop() {
    uint16_t val = memory->readWord(sp);
    //dbg_printf("\tPop %04x from %04x", val, sp);
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

bool cpuZ80::addition_overflows(int8_t a, int8_t b) {
    return (b >= 0) && ( a > std::numeric_limits<int8_t>::max() - b);
}

bool cpuZ80::addition_underflows(int8_t a, int8_t b) {
    return (b < 0) && (a < std::numeric_limits<int8_t>::min() - b);
}

template<typename T>
bool cpuZ80::subtraction_overflows(T a, T b) {
    return (b < 0) && (a > std::numeric_limits<T>::max() + b);
}

template<typename T>
bool cpuZ80::subtraction_underflows(T a, T b) {
    return (b >= 0) && (a < std::numeric_limits<T>::min() + b);
}

void cpuZ80::print_registers() {
    dbg_printf("\t\tA: %02x BC: %04x DE: %04x HL: %04x IX: %04x IY: %04x SP: %04x status: %c%c0%c0%c%c%c", af.hi, bc.pair, de.pair, hl.pair, ix.pair, iy.pair, sp,
     sign()?'S':'s',
     zero()?'Z':'z',
     hc()?'H':'h',
     parity()?'P':'p',
     sub()?'N':'n',
     carry()?'C':'c'
     );
}

bool cpuZ80::condition(int condition_number) {
    switch(condition_number & 0x7) {
    case 0: // Non-Zero (NZ)
        return !zero();
    case 1: // Zero (Z)
        return zero();
    case 2: // No Carry (NC)
        return !carry();
    case 3: // Carry (C)
        return carry();
    case 4: // Parity Odd (PO)
        return !parity();
    case 5: // Parity Even (PE)
        return parity();
    case 6: // Sign Positive (P)
        return !sign();
    case 7: // Sign Negative (M)
        return sign();
    }
    //throw std::string("This condition can't/shouldn't be reached.");
    return false;
}
template <uint32_t OPCODE> uint64_t cpuZ80::op_unimpl(uint8_t opcode) {
    std::cout<<"\nOpcode "<<std::hex<<OPCODE<<" not implemented.\n";
    return -1;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_adc16(uint8_t opcode) { // ADC HL, ss 4
    uint16_t* const regset[] {&(bc.pair), &(de.pair), &(hl.pair), &(sp)};
    int reg = ((OPCODE>>4) & 0x3);
    int32_t temp = hl.pair + *regset[reg] + carry();

    clear(SUB_FLAG);

    if(uint16_t(temp) > 0xffff) set(CARRY_FLAG);
    else              clear(CARRY_FLAG);

    if((hl.pair & 0xfff) + ((*regset[reg] + carry()) & 0xfff) >= 4096) set(HALF_CARRY_FLAG);
    else clear(HALF_CARRY_FLAG);

    if(temp > 32767 || temp < -32768) set(OVERFLOW_FLAG);
    else clear(OVERFLOW_FLAG);

    if(!temp) set(ZERO_FLAG);
    else clear(ZERO_FLAG);

    // TODO: Fix flags

    hl.pair = temp & 0xffff;

    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_add16(uint8_t opcode) { // 16-bit r-r adds 11
    uint16_t* const regset[] = {&(bc.pair), &(de.pair), &(hl.pair), &(sp), &(ix.pair), &(iy.pair)};
    uint16_t* dest = &(hl.pair);
    uint8_t reg = ((OPCODE>>4) & 0x03);
    uint64_t cycles = 11;
    if((OPCODE & 0xff00) == 0xdd00) {
        dest = &(ix.pair);
        cycles = 15;
        if(reg == 2) reg = 4;
    }
    else if((OPCODE & 0xff00) == 0xfd00) {
        dest = &(iy.pair);
        cycles = 15;
        if(reg == 2) reg = 5;
    }

    clear(SUB_FLAG);
    if(((*dest) & 0xfff) + ((*regset[reg]) & 0xfff) >= 4096) set(HALF_CARRY_FLAG);
    else clear(HALF_CARRY_FLAG);
    uint32_t temp = (*regset[reg]) + (*dest);
    if(temp > 0xffff) set(CARRY_FLAG);
    else clear(CARRY_FLAG);
    *dest = (temp & 0xffff);

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_alu(uint8_t opcode) { // 8-bit mostly r-r add, adc, sub, sbc, and, xor, or, cp
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    constexpr uint8_t operation = ((OPCODE>>3) & 0x07);
    constexpr uint8_t reg = (OPCODE & 0x07);
    uint64_t cycles = 4;
    if(OPCODE < 0xc0 && reg == 6) { // ops [89ab][6e]
        dummy8 = memory->readByte(hl.pair);
        cycles = 7;
    }
    else if(OPCODE >= 0xc0 && reg == 6) { // ops [cdef][6e]
        dummy8 = memory->readByte(pc);
        pc++;
        cycles = 7;
    }
    uint16_t temp_a = af.hi;
    switch(operation) {
    case 0x00: // add
        temp_a += *regset[reg];

        clear(SUB_FLAG);

        if(temp_a > 0xff) set(CARRY_FLAG);
        else              clear(CARRY_FLAG);

        if(addition_overflows(af.hi, *regset[reg]) || addition_underflows(af.hi, *regset[reg])) set(OVERFLOW_FLAG);
        else clear(OVERFLOW_FLAG);

        if((af.hi & 0xf) + (*regset[reg] & 0xf) >= 0x10) set(HALF_CARRY_FLAG);
        else clear(HALF_CARRY_FLAG);

        af.hi = (temp_a & 0xff);
        break;
    case 0x01: // adc
        temp_a += *regset[reg];
        if(carry()) temp_a++;

        clear(SUB_FLAG);

        if(temp_a > 0xff) set(CARRY_FLAG);
        else              clear(CARRY_FLAG);

        if(addition_overflows(af.hi, *regset[reg] + carry()) || addition_underflows(af.hi, *regset[reg] + carry())) set(OVERFLOW_FLAG);
        else clear(OVERFLOW_FLAG);

        if((af.hi & 0xf) + (*regset[reg] & 0xf) >= 0x10) set(HALF_CARRY_FLAG);
        else clear(HALF_CARRY_FLAG);

        af.hi = (temp_a & 0xff);
        break;
    case 0x02: // sub
        temp_a -= *regset[reg];
        set(SUB_FLAG);

        if(temp_a > af.hi) set(CARRY_FLAG);
        else               clear(CARRY_FLAG);

        if((*regset[reg] & 0xf) > (af.hi & 0xf)) set(HALF_CARRY_FLAG);
        else clear(HALF_CARRY_FLAG);

        if(subtraction_overflows(int8_t(af.hi), int8_t(*regset[reg] - carry())) || subtraction_underflows(int8_t(af.hi), int8_t(*regset[reg] - carry()))) set(OVERFLOW_FLAG);
        else clear(OVERFLOW_FLAG);

        // TODO: Fix flags

        af.hi = temp_a;
        break;
    case 0x03: // sbc
        temp_a -= *regset[reg];
        if(carry()) temp_a--;
        set(SUB_FLAG);

        if(temp_a > af.hi) set(CARRY_FLAG);
        else               clear(CARRY_FLAG);

        if((*regset[reg] & 0xf) + carry() > (af.hi & 0xf)) set(HALF_CARRY_FLAG);

        if(subtraction_overflows(int8_t(af.hi), int8_t(*regset[reg] - carry())) || subtraction_underflows(int8_t(af.hi), int8_t(*regset[reg] - carry()))) set(OVERFLOW_FLAG);
        else clear(OVERFLOW_FLAG);

        // TODO: Fix flags

        af.hi = temp_a;
        break;
    case 0x04: // and
        af.hi &= *regset[reg];
        clear(SUB_FLAG);
        clear(CARRY_FLAG);
        set(HALF_CARRY_FLAG);
        if(parity[af.hi]) set(PARITY_FLAG);
        else              clear(PARITY_FLAG);
        break;
    case 0x05: // xor
        af.hi ^= *regset[reg];
        clear(SUB_FLAG);
        clear(CARRY_FLAG);
        clear(HALF_CARRY_FLAG);
        if(parity[af.hi]) set(PARITY_FLAG);
        else              clear(PARITY_FLAG);
        break;
    case 0x06: // or
        af.hi |= *regset[reg];
        clear(SUB_FLAG);
        clear(CARRY_FLAG);
        clear(HALF_CARRY_FLAG);
        if(parity[af.hi]) set(PARITY_FLAG);
        else              clear(PARITY_FLAG);
        break;
    case 0x07: // cp
        temp_a -= *regset[reg];
        set(SUB_FLAG);

        if(temp_a > af.hi) set(CARRY_FLAG);
        else               clear(CARRY_FLAG);

        if((*regset[reg] & 0xf) > (af.hi & 0xf)) set(HALF_CARRY_FLAG);
        else clear(HALF_CARRY_FLAG);

        if(subtraction_overflows(int8_t(af.hi), int8_t(*regset[reg] - carry())) || subtraction_underflows(int8_t(af.hi), int8_t(*regset[reg] - carry()))) set(OVERFLOW_FLAG);
        else clear(OVERFLOW_FLAG);

        if(!temp_a) set(ZERO_FLAG);
        else clear(ZERO_FLAG);

        if((temp_a & 0x80) > 0) set(SIGN_FLAG);
        else clear(SIGN_FLAG);

        // TODO: Fix flags

        break;
    }

    if(operation != 0x07) { //if op isn't a compare
        if((af.hi & 0x80) > 0) set(SIGN_FLAG);
        else                   clear(SIGN_FLAG);
        if(!af.hi) set(ZERO_FLAG);
        else       clear(ZERO_FLAG);
    }

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_call(uint8_t opcode) { // CALL 17, RST 11
    uint64_t cycles = 17;
    uint16_t address = 0;
    if(OPCODE == 0xcd) {
        address = memory->readWord(pc);
        dbg_printf(" %04x", address);
        pc+=2;
    }
    else {
        address = ((OPCODE >> 3) & 0x7 ) * 0x08;
        cycles = 11;
    }
    push(pc);
    pc = address;
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_call_cc(uint8_t opcode) { // CALL cc 17/10
    uint64_t cycles = 10;
    uint16_t address = memory->readWord(pc);
    if(condition((OPCODE>>3) & 0x7)) {
        cycles = 17;
        push(pc+2);
        pc = address;
    }
    else {
        pc+=2;
    }
    dbg_printf(" %04x", address);
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_cbrot(uint8_t opcode) { // CB00 -> CB3F, DDCB00->DDCB3F, FDCB00->FDCB3F
    constexpr uint8_t op = ((OPCODE>>3) & 0x7);
    constexpr uint8_t reg = (OPCODE & 0x07);
    const int8_t offset = int8_t(opcode);
    uint8_t c = carry();
    uint64_t cycles = 8;
    if(reg == 0x06) {
        if((OPCODE & 0xFFFF00) == 0xCB00) { //CB group
            dummy8 = memory->readByte(hl.pair);
            cycles = 15;
        }
        else if((OPCODE & 0xFFFF00) == 0xDDCB00) {
            dummy8 = memory->readByte(ix.pair + offset);
            cycles = 23;
        }
        else if((OPCODE & 0xFFFF00) == 0xFDCB00) {
            dummy8 = memory->readByte(iy.pair + offset);
            cycles = 23;
        }
    }
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    uint8_t high = (*regset[reg] & 0x80)>>7;
    uint8_t low = (*regset[reg] & 0x01);

    switch(op) {
    case 0: //rlc:
        *regset[reg] <<= 1;
        if(high) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        *regset[reg] |= high;
        break;
    case 1: //rrc:
        *regset[reg] >>= 1;
        if(low) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        *regset[reg] |= (low<<7);
        break;
    case 2: //rl:
        *regset[reg] <<= 1;
        if(high) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        *regset[reg] |= c;
        break;
    case 3: //rr:
        *regset[reg] >>= 1;
        if(low) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        *regset[reg] |= (c << 7);
        break;
    case 4: //sla:
        *regset[reg]<<=1;
        if(high) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        break;
    case 5: //sra:
        *regset[reg]>>=1;
        if(high) *regset[reg] |= 0x80;
        if(low) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        break;
    case 6: //sll:
        *regset[reg]<<=1;
        *regset[reg]|=0x01;
        if(high) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        break;
    case 7: //srl:
        *regset[reg]>>=1;
        if(low) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        break;
    }
    if(reg == 0x06) {
        if((OPCODE & 0xFFFF00) == 0xCB00) { //CB group
            memory->writeByte(hl.pair, dummy8);
        }
        else if((OPCODE & 0xFFFF00) == 0xDDCB00) {
            memory->writeByte(ix.pair + offset, dummy8);
        }
        else if((OPCODE & 0xFFFF00) == 0xFDCB00) {
            memory->writeByte(iy.pair + offset, dummy8);
        }
    }

    clear(SUB_FLAG);
    if(parity[*regset[reg]]) set(PARITY_FLAG);
    else clear(PARITY_FLAG);
    clear(HALF_CARRY_FLAG);
    if(*regset[reg]) clear(ZERO_FLAG);
    else set(ZERO_FLAG);
    if(*regset[reg] & 0x80) set(SIGN_FLAG);
    else clear(SIGN_FLAG);

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_cbbit(uint8_t opcode) { // CB40 -> CB7F, DDCB40->DDCB7F, FDCB40->FDCB7F
    constexpr uint8_t bit = (1<<((OPCODE>>3) & 0x7));
    constexpr uint8_t reg = (OPCODE & 0x07);
    const int8_t offset = int8_t(opcode);
    uint64_t cycles = 8;
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    if(reg == 0x06) {
        if((OPCODE & 0xFFFF00) == 0xCB00) { //CB group
            dummy8 = memory->readByte(hl.pair);
            cycles = 12;
        }
        else if((OPCODE & 0xFFFF00) == 0xDDCB00) {
            dummy8 = memory->readByte(ix.pair + offset);
            cycles = 20;
        }
        else if((OPCODE & 0xFFFF00) == 0xFDCB00) {
            dummy8 = memory->readByte(iy.pair + offset);
            cycles = 20;
        }
    }

    uint8_t val = ((*regset[reg]) & bit);

    if(val & 0x80) set(SIGN_FLAG);
    else clear(SIGN_FLAG);

    if(val) clear(ZERO_FLAG|PARITY_FLAG);
    else set(ZERO_FLAG|PARITY_FLAG);
    clear(SUB_FLAG);
    set(HALF_CARRY_FLAG);

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_cbres(uint8_t opcode) { // CB80 -> CBBF, DDCB80->DDCBBF, FDCB80->FDCBBF
    constexpr uint8_t bit = (1<<((OPCODE>>3) & 0x7));
    constexpr uint8_t reg = (OPCODE & 0x07);
    int8_t offset = int8_t(opcode);
    uint64_t cycles = 8;
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};

    if(reg == 0x06) {
        if((OPCODE & 0xFFFF00) == 0xCB00) { //CB group
            dummy8 = memory->readByte(hl.pair);
            cycles = 15;
        }
        else if((OPCODE & 0xFFFF00) == 0xDDCB00) {
            dummy8 = memory->readByte(ix.pair + offset);
            cycles = 23;
        }
        else if((OPCODE & 0xFFFF00) == 0xFDCB00) {
            dummy8 = memory->readByte(iy.pair + offset);
            cycles = 23;
        }
    }

    *regset[reg] &= (~bit);

    if(reg == 0x06) {
        if((OPCODE & 0xFFFF00) == 0xCB00) { //CB group
            memory->writeByte(hl.pair, dummy8);
        }
        else if((OPCODE & 0xFFFF00) == 0xDDCB00) {
            memory->writeByte(ix.pair + offset, dummy8);
        }
        else if((OPCODE & 0xFFFF00) == 0xFDCB00) {
            memory->writeByte(iy.pair + offset, dummy8);
        }
    }
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_cbset(uint8_t opcode) { // CBC0 -> CBFF, DDCBC0->DDCBFF, FDCC40->FDCBFF
    constexpr uint8_t bit = (1<<((OPCODE>>3) & 0x7));
    constexpr uint8_t reg = (OPCODE & 0x07);
    const int8_t offset = int8_t(opcode);
    uint64_t cycles = 8;
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    if(reg == 0x06) {
        if((OPCODE & 0xFFFF00) == 0xCB00) { //CB group
            dummy8 = memory->readByte(hl.pair);
            cycles = 15;
        }
        else if((OPCODE & 0xFFFF00) == 0xDDCB00) {
            dummy8 = memory->readByte(ix.pair + offset);
            cycles = 23;
        }
        else if((OPCODE & 0xFFFF00) == 0xFDCB00) {
            dummy8 = memory->readByte(iy.pair + offset);
            cycles = 23;
        }
    }

    *regset[reg] |= bit;

    if(reg == 0x06) {
        if((OPCODE & 0xFFFF00) == 0xCB00) { //CB group
            memory->writeByte(hl.pair, dummy8);
        }
        else if((OPCODE & 0xFFFF00) == 0xDDCB00) {
            memory->writeByte(ix.pair + offset, dummy8);
        }
        else if((OPCODE & 0xFFFF00) == 0xFDCB00) {
            memory->writeByte(iy.pair + offset, dummy8);
        }
    }
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ccf(uint8_t opcode) { // CCF 4
    if(carry()) {
        clear(CARRY_FLAG);
        set(HALF_CARRY_FLAG);
    }
    else {
        set(CARRY_FLAG);
        clear(HALF_CARRY_FLAG);
    }
    clear(SUB_FLAG);
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_cpl(uint8_t opcode) { // CPL 4
    af.hi = ~(af.hi);
    set(HALF_CARRY_FLAG);
    set(SUB_FLAG);
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_cp(uint8_t) { // CPI CPD CPIR CPDR 16, 21 if repeated
    uint8_t val = memory->readByte(hl.pair);
    bool incr = (OPCODE == 0xeda1 || OPCODE == 0xedb1);
    bool repeat = (OPCODE == 0xedb1 || OPCODE == 0xedb9);
    uint64_t cycles = 16;
    uint8_t result = af.hi - val;

    set(SUB_FLAG);

    if((af.hi ^ val ^ result) & 0x10) set(HALF_CARRY_FLAG);
    else clear(HALF_CARRY_FLAG);

    if(!result) set(ZERO_FLAG);
    else clear(ZERO_FLAG);

    if(result >= 128) set(SIGN_FLAG);
    else clear(SIGN_FLAG);

    if(incr) hl.pair++;
    else     hl.pair--;

    bc.pair--;

    if(bc.pair != 0) set(PARITY_FLAG);
    else clear(PARITY_FLAG);

    if(repeat && bc.pair && !zero()) {
            cycles += 5;
            pc -= 2;
            dbg_printf(" cp loop");
    }
    else if(zero()) {
        dbg_printf(" cp match");
    }
    else if(repeat && !bc.pair) {
        dbg_printf(" cp end without match");
    }
    else {
        dbg_printf(" cp no match");
    }

    return cycles;
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
   /*
   if (sub() && !hc()) {
      clear(HALF_CARRY_FLAG);
   }
   else if (sub() && hc() && ((af.hi & 0x0F)) < 6) set(HALF_CARRY_FLAG);
    else if ((af.hi & 0x0f) >= 0x0a) set(HALF_CARRY_FLAG);
   else clear(HALF_CARRY_FLAG);
*/
   // builds final H flag
   if (sub() && !hc()) clear(HALF_CARRY_FLAG);
   else {
       if (sub() && hc()) {
            if((af.hi & 0x0f) < 6) set(HALF_CARRY_FLAG);
            else clear(HALF_CARRY_FLAG);
        }
       else {
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
    uint16_t* const regset[] = {&(bc.pair), &(de.pair), &(hl.pair), &(sp), &(ix.pair), &(iy.pair)};
    uint32_t index = OPCODE>>4;
    uint64_t cycles = 6;
    if(OPCODE == 0xdd2b) {
        index = 4;
        cycles = 10;
    }
    else if(OPCODE == 0xfd2b) {
        index = 5;
        cycles = 10;
    }
    (*regset[index])--;
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_decr8(uint8_t opcode) {
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi),
                               &(ix.hi), &(ix.low), &(iy.hi), &(iy.low)};
    uint8_t index = (OPCODE>>3) & 0x07;
    int8_t offset = 0;
    uint64_t cycles = 4;
    if((OPCODE & 0xFF00) == 0xDD00 && index != 6) {
        index += 4;
        cycles = 10;
    }
    if((OPCODE & 0xFF00) == 0xFD00 && index != 6) {
        index += 6;
        cycles = 10;
    }
    if(index == 6) {
        switch(OPCODE & 0xFF00) {
        case 0x0000:
            dummy8 = memory->readByte(hl.pair);
            cycles = 11;
            break;
        case 0xdd00:
            offset = memory->readByte(pc++);
            dummy8 = memory->readByte(ix.pair + offset);
            cycles = 23;
            break;
        case 0xfd00:
            offset = memory->readByte(pc++);
            dummy8 = memory->readByte(iy.pair + offset);
            cycles = 23;
            break;
        }
    }

    (*regset[index])--;

    if(*regset[index] >= 0x80) { set(SIGN_FLAG); } else { clear(SIGN_FLAG); }
    if(*regset[index] == 0)   { set(ZERO_FLAG); } else { clear(ZERO_FLAG); }
    if(((*regset[index]) & 0x0f) == 0x0f)  { set(HALF_CARRY_FLAG);} else { clear(HALF_CARRY_FLAG); }
    if(*regset[index] == 0x7f) { set(OVERFLOW_FLAG); } else { clear(OVERFLOW_FLAG); }
    set(SUB_FLAG);
    if(index == 6) {
        switch(OPCODE & 0xFF00) {
        case 0x0000:
            memory->writeByte(hl.pair, dummy8);
            break;
        case 0xdd00:
            memory->writeByte(ix.pair + offset, dummy8);
            break;
        case 0xfd00:
            memory->writeByte(iy.pair + offset, dummy8);
        }
    }
    return cycles;
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

template <uint32_t OPCODE> uint64_t cpuZ80::op_ex16(uint8_t opcode) { // EX (sp), HL 19, EX DE, HL 4
    uint16_t cycles = 0;
    if(OPCODE == 0xe3) {
        uint16_t val = memory->readWord(sp);
        uint16_t temp = hl.pair;
        hl.pair = val;
        memory->writeWord(sp, temp);
        cycles = 19;
    }
    else if(OPCODE == 0xeb) { // ex de, hl
        std::swap(de.pair, hl.pair);
        cycles = 4;
    }

    return cycles;
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

template <uint32_t OPCODE> uint64_t cpuZ80::op_halt(uint8_t opcode) {
    if(check_interrupts() == int_type_t::no_int) {
        pc--;
        dbg_printf("halted\n");
    }
    if(OPCODE > 0x76) {
        return 8;
    }
    else {
        return 4;
    }
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
    return 8;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_in(uint8_t opcode) { // OUTI 16 OTIR 21/16 OUT 11
    uint8_t* const regset[] {&(bc.hi), &(bc.low), &(de.hi), &(de.low), &(hl.hi), &(hl.low), &dummy8, &(af.hi)};
    uint8_t dest = ((OPCODE>>3) & 0x7);
    uint8_t port = bc.low;
    uint8_t val = 0;
    uint64_t cycles = 12;

    if(OPCODE == 0xdb) { // IN a, (*)
        port = memory->readByte(pc);
        pc++;
        dest = 7; // register 'a'
        cycles = 11;
        dbg_printf(" %02x", port);
    }

    val = memory->readPortByte(port, total_cycles);

    //dbg_printf(" read %02x from port %02x", val, port);

    if(OPCODE < 0xed80) { // non-inc/dec/repeat versions of the opcode
        *regset[dest] = val;
        if(OPCODE > 0xed00) {
            clear(SUB_FLAG);
            clear(HALF_CARRY_FLAG);
            if(parity[val]) set(PARITY_FLAG);
            else clear(PARITY_FLAG);
            if(val) clear(ZERO_FLAG);
            else set(ZERO_FLAG);
            if(val & 0x80) set(SIGN_FLAG);
            else clear(SIGN_FLAG);
        }
    }
    else { // versions of the opcode that auto-inc/dec and repeat
        memory->writeByte(hl.pair, val);
        if(OPCODE == 0xeda2 || OPCODE == 0xedb2) { //increment opcodes
            hl.pair++;
        }
        else if(OPCODE == 0xedaa || OPCODE == 0xedba) { //decrement opcodes
            hl.pair--;
        }
        bc.hi--;

        if(bc.hi && (OPCODE == 0xedb2 || OPCODE == 0xedba)) { //repeat opcodes
            pc-=2; //repeat the instruction
            cycles = 21;
        }
        else {
            cycles = 16;
        }

        // TODO: Fix flags according to documentation
    }

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_incr16(uint8_t opcode) {
    uint16_t* const regset[] = {&(bc.pair), &(de.pair), &(hl.pair), &(sp), &(ix.pair), &(iy.pair)};
    uint32_t index = OPCODE>>4;
    uint64_t cycles = 6;
    if(OPCODE == 0xdd23) {
        index = 4;
        cycles = 10;
    }
    else if(OPCODE == 0xfd23) {
        index = 5;
        cycles = 10;
    }
    (*regset[index])++;
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_incr8(uint8_t opcode) {
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi),
                               &(ix.hi), &(ix.low), &(iy.hi), &(iy.low)};
    uint8_t index = (OPCODE>>3) & 0x07;
    uint64_t cycles = 4;
    int8_t offset = 0;

    if((OPCODE & 0xFF00) == 0xDD00 && index != 6) {
        index += 4;
        cycles = 10;
    }
    if((OPCODE & 0xFF00) == 0xFD00 && index != 6) {
        index += 6;
        cycles = 10;
    }
    if(index == 6) {
        switch(OPCODE & 0xFF00) {
        case 0x0000:
            dummy8 = memory->readByte(hl.pair);
            cycles = 11;
            break;
        case 0xdd00:
            offset = memory->readByte(pc++);
            dummy8 = memory->readByte(ix.pair + offset);
            cycles = 23;
            break;
        case 0xfd00:
            offset = memory->readByte(pc++);
            dummy8 = memory->readByte(iy.pair + offset);
            cycles = 23;
            break;
        }
    }

    (*regset[index])++;

    if(*regset[index] >= 128) { set(SIGN_FLAG); } else { clear(SIGN_FLAG); }
    if(*regset[index] == 0)   { set(ZERO_FLAG); } else { clear(ZERO_FLAG); }
    if(((*regset[index]) & 0x0f) == 0x00)  { set(HALF_CARRY_FLAG);} else { clear(HALF_CARRY_FLAG); }
    if(*regset[index] == 0x80) { set(OVERFLOW_FLAG); } else { clear(OVERFLOW_FLAG); }
    clear(SUB_FLAG);

    if(index == 6) {
        switch(OPCODE & 0xFF00) {
        case 0x00:
            memory->writeByte(hl.pair, dummy8);
            break;
        case 0xdd00:
            memory->writeByte(ix.pair+offset, dummy8);
            break;
        case 0xfd00:
            memory->writeByte(iy.pair+offset, dummy8);
            break;
        }

    }
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_jp(uint8_t opcode) {
    uint16_t jump_addr = 0;
    if(OPCODE == 0xe9) {
        jump_addr = memory->readWord(hl.pair);
    }
    else {
        jump_addr = memory->readWord(pc);
    }

    dbg_printf(" %04x", jump_addr);

    switch(OPCODE) {
    case 0xc3: //JP nn 4,3,3
        pc = jump_addr;
        return 10;
    case 0xe9: //JP (HL)
        pc = jump_addr;
        return 4;
    case 0xc2: case 0xca: case 0xd2: case 0xda: case 0xe2: case 0xea: case 0xf2: case 0xfa: // JP cc, nn
        if(condition((OPCODE>>3) & 7)) {
            pc = jump_addr;
        }
        else {
            // skip the operand
            pc += 2;
        }
        return 10;
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

    int8_t offset = memory->readByte(pc++);
    uint16_t address = pc + offset;
    dbg_printf(" %04x", address);

    if(branch) {
        pc = address;
    }

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld16rim(uint8_t opcode) {
    uint16_t * const regset[] = {&(bc.pair), &(de.pair), &(hl.pair), &sp, &(ix.pair), &(iy.pair)};
    uint16_t val = memory->readWord(pc);
    pc+=2;
    dbg_printf(" %04x", val);
    uint8_t index = ((OPCODE>>4) & 0x03);
    uint8_t operation = ((OPCODE>>3) & 0x01);

    if((OPCODE & 0xff00) == 0xDD00) { //dd22+dd2a
        index = 4;
    }
    else if((OPCODE & 0xff00) == 0xFD00) { //fd22+fd2a
        index = 5;
    }
    if(!operation) {
        memory->writeWord(val, *regset[index]);
    }
    else {
        *regset[index] = memory->readWord(val);
    }
    return 20;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld16rm(uint8_t opcode) {  //LD r16,(**), LD (**), r16
    uint16_t address = memory->readWord(pc);
    dbg_printf(" %04x", address);
    pc+=2;
    if(OPCODE == 0x22) {
        memory->writeWord(address, hl.pair);
    }
    else if(OPCODE == 0x2a) {
        hl.pair = memory->readWord(address);
    }
    return 16;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8mm(uint8_t opcode) {  //LDI 16, LDIR 21/16, LDD 16, LDDR 21/16
    uint8_t val = memory->readByte(hl.pair);
    memory->writeByte(de.pair, val);
    uint64_t cycles = 16;
    bool incr = (OPCODE == 0xEDA0 || OPCODE == 0xEDB0);
    bool rep = (OPCODE == 0xEDB0 || OPCODE == 0xEDB8);
    if(incr) {
        hl.pair++;
        de.pair++;
    }
    else {
        hl.pair--;
        de.pair--;
    }
    bc.pair--;

    if(bc.pair && rep) {
        pc -= 2; // repeat the instruction
        cycles = 21;
    }

    if(bc.pair == 0) clear(PARITY_FLAG);
    else set(PARITY_FLAG);

    clear(SUB_FLAG);
    clear(HALF_CARRY_FLAG);
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8ri(uint8_t opcode) { //LD r,immed 7
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    constexpr uint8_t dest_index = ((OPCODE>>3) & 0x07);
    uint64_t cycles = 7;
    *regset[dest_index] = memory->readByte(pc++);
    if(dest_index == 6) {
        cycles = 10;
        memory->writeByte(hl.pair, dummy8);
    }
    dbg_printf(" %02x", *regset[dest_index]);

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8idxri(uint8_t opcode) { //LD idxr,immed 11
    uint8_t val = memory->readByte(pc++);
    switch(OPCODE) {
        case 0xdd26: ix.hi = val; break;
        case 0xdd2e: ix.low = val; break;
        case 0xfd26: iy.hi = val; break;
        case 0xfd2e: iy.low = val; break;
    }

    dbg_printf(" %02x", val);

    return 11;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8mioff(uint8_t opcode) { //LD (ix+immed),immed 19
    uint16_t reg = 0;
    if((OPCODE & 0xff00) == 0xdd00) {
        reg = ix.pair;
    }
    else if((OPCODE & 0xff00) == 0xfd00) {
        reg = iy.pair;
    }
    int8_t offset = memory->readByte(pc++);
    uint8_t val = memory->readByte(pc++);
    memory->writeByte(reg+offset, val);

    return 19;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8rm(uint8_t opcode) { //LD a,(r16) 7,  LD (r16), a 7, LD (**), a 13, LD a, (**) 13
    uint16_t* const regset[] = {&(bc.pair), &(de.pair), &dummy16, &dummy16};
    constexpr uint8_t index = (OPCODE>>4);
    uint64_t cycles = 7;
    if(index == 3) {
        dummy16 = memory->readWord(pc);
        pc+=2;
        cycles = 13;
        dbg_printf(" %04x", dummy16);
    }
    if((OPCODE & 0x8) == 0x8) { // read from memory
        af.hi = memory->readByte(*regset[index]);
    }
    else { // write to memory
        memory->writeByte(*regset[index], af.hi);
    }
    return cycles;
}


template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8rr(uint8_t opcode) { //LD r,r 4
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(hl.hi), &(hl.low),  &dummy8, &(af.hi)};
    constexpr uint8_t src_index = (OPCODE & 0x07);
    constexpr uint8_t dest_index = ((OPCODE>>3) & 0x07);

    uint64_t cycles = 4;

    if(src_index == 6) {
        int8_t offset = 0;
        switch(OPCODE & 0xff00) {
        case 0:
            dummy8 = memory->readByte(hl.pair);
            cycles = 7;
            break;
        case 0xdd00:
            offset = memory->readByte(pc++);
            dbg_printf(" %02x", offset);
            dummy8 = memory->readByte(ix.pair + offset);
            cycles = 19;
            break;
        case 0xfd00:
            offset = memory->readByte(pc++);
            dbg_printf(" %02x", offset);
            dummy8 = memory->readByte(iy.pair + offset);
            cycles = 19;
            break;
        }
    }

    *regset[dest_index] = *regset[src_index];

    if(dest_index == 6) {
        int8_t offset = 0;
        switch(OPCODE & 0xff00) {
        case 0:
            memory->writeByte(hl.pair, dummy8);
            cycles = 7;
            break;
        case 0xdd00:
            offset = memory->readByte(pc++);
            dbg_printf(" %02x", offset);
            memory->writeByte(ix.pair + offset, dummy8);
            cycles = 19;
            break;
        case 0xfd00:
            offset = memory->readByte(pc++);
            dbg_printf(" %02x", offset);
            memory->writeByte(iy.pair + offset, dummy8);
            cycles = 19;
            break;
        }
    }
    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8rrix(uint8_t opcode) { // DD/FD LD r,r 8
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(ix.hi), &(ix.low),  &dummy8, &(af.hi)};
    constexpr uint8_t src_index = (OPCODE & 0x07);
    constexpr uint8_t dest_index = ((OPCODE>>3) & 0x07);

    uint64_t cycles = 8;

    *regset[dest_index] = *regset[src_index];

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld8rriy(uint8_t opcode) { // DD/FD LD r,r 8
    uint8_t* const regset[] = {&(bc.hi), &(bc.low), &(de.hi), &(de.low),
                               &(iy.hi), &(iy.low),  &dummy8, &(af.hi)};
    constexpr uint8_t src_index = (OPCODE & 0x07);
    constexpr uint8_t dest_index = ((OPCODE>>3) & 0x07);

    uint64_t cycles = 8;

    *regset[dest_index] = *regset[src_index];

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_ld16(uint8_t opcode) {
    uint16_t immediate = memory->readWord(pc);
    dbg_printf(" %04x", immediate);
    pc+=2;
    uint64_t cycles = 10;
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
    case 0xdd21:
            ix.pair = immediate;
            cycles = 14;
            break;
    case 0xfd21:
            iy.pair = immediate;
            cycles = 14;
            break;
    }

    return cycles;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_neg(uint8_t opcode) { // NEG 8

    if(af.hi == 0x80) set(OVERFLOW_FLAG);
    else clear(OVERFLOW_FLAG);

    if(af.hi) set(CARRY_FLAG);
    else clear(CARRY_FLAG);

    af.hi *= -1;

    if(af.hi >= 0x80) set(SIGN_FLAG);
    else clear(SIGN_FLAG);

    if(!af.hi) set(ZERO_FLAG);
    else clear(ZERO_FLAG);

    if(af.hi & 0x0f) set(HALF_CARRY_FLAG);
    else clear(HALF_CARRY_FLAG);

    set(SUB_FLAG);

    return 8;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_nop(uint8_t opcode) { // NOP 4
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_out(uint8_t opcode) { // OUTI 16 OTIR 21/16 OUT 11
    uint8_t* const regset[] {&(bc.hi), &(bc.low), &(de.hi), &(de.low), &(hl.hi), &(hl.low), &dummy8, &(af.hi)};
    uint8_t src = ((OPCODE>>3) & 0x7);
    uint8_t port = bc.low;
    uint8_t val = 0;
    uint64_t cycles = 12;
    dummy8 = 0;

    if(OPCODE == 0xd3) { // OUT a, (*)
        port = memory->readByte(pc);
        pc++;
        src = 7; // register 'a'
        cycles = 11;
        dbg_printf(" %02x", port);
    }

    if(OPCODE > 0xed80) { // versions of the opcode that auto-inc/dec and repeat
        val = memory->readByte(hl.pair);
    }
    else {
        val = *regset[src];
    }

    memory->writePortByte(port, val, total_cycles);

    //dbg_printf(" wrote %02x to port %02x", val, port);

    if(OPCODE > 0xed80) { // versions of the opcode that auto-inc/dec and repeat
        if(OPCODE == 0xeda3 || OPCODE == 0xedb3) { //increment opcodes
            hl.pair++;
        }
        else if(OPCODE == 0xedab || OPCODE == 0xedbb) { //decrement opcodes
            hl.pair--;
        }
        bc.hi--;

        if(bc.hi && (OPCODE == 0xedb3 || OPCODE == 0xedbb)) { //repeat opcodes
            pc-=2; //repeat the instruction
            cycles = 21;
        }
        else {
            cycles = 16;
        }
    }

    return cycles;
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

template <uint32_t OPCODE> uint64_t cpuZ80::op_ret(uint8_t opcode) { // RET 10, RET cc 11/5
    uint64_t cycles = 10;
    if(OPCODE == 0xc9) {
        pc = pop();
    }
    else if (condition((OPCODE>>3) & 0x7)) {
        pc = pop();
        cycles = 11;
    }
    else {
        cycles = 5;
    }
    return cycles;
}


template <uint32_t OPCODE> uint64_t cpuZ80::op_reti(uint8_t opcode) { // RETI 14
    if(iff1) pc = pop();
    return 14;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_retn(uint8_t opcode) { // RETN 14
    pc = pop();
    iff1 = iff2;
    return 14;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_rot_a(uint8_t opcode) { // RLCA, RRCA, RLA, RRA 4
    uint8_t c = carry();
    uint8_t high = (af.hi & 0x80)>>7;
    uint8_t low = (af.hi & 0x01);
    switch(OPCODE) {
    case 0x07: //RLCA
        af.hi <<= 1;
        if(high) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        af.hi |= high;
        break;
    case 0x0f: //RRCA
        af.hi >>= 1;
        if(low) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        af.hi |= (low<<7);
        break;
    case 0x17: //RLA
        af.hi <<= 1;
        if(high) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        af.hi |= c;
        break;
    case 0x1f:
        af.hi >>= 1;
        if(low) set(CARRY_FLAG);
        else clear(CARRY_FLAG);
        af.hi |= (c << 7);
        break;
    default:
        return 0;
    }

    clear(HALF_CARRY_FLAG);
    clear(SUB_FLAG);

    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_rxd(uint8_t opcode) { // RLD, RRD 18
    uint8_t val = memory->readByte(hl.pair);
    uint8_t low_a = (af.hi & 0x0f);
    uint8_t high_a = (af.hi & 0xf0);
    uint8_t low_hl = (val & 0x0f);
    uint8_t high_hl = (val>>4);
    if(OPCODE == 0xed67) { //RRD
        af.hi = high_a | low_hl;
        val = (low_a<<4) | high_hl;
    }
    else if(OPCODE == 0xed6f) { // RLD
        af.hi = high_a | high_hl;
        val = (low_hl<<4) | low_a;
    }

    if(af.hi >= 0x80) set(SIGN_FLAG);
    else clear(SIGN_FLAG);

    if(af.hi == 0) set(ZERO_FLAG);
    else clear(ZERO_FLAG);

    clear(HALF_CARRY_FLAG);

    if(parity[af.hi]) set(PARITY_FLAG);
    else clear(PARITY_FLAG);

    clear(SUB_FLAG);

    memory->writeByte(hl.pair, val);

    return 18;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_scf(uint8_t opcode) { // SCF 4
    set(CARRY_FLAG);
    clear(HALF_CARRY_FLAG);
    clear(SUB_FLAG);
    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_sbc16(uint8_t opcode) { // SBC HL, ss 4
    uint16_t* const regset[] {&(bc.pair), &(de.pair), &(hl.pair), &(sp)};
    int reg = ((OPCODE>>4) & 0x3);
    int32_t temp = hl.pair - (*regset[reg] + carry());

    set(SUB_FLAG);
    if(temp < 0) set(CARRY_FLAG);
    else         clear(CARRY_FLAG);

    if((((*regset[reg]) + carry()) & 0xfff) > (hl.pair & 0xfff)) set(HALF_CARRY_FLAG);
    else clear(HALF_CARRY_FLAG);

    if(temp < -32768 || temp > 32767) set(OVERFLOW_FLAG);
    else clear(OVERFLOW_FLAG);

    if(!temp) set(ZERO_FLAG);
    else clear(ZERO_FLAG);

    hl.pair = temp;

    return 4;
}

template <uint32_t OPCODE> uint64_t cpuZ80::op_wtf(uint8_t arg) {
    std::printf("WTF opcode: %08x, arg: %02x, next: %02x(PC %04x) next+1: %02x", OPCODE, arg, memory->readByte(pc), pc, memory->readByte(pc+1));
    return 4;
}
