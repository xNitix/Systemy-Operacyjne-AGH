CC = gcc
CFLAGS = -Wall -O0 -g

.PHONY: all clean start

all:
	make static
	make shared
	make dynamic

start: all
	./static
	./shared
	./dynamic

// --------------------------------------------------------
// biblioteki
collatz_static: collatz.c
	$(CC) $(CFLAGS) -c collatz.c
	ar rcs libcollatz.a collatz.o 

collatz_shared: collatz.c
	$(CC) $(CFLAGS) -c -fPIC collatz.c
	$(CC) $(CFLAGS) -shared -o libcollatz.so collatz.o

// ---------------------------------------------------------
// klienci

static: client.c collatz_static
	$(CC) $(CFLAGS) -o static client.c libcollatz.a

shared: client.c collatz_shared
	$(CC) $(CFLAGS) -o shared client.c -L ./ -Wl,-rpath,. -lcollatz

dynamic: client.c collatz_shared
	$(CC) $(CFLAGS) -o dynamic client.c -ldl -D DYNAMIC


clean:
	rm -f *.o *.a *.so static shared dynamic