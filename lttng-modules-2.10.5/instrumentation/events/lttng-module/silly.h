#undef TRACE_SYSTEM
#define TRACE_SYSTEM silly

#if !defined(_LTTNG_SILLY_H) || defined(TRACE_HEADER_MULTI_READ)
#define _LTTNG_SILLY_H

#include "../../../probes/lttng-tracepoint-event.h"
#include <linux/tracepoint.h>

LTTNG_TRACEPOINT_EVENT(me_silly,

	TP_PROTO(unsigned long time, unsigned long count),

	TP_ARGS(time, count),

	TP_FIELDS(
		ctf_integer(int, time_field, time)
		ctf_integer(int, count_field, count)
	)
)

#endif

#include "../../../probes/define_trace.h"
