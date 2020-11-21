/*#include "memmapZ80Console.h"

uint8_t memmapZ80Console::readByte(uint32_t addr) {
    return dummy8;
}

uint16_t memmapZ80Console::readWord(uint32_t addr) {
    return dummy16;
}

uint32_t memmapZ80Console::readLong(uint32_t addr) {
    return dummy32;
}

void memmapZ80Console::writeByte(uint32_t addr, uint8_t val) {}
void memmapZ80Console::writeWord(uint32_t addr, uint16_t val) {}
void memmapZ80Console::writeLong(uint32_t addr, uint32_t val) {}
*/

#include "memmapZ80Console.h"
#include<iostream>
#include<fstream>

memmapZ80Console::memmapZ80Console(std::shared_ptr<config> cfg) : map_ctrl(0), map_slot0(0), map_slot1(1), map_slot2(2) {
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

uint8_t& memmapZ80Console::readByte(uint32_t addr) {
    return map(addr);
}

uint8_t memmapZ80Console::readPortByte(uint8_t port) {
    // TODO: Implement :-D
    return 0xff;
}

uint16_t& memmapZ80Console::readWord(uint32_t addr) {
    return reinterpret_cast<uint16_t&>(map(addr));
}

uint32_t& memmapZ80Console::readLong(uint32_t addr) {
    return reinterpret_cast<uint32_t&>(map(addr));
}

void memmapZ80Console::writeByte(uint32_t addr, uint8_t val) {
    if(addr >= 0xC000) {
        ram[addr & 0x1fff] = val;
    }
}
void memmapZ80Console::writeWord(uint32_t addr, uint16_t val) {
    if(addr >= 0xC000) {
        ram[addr & 0x1fff] = (val & 0xff);
        ram[(addr + 1) & 0x1fff] = (val >> 8);
    }
}
void memmapZ80Console::writeLong(uint32_t addr, uint32_t val) {}

void memmapZ80Console::writePortByte(uint8_t port, uint8_t val) {}

uint8_t& memmapZ80Console::map(uint32_t addr) {
    if(addr < 0x0400) { // unpaged rom
        return rom[addr];
    }
    else if(addr < 0x4000) { // slot0 rom
        return rom[(addr & 0x3fff) + map_slot0 * 0x4000];
    }
    else if(addr < 0x8000) { // slot1 rom
        return rom[(addr & 0x3fff) + map_slot1 * 0x4000];
    }
    else if(addr < 0xC000) { // slot2 rom TODO: implement rom-ram swapout
        return rom[(addr & 0x3fff) + map_slot2 * 0x4000];
    }
    else if(addr < 0x10000) { // system ram TODO: implement paging control
        return ram[addr & 0x1fff];
    }
    else {
        std::cerr<<"Unmapped address encountered: 0x"<<std::hex<<addr<<"\n";
        return reinterpret_cast<uint8_t&>(dummyLong);
    }
}