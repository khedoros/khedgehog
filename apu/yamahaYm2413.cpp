#include<iostream>
#include "yamahaYm2413.h"

YamahaYm2413::YamahaYm2413(std::shared_ptr<config>& conf) : apu(conf), curReg(0), statusVal(0) {
    buffer.fill(0);
}
void YamahaYm2413::mute(bool) {}
void YamahaYm2413::writeRegister(uint8_t val) {}
void YamahaYm2413::writeRegister(uint8_t port, uint8_t val) {
    switch(port) {
        case 0xf0:
            curReg = val;
            break;
        case 0xf1:
            // TODO: enqueue reg write value
            std::cout<<" Reg "<<std::hex<<int(curReg)<<" set to "<<int(val)<<"\n";
            break;
        case 0xf2: // tricks the hardware detection. YM2413 docs make it sound like some kind of test register?
            statusVal = val;
            break;
    }
}
uint8_t YamahaYm2413::readRegister(uint8_t port) {
    if(port == 0xf2) return statusVal;
    return 0;
}

void YamahaYm2413::setStereo(uint8_t) {}

std::array<int16_t, 882 * 2>& YamahaYm2413::getSamples() {
    buffer.fill(0);
    // TODO: generate a frame of audio (draw the rest of the bleeping owl)
    return buffer;
}

