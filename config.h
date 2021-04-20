#pragma once
#include<string>
#include<array>
#include<tuple>
#include "emulator.h"

//#define DISABLE_AUDIO
//#define AUDIO_RAW_OUT
enum systemType {
    uncheckedSystem,
    sg_1000,
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
    std::pair<int,int> getResolution();
    std::pair<int,int> getResolution(systemType,systemRegion);
    unsigned int getFrameLines();
    std::string& getRomPath();
    size_t getHeaderOffset();
private:
    constexpr static const char* default_cfg_file = "khedgehog.ini";
    systemType detectRomType();
    systemRegion detectRomRegion();
    systemType type = systemType::uncheckedSystem;
    systemRegion region = systemRegion::uncheckedRegion;
    std::pair<int,int> resolution;
    std::string romPath = "dummy.bin";
    const std::string genesisName = "SEGA GENESIS    ";
    const std::string megadriveName = "SEGA MEGA DRIVE ";
    const std::string smsGgMagic = "TMR SEGA";
    const std::array<size_t, 3> smsGgHeaderOffset {0x1ff0, 0x3ff0, 0x7ff0};
    size_t romSize = 0;
    size_t headerOffset = 0;
};

