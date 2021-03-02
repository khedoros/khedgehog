#include "tiPsg.h"

TiPsg::TiPsg() {}
void TiPsg::mute(bool) {}
void TiPsg::writeRegister(uint8_t val) {
    if(val & 0x80) { //register
        //        %1cctdddd
        //        cc = channel
        //        t = type (0 = data, 1 = volume)
        //        dddd = data
        //
        //        Noise: Lowest 2 bits are the shift rate, 3rd is noise mode.

    }
    else { //data
        // %0_dddddd
        // dddddd = data, either top 6 bits of 10-bit wavelength counter, 4 bits of volume, 3 bits of noise settings.

    }
}
void TiPsg::setStereo(uint8_t) {

}
