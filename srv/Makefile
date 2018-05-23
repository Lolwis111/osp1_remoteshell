#!/usr/bin/make
.SUFFIXES:
TAR = server
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

%.o: %.c %.h
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

server: $(OBJ)
	$(CC) $^ -o $@

all: $(TAR)

clean:
	$(RM) $(RMFILES) $(OBJ) $(TAR)
