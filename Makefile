objects:=config.o emulator.o main.o

khedgehog: $(objects)
	g++ -o $@ $^

clean:
	-rm khedgehog $(objects) 
