
# Driver configuration file
include $(PWD)/../Settings.mak


ifeq ($(KERNELRELEASE),)
# kbuild

default:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSOURCE_DIR) M=`pwd` modules

clean:
	make -C $(KSOURCE_DIR) M=`pwd` clean

else
# run from Kernel Makefile

obj-m 	  := pll_control.o
ccflags-y := -DDRIVER_NODE_NAME="\"$(DRIVER_NODE_NAME)\"" \
			 -DCOMPATIBLE_STRING="\"$(COMPATIBLE_STRING)\"" \
			 -DDEBUG_PLL_CTL=$(DEBUG_PLL_CTL) \
			 -DPLL_IOC_MAGIC=$(PLL_IOC_MAGIC) 
endif
