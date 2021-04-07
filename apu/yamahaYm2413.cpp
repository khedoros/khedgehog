#include<iostream>
#include "yamahaYm2413.h"

YamahaYm2413::YamahaYm2413(std::shared_ptr<config>& conf) : apu(conf), curReg(0), statusVal(0), writeIndex(0), cfg(conf) {
    buffer.fill(0);

    if(cfg->getSystemRegion() == systemRegion::pal) {
		sampleCnt = 882;
		ticksPerSample = 5.03;
	}
	else {
        sampleCnt = 735;
        ticksPerSample = 5.07;
    }

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
            regWrites[writeIndex++] = std::make_pair(curReg, val);
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

void YamahaYm2413::applyRegister(std::pair<uint8_t, uint8_t>& write) {
	uint8_t reg = write.first;
	uint8_t val = write.second;
	switch(reg) {
        default:
			std::cout<<"Unhandled register: "<<int(reg)<<" = "<<int(val)<<"\n";
	}
}

void YamahaYm2413::setStereo(uint8_t) {}

std::array<int16_t, 882 * 2>& YamahaYm2413::getSamples() {
	if(cfg->getSystemType() != systemType::masterSystem) return buffer;

    buffer.fill(0);

	int curIndex = 0;
    for(int i=0;i<sampleCnt;i++) {
	// TODO: Super-hacky. Spread register writes evenly through the frame.
	// Fixing it properly would require the CPU to send the write cycle to the memmap, so that writes could be
	// timed properly in the frame. But...this works, so far. Easy way out.
	/**/ float percent = float(i) / float(sampleCnt);
	/**/ if(int(percent * writeIndex) >= curIndex) {
	/**/    applyRegister(regWrites[curIndex++]);
	/**/}
	}

    // TODO: generate a frame of audio (draw the rest of the bleeping owl)
    writeIndex = 0;
    return buffer;
}

