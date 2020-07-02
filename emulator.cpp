#include<memory>
#include<iostream>

#include "emulator.h"
#include "config.h"
#include "memmap.h"
#include "cpu/genesis/memmapGenesisCpu.h"
#include "cpu/m68k/cpuM68k.h"
#include "apu/genesis/apuGenesis.h"
#include "io/ioMgr.h"
#include "vdp/genesis/vdpGenesis.h"

std::shared_ptr<emulator> emulator::getEmulator(std::shared_ptr<config> cfg) {
    switch(cfg->getSystemType()) {
        case masterSystem: return std::make_shared<smsEmulator>(cfg);
        case gameGear: return std::make_shared<ggEmulator>(cfg);
        case genesis: return std::make_shared<genesisEmulator>(cfg);
        default: std::cerr<<"Not supposed to be here ;-) I hereby declare this game...a Genesis game!!"<<std::endl;
    }
    return std::make_shared<genesisEmulator>(cfg);
}

genesisEmulator::genesisEmulator(std::shared_ptr<config> config) {
    cfg = config;
    cpu_map = std::make_shared<memmapGenesisCpu>(config);
    cpu_dev = std::make_shared<cpuM68k>(cpu_map);
    apu_dev = std::make_shared<apuGenesis>();
    io = std::make_shared<ioMgr>();
    vdp_dev = std::make_shared<vdpGenesis>();
}

int emulator::run() {
    uint64_t clock_total_cycles = 0;
    bool running = true;
    bool paused = false;
    bool muted = false;

    while(running) {
        //process events
        while(paused) {
            //process events
            //wait a frame
        }
        uint64_t cycle_chunk = cpu_dev->calc(1024);
        if(cycle_chunk == 0) {
            running = false;
            std::cerr<<"Found a bad op, I guess?\n";
        }
        //run VDP
        //run APU
        //pause for effect
        clock_total_cycles += cycle_chunk;
    }

    return 0;
}

int genesisEmulator::run() {
    std::cout<<"Hi, I'm the Genesis emulator!"<<std::endl;
    return emulator::run();
}

smsEmulator::smsEmulator(std::shared_ptr<config> config) {
    cfg = config;
}

int smsEmulator::run() {
    std::cout<<"Hi, I'm the Master System emulator!"<<std::endl;
    emulator::run();
    return -1;
}

ggEmulator::ggEmulator(std::shared_ptr<config> config) {
    cfg = config;
}

int ggEmulator::run() {
    std::cout<<"Hi, I'm the Game Gear emulator!"<<std::endl;
    emulator::run();
    return -1;
}
