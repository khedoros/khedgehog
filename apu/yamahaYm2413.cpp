#include<iostream>
#include<cmath>
#include "yamahaYm2413.h"
#include "../util.h"

YamahaYm2413::YamahaYm2413(std::shared_ptr<config>& conf) : apu(conf), curReg(0), statusVal(0), writeIndex(0), cfg(conf), envCounter(0) {
    buffer.fill(0);

    if(cfg->getSystemRegion() == systemRegion::eu_pal) {
        sampleCnt = 882;
        ticksPerSample = 5.03;
    }
    else {
        sampleCnt = 735;
        ticksPerSample = 5.07;
    }

    initTables();

    for(int ch = 0; ch < 9; ch++) {
        chan[ch].modOp.inst = &inst[0];
        chan[ch].modOp.totalLevel = 15;
        chan[ch].modOp.phaseInc = 0;
        chan[ch].modOp.phaseCnt = 0;
        chan[ch].modOp.envPhase = silent;
        chan[ch].modOp.envLevel = 127;
        chan[ch].modOp.amPhase = 0;
        chan[ch].modOp.vibPhase = 0;
        chan[ch].modOp.modFB1 = 0;
        chan[ch].modOp.modFB2 = 0;

        chan[ch].carOp.inst = &inst[0];
        chan[ch].carOp.phaseInc = 0;
        chan[ch].carOp.phaseCnt = 0;
        chan[ch].carOp.envPhase = silent;
        chan[ch].carOp.envLevel = 127;
        chan[ch].carOp.amPhase = 0;
        chan[ch].carOp.vibPhase = 0;

        chan[ch].fNum = 0;
        chan[ch].modOp.releaseSustain = false;
        chan[ch].modOp.releaseSustain = false;
        chan[ch].keyOn = false;
        chan[ch].octave = 0;
        chan[ch].volume = 15;
        chan[ch].instNum = 0;
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
            dbg_printf("APU::YM2413: Writing port %02x with value %02x\n", port, statusVal);
            break;
    }
}

void YamahaYm2413::clearWrites() {
    writeIndex = 0;
}

