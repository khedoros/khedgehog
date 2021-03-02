#include "memmapZ80Console.h"
#include "../../vdp/masterSystem/vdpMS.h"
#include "../../apu/masterSystem/apuMS.h"
#include "../../apu/tiPsg.h"
#include<iostream>
#include<fstream>
#include "../../util.h"
#include "../../debug_console.h"

memmapZ80Console::memmapZ80Console(std::shared_ptr<config> conf, std::shared_ptr<vdp> v, std::shared_ptr<TiPsg> a) : 
    map_ctrl(0), map_slot0_offset(0), map_slot1_offset(1 * 0x4000), map_slot2_offset(2 * 0x4000), 
    vdp_dev(v), apu_dev(a), cfg(conf), slot2RamActive(false), slot2RamPage(0)
    {
    ram.fill(0);
    std::ifstream romfile(cfg->getRomPath().c_str());
    if(!romfile.is_open()) {
        std::cerr<<"Couldn't open ROM at path \""<<cfg->getRomPath()<<"\"\n";
        return;
    }
    romfile.seekg(0,std::ios::end);
    romsize = romfile.tellg();
    romfile.seekg(0,std::ios::beg);
    if(romsize < rom.size()) {
        romfile.read(reinterpret_cast<char *>(rom.data()), romsize);
        romfile.close();
    }
    valid = true;
    std::cout<<"Opened ROM at path \""<<cfg->getRomPath()<<"\" with filesize "<<romsize<<" bytes.\n";
    dbg_con::init();
}

uint8_t& memmapZ80Console::readByte(uint32_t addr) {
    return map(addr);
}

