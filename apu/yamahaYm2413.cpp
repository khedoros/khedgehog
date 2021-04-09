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

    for(int instrument=0;instrument<16;instrument++) {
        int tableBase = instrument * 8;
        int reg0 = romInst[tableBase + 0];
        int reg1 = romInst[tableBase + 1];
        int reg2 = romInst[tableBase + 2];
        int reg3 = romInst[tableBase + 3];
        int reg4 = romInst[tableBase + 4];
        int reg5 = romInst[tableBase + 5];
        int reg6 = romInst[tableBase + 6];
        int reg7 = romInst[tableBase + 7];
        inst[instrument].amMod = (reg0 & 0x80);
        inst[instrument].vibMod = (reg0 & 0x40);
        inst[instrument].sustMod = (reg0 & 0x20);
        inst[instrument].keyScaleRateMod = (reg0 & 0x10);
        inst[instrument].multMod = (reg0 & 0x0f);

        inst[instrument].amCar = (reg1 & 0x80);
        inst[instrument].vibCar = (reg1 & 0x40);
        inst[instrument].sustCar = (reg1 & 0x20);
        inst[instrument].keyScaleRateCar = (reg1 & 0x10);
        inst[instrument].multCar = (reg1 & 0x0f);

        inst[instrument].keyScaleLevelMod = (reg2>>6);
        inst[instrument].totalLevelMod = (reg2 & 0x3f);

        inst[instrument].keyScaleLevelCar = (reg3>>6);
        inst[instrument].waveformCar = (reg3 & 0x10);
        inst[instrument].waveformMod = (reg3 & 0x08);
        inst[instrument].feedbackMod = (reg3 & 0x07);

        inst[instrument].attackMod = (reg4 & 0xf0)>>4;
        inst[instrument].decayMod = (reg4 & 0x0f);

        inst[instrument].attackCar = (reg5 & 0xf0)>>4;
        inst[instrument].decayCar = (reg5 & 0x0f);

        inst[instrument].sustainLevelMod = (reg6 & 0xf0)>>4;
        inst[instrument].releaseMod = (reg6 & 0x0f);

        inst[instrument].sustainLevelCar = (reg7 & 0xf0)>>4;
        inst[instrument].releaseCar = (reg7 & 0x0f);
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
    if(reg < 8) {
        switch(reg) {
            case 0:
                inst[0].amMod = (val & 0x80);
                inst[0].vibMod = (val & 0x40);
                inst[0].sustMod = (val & 0x20);
                inst[0].keyScaleRateMod = (val & 0x10);
                inst[0].multMod = (val & 0x0f);
                break;
            case 1:
                inst[0].amCar = (val & 0x80);
                inst[0].vibCar = (val & 0x40);
                inst[0].sustCar = (val & 0x20);
                inst[0].keyScaleRateCar = (val & 0x10);
                inst[0].multCar = (val & 0x0f);
                break;
            case 2:
                inst[0].keyScaleLevelMod = (val>>6);
                inst[0].totalLevelMod = (val & 0x3f);
                break;
            case 3:
                inst[0].keyScaleLevelCar = (val>>6);
                inst[0].waveformCar = (val & 0x10);
                inst[0].waveformMod = (val & 0x08);
                inst[0].feedbackMod = (val & 0x07);
                break;
            case 4:
                inst[0].attackMod = (val & 0xf0)>>4;
                inst[0].decayMod = (val & 0x0f);
                break;
            case 5:
                inst[0].attackCar = (val & 0xf0)>>4;
                inst[0].decayCar = (val & 0x0f);
                break;
            case 6:
                inst[0].sustainLevelMod = (val & 0xf0)>>4;
                inst[0].releaseMod = (val & 0x0f);
                break;
            case 7:
                inst[0].sustainLevelCar = (val & 0xf0)>>4;
                inst[0].releaseCar = (val & 0x0f);
                break;
        }
    }
    else if(reg == 0x0e) {
        rhythm = val & 0x20;
        bass_trigger = val & 0x10;
        snare_trigger = val & 0x8;
        tom_trigger = val & 0x4;
        top_cymbal_trigger = val & 0x2;
        high_hat_trigger = val & 0x1;
    }
    else if(reg >= 0x10 && reg <= 0x18) {
        int num = (reg & 0x0f);
        chan[num].fNum &= 0x100;
        chan[num].fNum |= val;
        chan[num].modOp.phaseInc = ((chan[num].fNum * multVal[inst[chan[num].modOp.instNum].multMod]) << chan[num].octave) >> 1;
        chan[num].carOp.phaseInc = ((chan[num].fNum * multVal[inst[chan[num].carOp.instNum].multCar]) << chan[num].octave) >> 1;
    }
    else if(reg >= 0x20 && reg <= 0x28) {
        int num = (reg & 0x0f);
        chan[num].fNum &= 0xff;
        chan[num].fNum |= ((val&0x01)<<8);
        chan[num].octave = ((val>>1) & 0x07);
        chan[num].sustain = ((val>>5) & 0x01);
        chan[num].trigger = ((val>>4) & 0x01);
        chan[num].modOp.phaseInc = ((chan[num].fNum * multVal[inst[chan[num].modOp.instNum].multMod]) << chan[num].octave) >> 1;
        chan[num].carOp.phaseInc = ((chan[num].fNum * multVal[inst[chan[num].carOp.instNum].multCar]) << chan[num].octave) >> 1;
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
        int chanMax = (rhythm)?6:9;
        for(int ch=0;ch<9;ch++) {
            if(chan[ch].trigger) {
                int modOp = (ch / 3) * 6;
                int carOp = (ch / 3) * 6 + 3;
                chan[ch].modOp.phaseCnt += chan[ch].modOp.phaseInc;
                chan[ch].carOp.phaseCnt += chan[ch].carOp.phaseInc;
                int phase = chan[ch].carOp.phaseCnt / 512;
                if(phase >= 512) buffer[i] -= (sine[phase - 512] * 512);
                else buffer[i] += (sine[phase] * 512);
            }
        }
        if(rhythm) { // TODO: handle the 5 rhythm instruments
        }
    }

    writeIndex = 0;
    return buffer;
}

