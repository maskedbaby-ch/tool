TARGET = filter

KDIR = /lib/modules/$(shell uname -r)/build
 
PWD = $(shell pwd)
 
obj-m += $(TARGET).o
 
default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm *.o *.ko *.mod *.order *.symvers *.mod.c