uint8_t memmapZ80Console::readPortByte(uint8_t port, uint64_t cycle) {
    // TODO: Implement :-D
    dbg_printf(" read port %02x >> [dummy]", port);
    if(port == 0 && cfg->getSystemType() == systemType::gameGear) {
        return 0x7f | (!(gg_port_0.start))<<7;
    }
    switch(port & 0b11000001) {
        case 0x00:
            dbg_printf(" (memory control register)");
            return 0xff;
        case 0x01:
            dbg_printf(" (i/o control register)");
            return 0xff;
        case 0x40:
            dbg_printf(" (V counter)");
            return vdp_dev->readByte(port, cycle);
        case 0x41:
            dbg_printf(" (H counter)");
            return vdp_dev->readByte(port, cycle);
        case 0x80:
            dbg_printf(" (read VDP data)");
            return vdp_dev->readByte(port, cycle);
        case 0x81:
            dbg_printf(" (read VDP status bits)");
            return vdp_dev->readByte(port, cycle);
        // Note: Probably need to be implemented as an io controller device
        case 0xc0:
            dbg_printf(" (joystick port 1)");
            return (
                    ((io_port_ab.port_a_up) ? 0 : p1_up) |
                    ((io_port_ab.port_a_down) ? 0 : p1_down) |
                    ((io_port_ab.port_a_left) ? 0 : p1_left) |
                    ((io_port_ab.port_a_right) ? 0 : p1_right) |
                    ((io_port_ab.port_a_tl) ? 0 : p1_b1) |
                    ((io_port_ab.port_a_tr) ? 0 : p1_b2) |
                    ((io_port_ab.port_b_up) ? 0 : p2_up) |
                    ((io_port_ab.port_b_down) ? 0 : p2_down) 
                   );
        case 0xc1:
            dbg_printf(" (joystick port 2 + nationalization)");
            break;
        //case 0xf2: dbg_printf(" (YM2413 status register)"); break;
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
    if(addr >= 0x8000 && addr < 0xc000 && slot2RamActive) {
        cartRam[addr & 0x1fff] = val;
    }
    else if(addr >= 0xC000) {
        ram[addr & 0x1fff] = val;
    }
    switch(addr) {
    case 0xfffc:
        slot2RamActive = (val & 0b00001000)? true: false;
        slot2RamPage = (val & 0b00000100)? 1: 0;
        //std::printf("Wrote %02x to %04x. Exiting.\n", val, addr);
        std::cerr<<"Slot 2 RAM: "<<std::hex<<int(val)<<" (active: "<<slot2RamActive<<", page: "<<int(slot2RamPage)<<"\n";
        //exit(1);
        break;
    case 0xfffd:
        map_slot0_offset = (0x4000 * val) % romsize;
        std::cerr<<"Slot 0 page: "<<std::hex<<int(val)<<"\n";
        break;
    case 0xfffe:
        map_slot1_offset = (0x4000 * val) % romsize;
        std::cerr<<"Slot 1 page: "<<std::hex<<int(val)<<"\n";
        break;
    case 0xffff:
        map_slot2_offset = (0x4000 * val) % romsize;
        std::cerr<<"Slot 2 page: "<<std::hex<<int(val)<<"\n";
        break;
    }
}
void memmapZ80Console::writeWord(uint32_t addr, uint16_t val) {
    if(addr >= 0xC000) {
        ram[addr & 0x1fff] = (val & 0xff);
        ram[(addr + 1) & 0x1fff] = (val >> 8);
    }
}
void memmapZ80Console::writeLong(uint32_t addr, uint32_t val) {}

void memmapZ80Console::writePortByte(uint8_t port, uint8_t val, uint64_t cycle) {
//    dbg_printf(" wrote %02x to port %02x", val, port);
    switch(port & 0b11000001) {
        case 0x00:
//            dbg_printf(" (memory control)");
              std::cerr<<"Really not implemented. Got port "<<std::hex<<int(port)<<" = "<<int(val)<<"\n";
              //exit(1);
            break;
        case 0x01:
//            dbg_printf(" (I/O control + automatic nationalization)"); break;
            break;
        case 0x40: case 0x41:
//            dbg_printf(" (PSG SN76489 output control)"); break;
              apu_dev->writeRegister(val);
            break;
        case 0x80:
            vdp_dev->writeByte(port, val, cycle);
//            dbg_printf(" (VDP data)");
            break;
        case 0x81:
            vdp_dev->writeByte(port, val, cycle);
//            dbg_printf(" (VDP address/register)");
            break;
        case 0xc0:
            dbg_con::write_control(val);
            std::printf("wrote %02x to port %02x (routed to console control)\n", val, port);
            break;
        case 0xc1:
            dbg_con::write_data(val);
            std::printf("wrote %02x to port %02x (routed to console data)\n", val, port);
            break;
    }
    /*
    switch(port) {
        case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06:
            dbg_printf(" (Game Gear registers)");
            break;
        case 0xde: case 0xdf: dbg_printf(" (keyboard control)"); break;
        case 0xf0: dbg_printf(" (YM2413 address register)"); break;
        case 0xf1: dbg_printf(" (YM2413 data register)"); break;
        case 0xf2: dbg_printf(" (YM2413 control register)"); break;
    }
    */
}

void memmapZ80Console::sendEvent(ioEvent e) {
    // React to key events
    if(e.type == ioEvent::eventType::smsKey) {
        bool pressed = (e.state == ioEvent::keyState::keydown);
        switch(e.key.sKey) {
            case ioEvent::smsKey::dpad_up:
                io_port_ab.port_a_up = pressed;
                break;
            case ioEvent::smsKey::dpad_down:
                io_port_ab.port_a_down = pressed;
                break;
            case ioEvent::smsKey::dpad_left:
                io_port_ab.port_a_left = pressed;
                break;
            case ioEvent::smsKey::dpad_right:
                io_port_ab.port_a_right = pressed;
                break;
            case ioEvent::smsKey::button_1:
                io_port_ab.port_a_tl = pressed;
                break;
            case ioEvent::smsKey::button_2:
                io_port_ab.port_a_tr = pressed;
                break;
            case ioEvent::smsKey::button_pause:
                if(cfg->getSystemType() == systemType::gameGear) {
                    gg_port_0.start = pressed;
                }
                break;
        }
    }
}

uint8_t& memmapZ80Console::map(uint32_t addr) {
    if(addr < 0x0400) { // unpaged rom
        return rom[addr];
    }
    else if(addr < 0x4000) { // slot0 rom
        return rom[(addr & 0x3fff) + map_slot0_offset];
    }
    else if(addr < 0x8000) { // slot1 rom
        return rom[(addr & 0x3fff) + map_slot1_offset];
    }
    else if(addr < 0xC000) { // slot2 rom TODO: implement rom-ram swapout
        if(slot2RamActive) {
            return cartRam[addr & 0x1fff + 0x2000 * slot2RamPage];
        }
        else {
            return rom[(addr & 0x3fff) + map_slot2_offset];
        }
    }
    else if(addr < 0x10000) { // system ram TODO: implement paging control
        return ram[addr & 0x1fff];
    }
    else {
        std::cerr<<"Unmapped address encountered: 0x"<<std::hex<<addr<<"\n";
        return reinterpret_cast<uint8_t&>(dummyLong);
    }
}
