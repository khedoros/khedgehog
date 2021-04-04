#include "apuMS.h"

apuMS::apuMS(std::shared_ptr<config>& conf) : cfg(conf), apu(conf), psg(std::make_shared<TiPsg>(cfg)), fm(std::make_shared<YamahaYm2413>(cfg)) {

}

void apuMS::mute(bool muted) {
    psg->mute(muted);
    fm->mute(muted);
}
void apuMS::writeRegister(uint8_t val) {
    psg->writeRegister(val);
}

void apuMS::writeRegister(uint8_t port, uint8_t val) {
    switch(port) {
        case 0x40: case 0x41:
            psg->writeRegister(val);
            break;
        case 0xf0: case 0xf1: case 0xf2:
            fm->writeRegister(port, val);
            break;
    }
}

void apuMS::setStereo(uint8_t val) {
    psg->setStereo(val);
    fm->setStereo(val);
}

std::array<int16_t, 882 * 2>& apuMS::getSamples() {
    auto&& psgs = psg->getSamples();
    auto&& fms = fm->getSamples();
    for(int i = 0; i < psgs.size(); i++) {
        psgs[i] += fms[i];
    }
    return psgs;
}
uint8_t apuMS::readRegister(uint8_t port) {
    switch(port) {
        case 0x40: case 0x41:
            return psg->readRegister(port);
        case 0xf0: case 0xf1: case 0xf2:
            return fm->readRegister(port);
        default:
            return 0xff;
    }
}

