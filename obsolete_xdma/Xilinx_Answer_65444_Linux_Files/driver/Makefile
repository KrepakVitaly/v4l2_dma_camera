SHELL = /bin/bash

TEGRA_KERNEL_OUT=/home/user/sources/xavier/build
LOCALVERSION=-tegra
CROSS_COMPILE=/home/user/sources/xavier/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
JETPACK=/home/user/nvidia/nvidia_sdk/JetPack_4.4_Linux_JETSON_AGX_XAVIER


ifneq ($(xvc_bar_num),)
	        XVC_FLAGS += -D__XVC_BAR_NUM__=$(xvc_bar_num)
	endif

ifneq ($(xvc_bar_offset),)
	        XVC_FLAGS += -D__XVC_BAR_OFFSET__=$(xvc_bar_offset)
	endif

$(warning XVC_FLAGS: $(XVC_FLAGS).)


topdir := $(shell cd $(src)/.. && pwd)

TARGET_MODULE:=xdma

EXTRA_CFLAGS := -I$(topdir)/include $(XVC_FLAGS)
EXTRA_CFLAGS += -D__LIBXDMA_DEBUG__
EXTRA_CFLAGS += -DINTERNAL_TESTING

obj-m += xdma.o
xdma-objs := xdma-core.o xdma-sgm.o xdma-ioctl.o xdma-bit.o

KERNELDIR ?= /home/user/nvidia/nvidia_sdk/JetPack_4.4_Linux_JETSON_AGX_XAVIER/Linux_for_Tegra/rootfs/lib/modules/4.9.140-tegra/build

PWD       := $(shell pwd)

ROOT := $(dir $(M))
XILINXINCLUDE := -I$(ROOT)../include -I$(ROOT)/include

#GCCVERSION = $(shell gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')

#GCC49 := $(shell expr $(GCCVERSION) \>= 40900)

all:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE)  ARCH=arm64 -C $(KERNELDIR) M=$(PWD) modules

install: all
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE)  ARCH=arm64 -C $(KERNELDIR) M=$(PWD) modules_install
	depmod -a
	install -m 644 10-xcldma.rules /etc/udev/rules.d

clean:
	rm -rf *.o *.o.d *~ core .depend .*.cmd *.ko *.ko.unsigned *.mod.c .tmp_versions *.symvers .#* *.save *.bak Modules.* modules.order Module.markers *.bin

CFLAGS_xdma-core.o := -Wall -DDEBUG $(XILINXINCLUDE)

ifeq ($(GCC49),1)
	CFLAGS_xdma-core.o += -Wno-error=date-time
endif

CFLAGS_xdma-sgm.o := $(XILINXINCLUDE)
CFLAGS_xdma-bit.o := $(XILINXINCLUDE)
CFLAGS_xdma-ioctl.o := $(XILINXINCLUDE)
