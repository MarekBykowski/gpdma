#!/bin/bash

#add ncessary modules, lttng probes and silly-module that we do the tracing out of
rm /lib/modules/$(uname -r)  -rf
ln -s /workspace/sw/mbykowsx/lionfish/lttng/lttng-modules-2.10.5-build/lib/modules/$(uname -r) /lib/modules/$(uname -r)

modprobe -r lttng-probe-netboot 2>/dev/null
modprobe lttng-probe-netboot
modprobe -r netboot_module 2>/dev/null
modprobe netboot_module

lttng create

lttng enable-event --userspace kubus:netboot_rw
lttng enable-event --kernel netboot_kernel_rw

lttng start
#if you built tracepoint provider package as .so preloaded
#LD_PRELOAD=./libnetboot-tp.so ./hello
sleep 4
lttng stop

lttng destroy
