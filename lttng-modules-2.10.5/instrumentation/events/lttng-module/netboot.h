#undef TRACE_SYSTEM
#define TRACE_SYSTEM netboot

#if !defined(_LTTNG_NETBOOT_H) || defined(TRACE_HEADER_MULTI_READ)
#define _LTTNG_NETBOOT_H

#include "../../../probes/lttng-tracepoint-event.h"
#include <linux/tracepoint.h>

LTTNG_TRACEPOINT_EVENT(
    netboot_kernel_rw,

        TP_PROTO(
        unsigned hwioType,
        unsigned devNum,
        unsigned regionId,
        unsigned long long offset,
        unsigned char *buffer,
        size_t bufferLen,
        unsigned count,
        unsigned xferWidth,
        unsigned flags
    ),

    TP_ARGS(hwioType, devNum, regionId, offset, buffer, bufferLen, count, xferWidth, flags),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned, regionId, regionId)
        ctf_integer(unsigned long long, offset, offset)
        ctf_sequence(unsigned char, buffer, buffer, size_t, bufferLen)
        ctf_integer(unsigned, count, count)
        ctf_integer(unsigned, xferWidth, xferWidth)
        ctf_integer(unsigned, flags, flags)
    )
)

LTTNG_TRACEPOINT_EVENT(
    netboot_kernel_poll,

    TP_PROTO(
        unsigned hwioType,
        unsigned devNum,
        unsigned regionId,
        unsigned long long offset,
        unsigned delayLoops,
        unsigned delayTime,
        unsigned mask,
        unsigned value
    ),

    TP_ARGS(hwioType, devNum, regionId, offset, delayLoops, delayTime, mask, value),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned, regionId, regionId)
        ctf_integer(unsigned long long, offset, offset)
        ctf_integer(unsigned, delayLoops, delayLoops)
        ctf_integer(unsigned, delayTime, delayTime)
        ctf_integer(unsigned, mask, mask)
        ctf_integer(unsigned, value, value)
    )
)

LTTNG_TRACEPOINT_EVENT(
    netboot_kernel_comment,

    TP_PROTO(
        unsigned hwioType,
        const char* comment
    ),

    TP_ARGS(hwioType, comment),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_string(comment, comment)
    )
)

LTTNG_TRACEPOINT_EVENT(
    netboot_kernel_generic,

    TP_PROTO(
        unsigned hwioType,
        unsigned devNum,
        unsigned long long hwioArg
    ),

    TP_ARGS(hwioType, devNum, hwioArg),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned long long, hwioArg, hwioArg)
    )
)

LTTNG_TRACEPOINT_EVENT(
    netboot_kernel_fill,

        TP_PROTO(
        unsigned hwioType,
        unsigned devNum,
        unsigned regionId,
        unsigned long long offset,
        unsigned *buffer,
        unsigned bufferLen,
        unsigned width,
        unsigned count,
        unsigned stride,
        unsigned flags
    ),

    TP_ARGS(hwioType, devNum, regionId, offset, buffer, bufferLen, width, count, stride, flags),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned, regionId, regionId)
        ctf_integer(unsigned long long, offset, offset)
        ctf_sequence(unsigned, buffer, buffer, unsigned, bufferLen)
        ctf_integer(unsigned, width, width)
        ctf_integer(unsigned, count, count)
        ctf_integer(unsigned, stride, stride)
        ctf_integer(unsigned, flags, flags)
    )
)

LTTNG_TRACEPOINT_EVENT(
    netboot_kernel_rmw,

        TP_PROTO(
        unsigned hwioType,
        unsigned devNum,
        unsigned regionId,
        unsigned long long offset,
        unsigned long long *buffer,
        unsigned bufferLen,
        unsigned count,
        unsigned flags
    ),

    TP_ARGS(hwioType, devNum, regionId, offset, buffer, bufferLen, count, flags),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned, regionId, regionId)
        ctf_integer(unsigned long long, offset, offset)
        ctf_sequence(unsigned long long, buffer, buffer, unsigned, bufferLen)
        ctf_integer(unsigned, count, count)
        ctf_integer(unsigned, flags, flags)
    )
)

LTTNG_TRACEPOINT_EVENT(
    netboot_kernel_sw,

        TP_PROTO(
        unsigned hwioType,
        unsigned devNum,
        unsigned regionId,
        unsigned long long *buffer,
        unsigned bufferLen,
        unsigned length,
        unsigned flags
    ),

    TP_ARGS(hwioType, devNum, regionId, buffer, bufferLen, length, flags),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned, regionId, regionId)
        ctf_sequence(unsigned long long, buffer, buffer, unsigned, bufferLen)
        ctf_integer(unsigned, length, length)
        ctf_integer(unsigned, flags, flags)
    )
)

LTTNG_TRACEPOINT_EVENT(
    netboot_kernel_bbw,

        TP_PROTO(
        unsigned hwioType,
        unsigned devNum,
        unsigned regionId,
        unsigned long long offset,
        unsigned long long *buffer,
        unsigned bufferLen,
        unsigned length,
        unsigned nvector,
        unsigned flags
    ),

    TP_ARGS(hwioType, devNum, regionId, offset, buffer, bufferLen, length, nvector, flags),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned, regionId, regionId)
        ctf_integer(unsigned long long, offset, offset)
        ctf_sequence(unsigned long long, buffer, buffer, unsigned, bufferLen)
        ctf_integer(unsigned, length, length)
        ctf_integer(unsigned, nvector, nvector)
        ctf_integer(unsigned, flags, flags)
    )
)

LTTNG_TRACEPOINT_EVENT(
    netboot_kernel_bsw,

        TP_PROTO(
        unsigned hwioType,
        unsigned devNum,
        unsigned regionId,
        unsigned *addrBuf,
        unsigned long long *dataBuf,
        unsigned bufferLen,
        unsigned length,
        unsigned nvector,
        unsigned flags
    ),

    TP_ARGS(hwioType, devNum, regionId, addrBuf, dataBuf, bufferLen, length, nvector, flags),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned, regionId, regionId)
        ctf_sequence(unsigned, addrBuf, addrBuf, unsigned, bufferLen)
        ctf_sequence(unsigned long long, dataBuf, dataBuf, unsigned, bufferLen)
        ctf_integer(unsigned, length, length)
        ctf_integer(unsigned, nvector, nvector)
        ctf_integer(unsigned, flags, flags)
    )
)
#endif

#include "../../../probes/define_trace.h"
