#include "apuGenesis.h"

apuGenesis::apuGenesis(std::shared_ptr<config>& conf) : apu(conf) {}
void apuGenesis::mute(bool) {}
void apuGenesis::writeRegister(uint8_t val) {}
void apuGenesis::setStereo(uint8_t) {}
std::array<int16_t, 882 * 2>& apuGenesis::getSamples() {
    return buffer;
}

