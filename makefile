CC=g++
CFLAGS=-I.

all: SIM

SIM: SIM.o
	$(CC) SIM.cpp -o SIM

clean: 
	-rm *.o
