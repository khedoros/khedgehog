#include<memory>
#include<iostream>
#include "emulator.h"
#include "config.h"

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
}

int genesisEmulator::run() {
    std::cout<<"Hi, I'm the Genesis emulator!"<<std::endl;
    return -1;
}

smsEmulator::smsEmulator(std::shared_ptr<config> config) {
    cfg = config;
}

int smsEmulator::run() {
    std::cout<<"Hi, I'm the Master System emulator!"<<std::endl;
    return -1;
}

ggEmulator::ggEmulator(std::shared_ptr<config> config) {
    cfg = config;
}

int ggEmulator::run() {
    std::cout<<"Hi, I'm the Game Gear emulator!"<<std::endl;
    return -1;
}
