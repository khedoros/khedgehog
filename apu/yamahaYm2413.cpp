#include<iostream>
#include<cmath>
#include<algorithm>
#include "yamahaYm2413.h"
#include "../util.h"

YamahaYm2413::YamahaYm2413(std::shared_ptr<config>& conf) : apu(conf), curReg(0), statusVal(0), writeIndex(0), cfg(conf), envCounter(0), galoisState(1) {
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

    for(auto& ch: chan) {
        ch.modOp.inst = &inst[0];
        ch.modOp.totalLevel = 15;
        ch.modOp.phaseInc = 0;
        ch.modOp.phaseCnt = 0;
        ch.modOp.envPhase = silent;
        ch.modOp.envLevel = 127;
        ch.modOp.envAccum = 0;
        ch.modOp.inst->amModAtten = 0;
        ch.fmModShift = 0;
        ch.modOp.modFB1 = 0;
        ch.modOp.modFB2 = 0;

        ch.carOp.inst = &inst[0];
        ch.carOp.phaseInc = 0;
        ch.carOp.phaseCnt = 0;
        ch.carOp.envPhase = silent;
        ch.carOp.envLevel = 127;
        ch.carOp.envAccum = 0;
        ch.carOp.inst->amCarAtten = 0;
        ch.fmCarShift = 0;

        ch.fNum = 0;
        ch.modOp.releaseSustain = false;
        ch.modOp.releaseSustain = false;
        ch.key = false;
        ch.octave = 0;
        ch.volume = 15;
        ch.instNum = 0;
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
    for(auto& pChan: percChan) {
        pChan.key = false;
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
    chan_t& channel = chan[chNum];
    op_t& modOp = channel.modOp;
    op_t& carOp = channel.carOp;
    if(reg < 8) { // The first 8 registers relate to the single custom instrument
        inst_t& userInst = inst[0];
        switch(reg) {
            case 0:
                userInst.amMod = (val & 0x80);
                if(!userInst.amMod) userInst.amModAtten = 0;
                else userInst.amModAtten = amTable[amPhase] * tremoloMultiplier;

                userInst.vibMod = (val & 0x40);
                for(auto& c:chan) {
                    if(c.instNum == 0)
                        if(userInst.vibMod) {
                            c.fmModShift = fmTable[c.fmRow + fmPhase] * vibratoMultiplier;
                        }
                        else c.fmModShift = 0;
                }

                userInst.sustMod = (val & 0x20);
                userInst.keyScaleRateMod = (val & 0x10);
                userInst.multMod = (val & 0x0f);
                break;
            case 1:
                userInst.amCar = (val & 0x80);
                if(!userInst.amCar) userInst.amCarAtten = 0;
                else userInst.amCarAtten = amTable[amPhase] * tremoloMultiplier;

                userInst.vibCar = (val & 0x40);
                for(auto& c:chan) {
                    if(c.instNum == 0)
                        if(userInst.vibCar) {
                            c.fmCarShift = fmTable[c.fmRow + fmPhase] * vibratoMultiplier;
                        }
                        else c.fmCarShift = 0;
                }

                userInst.sustCar = (val & 0x20);
                userInst.keyScaleRateCar = (val & 0x10);
                userInst.multCar = (val & 0x0f);
                break;
            case 2:
                userInst.keyScaleLevelMod = (val>>6);
                userInst.totalLevelMod = (val & 0x3f);
                break;
            case 3:
                userInst.keyScaleLevelCar = (val>>6);
                userInst.waveformCar = (val & 0x10);
                userInst.waveformMod = (val & 0x08);
                userInst.feedbackMod = (val & 0x07);
                break;
            case 4:
                userInst.attackMod = (val & 0xf0)>>4;
                userInst.decayMod = (val & 0x0f);
                break;
            case 5:
                userInst.attackCar = (val & 0xf0)>>4;
                userInst.decayCar = (val & 0x0f);
                break;
            case 6:
                userInst.sustainLevelMod = (val & 0xf0)>>4;
                userInst.releaseMod = (val & 0x0f);
                break;
            case 7:
                userInst.sustainLevelCar = (val & 0xf0)>>4;
                userInst.releaseCar = (val & 0x0f);
                break;
        }
    }
    else if(reg == 0x0e) { // Register 0xe is rhythm mode control
        bool newRhythm = val & 0x20;

        if(newRhythm && !rhythm) { // Turning rhythm mode on
            percChan[0].modOp->inst = percChan[0].instrument;
            for(int i=0;i<5;i++) {
                percChan[i].carOp->inst = percChan[i].instrument;
            }
        }
        else if(rhythm && !newRhythm) { // Shutting rhythm mode down
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
            if(percChan[i].key && !newKeyOn) { //keyoff event
                dbg_printf("APU::YM2413 perc   chan %d (%s) key-off\n", i, rhythmNames[i].c_str());
                percKeyOff(percChan[i].modOp, percChan[i].carOp);
            }
            else if(!percChan[i].key && newKeyOn) { //keyon event
                percKeyOn(percChan[i].modOp, percChan[i].carOp);

            }
            percChan[i].key = newKeyOn;
        }
    }
    else if(reg >= 0x10 && reg <= 0x18) { // Lower 8 bits of the fNum
        channel.fNum &= 0x100;
        channel.fNum |= val;
        modOp.phaseInc = convertWavelength((channel.fNum * multVal[modOp.inst->multMod]) << channel.octave);
        carOp.phaseInc = convertWavelength((channel.fNum * multVal[carOp.inst->multCar]) << channel.octave);
    }
    else if(reg >= 0x20 && reg <= 0x28) { // Highest bit of fNum, 3 bits of octave/block, Key, and whether to do extra sustain during release
        channel.fNum &= 0xff;
        channel.fNum |= ((val&0x01)<<8);
        channel.octave = ((val>>1) & 0x07);
        bool newKeyOn = ((val>>4) & 0x01);
        modOp.releaseSustain = ((val>>5) & 0x01);
        carOp.releaseSustain = ((val>>5) & 0x01);
        modOp.phaseInc = convertWavelength((channel.fNum * multVal[modOp.inst->multMod]) << channel.octave);
        carOp.phaseInc = convertWavelength((channel.fNum * multVal[carOp.inst->multCar]) << channel.octave);
        if(channel.key && !newKeyOn) { // keyOff event
            dbg_printf("APU::YM2413 melody chan %d (%s) key-off\n", chNum, instNames[channel.instNum].c_str());
            // std::cout<<"Channel "<<int(chNum)<<" key-off\n";
            channel.keyOff(chNum);
        }
        else if(newKeyOn && !channel.key) { // keyOn event
            // std::cout<<"Channel "<<int(chNum)<<" key-on\n";
            channel.keyOn(chNum);
        }
        channel.key = newKeyOn;
    }
    else if(reg >=0x30 && reg <= 0x38) {
        channel.modOp.inst = &inst[val>>4];
        channel.carOp.inst = &inst[val>>4];
        channel.instNum = val>>4;
        channel.volume = val & 0x0f;
        //std::cout<<"Reg: "<<int(reg)<<" Instrument: "<<int(channel.instNum)<<'\n';
    }
}

void YamahaYm2413::setStereo(uint8_t pan) {}

void YamahaYm2413::percKeyOn(YamahaYm2413::op_t* modOp, YamahaYm2413::op_t* carOp) {
    if(modOp) {
        modOp->envAccum = 0;
        modOp->phaseCnt = 0;
        modOp->keyOn = true;
        if(modOp->envPhase == silent) {
            if(modOp->inst->attackMod == 15) {
                modOp->envLevel = 0;
                modOp->envPhase = decay;
            }
            else {
                modOp->envPhase = attack;
            }
        }
        else {
            modOp->envPhase = dampen;
        }
    }

    carOp->envAccum = 0;
    carOp->phaseCnt = 0;
    carOp->keyOn = true;
    if(carOp->envPhase == silent) {
        if(carOp->inst->attackCar == 15) {
            carOp->envLevel = 0;
            carOp->envPhase = decay;
        }
        else {
            carOp->envPhase = attack;
        }
    }
    else {
        carOp->envPhase = dampen;
    }
}

void YamahaYm2413::percKeyOff(YamahaYm2413::op_t* modOp, YamahaYm2413::op_t* carOp) {
    if(modOp) {
        modOp->envAccum = 0;
        modOp->keyOn = false;
        modOp->envPhase = adsrPhase::release;
    }

    carOp->envAccum = 0;
    carOp->keyOn = false;
    carOp->envPhase = adsrPhase::release;
}

// Transition of release -> percussiveRelease, release -> sustainRelease, sustain->release, and sustain->sustainRelease happen in keyOff in applyRegister
void YamahaYm2413::chan_t::keyOff(int chNum) {
    //if(chNum==0 && instNum==12) {std::cout<<"Key-off\n";}
    modOp.envAccum = 0;
    carOp.envAccum = 0;
    modOp.keyOn = false;
    carOp.keyOn = false;
    if(modOp.releaseSustain) modOp.envPhase = adsrPhase::sustainRelease;
    else if(modOp.inst->sustMod) modOp.envPhase = adsrPhase::release;
    else modOp.envPhase = adsrPhase::percussiveRelease;

    if(carOp.releaseSustain) carOp.envPhase = adsrPhase::sustainRelease;
    else if(carOp.inst->sustCar) carOp.envPhase = adsrPhase::release;
    else carOp.envPhase = adsrPhase::percussiveRelease;
}

// Transition to attack, dampen, or decay due to key-on happens in applyRegister.
void YamahaYm2413::chan_t::keyOn(int chNum) {
    //if(chNum==0 && instNum==12) {std::cout<<"Key-on\n";}
    modOp.envAccum = 0;
    carOp.envAccum = 0;
    modOp.phaseCnt = 0;
    carOp.phaseCnt = 0;
    modOp.keyOn = true;
    carOp.keyOn = true;
    if(modOp.envPhase == silent) {
        if(modOp.inst->attackMod == 15) {
            modOp.envLevel = 0;
            modOp.envPhase = decay;
        }
        else {
            modOp.envPhase = attack;
        }
    }
    else {
        modOp.envPhase = dampen;
    }

    if(carOp.envPhase == silent) {
        if(carOp.inst->attackCar == 15) {
            carOp.envLevel = 0;
            carOp.envPhase = decay;
        }
        else {
            carOp.envPhase = attack;
        }
        dbg_printf("APU::YM2413 melody chan %d (%s) attack key-on\n", chNum, instNames[instNum].c_str());
    }
    else {
        carOp.envPhase = dampen;
        dbg_printf("APU::YM2413 melody chan %d (%s) dampen key-on\n", chNum, instNames[instNum].c_str());
    }
}

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
        updatePhases();
        updateEnvelopes();

        int chanMax = (rhythm)?6:9;
        for(int ch=0;ch<chanMax;ch++) {
            chan_t& channel = chan[ch];
            op_t& modOp = channel.modOp;
            op_t& carOp = channel.carOp;

            if(carOp.envPhase != silent) {

                modOp.phaseCnt += modOp.phaseInc;
                int feedback = (modOp.inst->feedbackMod) ? ((modOp.modFB1 + modOp.modFB2) >> (8 - modOp.inst->feedbackMod)) : 0;
                carOp.phaseCnt += carOp.phaseInc;

                int modSin = lookupSin((modOp.phaseCnt / 512) -1 +                              // phase
                                       channel.fmModShift +        // modification for vibrato
                                       (feedback),                                               // modification for feedback
                                       modOp.inst->waveformMod);

                int modOut = lookupExp((modSin) +                                                // sine input
                                       modOp.inst->amModAtten +                           // AM volume attenuation (tremolo)
                                       (modOp.envLevel * 0x10) +                                // Envelope
                                       //modOp.inst->kslModAtten +
                                       (modOp.totalLevel * 0x20));                         // Modulator volume
                modOp.modFB1 = modOp.modFB2;
                modOp.modFB2 = modOut;

                int carSin = lookupSin((carOp.phaseCnt / 512) +                                 // phase
                                       channel.fmCarShift +
                                       (modOut),
                                       carOp.inst->waveformCar);

                buffer[i]+=lookupExp((carSin) +                                                  // sine input
                                     carOp.inst->amCarAtten +                             // AM volume attenuation (tremolo)
                                     (carOp.envLevel * 0x10) +                                  // Envelope
                                     //carOp.inst->kslCarAtten +
                                     (channel.volume * 0x80)) & 0xfff0;                         // Channel volume
            }
        }
        if(rhythm) { // TODO: handle the 5 rhythm instruments (correctly)
            for(int ch = 0; ch < 5; ch++) {
                op_t * modOp = percChan[ch].modOp;
                op_t * carOp = percChan[ch].carOp;
                int modOut = 0, modSin = 0, carSin = 0, feedback = 0;
                int bitComponent;

                if(carOp->envPhase != adsrPhase::silent) {
                    switch(ch) {
                        case 0: // Bass Drum
                            feedback = ((modOp->modFB1 + modOp->modFB2) >> (8 - modOp->inst->feedbackMod));
                            modOp->phaseCnt += modOp->phaseInc;
                            carOp->phaseCnt += carOp->phaseInc;
                            modSin = lookupSin(((modOp->phaseCnt / 512) + feedback), modOp->inst->waveformMod);
                            modOut = lookupExp((modSin) + 
                                            (modOp->envLevel * 0x10) + 
                                            (modOp->totalLevel * 0x20));
                            modOp->modFB1 = modOp->modFB2;
                            modOp->modFB2 = modOut;

                            carSin = lookupSin((carOp->phaseCnt / 512) +                            // phase
                                        (modOut),
                                        carOp->inst->waveformCar);
                            buffer[i]+=(lookupExp((carSin) +                                        // sine input
                                        (carOp->envLevel * 0x10) +                                  // Envelope
                                        (*(percChan[ch].volume) * 0x80)) & 0xfff0)<<1;              // Channel volume
                            break;
                        case 1: // High Hat
                            {
                                bool c85 = chan[8].carOp.phaseCnt & 0x20;
                                bool c83 = chan[8].carOp.phaseCnt & 0x08;
                                bool m77 = chan[7].modOp.phaseCnt & 0x80;
                                bool m73 = chan[7].modOp.phaseCnt & 0x08;
                                bool m72 = chan[7].modOp.phaseCnt & 0x04;
                                bool out = ((c85 ^ c83) & (m77 ^ m72) & (c85 ^ m73));
                                modOut = ((galoisBit) ? (out ? 0xd0 : 0x234) :
                                                        (out) ? 0x34 : 0x2d0);
                                buffer[i] += (lookupExp((modOut) + 
                                                        (carOp->envLevel * 0x10) +
                                                        (*(percChan[ch].volume) * 0x80)) & 0xfff0)<<3;
                            }
                            break;
                        case 2: // Snare Drum
                            bitComponent = ((carOp->phaseCnt) & (1<8));
                            if(bitComponent && galoisBit) modOut = 0x8000; // -max value
                            else if(!bitComponent && !galoisBit) modOut = 0; // +max value
                            else modOut = 4095; // zero value
                            modOut += (carOp->envLevel * 0x10) +
                                      (*(percChan[ch].volume) * 0x80);
                            buffer[i] += (lookupExp(modOut) & 0xfff0)<<2;
                            break;
                        case 3: // Tom-tom
                            carSin = lookupSin((carOp->phaseCnt / 512),                             // phase
                                        carOp->inst->waveformMod); // Tom-tom actually uses a mod op, but it's named as a carrier so I have fewer edge cases
                            buffer[i]+=(lookupExp((carSin) +                                        // sine input
                                        (carOp->envLevel * 0x10) +                                  // Envelope
                                        (*(percChan[ch].volume) * 0x80)) & 0xfff0)<<3;              // Channel volume
                            break;
                        case 4: // Top Cymbal
                            {
                                bool c85 = chan[8].carOp.phaseCnt & 0x20;
                                bool c83 = chan[8].carOp.phaseCnt & 0x08;
                                bool m77 = chan[7].modOp.phaseCnt & 0x80;
                                bool m73 = chan[7].modOp.phaseCnt & 0x08;
                                bool m72 = chan[7].modOp.phaseCnt & 0x04;
                                bool out = ((c85 ^ c83) & (m77 ^ m72) & (c85 ^ m73));
                                modOut = ((out) ? 0 : 0x8000) +
                                         (carOp->envLevel * 0x10) +
                                         (*(percChan[ch].volume) * 0x80);
                                buffer[i] += (lookupExp(modOut) & 0xfff0)<<3;
                            }
                            break;
                    }
                }
            }
        }
        buffer[i] *= 4;
    }

    writeIndex = 0;
    return buffer;
}

