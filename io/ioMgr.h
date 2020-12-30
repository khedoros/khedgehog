#pragma once
#include<vector>
#include<cstdint>

#include "ioEvent.h"
#include "../config.h"

class ioMgr;

class ioMgr {
public:
    ioMgr(std::shared_ptr<config> cfg);
    bool createWindow(int xres, int yres);
    bool updateWindow(int startx, int starty, const std::vector<std::vector<uint8_t>>& image); // need to decide how I want to represent the image. Maybe std::vector<std::vector<uint8_t>>?
    ioEvent getEvent();

private:
    uiBackend backend;
    int xres, yres;

};
