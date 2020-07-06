objects:=config.o cpu/genesis/memmapGenesisCpu.o emulator.o main.o cpu/m68k/cpuM68k.o util.o
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
