CC = gcc
CFLAGS = -Wall -Wextra

all: client printers

client: client.c
	$(CC) $(CFLAGS) -o client client.c

printers: printers.c
	$(CC) $(CFLAGS) -o printers printers.c

clean:
	rm -f client printers
