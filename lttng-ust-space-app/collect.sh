#!/bin/bash

PRECEDING_DIR=$(dirname $0)

#load ncpDriver
pushd /workspace/sw/mbykowsx/lionfish/git-svn-rte/images/ncp_linux-arma53_rte_bin/ncp_rte/
source ./ncp.env && ncpDriver ncp update
popd

#add lttng kernel modules
rm /lib/modules/$(uname -r) -rf
ln -s /workspace/sw/mbykowsx/lionfish/lttng/lttng-modules-2.10.5-build/lib/modules/$(uname -r) /lib/modules/$(uname -r)

modprobe -r lttng-probe-netboot 2> /dev/null
modprobe lttng-probe-netboot
lttng create

echo "user space traces"
LD_PRELOAD=${PRECEDING_DIR}/libnetboot-tp.so lttng list --userspace | grep kubus
echo "kernel space traces"
lttng list --kernel | grep netboot_kernel_*

#lttng enable-event --userspace kubus:netboot_poll
#lttng enable-event --userspace kubus:netboot_rw
#lttng enable-event --userspace kubus:netboot_rmw
#lttng enable-event --userspace kubus:netboot_generic
lttng enable-channel --userspace --subbuf-size=1M --buffers-pid userspace-channel
lttng enable-event --userspace kubus:netboot_* --channel=userspace-channel
#lttng enable-event --userspace kubus:netboot_*
#lttng enable-event --kernel netboot_kernel_poll
#lttng enable-event --kernel netboot_kernel_rw
#lttng enable-event --kernel netboot_kernel_rmw
#lttng enable-event --kernel netboot_kernel_generic
#--subbuf-size=262144 for --kernel option
#--num-subbuf=4 for --kernel option
lttng enable-channel --kernel --subbuf-size=1M kernel-channel
lttng enable-event --kernel netboot_kernel_* --channel=kernel-channel


lttng start
LD_PRELOAD=${PRECEDING_DIR}/libnetboot-tp.so ncpCfgTool \
	-rc ${PRECEDING_DIR}/../ASE_configs/config.cfg \
	-O ${PRECEDING_DIR}/../ASE_configs/config.ovr
lttng stop

lttng destroy
