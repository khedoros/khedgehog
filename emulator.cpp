#include<memory>
#include<iostream>

#include "emulator.h"
#include "config.h"
#include "io/ioMgr.h"
#include "io/ioEvent.h"
#include "memmap.h"
#include "cpu/m68k/memmapM68k.h"
#include "cpu/z80/memmapZ80Console.h"
#include "cpu/m68k/cpuM68k.h"
#include "cpu/z80/cpuZ80.h"
#include "apu/genesis/apuGenesis.h"
//#include "apu/masterSystem/apuMS.h"
#include "apu/tiPsg.h"
#include "vdp/genesis/vdpGenesis.h"
#include "vdp/masterSystem/vdpMS.h"


std::shared_ptr<emulator> emulator::getEmulator(std::shared_ptr<config> cfg) {
    switch(cfg->getSystemType()) {
        case sg_1000: return std::make_shared<smsEmulator>(cfg);
        case masterSystem: return std::make_shared<smsEmulator>(cfg);
        case gameGear: return std::make_shared<smsEmulator>(cfg);
        case genesis: return std::make_shared<genesisEmulator>(cfg);
        case invalidSystem: std::cerr<<"Detected an invalid system type.\n"; break;
        case uncheckedSystem: std::cerr<<"System type hasn't been checked.\n"; break;
        default: std::cerr<<"Not supposed to be here ;-) I hereby declare this game...a Genesis game!!\n"; break;
    }
    return std::make_shared<noEmulator>(cfg);
}

genesisEmulator::genesisEmulator(std::shared_ptr<config> config) {
    cfg = config;
    io = std::make_shared<ioMgr>(cfg);
    cpu_map = std::make_shared<memmapM68k>(config);
    cpu_dev = std::make_shared<cpuM68k>(std::dynamic_pointer_cast<memmapM68k>(cpu_map));
    apu_dev = std::make_shared<apuGenesis>();
    vdp_dev = std::make_shared<vdpGenesis>(cfg->getSystemType(), cfg->getSystemRegion());
}

int emulator::run() {
    uint64_t clock_total_cycles = 0;
    bool running = true;
    bool paused = false;
    bool muted = false;

    //unsigned int debugWindow = io->createWindow(1024, 1024, "Khedgehog Debug Window");
    unsigned int mainWindow = 0;

    uint64_t line = 0;
    while(running) {
        //process events
        ioEvent e = io->getEvent();
        while(e.type != ioEvent::eventType::none) {
            if(e.type == ioEvent::eventType::window && e.key.winEvent == ioEvent::windowEvent::exit) {
                return 0;
            }
            else if(e.type == ioEvent::eventType::smsKey || e.type == ioEvent::eventType::genKey) {
                cpu_map->sendEvent(e);
            }
            e = io->getEvent();
        }
        while(paused) {
            //process events
            e = io->getEvent();
            while(e.type != ioEvent::eventType::none) {
                if(e.type == ioEvent::eventType::window && e.key.winEvent == ioEvent::windowEvent::exit) {
                    return 0;
                }
                e = io->getEvent();
            }
            //wait a frame
        }
        // 342 clock pixels per line, VDP clock has a 3:2 relation to CPU clock
        uint64_t cycle_chunk = cpu_dev->calc((342 * 2) / 3);
        vdp_dev->endLine(line++);
        if(cycle_chunk == 0) {
            running = false;
            std::cerr<<"Found a bad op, I guess?\n";
        }
        if(vdp_dev->frameInterrupt() || vdp_dev->lineInterrupt()) {
            cpu_dev->interrupt(0);
        }
        if(line == 192) {
            //vdp_dev->calc(cycle_chunk); //run VDP for amount matching the CPU
            //io -> updateWindow(mainWindow, 0, 0, vdp_dev->getPartialRender());
            io -> updateWindow(mainWindow, 0, 0, vdp_dev->getFrameBuffer());
            //io -> updateWindow(debugWindow, 0, 0, vdp_dev->getDebugRender());
        }
        else if(line == 262) {
            line = 0;
        }
        //run APU
        //pause for effect
        clock_total_cycles += cycle_chunk;
    }

    return 0;
}

int genesisEmulator::run() {
    std::cout<<"Hi, I'm the Genesis emulator!"<<std::endl;
    emulator::run();
    return 0;
}

smsEmulator::smsEmulator(std::shared_ptr<config> config) {
    cfg = config;
    io = std::make_shared<ioMgr>(cfg);
    apu_dev = std::make_shared<TiPsg>();
    vdp_dev = std::make_shared<vdpMS>(cfg->getSystemType(), cfg->getSystemRegion());
    cpu_map = std::make_shared<memmapZ80Console>(config, vdp_dev, std::dynamic_pointer_cast<TiPsg>(apu_dev));
    cpu_dev = std::make_shared<cpuZ80>(std::dynamic_pointer_cast<memmapZ80Console>(cpu_map));
}

int smsEmulator::run() {
    std::cout<<"Hi, I'm the Master System emulator!"<<std::endl;
    emulator::run();
    return 0;
}

noEmulator::noEmulator(std::shared_ptr<config> cfg) {

}

int noEmulator::run() {
    std::cout<<"ROM type wasn't recognized. Exiting."<<std::endl;
    return 1;
}
