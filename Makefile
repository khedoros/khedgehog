objects:=config.o cpu/m68k/memmapM68k.o emulator.o main.o cpu/m68k/cpuM68k.o util.o cpu/z80/cpuZ80.o cpu/z80/memmapZ80Console.o vdp/masterSystem/vdpMS.o debug_console.o io/ioMgr.o vdp/genesis/vdpGenesis.o apu/tiPsg.o apu/yamahaYm2413.o apu/masterSystem/apuMS.o apu/genesis/apuGenesis.o apu/apu.o
target:=khedgehog

SDLCFLAGS:=$(shell sdl2-config --cflags)
SDLLDFLAGS:=$(shell sdl2-config --libs)
CXXFLAGS:=-std=c++17 -flto -O3 $(SDLCFLAGS)

ifdef DEBUG
    debug:=-g
    CXXFLAGS+=-g
    target:=khedgehog-dbg
    objects:=$(subst .o,.od,$(objects))
endif

$(target): $(objects)
	$(CXX) $(debug) -flto -O3 -o $@ $^ $(SDLLDFLAGS)

%.od: %.cpp
	$(CXX) -c $(debug) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

clean:
	-rm $(target) $(objects) 
