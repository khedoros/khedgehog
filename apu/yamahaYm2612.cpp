#include "yamahaYm2612.h"

YamahaYm2612::YamahaYm2612(std::shared_ptr<config>& conf) : apu(conf) {
    buffer.fill(0);
}
void YamahaYm2612::mute(bool) {}
void YamahaYm2612::writeRegister(uint8_t val) {}
uint8_t YamahaYm2612::readRegister(uint8_t reg) { return 0; }
void YamahaYm2612::setStereo(uint8_t) {}
std::array<int16_t, 882 * 2>& YamahaYm2612::getSamples() {
    return buffer;
}

