DEBFLAGS = -O2

EXTRA_CFLAGS = $(DEBFLAGS)
EXTRA_CFLAGS += -I$(LDDINC)

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= aufgabe.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

build:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules	

endif



clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

#depend .depend dep:
	#$(CC) $(CFLAGS) -M *.c > .depend


#ifeq (.depend,$(wildcard .depend))
#include .depend
#endif

