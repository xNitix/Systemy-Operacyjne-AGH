CC = gcc
Cflags = -Wall -g

.PHONY: start clean

all: 
	$(CC) $(Cflags) -o descendant.o descendant.c

start: all
	./descendant.o 10

clean:	
	rm descendant.o