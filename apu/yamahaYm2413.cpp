#include<iostream>
#include<cmath>
#include "yamahaYm2413.h"

YamahaYm2413::YamahaYm2413(std::shared_ptr<config>& conf) : apu(conf), curReg(0), statusVal(0), writeIndex(0), cfg(conf), envCounter(0) {
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

    for(int ch = 0; ch < 9; ch++) {
        chan[ch].modOp.inst = &inst[0];
        chan[ch].modOp.totalLevel = 0;
        chan[ch].modOp.phaseInc = 0;
        chan[ch].modOp.phaseCnt = 0;
        chan[ch].modOp.envPhase = silent;
        chan[ch].modOp.envLevel = 127 * 0x10;

        chan[ch].carOp.inst = &inst[0];
        chan[ch].carOp.phaseInc = 0;
        chan[ch].carOp.phaseCnt = 0;
        chan[ch].carOp.envPhase = silent;
        chan[ch].carOp.envLevel = 127 * 0x10;

        chan[ch].fNum = 0;
        chan[ch].sustain = false;
        chan[ch].keyOn = false;
        chan[ch].octave = 0;
        chan[ch].volume = 15;
    }

    for(int instrument=0;instrument<19;instrument++) {
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
            //std::cout<<" Reg "<<std::hex<<int(curReg)<<" set to "<<int(val)<<"\n";
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
    uint8_t chNum = reg & 0x0f;
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
        rhythmInsts ins[] = {bassDrum, highHat, snareDrum, tomTom, topCymbal};
        int key[] = { 0x10, 0x08, 0x04, 0x02, 0x01};
        for(int i=0;i<5;i++) {
            bool newKeyOn = (val & key[i]);
            if(percChan[i].keyOn && !newKeyOn) { //keyoff event
                if(percChan[i].modOp) {
                    std::cout<<"perc chan "<<i<<" key-off\n";
                    percChan[i].modOp->envPhase = release;
                }
                percChan[i].carOp->envPhase = release;
            }
            else if(!percChan[i].keyOn && newKeyOn) { //keyon event
                if(percChan[i].modOp) {
                    if(percChan[i].modOp->envPhase == silent) {
                        percChan[i].modOp->envPhase = attack;
                    }
                    else {
                        percChan[i].modOp->envPhase = dampen;
                    }
                }
                if(percChan[i].carOp->envPhase != silent) {
                    percChan[i].carOp->envPhase = dampen;
                    std::cout<<"perc chan "<<i<<" dampen key-on\n";
                }
                else {
                    percChan[i].carOp->envPhase = attack;
                    std::cout<<"perc chan "<<i<<" attack key-on\n";
                }
            }
            percChan[i].keyOn = newKeyOn;
        }
    }
    else if(reg >= 0x10 && reg <= 0x18) {
        chan[chNum].fNum &= 0x100;
        chan[chNum].fNum |= val;
        chan[chNum].modOp.phaseInc = ((chan[chNum].fNum * multVal[chan[chNum].modOp.inst->multMod]) << chan[chNum].octave) >> 1;
        chan[chNum].carOp.phaseInc = ((chan[chNum].fNum * multVal[chan[chNum].carOp.inst->multCar]) << chan[chNum].octave) >> 1;
    }
    else if(reg >= 0x20 && reg <= 0x28) {
        chan[chNum].fNum &= 0xff;
        chan[chNum].fNum |= ((val&0x01)<<8);
        chan[chNum].octave = ((val>>1) & 0x07);
        chan[chNum].sustain = ((val>>5) & 0x01);
        bool newKeyOn = ((val>>4) & 0x01);
        if(chan[chNum].keyOn && !newKeyOn) { // keyOff event
            std::cout<<"melody chan "<<int(chNum)<<" key-off\n";
            chan[chNum].modOp.envPhase = release;
            chan[chNum].carOp.envPhase = release;
        }
        else if(!chan[chNum].keyOn && newKeyOn) { // keyOn event
            if(chan[chNum].modOp.envPhase == silent) {
                chan[chNum].modOp.envPhase = attack;
            }
            else {
                chan[chNum].modOp.envPhase = dampen;
            }
            if(chan[chNum].carOp.envPhase == silent) {
                chan[chNum].carOp.envPhase = attack;
                std::cout<<"melody chan "<<int(chNum)<<" attack key-on\n";
            }
            else {
                chan[chNum].carOp.envPhase = dampen;
                std::cout<<"melody chan "<<int(chNum)<<" dampen key-on\n";
            }
        }
        chan[chNum].keyOn = newKeyOn;

        chan[chNum].modOp.phaseInc = ((chan[chNum].fNum * multVal[chan[chNum].modOp.inst->multMod]) << chan[chNum].octave) >> 1;
        chan[chNum].carOp.phaseInc = ((chan[chNum].fNum * multVal[chan[chNum].carOp.inst->multCar]) << chan[chNum].octave) >> 1;
    }
    else if(reg >=0x30 && reg <= 0x38) {
        chan[chNum].modOp.inst = &inst[val>>4];
        chan[chNum].instNum = val>>4;
        chan[chNum].volume = val & 0x0f;
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
         
        envCounter++;

        int chanMax = (rhythm)?6:9;
        for(int ch=0;ch<chanMax;ch++) {
            if(chan[ch].carOp.envPhase != silent) {
                op_t * modOp = &(chan[ch].modOp);
                op_t * carOp = &(chan[ch].carOp);
                modOp->phaseCnt += modOp->phaseInc;
                carOp->phaseCnt += carOp->phaseInc;
                int modSin = lookupSin((modOp->phaseCnt / 512) +                                 // phase
                                       (modOp->vibPhase) +                                          // modification for vibrato
                                       (((modOp->modFB1 + modOp->modFB2) / 2) * ((modOp->inst->feedbackMod)))); // modification for feedback

                int modOut = lookupExp((modSin) +                                                // sine input
                                       (modOp->amPhase * 0x10) +                                   // AM volume attenuation (tremolo)
                                       (modOp->envLevel * 0x10) +                                // Envelope
                                       (modOp->totalLevel * 0x20));                              // Modulator volume

                int carSin = lookupSin((carOp->phaseCnt / 512) +                                 // phase
                                       (modOut * 8) +                                            // fm modulation
                                       (carOp->vibPhase));                                          // modification for vibrato

                buffer[i]+=lookupExp((carSin) +                                                  // sine input
                                     (carOp->amPhase * 0x10) +                                     // AM volume attenuation (tremolo)
                                     (carOp->envLevel * 0x10) +                                  // Envelope
                                     (chan[ch].volume * 0x80));                                  // Channel volume
            }
        }
        if(rhythm) { // TODO: handle the 5 rhythm instruments
            for(int ch = 0; ch < 5; ch++) {
                if(percChan[ch].carOp->envPhase != silent) {
                    op_t * modOp = percChan[ch].modOp;
                    op_t * carOp = percChan[ch].carOp;
                    int modOut = 0;
                    carOp->phaseCnt += carOp->phaseInc;
                    if(modOp) {
                        modOp->phaseCnt += modOp->phaseInc;
                        int modSin = lookupSin((modOp->phaseCnt / 512) +                                 // phase
                                               (modOp->vibPhase) +                                          // modification for vibrato
                                               (((modOp->modFB1 + modOp->modFB2) / 2) * ((percChan[ch].instrument->feedbackMod)))); // modification for feedback

                        modOut = lookupExp((modSin) +                                                    // sine input
                                           (modOp->amPhase * 0x10) +                                       // AM volume attenuation (tremolo)
                                           (modOp->envLevel * 0x10) +                                    // Envelope
                                           (modOp->totalLevel * 0x20));                                  // Modulator volume

                    }
                    int carSin = lookupSin((carOp->phaseCnt / 512) +                                 // phase
                                           (modOut * 8) +                                            // fm modulation
                                           (carOp->vibPhase));                                          // modification for vibrato

                    buffer[i]+=lookupExp((carSin) +                                                  // sine input
                                         (carOp->amPhase * 0x10) +                                     // AM volume attenuation (tremolo)
                                         (carOp->envLevel * 0x10) +                                  // Envelope
                                         (*percChan[ch].volume * 0x80));                             // Channel volume
                }
            }
        }
    }

    writeIndex = 0;
    return buffer;
}

void YamahaYm2413::initTables() {
    for (int i = 0; i < 256; ++i) {
        logsinTable[i] = round(-log2(sin((i + 0.5) * M_PI_2 / 256.0)) * 256.0);
        expTable[i] = round(exp2(i / 256.0) * 1024.0) - 1024;
    }
}

int YamahaYm2413::lookupSin(int val) {
    bool sign   = val & 512;
    bool mirror = val & 256;
    val &= 255;
    int result = logsinTable[mirror ? val ^ 255 : val];
    if (sign) result |= 0x8000;
    return result;
}

int YamahaYm2413::lookupExp(int val) {
    bool sign = val & 0x8000;
    int t = (expTable[(val & 255) ^ 255] | 1024) << 1;
    int result = t >> ((val & 0x7F00) >> 8);
    if (sign) result = ~result;
    return result;
}

int YamahaYm2413::logsinTable[256];
int YamahaYm2413::expTable[256];
