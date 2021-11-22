#include<fstream>
#include<iostream>
#include "memmapM68k.h"
#include "../../util.h"

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

 memmapM68k::memmapM68k(std::shared_ptr<config> cfg): valid(false) {
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

uint8_t& memmapM68k::readByte(uint32_t addr) {
    return map(addr);
}

uint16_t& memmapM68k::readWord(uint32_t addr) {
    return reinterpret_cast<uint16_t&>(map(addr));
}

uint32_t& memmapM68k::readLong(uint32_t addr) {
    return reinterpret_cast<uint32_t&>(map(addr));
}



void memmapM68k::writeByte(uint32_t addr, uint8_t val) {}
void memmapM68k::writeWord(uint32_t addr, uint16_t val) {}
void memmapM68k::writeLong(uint32_t addr, uint32_t val) {}

void memmapM68k::sendEvent(ioEvent e) {
    // React to key events
}

uint8_t& memmapM68k::map(uint32_t addr) {
    if(addr < 0x400000) {
        return rom[addr];
    }
    else if(addr >= 0xa00000 && addr < 0xa10000) {
        std::cerr<<"Unmapped Z80 address encountered: 0x"<<std::hex<<addr<<"\n";
    }
    else if(addr >= 0xa10000 && addr < 0xa11000) {
        std::cerr<<"Unmapped I/O address encountered: 0x"<<std::hex<<addr<<"\n";
    }
    else if(addr >= 0xa11000 && addr < 0xa12000) {
        std::cerr<<"Unmapped Control address encountered: 0x"<<std::hex<<addr<<"\n";
    }
    else if(addr >= 0xd00000 && addr < 0xe00000) {
        std::cerr<<"Unmapped VDP address encountered: 0x"<<std::hex<<addr<<"\n";
    }
    else if(addr >= 0xff0000 && addr < 0x1000000) {
        return ram[addr&0xffff];
    }
    else {
        std::cerr<<"Unmapped address encountered: 0x"<<std::hex<<addr<<"\n";

    }
    return reinterpret_cast<uint8_t&>(dummyVal);
}