void YamahaYm2413::initTables() {
    for (int i = 0; i < 256; ++i) {
        logsinTable[i] = round(-log2(sin((double(i) + 0.5) * M_PI_2 / 256.0)) * 256.0);
        logsinTable[511 - i] = logsinTable[i];
        logsinTable[512 + i] = 0x8000 | logsinTable[i];
        logsinTable[1023 - i] = logsinTable[512+i];
        expTable[255-i] = int(round(exp2(double(i) / 256.0) * 1024.0)) << 1;
    }
    for(int i = 0; i < 1024; ++i) {
        bool sign = i & 512;
        bool mirror = i & 256;
        // half sine wave (positive half, set negative half to 0)
        if(!sign) logsinTable[1024+i] = logsinTable[i];
        else      logsinTable[1024+i] = 0x8000; // constant for -0 value
    }
}

int YamahaYm2413::lookupSin(int val, bool wf) {
    val &= 1023;
    if(wf) {
        return logsinTable[1024+val];
    }
    else {
        return logsinTable[val];
    }
}

int YamahaYm2413::lookupExp(int val) {
    bool sign = val & 0x8000;
    int t = expTable[(val & 255)];
    int result = (t >> ((val & 0x7F00) >> 8)) >> 2;
    if(sign) result = ~result;
    return result;
}

