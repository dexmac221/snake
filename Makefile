CFLAGS = -O -t $(TARGET)
LDFLAGS = -t $(TARGET)
CC = cc65
CA = ca65
LD = cl65

c64dnn.prg: c64dnn.o 
	$(LD) $(LDFLAGS) -o snake.prg main.o $(CC65_HOME)/lib/$(TARGET).lib

c64dnn.o: main.c
	$(CC) $(CFLAGS) main.c ; $(CA) main.s

# remove object files and executable when user executes "make clean"
clean:
	rm *.s *.o snake.prg
