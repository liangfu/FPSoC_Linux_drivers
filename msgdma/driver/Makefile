PATH_SETTINGS?=$(PWD)/../Settings.mak
# Driver configuration file
include $(PATH_SETTINGS)

# kbuild 
ifeq ($(KERNELRELEASE),)

ARCH=arm


default:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSOURCE_DIR) M=`pwd` modules

clean:
	make -C $(KSOURCE_DIR) M=`pwd` clean

else

INC=-I$(PWD)/../include/

obj-m 	  := msgdma.o
ccflags-y := -DCOMPATIBLE_STRING="\"$(COMPATIBLE_STRING)\"" \
			 -DDRIVER_NODE_NAME="\"$(DRIVER_NODE_NAME)\"" \
			 -DMSGDMA_IOCTL_MAGIC=$(MSGDMA_IOCTL_MAGIC) \
			 -DMSGDMA_DEBUG=$(MSGDMA_DEBUG) \
			 $(INC)

endif
