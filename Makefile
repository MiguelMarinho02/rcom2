CC = gcc
CFLAGS = -Wall

SRC = src/
INCLUDE = include/
BIN = bin/

.PHONY: download
download: $(SRC)/download.c $(SRC)/url.c
	$(CC) $(CFLAGS) -o download $^

.PHONY: clean
clean:
	rm -rf download