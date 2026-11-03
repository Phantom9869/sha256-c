CC ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Werror -pedantic

all: sha256

sha256: main.o sha256.o
	$(CC) $(CFLAGS) -o $@ main.o sha256.o

test_sha256: test_sha256.o sha256.o
	$(CC) $(CFLAGS) -o $@ test_sha256.o sha256.o

%.o: %.c sha256.h
	$(CC) $(CFLAGS) -c $<

test: test_sha256 sha256
	./test_sha256
	./compare.sh

clean:
	rm -f *.o sha256 test_sha256

.PHONY: all test clean
