obj-m += inputDevice.o
obj-m += outputDevice.o

all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
