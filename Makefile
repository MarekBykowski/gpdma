
all:  
	#build lttng netboot probes module and remaining of lttng
	make -C lttng-modules-2.10.5
	#install modules
	make -C lttng-modules-2.10.5 modules_install
	#build userspace tracepoint provider
	make -C lttng-ust-space-app

.PHONY: clean
clean:
	@rm -rf lttng-modules-2.10.5-build/*
	make -C lttng-modules-2.10.5 clean
	make -C lttng-ust-space-app clean
