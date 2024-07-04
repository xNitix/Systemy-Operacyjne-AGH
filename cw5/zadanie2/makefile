CC = gcc
FLAGS = -Wall -g

.PHONY: clean

all:
	$(CC) $(FLAGS) catcher.c -o catcher
	$(CC) $(FLAGS) sender.c -o sender

clean:
	rm -f catcher sender