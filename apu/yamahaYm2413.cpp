#include<iostream>
#include<cmath>
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

    for(double i=0.0;i<512.0;i+=1.0) {
        //std::cout<<"i: "<<i<<" sine: "<<sin((i+.5)*M_PI/512)<<"\n";
        sine[int(i)] = sin((i+.5)*M_PI/512);
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
    if(reg >= 0x10 && reg <= 0x18) {
        int num = (reg & 0x0f);
        int modOp = (num / 3) * 6;
        int carOp = (num / 3) * 6 + 3;
        chan[num].fNum &= 0x100;
        chan[num].fNum |= val;
        op[modOp].phaseInc = 256 * chan[num].octave + chan[num].fNum;
        //if(!op[modOp].mult) op[modOp].phaseInc /= 2;
        //else op[modOp.phaseInc] *= multVal[op[modOp].mult];
        op[carOp].phaseInc = 256 * chan[num].octave + chan[num].fNum;
        //if(!op[carOp].mult) op[carOp].phaseInc /= 2;
        //else op[carOp.phaseInc] *= multVal[op[carOp].mult];
    }
    else if(reg >= 0x20 && reg <= 0x28) {
        int num = (reg & 0x0f);
        int modOp = (num / 3) * 6;
        int carOp = (num / 3) * 6 + 3;
        chan[num].fNum &= 0xff;
        chan[num].fNum |= ((val&0x01)<<8);
        chan[num].octave = ((val>>1) & 0x07);
        chan[num].sustain = ((val>>5) & 0x01);
        chan[num].trigger = ((val>>4) & 0x01);
        op[modOp].phaseInc = 256 * chan[num].octave + chan[num].fNum;
        //if(!op[modOp].mult) op[modOp].phaseInc /= 2;
        //else op[modOp.phaseInc] *= multVal[op[modOp].mult];
        op[carOp].phaseInc = 256 * chan[num].octave + chan[num].fNum;
        //if(!op[carOp].mult) op[carOp].phaseInc /= 2;
        //else op[carOp.phaseInc] *= multVal[op[carOp].mult];
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
	/**/ }
        for(int ch=0;ch<9;ch++) {
            if(chan[ch].trigger) {
                int modOp = (ch / 3) * 6;
                int carOp = (ch / 3) * 6 + 3;
                op[modOp].phaseCnt += op[modOp].phaseInc;
                op[carOp].phaseCnt += op[carOp].phaseInc;
                int phase = op[carOp].phaseCnt / 256;
                if(phase >= 512) buffer[i] -= (sine[phase - 512] * 512);
                else buffer[i] += (sine[phase] * 512);
            }
        }
	}

    // TODO: generate a frame of audio (draw the rest of the bleeping owl)
    writeIndex = 0;
    return buffer;
}

