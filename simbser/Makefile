CC = gcc
CFLAGS = -c -std=gnu99
IFLAGS = -I./
LFLAGS = -pthread

all: server.o
	$(CC) $(LFLAGS) server.o -o server

server.o: server.c server.h
	$(CC) $(CFLAGS) $(IFLAGS) server.c -o server.o

clean:
	$(RM) *.o server

.PHONY: all clean
