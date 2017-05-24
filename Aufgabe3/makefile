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
