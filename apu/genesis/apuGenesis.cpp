#include "apuGenesis.h"

apuGenesis::apuGenesis(){}
void apuGenesis::mute(bool) {}
void apuGenesis::writeRegister(uint8_t val) {}
void apuGenesis::setStereo(uint8_t) {}
std::array<int16_t, 735 * 2>& apuGenesis::getSamples() {
    return buffer;
}

