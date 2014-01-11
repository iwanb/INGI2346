CC=gcc
CFLAGS= -Wall 
LDFLAGS= -lnsl 
EXEC=pvm3
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
	
all: ircd.c irc.c response.c arrays.c client.c
	$(CC) -o ircd ircd.c response.c arrays.c client.c $(LDFLAGS) $(CFLAGS)
	$(CC) -o irc irc.c response.c arrays.c client.c $(LDFLAGS) $(CFLAGS)
