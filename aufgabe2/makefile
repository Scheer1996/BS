CC = gcc
CFLAGS = -g -mno-ms-bitfields 
LDFLAGS = -g -pthread

SRC = main.c FIFO.c
OBJ = $(SRC:%.c=%.o)

all: aufgabe2

aufgabe2: $(OBJ)
	$(CC) -o aufgabe2 $(LDFLAGS) $(OBJ)

.PHONY: deps
deps:
	$(CC) -MM $(SRC) > makefile.dependencies

clean:
	rm *.o aufgabe2
include makefile.dependencies