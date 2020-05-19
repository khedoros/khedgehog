objects:=config.o emulator.o main.o
ifdef DEBUG
    debug:=-g
else
	debug:=
endif

khedgehog: $(objects)
	g++ $(debug) -o $@ $^

clean:
	-rm khedgehog $(objects) 
