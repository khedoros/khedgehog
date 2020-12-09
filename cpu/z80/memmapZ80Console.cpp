#include "memmapZ80Console.h"
#include "../../vdp/masterSystem/vdpMS.h"
#include "../../apu/masterSystem/apuMS.h"
#include<iostream>
#include<fstream>
#include "../../util.h"
#include "../../debug_console.h"

memmapZ80Console::memmapZ80Console(std::shared_ptr<config> cfg, std::shared_ptr<vdpMS> v, std::shared_ptr<apuMS> a) : map_ctrl(0), map_slot0(0), map_slot1(1), map_slot2(2), vdp(v), apu(a) {
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
    dbg_con::init();
}

uint8_t& memmapZ80Console::readByte(uint32_t addr) {
    return map(addr);
}

uint8_t memmapZ80Console::readPortByte(uint8_t port) {
    // TODO: Implement :-D
    dbg_printf(" read port %02x >> [dummy]", port);
    switch(port) {
        case 0x7e: dbg_printf(" (V counter)"); break;
        case 0x7f: dbg_printf(" (H counter)"); break;
        case 0xbd: case 0xbf: dbg_printf(" (read VDP status bits)"); break;
        case 0xbe: dbg_printf(" (read VDP data)"); break;
        case 0xc0: case 0xdc: dbg_printf(" (joystick port 1)"); break;
        case 0xc1: case 0xdd: dbg_printf(" (joystick port 2 + nationalization)"); break;
        case 0xde: case 0xdf: dbg_printf(" (unknown port)"); break;
        case 0xf2: dbg_printf(" (YM2413 control register)"); break;
        default: dbg_printf(" (no info on port)"); break;
    }
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

void memmapZ80Console::writePortByte(uint8_t port, uint8_t val) {
    dbg_printf(" wrote %02x to port %02x", val, port);
    switch(port) {
        case 0x3f: dbg_printf(" (automatic nationalization)"); break;
        case 0x7e: case 0x7f: dbg_printf(" (PSG SN76489 output control)"); break;
        case 0xbd: case 0xbf:
            vdp->writeAddress(val);
            dbg_printf(" (VDP address/register)");
            break;
        case 0xbe:
            vdp->writeData(val);
            dbg_printf(" (VDP data)");
            break;
        case 0xde: case 0xdf: dbg_printf(" (unknown port)"); break;
        case 0xf0: dbg_printf(" (YM2413 address register)"); break;
        case 0xf1: dbg_printf(" (YM2413 data register)"); break;
        case 0xf2: dbg_printf(" (YM2413 control register)"); break;
        case 0xfc: dbg_con::write_control(val); break;
        case 0xfd: dbg_con::write_data(val); break;
        default: dbg_printf(" (no info on port)"); break;
    }
}

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
