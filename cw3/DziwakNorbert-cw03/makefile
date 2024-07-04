CC = gcc
CFLAGS = -Wall -g

.PHONY: start clean

all:
	$(CC) $(CFLAGS) -o flipper flipper.c

start : all
	./flipper ./art ./results 

clean:
	rm -f flipper
	rm -f results/*