#pragma once
#include "../cpu.h"
#include "memmapZ80.h"
#include<memory>

class cpuZ80 : public cpu {
private:
    std::shared_ptr<memmapZ80> memory;
public:
    cpuZ80(std::shared_ptr<memmapZ80>);
    uint64_t calc(uint64_t);
};
