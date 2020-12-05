CONFIG_MODULE_SIG=n
obj-m		:= step1.o
step1-y := main.o rbtreeM.o



KERNEL_DIR	:= /lib/modules/$(shell uname -r)/build
PWD 		:= $(shell pwd)


default:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
