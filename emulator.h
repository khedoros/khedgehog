#pragma once
#include<memory>

//#include "io/ioMgr.h"

class config;
class ioMgr;
class apu;
class ti_psg;
class yamaha_fm;
class memmap_apu;
class cpu_z80;
class memmap_m68k;
class cpu_m68k;
class input_mapper;
class memmap_vdp;
class vdp;

class emulator {
public:
    static std::shared_ptr<emulator> getEmulator(std::shared_ptr<config> cfg);
    virtual int run() = 0;
protected:
    std::shared_ptr<config> cfg;
    std::shared_ptr<ioMgr>  io;
};

class genesisEmulator: public emulator {
public:
    genesisEmulator(std::shared_ptr<config> cfg);
    int run() override;
private:
    std::shared_ptr<apu>            apu_dev;
    std::shared_ptr<ti_psg>         psg_dev;
    std::shared_ptr<yamaha_fm>      yama_dev;
    std::shared_ptr<memmap_apu>     apu_map;
    std::shared_ptr<cpu_z80>        apu_cpu;

    std::shared_ptr<memmap_m68k>    cpu_map;
    std::shared_ptr<cpu_m68k>       main_cpu;
    std::shared_ptr<input_mapper>   input_map;
    std::shared_ptr<ioMgr>          io_manager;

    std::shared_ptr<memmap_vdp>     vdp_map;
    std::shared_ptr<vdp>            vdp_dev;
};

class smsEmulator: public emulator {
public:
    smsEmulator(std::shared_ptr<config> cfg);
    int run() override;
private:
};

class ggEmulator: public emulator {
public:
    ggEmulator(std::shared_ptr<config> cfg);
    int run() override;
private:
};

