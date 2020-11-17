#!/bin/bash

LMBENCH=/workspace/sw/mbykowsx/lionfish/perf/nuevo/cpu/axm6732/asic_v1.1/lmbench3/test_l3_lockdown_issue
LMBENCH_PID=()

run_lmbench() {
	cd $LMBENCH
	export PATH=$LMBENCH/../bin/armv8l-linux-gnu:$PATH

	LM_SIZES=(
		1m 2m 3m 4m 5m 6m 7m 8m 9m 10m 11m
		12m 13m 14m 15m 16m 17m 1m 2m 3m
		1m 2m 3m 4m 5m 6m 7m 8m 9m 10m 11m
		4m
	)

	for ((cpu=0; cpu<=31; cpu++)); do
		if [[ cpu -eq 18 ]]; then
			continue
		fi
		echo spwaning LmBench on $cpu
		taskset -c ${cpu} ./l3_dma_run_bw_mem.sh ${LM_SIZES[$cpu]} >/dev/null 2>&1 & 
		#usleep 1; taskset -p $!
		LMBENCH_PID+=($!)
		#echo 0.00$((RANDOM%20))
		sleep 0.00$((RANDOM%20))
	done
}

stop_lmbench(){
	for pid in "${LMBENCH_PID[@]}"; do
		kill $pid
	done
}
