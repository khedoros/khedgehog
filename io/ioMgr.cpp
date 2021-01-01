#include "ioMgr.h"
#include<tuple>

ioMgr::ioMgr(std::shared_ptr<config> cfg): screen(nullptr), renderer(nullptr), buffer(nullptr), texture(nullptr), overlay(nullptr) {
    Uint32 sdl_init_flags = SDL_INIT_EVERYTHING|SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER;
    //if(headless) sdl_init_flags &= (~SDL_INIT_VIDEO);
    //if(!audio)   sdl_init_flags &= (~SDL_INIT_AUDIO);
    if(SDL_Init(sdl_init_flags) < 0 ) {
        fprintf(stderr,"Couldn't initialize SDL: %s\n--nosound would disable audio, --headless would disable video. Those might be worth a try, depending on the above error message.\n", SDL_GetError());
    }
    auto res = cfg->getResolution();

    if(reinitWindow(res.first, res.second)) {
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderFillRect(renderer, nullptr);
        SDL_RenderPresent(renderer);
    }
}

bool ioMgr::updateWindow(int startx, int starty, const std::vector<std::vector<uint8_t>>& image) { return false; }

ioEvent ioMgr::getEvent() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_KEYDOWN:  /* Handle a KEYDOWN event */
                if(event.key.keysym.scancode==SDL_SCANCODE_Q||
                        (event.key.keysym.scancode==SDL_SCANCODE_C&&(event.key.keysym.mod==KMOD_RCTRL))||
                        (event.key.keysym.scancode==SDL_SCANCODE_C&&(event.key.keysym.mod==KMOD_LCTRL))) {
                    SDL_Quit();
                    return ioEvent{ioEvent::eventType::window, ioEvent::windowEvent::exit};
                }
                break;
            case SDL_KEYUP: /* Handle a KEYUP event*/
                break;
            case SDL_WINDOWEVENT:
                switch(event.window.event) {
                    case SDL_WINDOWEVENT_SHOWN:
                        break;
                    case SDL_WINDOWEVENT_HIDDEN:
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        //newx=event.window.data1;
                        //newy=event.window.data2;
                        //bus->win_resize(newx, newy);
                        break;
                    case SDL_WINDOWEVENT_EXPOSED:
                        break;
                    case SDL_WINDOWEVENT_CLOSE:
                        SDL_Quit();
                        return ioEvent(ioEvent::eventType::window, ioEvent::windowEvent::exit);
                    default:
                        //printf("something else (%d)\n", event.window.event);
                        break;
                }
                break;
            case SDL_QUIT:
                SDL_Quit();
                return ioEvent(ioEvent::eventType::window, ioEvent::windowEvent::exit);
                break;
            case SDL_MOUSEMOTION: case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: case SDL_MOUSEWHEEL:
                break;
            case SDL_TEXTINPUT:
                break;
            case SDL_KEYMAPCHANGED:
                break;
            case SDL_AUDIODEVICEADDED:
                break;
            default: /* Report an unhandled event */
                //printf("util::I don't know what this event is (%d)! Flushing it.\n", event.type);
                SDL_FlushEvent(event.type);
                break;
        }
    }
    return ioEvent(ioEvent::eventType::none);
}

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
