#!/bin/sh
#DEV="/sys/bus/pci/devices/0000:01:00.0"
DEV=/sys/bus/pci/devices/0000\:01\:00.0

OVERHEAD=64

if [ -z "$1" ]; then
   TESTS="memcpy memcpy_read dma_read memcpy_write dma_write ep2ep_read ep2ep_write mem2mem"
else
   TESTS="$*"
fi


echo 8 > /proc/sys/kernel/printk
cd /workspace/sw/mbykowsx/lionfish/perf/trunk/nuevo/peripherals/pcie/axm6732/asic-1.1/pcitest
if lsmod | grep -q pcitest; then
	rmmod pcitest
fi
insmod ./pcitest.ko

echo "Tests: ${TESTS}"
if [ ! -r ${DEV}/device ]; then
   echo "${DEV}: Device not found"
   exit 1
fi
#for burst in 0 3 5 7
for burst in 0
do
    echo burst set to $burst  -- 3 = 64B, 5 = 128B, ...
    echo $burst >> /sys/module/lsi_dma32/parameters/burst
    for tc in ${TESTS}
    do
        echo testing  $tc; sleep 1
    
        for size in ${OVERHEAD}; do
			echo $size > ${DEV}/testsize
			echo $tc > ${DEV}/testcase
			cat ${DEV}/result
        done
    done
done
