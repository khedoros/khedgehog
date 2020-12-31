#include "config.h"
#include<filesystem>
#include<iostream>
#include<fstream>
#include<cstring>

config::config(int argc, char **argv, char *config_file) {
    // TODO: parse command-line arguments
    if(argc == 2) {
        romPath = std::string(argv[1]);
    }

    // TODO: verify existence of ROM file
    std::filesystem::path path(romPath);
    if(std::filesystem::exists(path) &&
       std::filesystem::is_regular_file(path)) {
        romSize = std::filesystem::file_size(path);
    }
    else {
        std::cerr<<"Couldn't open file at path \""<<romPath<<"\".\n";
        return;
    }

    type = detectRomType();
    region = detectRomRegion();
    std::cout<<"Header found at: "<<headerOffset<<".\n";
}

systemType config::getSystemType() {
    if(type == uncheckedSystem) {
        type = detectRomType();
    }
    return type;
}

systemRegion config::getSystemRegion() {
    if(region == uncheckedRegion) {
        region = detectRomRegion();
    }
    return region;
}

std::string& config::getRomPath() {
    return romPath;
}

systemType config::detectRomType() {
    char magic[17] = {0};
    std::ifstream rom(romPath);

    // Check for Genesis header
    rom.seekg(0x100);
    rom.read(magic, 0x10);
    if(genesisName == magic || megadriveName == magic) {
        headerOffset = 0x100;
        return systemType::genesis;
    }

    // Check for SMS/GG header
    headerOffset = 0x0;
    systemType detected = systemType::uncheckedSystem;
    for(auto offset: smsGgHeaderOffset) {
        if(offset < romSize) {
            rom.seekg(offset);
            rom.read(magic, 0x10);
            if(std::strncmp(magic, smsGgMagic.c_str(), 8) == 0) {
                headerOffset = offset;
                detected = systemType::uncheckedSystem;
                break;
            }
            else detected = systemType::invalidSystem;
        }
    }

    // SG-1000 games are headerless, but have specific sizes.
    if(detected == systemType::invalidSystem && romSize <= 48 * 1024 && romSize % 8192 == 0) {
            return systemType::sg_1000;
    }
    else if(detected == systemType::invalidSystem) {
        return systemType::invalidSystem;
    }

    // Interpret SMS/GG header for system type
    uint8_t smsGgType = (magic[0xf] & 0xf0);
    std::cout<<"SMS/GG System type: "<<std::hex<<uint32_t(smsGgType)<<"\n";
    switch(smsGgType) {
        case 0x30: case 0x40:
            return systemType::masterSystem;
        case 0x50: case 0x60: case 0x70:
            return systemType::gameGear;
        default:
            return systemType::invalidSystem;
    }
}

systemRegion config::detectRomRegion() {
    //TODO: detect ROM region from file
    return ntsc;
}

size_t config::getHeaderOffset() {
    return headerOffset;
}
/*
int main() {
    std::cout<<"Hello"<<std::endl;
}
*/