std::array<int,1024*2> YamahaYm2413::logsinTable;
std::array<int,256> YamahaYm2413::expTable;

int YamahaYm2413::convertWavelength(int wavelength) {
    //return (static_cast<int64_t>(wavelength) * static_cast<int64_t>(nativeOplSampleRate)) / static_cast<int64_t>(sampleRate);
    return (static_cast<int64_t>(wavelength) * static_cast<int64_t>(sampleRate)) / static_cast<int64_t>(nativeOplSampleRate);
}

void YamahaYm2413::updatePhases() {
    if(envCounter % amPhaseSampleLength == 0) {
        amPhase++;
        amPhase %= amTable.size();
        int amAtten = amTable[amPhase] * tremoloMultiplier;
        for(auto& ins: inst) {
            if(ins.amCar) ins.amCarAtten = amAtten;
            if(ins.amMod) ins.amModAtten = amAtten;
        }
    }

    if(envCounter % fmPhaseSampleLength == 0) {
        fmPhase++;
        fmPhase &= 7;
        for(auto& ch:chan) {
            if(ch.modOp.inst->vibMod) ch.fmModShift = fmTable[ch.fmRow + fmPhase] * vibratoMultiplier;
            if(ch.carOp.inst->vibMod) ch.fmCarShift = fmTable[ch.fmRow + fmPhase] * vibratoMultiplier;
        }
    }

    galoisBit = lfsrStepGalois();
}