uint8_t YamahaYm2413::readRegister(uint8_t port) {
    dbg_printf("APU::YM2413: Reading port %02x, getting value %02x", port, statusVal);
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
        bool newRhythm = val & 0x20;

        if(newRhythm && !rhythm) {
            percChan[0].modOp->inst = percChan[0].instrument;
            for(int i=0;i<5;i++) {
                percChan[i].carOp->inst = percChan[i].instrument;
            }
        }
        else if(rhythm && !newRhythm) {
            for(int i=7;i<9;i++) {
                chan[i].modOp.inst = &inst[chan[i].instNum];
                chan[i].carOp.inst = &inst[chan[i].instNum];
            }
        }
        rhythm = newRhythm;

        int key[] = { 0x10, 0x01, 0x08, 0x04, 0x02};
        dbg_printf("APU::YM2413 Rhythm: %02x\n", rhythm);
        for(int i=0;i<5;i++) {
            bool newKeyOn = (val & key[i]);
            if(percChan[i].keyOn && !newKeyOn) { //keyoff event
                if(percChan[i].modOp) {
                    percChan[i].modOp->envPhase = release;
                }
                dbg_printf("APU::YM2413 perc   chan %d (%s) key-off\n", i, rhythmNames[i].c_str());
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
                    dbg_printf("APU::YM2413 perc   chan %d (%s) dampen key-on\n", i, rhythmNames[i].c_str());
                }
                else {
                    percChan[i].carOp->envPhase = attack;
                    dbg_printf("APU::YM2413 perc   chan %d (%s) attack key-on\n", i, rhythmNames[i].c_str());
                }
            }
            percChan[i].keyOn = newKeyOn;
        }
    }
    else if(reg >= 0x10 && reg <= 0x18) {
        chan[chNum].fNum &= 0x100;
        chan[chNum].fNum |= val;
        chan[chNum].modOp.phaseInc = (((chan[chNum].fNum * multVal[chan[chNum].modOp.inst->multMod]) << chan[chNum].octave) * 44100) / 49716;
        chan[chNum].carOp.phaseInc = (((chan[chNum].fNum * multVal[chan[chNum].carOp.inst->multCar]) << chan[chNum].octave) * 44100) / 49716;
    }
    else if(reg >= 0x20 && reg <= 0x28) {
        chan[chNum].fNum &= 0xff;
        chan[chNum].fNum |= ((val&0x01)<<8);
        chan[chNum].octave = ((val>>1) & 0x07);
        chan[chNum].modOp.releaseSustain = ((val>>5) & 0x01);
        chan[chNum].carOp.releaseSustain = ((val>>5) & 0x01);
        chan[chNum].modOp.phaseInc = (((chan[chNum].fNum * multVal[chan[chNum].modOp.inst->multMod]) << chan[chNum].octave) * 44100) / 49716;
        chan[chNum].carOp.phaseInc = (((chan[chNum].fNum * multVal[chan[chNum].carOp.inst->multCar]) << chan[chNum].octave) * 44100) / 49716;
        bool newKeyOn = ((val>>4) & 0x01);
        if(chan[chNum].keyOn && !newKeyOn) { // keyOff event
            dbg_printf("APU::YM2413 melody chan %d (%s) key-off\n", chNum, instNames[chan[chNum].instNum].c_str());
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
                dbg_printf("APU::YM2413 melody chan %d (%s) attack key-on\n", chNum, instNames[chan[chNum].instNum].c_str());
            }
            else {
                chan[chNum].carOp.envPhase = dampen;
                dbg_printf("APU::YM2413 melody chan %d (%s) dampen key-on\n", chNum, instNames[chan[chNum].instNum].c_str());
            }
        }
        chan[chNum].keyOn = newKeyOn;

    }
    else if(reg >=0x30 && reg <= 0x38) {
        chan[chNum].modOp.inst = &inst[val>>4];
        chan[chNum].carOp.inst = &inst[val>>4];
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
         
        envCounter+=16;

        int chanMax = (rhythm)?6:9;
        for(int ch=0;ch<chanMax;ch++) {
            op_t * modOp = &(chan[ch].modOp);
            op_t * carOp = &(chan[ch].carOp);
            modOp->updateEnvelope(envCounter, true);
            carOp->updateEnvelope(envCounter, false);
            if(chan[ch].carOp.envPhase != silent) {
                modOp->phaseCnt += modOp->phaseInc;
                int feedback = (modOp->inst->feedbackMod) ? ((modOp->modFB1 + modOp->modFB2) >> (8 - modOp->inst->feedbackMod)) : 0;
                carOp->phaseCnt += carOp->phaseInc;
                int modSin = lookupSin((modOp->phaseCnt / 512) -1 +                              // phase
                                       (modOp->vibPhase) +                                       // modification for vibrato
                                       (feedback),                                               // modification for feedback
                                       modOp->inst->waveformMod);

                int modOut = lookupExp((modSin) +                                                // sine input
                                       (modOp->amPhase * 0x10) +                                 // AM volume attenuation (tremolo)
                                       (modOp->envLevel * 0x10) +                                // Envelope
                                       //TODO: KSL
                                       (modOp->totalLevel * 0x20)) >> 1;                         // Modulator volume
                modOp->modFB1 = modOp->modFB2;
                modOp->modFB2 = modOut;

                int carSin = lookupSin((carOp->phaseCnt / 512) +                                 // phase
                                       (2 * modOut) +                                            // fm modulation
                                       (carOp->vibPhase),                                        // modification for vibrato
                                       carOp->inst->waveformCar);

                buffer[i]+=lookupExp((carSin) +                                                  // sine input
                                     (carOp->amPhase * 0x10) +                                   // AM volume attenuation (tremolo)
                                     (carOp->envLevel * 0x10) +                                  // Envelope
                                     //TODO: KSL
                                     (chan[ch].volume * 0x80)) & 0xfff0;                         // Channel volume
            }
        }
        if(rhythm) { // TODO: handle the 5 rhythm instruments (correctly)
            for(int ch = 0; ch < 5; ch++) {
                op_t * modOp = percChan[ch].modOp;
                op_t * carOp = percChan[ch].carOp;
                if(modOp) {
                    modOp->updateEnvelope(envCounter, true);
                }
                carOp->updateEnvelope(envCounter, false);
                if(percChan[ch].carOp->envPhase != silent) {
                    int modOut = 0;
                    carOp->phaseCnt += carOp->phaseInc;
                    if(modOp) {
                        int feedback = (modOp->inst->feedbackMod) ? ((modOp->modFB1 + modOp->modFB2) >> (8 - modOp->inst->feedbackMod)) : 0;
                        modOp->phaseCnt += modOp->phaseInc;
                        int modSin = lookupSin((modOp->phaseCnt / 512) - 1 +                         // phase
                                               (modOp->vibPhase) +                                   // modification for vibrato
                                               (feedback),                                           // modification for feedback
                                               modOp->inst->waveformMod);

                        modOut = lookupExp((modSin) +                                                // sine input
                                           (modOp->amPhase * 0x10) +                                 // AM volume attenuation (tremolo)
                                           (modOp->envLevel * 0x10) +                                // Envelope
                                           //TODO: KSL
                                           (modOp->totalLevel * 0x20)) >> 1;                         // Modulator volume
                        modOp->modFB1 = modOp->modFB2;
                        modOp->modFB2 = modOut;

                    }
                    int carSin = lookupSin((carOp->phaseCnt / 512) +                                 // phase
                                           (2 * modOut) +                                            // fm modulation
                                           (carOp->vibPhase),                                        // modification for vibrato
                                           carOp->inst->waveformCar);

                    buffer[i]+= lookupExp((carSin) +                                                 // sine input
                                         (carOp->amPhase * 0x10) +                                   // AM volume attenuation (tremolo)
                                         (carOp->envLevel * 0x10) +                                  // Envelope
                                         //TODO: KSL
                                         (*percChan[ch].volume * 0x80)) & 0xfff0;                    // Channel volume
                }
            }
        }
    }

    writeIndex = 0;
    return buffer;
}

