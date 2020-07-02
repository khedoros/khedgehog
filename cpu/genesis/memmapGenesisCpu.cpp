#include<fstream>
#include<iostream>

#include "memmapGenesisCpu.h"

/* Mapping
 * Component     |Start Addr| End Addr |
 * Cartridge       0x000000   0x3FFFFF
 * Sega CD         0x400000   0x7FFFFF
 * Sega 32x        0x800000   0x9FFFFF
 * Z80 addr space  0xA00000   0xA0FFFF
 * I/O ports       0xA10000   0xA10FFF
 * Z80 control     0xA11000   0xAFFFFF
 * Unallocated     0xB00000   0xBFFFFF
 * VDP             0xC00000   0xDFFFFF
 * 68K RAM         0xE00000   0xFFFFFF (mirrored every 64KiB)
 */

 memmapGenesisCpu::memmapGenesisCpu(std::shared_ptr<config> cfg): valid(false) {
    std::ifstream romfile(cfg->getRomPath().c_str());
    if(!romfile.is_open()) {
        std::cerr<<"Couldn't open ROM at path \""<<cfg->getRomPath()<<"\"\n";
        return;
    }
    romfile.seekg(0,std::ios::end);
    std::size_t filesize = romfile.tellg();
    romfile.seekg(0,std::ios::beg);
    if(filesize < rom.size()) {
        romfile.read(reinterpret_cast<char *>(rom.data()), filesize);
        romfile.close();
    }
    valid = true;
    std::cout<<"Opened ROM at path \""<<cfg->getRomPath()<<"\" with filesize "<<filesize<<" bytes.\n";
}

uint8_t& memmapGenesisCpu::readByte(uint32_t addr) {
    if(addr < 0x400000) {
        return rom[addr];
    }
    else {
        std::cerr<<"Unmapped address encountered: 0x"<<std::hex<<addr<<"\n";
        return dummyByte;
    }
}

uint16_t& memmapGenesisCpu::readWord(uint32_t addr) {
    if(addr < 0x400000) {
        return reinterpret_cast<uint16_t&>(rom[addr]);
        //return uint16_t(rom[addr]) * 256 + uint16_t(rom[addr+1]);
    }
    else {
        std::cerr<<"Unmapped address encountered: 0x"<<std::hex<<addr<<"\n";
        return dummyWord;
    }
}

uint32_t& memmapGenesisCpu::readLong(uint32_t addr) {
    if(addr < 0x400000) {
        return reinterpret_cast<uint32_t&>(rom[addr]);
        //return (uint32_t(rom[addr])<<(24)) + (uint32_t(rom[addr + 1])<<(16)) + (uint32_t(rom[addr + 2])<<(8)) + rom[addr + 3];
    }
    else {
        std::cerr<<"Unmapped address encountered: 0x"<<std::hex<<addr<<"\n";
        return dummyLong;
    }
}

void memmapGenesisCpu::writeByte(uint32_t addr, uint8_t val) {}
void memmapGenesisCpu::writeWord(uint32_t addr, uint16_t val) {}
void memmapGenesisCpu::writeLong(uint32_t addr, uint32_t val) {}
