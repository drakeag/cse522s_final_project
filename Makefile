# TODO: Change this to the location of your kernel source code
KERNEL_SOURCE=/home/pi/linux_source/linux

EXTRA_CFLAGS += -DMODULE=1 -D__KERNEL__=1

mmu_module-objs := $(mmu_module-y)
obj-m := mmu_module.o

PHONY: all

all:
	$(MAKE) -C $(KERNEL_SOURCE) M=$(PWD) modules
	gcc mmu_demo.c -o mmu_demo.out

clean:
	$(MAKE) -C $(KERNEL_SOURCE) M=$(PWD) clean
	rm *.out
