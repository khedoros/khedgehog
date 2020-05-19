#pragma once
#include<string>
#include "emulator.h"

enum systemType {
    masterSystem,
    gameGear,
    genesis
};

enum systemRegion {
    pal,
    ntsc
};

class config {
public:
    config(int argc, char **argv, char *config_file = nullptr);
    systemType getSystemType();
    systemRegion getSystemRegion();
    std::string& getRomPath(); 
private:
    constexpr static const char* default_cfg_file = "khedgehog.ini";
    systemType detectRomType();
    systemRegion detectRomRegion();
    systemType type;
    systemRegion region;
    std::string romPath;
};

