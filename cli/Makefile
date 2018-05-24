#!/usr/bin/make
.SUFFIXES:
TAR = client
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

%.o: %.c %.h
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

client: $(OBJ)
	$(CC) $^ -lpthread -o $@

all: $(TAR)

clean:
	$(RM) $(RMFILES) $(OBJ) $(TAR)
