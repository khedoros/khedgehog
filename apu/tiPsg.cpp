#include "tiPsg.h"

TiPsg::TiPsg(): noiseLfsr(1) {
    for(int i=0;i<4;i++) {
        stereoLeft[i] = true;
        stereoRight[i] = true;
        attenuation[i] = 15;
        toneCountReset[i] = 0;
        currentOutput[i] = true;
    }

    for(int i=0;i*40<735*2;i++) {
        for(int j = 0; j < 20 && i*40+j < 735*2;j++) {
            buffer[i*40+j] = 10000;
        }
        for(int j = 20; j < 40 && i*40+j < 735*2;j++) {
            buffer[i*40+j] = -10000;
        }
    }
}

void TiPsg::mute(bool) {}
void TiPsg::writeRegister(uint8_t val) {
    if(val & 0x80) { //register
        //        %1cctdddd
        //        cc = channel
        //        t = type (0 = data, 1 = volume)
        //        dddd = data
        //
        //        Noise: Lowest 2 bits are the shift rate, 3rd is noise mode.
        latchedChannel = ((val>>5)&0x03);
        latchedType = ((val>>4)&0x01);
        if(latchedType) { // volume
            attenuation[latchedChannel] = (val & 0x0f);
        }
        else { // wavelength data
            if(latchedChannel < 3) { // Square waves
                toneCountReset[latchedChannel] &= 0b1111110000; // clear lowest 4 bits
                toneCountReset[latchedChannel] |= (val & 0x0f); // set lowest 4 bits
                if(latchedChannel == 2 && noiseShiftRate == 3) {
                    toneCountReset[3] = toneCountReset[2];
                }
            }
            else { // Noise channel
                noiseMode = ((val>>2) & 1);
                noiseShiftRate = (val & 0x03);
                noiseLfsr = 1;
                switch(noiseShiftRate) {
                    case 0: toneCountReset[3] = 0x10; break;
                    case 1: toneCountReset[3] = 0x20; break;
                    case 2: toneCountReset[3] = 0x40; break;
                    case 3: toneCountReset[3] = toneCountReset[2]; break;
                }
            }
        }
    }
    else { //data
        // %0_dddddd
        // dddddd = data, either top 6 bits of 10-bit wavelength counter, 4 bits of volume, 3 bits of noise settings.
        if(latchedChannel < 3) {
            toneCountReset[latchedChannel] &= 0b1111;
            toneCountReset[latchedChannel] |= ((val & 0b00111111)<<4);
            if(latchedChannel == 2 && noiseShiftRate == 3) {
                toneCountReset[3] = toneCountReset[2];
            }
        }
        else {
            noiseMode = ((val>>2) & 1);
            noiseShiftRate = (val & 0x03);
            noiseLfsr = 1;
            switch(noiseShiftRate) {
                case 0: toneCountReset[3] = 0x10; break;
                case 1: toneCountReset[3] = 0x20; break;
                case 2: toneCountReset[3] = 0x40; break;
                case 3: toneCountReset[3] = toneCountReset[2]; break;
            }
        }
    }
}

void TiPsg::setStereo(uint8_t val) {
    for(int i=0;i<4;i++) {
        stereoRight[i] = (val & 0x01);
        val>>=1;
    }
    for(int i=0;i<4;i++) {
        stereoLeft[i] = (val & 0x01);
        val>>=1;
    }
}

std::array<int16_t, 735 * 2>& TiPsg::getSamples() {
    return buffer;
}

