#include "ioMgr.h"

ioMgr::ioMgr(std::shared_ptr<config>) {

}

bool ioMgr::createWindow(int xres, int yres) { return false; }
bool ioMgr::updateWindow(int startx, int starty, const std::vector<std::vector<uint8_t>>& image) { return false; }
ioEvent ioMgr::getEvent() { return ioEvent{}; }

