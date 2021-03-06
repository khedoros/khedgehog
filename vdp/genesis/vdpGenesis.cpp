#include "vdpGenesis.h"

vdpGenesis::vdpGenesis(systemType t, systemRegion r) {

}

std::vector<uint8_t> vdpGenesis::getPartialRender() {
    return std::vector<uint8_t>(224 * 256 * 3, 0);
}

std::vector<uint8_t> vdpGenesis::getDebugRender() {
    std::vector<uint8_t> buffer(512 * 512 * 3, 0);
    return buffer;
}

int vdpGenesis::getStride() {
    return 0;
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

std::vector<uint8_t>& vdpGenesis::getFrameBuffer() {
    return buffer;
}


void vdpGenesis::endLine(uint64_t line) {}
