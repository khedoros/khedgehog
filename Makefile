objects:=config.o cpu/m68k/memmapM68k.o emulator.o main.o cpu/m68k/cpuM68k.o util.o cpu/z80/cpuZ80.o cpu/z80/memmapZ80Console.o

CXXFLAGS:=-std=c++17

ifdef DEBUG
    debug:=-g
    CXXFLAGS+=-g
else
	debug:=
endif

khedgehog: $(objects)
	g++ $(debug) -o $@ $^

clean:
	-rm khedgehog $(objects) 