void YamahaYm2413::initTables() {
    for (int i = 0; i < 256; ++i) {
        logsinTable[i] = round(-log2(sin((double(i) + 0.5) * M_PI_2 / 256.0)) * 256.0);
        expTable[i] = round(exp2(double(i) / 256.0) * 1024.0) - 1024.0;
    }
}

int YamahaYm2413::lookupSin(int val, bool wf) {
    bool sign   = val & 512;
    bool mirror = val & 256;
    val &= 255;
    int result = logsinTable[mirror ? val ^ 255 : val];
    if (sign) {
        if(wf) result = 0xfff; // set to 0 for negative part of second waveform
        result |= 0x8000;      // still outputs positive and negative 0
    }
    return result;
}

int YamahaYm2413::lookupExp(int val) {
    bool sign = val & 0x8000;
    int t = (expTable[(val & 255) ^ 255] << 1) | 0x800;
    int result = t >> ((val & 0x7F00) >> 8);
    if (sign) result = ~result;
    return result;
}

int YamahaYm2413::logsinTable[256];
int YamahaYm2413::expTable[256];

const std::string YamahaYm2413::instNames[]  {"Custom", "Violin", "Guitar", "Piano",
                                              "Flute", "Clarinet", "Oboe", "Trumpet",
                                              "Organ", "Horn", "Synth", "Harpsichord",
                                              "Vibraphone", "Synth Bass", "Acoustic Bass",
                                              "Electric Guitar"};
const std::string YamahaYm2413::rhythmNames[] {"Bass Drum", "High Hat", 
                                               "Snare Drum", "Tom-tom", "Top Cymbal"};

void YamahaYm2413::op_t::updateEnvelope(unsigned int counter, bool mod) {
    if(envPhase == dampen && envLevel >= 123) {
        envPhase = attack;
    }
    else if(envPhase == attack && (envLevel == 0 || envLevel > 130)) {
        envPhase = decay;
    }
    else if(envPhase == decay && 
            ((mod && envLevel >= (inst->sustainLevelMod) * 8) || (!mod && envLevel >= (inst->sustainLevelCar) * 8))) {
        if((mod && inst->sustMod) || (!mod && inst->sustCar)) {
            envPhase = sustain;
        }
        else {
            envPhase = sustainRelease;
        }
        if(mod) envLevel = (inst->sustainLevelMod * 8);
        else    envLevel = (inst->sustainLevelCar * 8);
    }
    else if((envPhase == sustainRelease || envPhase == release) && envLevel >= 123) {
        envPhase = silent;
    }

    unsigned int activeRate = 0;
    switch(envPhase) {
        case silent: activeRate = 0; break;
        case dampen: activeRate = 12; break;
        case attack: if(mod) activeRate = inst->attackMod;
                     else    activeRate = inst->attackCar;
                     break;
        case decay:  if(mod) activeRate = inst->decayMod;
                     else    activeRate = inst->decayCar;
                     break;
        case sustain: activeRate = 0; break;
        case sustainRelease: if(mod) activeRate = inst->releaseMod;
                             else    activeRate = inst->releaseCar;
                             break;
        case release: if(releaseSustain) activeRate = 5;
                      else        activeRate = 7;
                      break;
        default: activeRate = 0;
                 std::cout<<"Unhandled envPhase: "<<envPhase<<"\n";
                 break;
    }

    int changeAmount = 1;
    if(envPhase == attack) {
        if(activeRate == 15) {
            envLevel = 0;
        }
        else {
            envLevel += ~envLevel >> 2;
        }
    }
    else if(activeRate == 0) changeAmount = 0;
    else if(activeRate == 15) {
        changeAmount = 2;
    }

              //    0      1      2     3     4     5     6     7    8   9   a   b   c  d  e  f
    int checks[] {65536, 32768, 16384, 8192, 4096, 2048, 1024, 236, 128, 64, 32, 16, 8, 4, 2, 1};

    if(!(counter & (checks[activeRate] - 1))) {
        if(envPhase != attack) {
            envLevel += changeAmount;
        }
    }

    //if(envLevel > 127) std::cout<<std::dec<<"Env at "<<envLevel<<"\n";
    if(envLevel > 130) envLevel = 0; // assume wrap-around
    else if(envLevel > 127) envLevel = 127; //assume it just overflowed the 7-bit value
}
