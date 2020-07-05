#pragma once
#include<string>
#include<array>
#include "emulator.h"

enum systemType {
    uncheckedSystem,
    masterSystem,
    gameGear,
    genesis,
    invalidSystem
};

enum systemRegion {
    uncheckedRegion,
    pal,
    ntsc,
    invalidRegion
};

class config {
public:
    config(int argc, char **argv, char *config_file = nullptr);
    systemType getSystemType();
    systemRegion getSystemRegion();
    std::string& getRomPath();
    size_t getHeaderOffset();
private:
    constexpr static const char* default_cfg_file = "khedgehog.ini";
    systemType detectRomType();
    systemRegion detectRomRegion();
    systemType type = systemType::uncheckedSystem;
    systemRegion region = systemRegion::uncheckedRegion;
    std::string romPath = "dummy.bin";
    const std::string genesisName = "SEGA GENESIS    ";
    const std::string megadriveName = "SEGA MEGA DRIVE ";
    const std::string smsGgMagic = "TMR SEGA";
    const std::array<size_t, 3> smsGgHeaderOffset {0x1ff0, 0x3ff0, 0x7ff0};
    size_t romSize = 0;
    size_t headerOffset = 0;
};