void YamahaYm2413::updateEnvelopes() {
    int c = 1;
    for(auto& ch: chan) {
        // if(ch.modOp.envPhase != silent || ch.carOp.envPhase != silent) std::cout<<"start chan "<<c<<'\n';
        if(ch.modOp.envPhase != adsrPhase::silent) {
            ch.modOp.updateEnvelope(envCounter, true);
        }
        if(ch.carOp.envPhase != adsrPhase::silent) {
            ch.carOp.updateEnvelope(envCounter, false);
        }
        // if(ch.modOp.envPhase != silent || ch.carOp.envPhase != silent) std::cout<<"end chan "<<c<<'\n';
        c++;
    }
}

const std::string YamahaYm2413::instNames[]  {"Custom", "Violin", "Guitar", "Piano",
                                              "Flute", "Clarinet", "Oboe", "Trumpet",
                                              "Organ", "Horn", "Synth", "Harpsichord",
                                              "Vibraphone", "Synth Bass", "Acoustic Bass",
                                              "Electric Guitar"};
const std::string YamahaYm2413::rhythmNames[] {"Bass Drum", "High Hat", 
                                               "Snare Drum", "Tom-tom", "Top Cymbal"};

void YamahaYm2413::op_t::updateEnvelope(unsigned int counter, bool mod) {
    // Phase transitions
    /*
    if(mod) {
        std::cout<<"MOD: phase: "<<adsrPhaseNames[envPhase]<<" envLevel: "<<envLevel<<'\n';
    }
    else {
        std::cout<<"CAR: phase: "<<adsrPhaseNames[envPhase]<<" envLevel: "<<envLevel<<'\n';
    }
    */

    // Transition to attack, dampen, or decay due to key-on happens in applyRegister.
    if(envPhase == dampen && envLevel >= 123) { // Dampen phase ends, so transition to attack (or direct to decay if AR==15)
        envAccum = 0;
        if(mod && inst->attackMod == 15) {
            envLevel = 0;
            envPhase = decay;
        }
        else if(!mod && inst->attackCar == 15) {
            envLevel = 0;
            envPhase = decay;
        }
        else {
            envPhase = attack;
            envLevel = 127;
        }
    }
    else if(envPhase == attack && envLevel <= 0) { // Attack phase ends, so transition to decay
        envPhase = decay;
        envAccum = 0;
        envLevel = 0;
    }
    else if(envPhase == decay && 
            ((mod && envLevel >= (inst->sustainLevelMod) * 8) || 
            (!mod && envLevel >= (inst->sustainLevelCar) * 8))) { // Decay has reached sustain level
        envAccum = 0;
        if (mod && inst->sustMod) { // sound is sustained
            envPhase = sustain;
            envLevel = (inst->sustainLevelMod * 8);
        }
        else if (!mod && inst->sustCar) {
            envPhase = sustain;
            envLevel = (inst->sustainLevelCar * 8);
        }
        else {  // Sound is percussive
            envPhase = release;
        }
    }
    else if(!keyOn && (envPhase == percussiveRelease || envPhase == release || envPhase == releaseSustain) && envLevel >= 123) { // Key released, channel went close enough to silent
        envPhase = silent;
        envAccum = 0;
        envLevel = 127;
    }

    // Transition of release -> percussiveRelease, release -> sustainRelease, sustain->release, and sustain->sustainRelease happen in keyOff in applyRegister

    int activeRate = 0;
    bool attack = false;
    switch(envPhase) {
        case adsrPhase::silent: activeRate = 0; break;
        case adsrPhase::dampen: activeRate = 12; break; // 10ms "DP" is about decay rate 12
        case adsrPhase::attack: 
            if(mod) activeRate = inst->attackMod;
            else    activeRate = inst->attackCar;
            attack = true;
            break;
        case adsrPhase::decay:
            if(mod) activeRate = inst->decayMod;
            else    activeRate = inst->decayCar;
            break;
        case adsrPhase::sustain:
            activeRate = 0;
            break;
        case adsrPhase::percussiveRelease:
            activeRate = 7; // 310 ms "RR" is about decay rate 7
            break;
        case adsrPhase::sustainRelease:
            activeRate = 5; // 1.2s "RS" is about decay rate 5
            break;
        case adsrPhase::release:
            if(mod) activeRate = inst->releaseMod;
            else    activeRate = inst->releaseCar;
            break;
        default: activeRate = 0;
                 std::cout<<"Unhandled envPhase: "<<envPhase<<"\n";
                 break;
    }

    if(activeRate != 0 && (!attack || activeRate != 15)) {
        envAccum += envAccumRate;
        int targetValue = 0;
        int levelsToChange = 0;
        if(attack) {
            int index;
            if(mod) index = std::min(63, activeRate * 4);// + ksrModIndex);
            if(!mod) index = std::min(63, activeRate * 4);// + ksrCarIndex);
            targetValue = attackTable[index];
            levelsToChange = envAccum / targetValue;
            envAccum = envAccum % targetValue;
            envLevel -= levelsToChange;
        }
        else {
            int index;
            if(mod) index = std::min(63, activeRate * 4);// + ksrModIndex);
            if(!mod) index = std::min(63, activeRate * 4);// + ksrCarIndex);
            targetValue = decayTable[index];
            levelsToChange = envAccum / targetValue;
            envAccum = envAccum % targetValue;
            envLevel += levelsToChange;
        }
    }

    //if(envLevel > 127) std::cout<<std::dec<<"Env at "<<envLevel<<"\n";
    if(envLevel < 0) envLevel = 0; // assume wrap-around
    else if(envLevel > 127) envLevel = 127; //assume it just overflowed the 7-bit value
}

