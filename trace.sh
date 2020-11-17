#!/bin/bash


sample() {
#A common run might be:
	pushd /sys/kernel/debug/tracing
	#clear trace
	echo 0 > tracing_on; echo > trace
    echo function_graph > current_tracer
    #echo function > current_tracer
    echo 1 > tracing_on
	insmod /workspace/sw/mbykowsx/lionfish/perf/nuevo/peripherals/pcie/axm6732/asic-1.1/dma/dmatest.ko
	echo 0 > tracing_on
	popd
}


set_trace_rte() {
	cd /sys/kernel/debug/tracing
	echo nop > current_tracer
	echo > trace # clear buffer
	#echo 0 > tracing_on
	echo 0 > events/enable # disable all the tracepoints
	#echo 1 > events/printk/enable #this is printk not trace_printk
    #echo function_graph > current_tracer
	popd
}

set_trace() {
		pushd /sys/kernel/debug/tracing
		echo 0 > tracing_on
		#echo ${TRACE_FUNCS} > set_ftrace_filter
		echo 0 > events/enable # disable all the tracepoints
		echo notrace_printk > trace_options #don't log trace_printk
		echo 0 > options/latency-format #latency format on
		echo nop > current_tracer #no ftrace
		#echo function > current_tracer
		#echo 1 > events/l3_dma/enable #enable my tracepoints
		echo 1 > events/l3_dma/l3_dma_duration/enable
		echo 1 > tracing_on
		popd
}

# read_trace l3_locked|l3_unlocked
read_trace() {
		pushd /sys/kernel/debug/tracing
		echo 0 > tracing_on
		popd
		pwd
		tail -n 1 /sys/kernel/debug/tracing/trace | tee -a $1
}

