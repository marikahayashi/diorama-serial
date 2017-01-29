CC=gcc -Wall
CFLAGS = -g -I/usr/include

all: main

main: main.o serial.o
	$(CC) -o main main.o serial.o

main.o: main.c
	$(CC) -c main.c

serial.o: serial.c
	$(CC) -c serial.c

clean:
	rm -f *~ *.o main
