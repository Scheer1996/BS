FOO=$PATH
GCC=gcc
CFLAGS = -DVMEM_PAGESIZE=$(VMEM_PAGESIZE)

all: mmanage vmappl

mmanage: mmanage.o logger.o pagefile.o
	gcc -DVMEM_PAGESIZE=$(VMEM_PAGESIZE) -o mmanage mmanage.o logger.o pagefile.o -pthread
vmappl: vmappl.o vmaccess.o 
	gcc -DVMEM_PAGESIZE=$(VMEM_PAGESIZE) -o vmappl vmappl.o vmaccess.o -pthread
mmanage.o:mmanage.h mmanage.c logger.h pagefile.h debug.h
	gcc -c mmanage.c
logger.o: logger.h logger.c
	gcc -c logger.c
pagefile.o: pagefile.c pagefile.h
	gcc -c pagefile.c
vmaccess.o: vmaccess.h vmaccess.c
	gcc -c vmaccess.c
vmappl.o:vmappl.c vmappl.h
	gcc -c vmappl.c
	
clean:
	rm *.o mmanage vmappl
	
	
VMEM_PAGESIZE=8
	
CC = gcc
CFLAGS = -DVMEM_PAGESIZE=$(VMEM_PAGESIZE)
LDFLAGS = -lpthread

SRC1 = mmanage.c pagefile.c logger.c
SCR2 = vmaccess.c vmappl.c 
OBJ1 = $(SRC1:%.c=%.o)
OBJ2 = $(SRC2:%.c=%.o)

all: mmanage vmappl

mmanage: $(OBJ1)
	$(CC) -o mmanage $(OBJ1) $(LDFLAGS)
vmappl: $(OBJ2)
	 $(CC) -o vmappl $(OBJ2) $(LDFLAGS)


.PHONY: clean
clean:
	rm -rf $(OBJ1)
	rm -rf $(OBJ2)
	rm -rf mmanage vmappl
	rm -rf logfile.txt pagefile.bin

.PHONY: deps
deps: 	
	$(CC) $(SRC) > makefile.dependencies
