#include<memory>
#include "emulator.h"

std::shared_ptr<emulator> emulator::getEmulator(std::shared_ptr<config> cfg) {
    return std::make_shared<genesisEmulator>(cfg);
}

genesisEmulator::genesisEmulator(std::shared_ptr<config> cfg) {}
int genesisEmulator::run() { return -1; }

smsEmulator::smsEmulator(std::shared_ptr<config> cfg) {}
int smsEmulator::run() { return -1; }

ggEmulator::ggEmulator(std::shared_ptr<config> cfg) {}
int ggEmulator::run() { return -1; }
