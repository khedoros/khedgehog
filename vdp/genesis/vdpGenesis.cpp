#include "vdpGenesis.h"
#include<iostream>

vdpGenesis::vdpGenesis(systemType t, systemRegion r) : curXRes(256), curYRes(224) {
    resizeBuffer(curXRes, curYRes);
    std::cerr<<"Genesis vdp constructor\n";
}

std::vector<uint8_t> vdpGenesis::getPartialRender() {
    return std::vector<uint8_t>(curXRes * curYRes * 4, 0);
}

std::vector<uint8_t> vdpGenesis::getDebugRender() {
    std::vector<uint8_t> buffer(512 * 512 * 4, 0);
    return buffer;
}

int vdpGenesis::getStride() {
    return curXRes*4;
}

uint64_t vdpGenesis::calc(uint64_t) {
    return 0;
}

void vdpGenesis::resizeBuffer(unsigned int x, unsigned int y) {
    std::cerr<<"Resize buffer to "<<x<<"x"<<y<<"\n";
    curXRes = x;
    curYRes = y;
    buffer.resize(curYRes * getStride(), 0xff);
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

unsigned int vdpGenesis::resLine() {
    return curYRes;
}
