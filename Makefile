CC=gcc
CFLAGS+=-g

all: main

main: main.o nes_system.o nes_cpu.o nes_memory.o

main.o: main.c nes_system.h nes_cpu.h nes_memory.h

nes_system.o: nes_system.c nes_system.h nes_cpu.h nes_memory.h

nes_cpu.o: nes_cpu.c nes_cpu.h

nes_memory.o: nes_memory.c nes_memory.h

clean:
	rm -rf *.o main