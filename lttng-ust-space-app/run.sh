#!/bin/bash

#add ncessary modules, lttng probes and silly-module that we do the tracing out of
rm /lib/modules/$(uname -r)  rf
ln -s /workspace/sw/mbykowsx/lionfish/lttng/lttng-modules-2.10.5-build/lib/modules/$(uname -r) /lib/modules/$(uname -r)

modprobe sillymod-event

lttng create
# add lttng-probe-silly if doesnt exist
test -z "$(lsmod |grep lttng_probe_silly)" && modprobe lttng-probe-silly

lttng list --kernel| grep silly
lttng enable-event --userspace hello_world:my_first_tracepoint
lttng enable-event --kernel me_silly

lttng start
#if you built tracepoint provider package as .so preloaded
LD_PRELOAD=./libhello-tp.so ./hello 1 234 hello

#if it is in statically compiled in
#./hello 1 234 hello
lttng stop

lttng destroy
