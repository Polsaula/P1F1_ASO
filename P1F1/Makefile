obj-m := P1F1_LKM.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
install:
	sudo insmod P1F1_LKM.ko
remove:
	sudo rmmod P1F1_LKM.ko