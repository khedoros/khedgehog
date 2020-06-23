objects:=config.o memmap_m68k.o emulator.o main.o cpu/cpu_m68k.o
ifdef DEBUG
    debug:=-g
else
	debug:=
endif

khedgehog: $(objects)
	g++ $(debug) -o $@ $^

clean:
	-rm khedgehog $(objects) 
