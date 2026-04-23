CC = gcc
CFLAGS = -Wall -O2
INSTALL_DIR = /usr/local/bin

all: 2561

TSL2561.o: TSL2561.c TSL2561.h
	$(CC) $(CFLAGS) -o TSL2561.o -c TSL2561.c

2561.o: 2561.c TSL2561.h
	$(CC) $(CFLAGS) -o 2561.o -c 2561.c

2561: TSL2561.o 2561.o
	$(CC) $(CFLAGS) -o 2561 TSL2561.o 2561.o

install: 2561
	install -m 755 2561 $(INSTALL_DIR)/2561

clean:
	rm -f *.o 2561
