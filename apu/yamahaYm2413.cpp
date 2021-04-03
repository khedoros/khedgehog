#include "yamahaYm2413.h"

YamahaYm2413::YamahaYm2413(std::shared_ptr<config>& conf) : apu(conf) {
    buffer.fill(0);
}
void YamahaYm2413::mute(bool) {}
void YamahaYm2413::writeRegister(uint8_t val) {}
void YamahaYm2413::setStereo(uint8_t) {}
std::array<int16_t, 882 * 2>& YamahaYm2413::getSamples() {
    return buffer;
}

