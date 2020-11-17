#undef TRACE_SYSTEM
#define TRACE_SYSTEM l3_dma

#if !defined(_TRACE_L3_DMA_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_L3_DMA_H

#include <linux/tracepoint.h>

DECLARE_EVENT_CLASS(l3_dma,

	TP_PROTO(const char * type),

	TP_ARGS(type),

	TP_STRUCT__entry(
		__field(const char *, name)
	),

	TP_fast_assign(
		__entry->name = type;
	),

	TP_printk("%s", __entry->name)
);

DEFINE_EVENT(l3_dma, l3_dma_entry,

	TP_PROTO(const char *reason),

	TP_ARGS(reason)
);

DEFINE_EVENT(l3_dma, l3_dma_exit,

	TP_PROTO(const char *reason),

	TP_ARGS(reason)
);

TRACE_EVENT(l3_dma_duration,

   TP_PROTO(unsigned int cpu, struct timespec duration, struct timespec avg_duration),

   TP_ARGS(cpu, duration, avg_duration),

   TP_STRUCT__entry(
       __field(unsigned int, cpu)
       __field(__kernel_time_t, duration_tv_sec)
       __field(long, duration_tv_nsec)
       __field(__kernel_time_t, avg_duration_tv_sec)
       __field(long, avg_duration_tv_nsec)
   ),

   TP_fast_assign(
       __entry->cpu = cpu;
       __entry->duration_tv_sec = duration.tv_sec;
       __entry->duration_tv_nsec = duration.tv_nsec;
       __entry->avg_duration_tv_sec = avg_duration.tv_sec;
       __entry->avg_duration_tv_nsec = avg_duration.tv_nsec;
   ),

   TP_printk("cpu=%u (%ld.%09ld) avg(%ld.%09ld)", __entry->cpu, __entry->duration_tv_sec,
                   __entry->duration_tv_nsec, __entry->avg_duration_tv_sec, __entry->avg_duration_tv_nsec)
);


#endif /* _TRACE_L3_DMA_H */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE l3-dma-trace
#include <trace/define_trace.h>


