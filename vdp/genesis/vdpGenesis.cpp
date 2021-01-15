#include "vdpGenesis.h"

vdpGenesis::vdpGenesis(systemType t = systemType::genesis, systemRegion r = systemRegion::ntsc) {

}

std::vector<std::vector<uint8_t>> vdpGenesis::getPartialRender() {
    return std::vector<std::vector<uint8_t>>(224, std::vector<uint8_t>(256*3, 0));
}

uint64_t vdpGenesis::calc(uint64_t) {
    return 0;
}

void vdpGenesis::writeByte(uint8_t port, uint8_t val, uint64_t cycle) {

}

uint8_t vdpGenesis::readByte(uint8_t port, uint64_t cycle) {
    return 0;
}

bool vdpGenesis::lineInterrupt() {
    return false;
}

bool vdpGenesis::frameInterrupt() {
    return false;
}

void vdpGenesis::endLine(uint64_t line) {}
