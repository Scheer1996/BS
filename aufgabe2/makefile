CC = gcc
CFLAGS = -std=c99 -g -mno-ms-bitfields
LDFLAGS = -g

SRC = main.c FIFO.c
OBJ = $(SRC:%.c=%.o)

all: aufgabe2

aufgabe2: $(OBJ)
	$(CC) -o aufgabe2 $(LDFLAGS) $(OBJ)

.PHONY: deps
deps:
	$(CC) -MM $(SRC) > makefile.dependencies

include makefile.dependencies