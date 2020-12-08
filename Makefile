objects:=config.o cpu/m68k/memmapM68k.o emulator.o main.o cpu/m68k/cpuM68k.o util.o cpu/z80/cpuZ80.o cpu/z80/memmapZ80Console.o vdp/masterSystem/vdpMS.o debug_console.o

CXXFLAGS:=-std=c++17 -flto -O3

ifdef DEBUG
    debug:=-g
    CXXFLAGS+=-g
else
	debug:=
endif

khedgehog: $(objects)
	g++ $(debug) -flto -O3 -o $@ $^

clean:
	-rm khedgehog $(objects) 
