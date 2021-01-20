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
    resolution = getResolution(type,region);
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

std::pair<int,int> config::getResolution() {
    return getResolution(type, region);
}

std::pair<int,int> config::getResolution(systemType t, systemRegion r) {
    switch(t) {
    case systemType::gameGear:
        return std::make_pair(160, 144);
    case systemType::sg_1000: case systemType::masterSystem:
        return std::make_pair(256,192);
    case systemType::genesis:
        return std::make_pair(256,224);
        // NTSC mode 5: 32x28 CELL image (256x224 pixels) or 40x28 (320x224 pixels)
        // PAL mode 5 also has: 32x30 (256x240 pixels) or 40x30 (320x240 pixels)
    default:
        return std::make_pair(0,0);
    }
}

std::string& config::getRomPath() {
    return romPath;
}

systemType config::detectRomType() {
    std::string ending = romPath.substr(romPath.size() - 3);
    if(ending == "sms") return systemType::masterSystem;
    else if(ending == "bin" || ending == ".md") return systemType::genesis;
    else if(ending == ".gg") return systemType::gameGear;
    else if(ending == ".sg") return systemType::sg_1000;
    else {
        std::cout<<"Invalid based on file ending: "<<ending<<".\n";
        return systemType::invalidSystem;
    }

    /*
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
    */
}

systemRegion config::detectRomRegion() {
    //TODO: detect ROM region from file
    return ntsc;
}

size_t config::getHeaderOffset() {
    return headerOffset;
}

