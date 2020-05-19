#pragma once
#include<memory>

#include "io/ioMgr.h"

class config;

class emulator {
public:
    static std::shared_ptr<emulator> getEmulator(std::shared_ptr<config> cfg);
    virtual int run() = 0;
protected:
    std::shared_ptr<config> cfg;
    std::shared_ptr<ioMgr>  io;
};

class genesisEmulator: public emulator {
public:
    genesisEmulator(std::shared_ptr<config> cfg);
    int run() override;
private:
};

class smsEmulator: public emulator {
public:
    smsEmulator(std::shared_ptr<config> cfg);
    int run() override;
private:
};

class ggEmulator: public emulator {
public:
    ggEmulator(std::shared_ptr<config> cfg);
    int run() override;
private:
};

