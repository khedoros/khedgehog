#include "ioMgr.h"
#include<tuple>

ioMgr::ioMgr(std::shared_ptr<config> cfg) {
    Uint32 sdl_init_flags = SDL_INIT_EVERYTHING|SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER;
    //if(headless) sdl_init_flags &= (~SDL_INIT_VIDEO);
    //if(!audio)   sdl_init_flags &= (~SDL_INIT_AUDIO);
    if(SDL_Init(sdl_init_flags) < 0 ) {
        fprintf(stderr,"Couldn't initialize SDL: %s\n--nosound would disable audio, --headless would disable video. Those might be worth a try, depending on the above error message.\n", SDL_GetError());
    }
    auto res = cfg->getResolution();

    reinitWindow(res.first, res.second);
}

bool ioMgr::updateWindow(int startx, int starty, const std::vector<std::vector<uint8_t>>& image) { return false; }
ioEvent ioMgr::getEvent() { return ioEvent{}; }

bool ioMgr::reinitWindow(unsigned int xres, unsigned int yres) {
    if(screen) {
        SDL_DestroyWindow(screen);
        screen = NULL;
    }
    if(renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if(texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }

    /* Initialize the SDL library */
    screen = SDL_CreateWindow("Khedgehog",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              xres*2, yres*2,
                              SDL_WINDOW_RESIZABLE);
    if ( !screen ) {
        fprintf(stderr, "ioMgr::Couldn't set %dx%dx32 video mode: %s\nStarting without video output.\n", xres*2, yres*2, SDL_GetError());
        return false;
    }

    SDL_SetWindowMinimumSize(screen, xres, yres);

    renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED/*|SDL_RENDERER_PRESENTVSYNC*/);
    //renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    //renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE|SDL_RENDERER_PRESENTVSYNC);
    //renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE/*|SDL_RENDERER_PRESENTVSYNC*/);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    if(!renderer) {
        fprintf(stderr, "ioMgr::Couldn't create a renderer: %s\nStarting without video output.\n", SDL_GetError());
        SDL_DestroyWindow(screen);
        screen = NULL;
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,xres,yres);
    if(!texture) {
        fprintf(stderr, "ioMgr::Couldn't create a texture: %s\nStarting without video output.\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
        SDL_DestroyWindow(screen);
        screen = NULL;
        return false;
    }
    return true;
}
