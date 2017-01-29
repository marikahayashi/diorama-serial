CC=gcc -Wall
CFLAGS = -g -I/usr/include

all: main

main: main.o sio.o
	$(CC) -o main main.o sio.o

main.o: main.c
	$(CC) -c main.c

sio.o: sio.c
	$(CC) -c sio.c

clean:
	rm -f *~ *.o main
