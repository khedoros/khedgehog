#include<iostream>
#include<memory>

#include "config.h"
#include "emulator.h"

int main(int argc, char **argv) {
    auto cfg = std::make_shared<config>(argc, argv);
    auto emu = emulator::getEmulator(cfg);
    return emu->run();
}
