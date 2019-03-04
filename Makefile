all: build run clean

build: ksender kreceiver

ksender: ksender.o link_emulator/lib.o utils.h
	gcc -Wall -g ksender.o link_emulator/lib.o -o ksender

kreceiver: kreceiver.o link_emulator/lib.o utils.h
	gcc -Wall -g kreceiver.o link_emulator/lib.o -o kreceiver

run:
	./run_experiment.sh

.c.o:
	gcc -Wall -g -c $?
clean:
	-rm -f *.o ksender kreceiver 