int YamahaYm2413::lfsrStepGalois() {
    bool output = galoisState & 1;
    galoisState >>= 1;
    if (output) galoisState ^= 0x400181;
    return output;
}            

const std::array<int,210> YamahaYm2413::amTable {
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4,
    4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9,
    9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,
    15,15,15,15,16,16,16,16,17,17,17,17,18,18,18,18,19,19,19,19,20,
    20,20,20,21,21,21,21,22,22,22,22,23,23,23,23,24,24,24,24,25,25,
    25,25,26,26,26,25,25,25,25,24,24,24,24,23,23,23,23,22,22,22,22,
    21,21,21,21,20,20,20,20,19,19,19,19,18,18,18,18,17,17,17,17,16,
    16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11,11,
    11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6,
    6, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1
    };

const std::array<int,64> YamahaYm2413::fmTable {{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0,-1, 0,
    0, 1, 2, 1, 0,-1,-2,-1,
    0, 1, 3, 1, 0,-1,-3,-1,
    0, 2, 4, 2, 0,-2,-4,-2,
    0, 2, 5, 2, 0,-2,-5,-2,
    0, 3, 6, 3, 0,-3,-6,-3,
    0, 3, 7, 3, 0,-3,-7,-3
}};

const std::array<int,128> YamahaYm2413::kslTable {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 8,
    0, 0, 0, 0, 0, 3, 5, 7, 8,10,11,12,13,14,15,16,
    0, 0, 0, 5, 8,11,13,15,16,18,19,20,21,22,23,24,
    0, 0, 8,13,16,19,21,23,24,26,27,28,29,30,31,32,
    0, 8,16,21,24,27,29,31,32,34,35,36,37,38,39,40,
    0,16,24,29,32,35,37,39,40,42,43,44,45,46,47,48,
    0,24,32,37,40,43,45,47,48,50,51,52,53,54,55,56
    };

