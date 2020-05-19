#include "config.h"

config::config(int argc, char **argv, char *config_file) {
    // TODO: parse command-line arguments
    if(argc == 2) {
        romPath = std::string(argv[1]);
    }
    else {
        romPath = std::string("dummy.bin");
    }
    // TODO: verify existence of ROM file
    type = detectRomType();
    region = detectRomRegion();
}

systemType config::getSystemType() {
    return type;
}

systemRegion config::getSystemRegion() {
    return region;
}

std::string& config::getRomPath() {
    return romPath;
}

systemType config::detectRomType() {
    //TODO: detect ROM type from file
    return genesis;
}

systemRegion config::detectRomRegion() {
    //TODO: detect ROM region from file
    return ntsc;
}
