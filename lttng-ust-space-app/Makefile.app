GCC=aarch64-poky-linux-gcc
GCCFLAGS=-g --sysroot=/tools/AGRreleases/yocto/jethro_20Feb2017/axxia-arm64-x9/tools/sysroots/aarch64-poky-linux
GCCINCLUDE=-I.

all: hello

hello-tp.o: hello-tp.c
	$(GCC) -c $(GCCINCLUDE) $(GCCFLAGS) hello-tp.c

hello.o: hello.c
	$(GCC) -c $(GCCINCLUDE) $(GCCFLAGS) hello.c

hello: hello-tp.o hello.o
	$(GCC) -o hello $(GCCINCLUDE) $(GCCFLAGS) hello.o hello-tp.o -llttng-ust -ldl

.PHONY: clean
clean:
	rm hello.o hello hello-tp.o -rf
