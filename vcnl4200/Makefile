PWD   := $(shell pwd)

obj-m := vcnl4200_drv.o

include ../rules.mak

KDIR := ../kernel

all:
	make -C $(KDIR) M=$(PWD) modules LDFLAGS= ARCH=arm CROSS_COMPILE=arm-fsl-linux-gnueabi-

clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers

