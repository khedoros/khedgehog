#pragma once
#include<memory>

//#include "io/ioMgr.h"

class config;
class ioMgr;
class apu;
class apuGenesis;
class apuGG;
class apuMS;
class tiPsg;
class yamahaFm;
class memmapApu;
class cpu;
class cpuZ80;
class cpuM68k;
class memmapM68k;
class memmapZ80Console;
class inputMapper;
class inputMapperGenesis;
class inputMapperGG;
class inputMapperMS;
class vdp;
class vdpGenesis;
class vdpGG;
class vdpMS;
class memmapVdp;
class memmapGenesisVdp;
class memmapGGVdp;
class memmapMSVdp;
class memmap;

class emulator {
public:
    static std::shared_ptr<emulator> getEmulator(std::shared_ptr<config> cfg);
    virtual int run();
protected:
    std::shared_ptr<config> cfg;
    std::shared_ptr<ioMgr>  io;
    std::shared_ptr<apu>    apu_dev;
    std::shared_ptr<cpu>    cpu_dev;
    std::shared_ptr<vdp>    vdp_dev;
    std::shared_ptr<inputMapper> input_map; //child of memmapCpu
    std::shared_ptr<memmap> cpu_map;
};

class genesisEmulator: public emulator {
public:
    genesisEmulator(std::shared_ptr<config> cfg);
    int run() override;
private:
    std::shared_ptr<memmapM68k> cpu_map; //child of cpu
    //std::shared_ptr<apu>            apu_dev;
    //std::shared_ptr<ti_psg>         psg_dev;
    //std::shared_ptr<yamaha_fm>      yama_dev;
    //std::shared_ptr<memmap_apu>     apu_map;
    //std::shared_ptr<cpu_z80>        apu_cpu;

    //std::shared_ptr<memmap_m68k>    cpu_map;
    //std::shared_ptr<cpu_m68k>       main_cpu;
    //std::shared_ptr<input_mapper>   input_map;
    //std::shared_ptr<ioMgr>          io_manager;

    //std::shared_ptr<memmap_vdp>     vdp_map;
    //std::shared_ptr<vdp>            vdp_dev;
};

class smsEmulator: public emulator {
public:
    smsEmulator(std::shared_ptr<config> cfg);
    int run() override;
private:
    std::shared_ptr<vdpMS> vdp_dev;
    std::shared_ptr<apuMS> apu_dev;
    std::shared_ptr<memmapZ80Console> cpu_map;
};

class noEmulator: public emulator {
public:
    noEmulator(std::shared_ptr<config> cfg);
    int run() override;
};
