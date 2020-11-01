#include "cpuZ80.h"
uint64_t cpuZ80::calc(uint64_t) {
    return 0;
}

cpuZ80::cpuZ80(std::shared_ptr<memmapZ80> memmap): memory(memmap) {

}