// These are measured in microseconds per change of level
const std::array<int,64> YamahaYm2413::attackTable {
    0,0,0,0,
    13623, 11028, 9082, 7785, 
    6818, 6144, 4541, 3892, 
    3406, 2820, 2208, 1946, 
    1703, 1379, 1138, 972, 
    851, 689, 574, 487, 
    432, 345, 284, 243, 
    213, 165, 142, 122, 
    106, 86, 71, 61, 
    53, 43, 36, 30, 
    26, 22, 18, 15, 
    13, 11, 9, 8, 
    7, 6, 5, 4, 
    4, 3, 3, 2, 
    2, 2, 1, 1, 
    0, 0, 0, 0
};

// These are measured in microseconds per change of level
const std::array<int,64> YamahaYm2413::decayTable {
    0,0,0,0,
    164776, 132340, 115014, 95108, 
    82388, 66170, 55142, 47357, 
    41194, 33085, 27571, 23678, 
    20597, 17172, 13785, 11839, 
    10299, 8271, 6893, 5920, 
    5149, 4136, 3446, 2960, 
    2575, 2068, 1723, 1480, 
    1287, 1034, 862, 740, 
    644, 517, 431, 370, 
    316, 252, 215, 185, 
    161, 129, 107, 93, 
    80, 65, 54, 46, 
    40, 32, 27, 23, 
    20, 16, 13, 12, 
    10, 10, 10, 10
};

const std::array<std::string,8> YamahaYm2413::adsrPhaseNames {
    "silent",
    "dampen",
    "attack",
    "decay",
    "sustain",
    "percussiveRelease",
    "sustainRelease",
    "release"
};