CC = gcc
FLAGS = -Wall -g

.PHONY: clean

all:
	$(CC) $(FLAGS) integral.c -o integral
	$(CC) $(FLAGS) sender.c -o sender

make clean:
	rm -f integral sender fifoInput fifoOutput