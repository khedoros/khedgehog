#include "memmapGGCpu.h"
#include<iostream>
#include<fstream>

memmapGGCpu::memmapGGCpu(std::shared_ptr<config> cfg) {
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

uint8_t& memmapGGCpu::readByte(uint32_t addr) {
    return map(addr);
}

uint16_t& memmapGGCpu::readWord(uint32_t addr) {
    return reinterpret_cast<uint16_t&>(map(addr));
}

uint32_t& memmapGGCpu::readLong(uint32_t addr) {
    return reinterpret_cast<uint32_t&>(map(addr));
}

void memmapGGCpu::writeByte(uint32_t addr, uint8_t val) {}
void memmapGGCpu::writeWord(uint32_t addr, uint16_t val) {}
void memmapGGCpu::writeLong(uint32_t addr, uint32_t val) {}

uint8_t& memmapGGCpu::map(uint32_t addr) {
    if(addr < 0xC000) {
        return rom[addr];
    }
    else if(addr < 0xE000) {
        return ram[addr];
    }
    else {
        std::cerr<<"Unmapped address encountered: 0x"<<std::hex<<addr<<"\n";
        return reinterpret_cast<uint8_t&>(dummyVal);
    }
}
