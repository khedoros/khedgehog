#include "apuGenesis.h"

apuGenesis::apuGenesis(std::shared_ptr<config>& conf) : apu(conf) {}
void apuGenesis::mute(bool) {}
void apuGenesis::writeRegister(uint8_t val) {}
void apuGenesis::writeRegister(uint8_t port, uint8_t val) {}
uint8_t apuGenesis::readRegister(uint8_t reg) { return 0; }
void apuGenesis::setStereo(uint8_t) {}
std::array<int16_t, 882 * 2>& apuGenesis::getSamples() {
    return buffer;
}

