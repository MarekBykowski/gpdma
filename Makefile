# set toolchain
ARCH ?= arm64
CROSS_COMPILE ?= aarch64-axxia-linux-
CC ?= /tools/AGRreleases/yocto/morty/2018_01_26/axxia-arm64-x9/tools/sysroots/x86_64-axxiasdk-linux/usr/bin/aarch64-axxia-linux/$(CROSS_COMPILE)gcc
LD ?= /tools/AGRreleases/yocto/morty/2018_01_26/axxia-arm64-x9/tools/sysroots/x86_64-axxiasdk-linux/usr/bin/aarch64-axxia-linux/$(CROSS_COMPILE)ld


# set KERNELDIR, INCLUDES, etc.
KERNELDIR = /workspace/sw/mbykowsx/lionfish/kernel
INCLUDES = -I. -I$(KERNELDIR)/include
KBUILD_CFLAGS += -g
EXTRA_AFLAGS += -S -save-temps
PWD    := $(shell pwd)

#if testing with l3 locked set CFLAGS below
CFLAGS_pcitest.o := -DITERATIONS=1000
CFLAGS_pcitest.o += -DL3_LOCKED=1
CFLAGS_pcitest.o += -I$(src)

#set either or both
CONFIG_PCITEST=m
CONFIG_TESTZONE=m
obj-$(CONFIG_PCITEST) += pcitest.o
obj-$(CONFIG_TESTZONE) += testzone.o

build:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

help:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) help

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
