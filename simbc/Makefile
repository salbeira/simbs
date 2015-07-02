CC = gcc
CFLAGS = -c
IFLAGS = -I./
LFLAGS = -pthread -lncurses

all: client.o client_screen.o
	$(CC) $(LFLAGS) client.o client_screen.o -o simbc

client.o: client.h client.c
	$(CC) $(CFLAGS) $(IFLAGS) client.c -o client.o

client_screen.o: client_screen.h client_screen.c
	$(CC) $(CFLAGS) $(IFLAGS) client_screen.c -o client_screen.o

clean:
	$(RM) *.o simbc

.PHONY: all clean
