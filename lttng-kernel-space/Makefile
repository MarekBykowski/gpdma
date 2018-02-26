INCLUDES = -I. -I$(KDIR)/include
KBUILD_CFLAGS += -g

all: kernel_modules

obj-m	:= netboot_module.o

KDIR    := /workspace/sw/mbykowsx/lionfish/axxia_yocto_linux_4.1_private
PWD    := $(shell pwd)

CFLAGS_netboot_module.o = -I$(src)

%.ko: %.o
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) $@

kernel_modules:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

modules_install: all
	$(MAKE) INSTALL_MOD_PATH=/workspace/sw/mbykowsx/lionfish/lttng/lttng-modules-2.10.5-build \
	-C $(KDIR) M=$(PWD) modules_install

install:	modules_install

clean:
	$(RM) *.o *~ *.ko *.mod.c .*.o.cmd .*.ko.cmd
	$(RM) -r .tmp_versions modules.order Module.symvers
