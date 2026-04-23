CC = gcc
CFLAGS = -Wall -O2
INSTALL_DIR = /usr/local/bin

all: 2591

2591: 2591.c
	$(CC) $(CFLAGS) -o 2591 2591.c

install: 2591
	install -m 755 2591 $(INSTALL_DIR)/2591

clean:
	rm -f 2591
