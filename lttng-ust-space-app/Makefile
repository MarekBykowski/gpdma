GCC=aarch64-poky-linux-gcc
AR=aarch64-poky-linux-gcc-ar
#GCCFLAGS=-g --sysroot=/tools/AGRreleases/yocto/jethro_20Feb2017/axxia-arm64-x9/tools/sysroots/aarch64-poky-linux
#GCCFLAGS=-g --sysroot=/tools/AGRreleases/yocto/tools_jethro/armv8/axxia-image-large/sysroots/aarch64-poky-linux

#GCCFLAGS=--sysroot=/tools/AGRreleases/yocto/jethro_20Feb2017/axxia-arm64-x9/tools/sysroots/aarch64-poky-linux
#GCCFLAGS=--sysroot=/tools/AGRreleases/yocto/jethro_20Feb2017/axxia-arm64-x9/tools/sysroots/aarch64-poky-linux
GCCFLAGS=--sysroot=/tools/AGRreleases/yocto/tools_jethro/armv8/axxia-image-large/sysroots/aarch64-poky-linux
GCCINCLUDE=-I.

all: hello

netboot-tp.o: netboot-tp.c
	$(GCC) -fpic $(GCCINCLUDE) $(GCCFLAGS) -c netboot-tp.c

hello.o: hello.c
	$(GCC) $(GCCINCLUDE) $(GCCFLAGS) -c hello.c

libnetboot-tp.so: netboot-tp.o
	$(GCC) -shared -o libnetboot-tp.so $(GCCFLAGS) netboot-tp.o -llttng-ust -ldl

hello: hello.o libnetboot-tp.so
	$(GCC) -o hello $(GCCINCLUDE) $(GCCFLAGS) hello.o -ldl


.PHONY: clean
clean:
	rm hello.o hello netboot-tp.o *.a *.so -rf
