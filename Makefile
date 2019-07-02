CC=gcc
CFLAGS+=-g

all: main

main: main.o

main.o: main.c

clean:
	rm -rf *.o main