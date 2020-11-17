#!/bin/sh

[[ -z $1 ]] && { echo "${0##*/} locked or ${0##*/} unlocked"; exit 0; }
WHAT_L3=l3_$1

#TESTS="memcpy memcpy_read dma_read memcpy_write dma_write ep2ep_read ep2ep_write mem2mem"
TESTS="mem2mem"

LOGFILE=${WHAT_L3}.log
[[ -f $LOGFILE ]] || { echo "no logfile $LOGFILE"
			echo "touch $LOGFILE && chmod 777 $LOGFILE"
			exit 0; }
echo $LOGFILE


#TRACE_FUNCS="test_dma ioremap_cache"

insert_module() {
	DEV=/sys/bus/pci/devices/0000\:01\:00.0
	echo 8 > /proc/sys/kernel/printk
	cd /workspace/sw/mbykowsx/lionfish/perf/trunk/nuevo/peripherals/pcie/axm6732/asic-1.1/pcitest
	if lsmod | grep -q pcitest; then
		rmmod pcitest
	fi
	echo insert module
	insmod ./pcitest.ko

	echo "Tests: ${TESTS}"
	if [ ! -r ${DEV}/device ]; then
	   echo "${DEV}: Device not found"
	   exit 1
	fi
}



#######################################################
# Script runs here
#######################################################

source ./lmbench.sh
source ./trace.sh

run_lmbench
insert_module
sleep 1

################
# Main loop
################
for burst in 7
#for burst in 0 3 5 7
do
    #echo burst set to $burst  -- 3 = 64B, 5 = 128B, ...
    #echo $burst >> /sys/module/lsi_dma32/parameters/burst
    for tc in ${TESTS}
    do
        echo testing  $tc
		sleep 0.5

		pwd
		echo "test $tc burst $burst 8M" | tee -a $LOGFILE

        # 4096 8192 16384 32768 65536 131072 524288 1048576 2097152
        #for size in 4096 8192 16384 32768 65536 131072 524288 1048576 2097152; do
        #for size in 16777216; do
        for size in 16777216; do
			set_trace

			echo size $size testcase $t
			echo $size > ${DEV}/testsize
			echo $tc > ${DEV}/testcase

			cat ${DEV}/result

			read_trace $LOGFILE

        done
    done
done

stop_lmbench
