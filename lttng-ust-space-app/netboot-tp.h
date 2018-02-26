#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER kubus

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./netboot-tp.h"

#if !defined(_NETBOOT_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _NETBOOT_TP_H

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT(
    kubus,

    netboot_rw,

    TP_ARGS(
            unsigned, hwioType,
            unsigned, devNum,
            unsigned, regionId,
            unsigned long long, offset,
            unsigned char *, buffer,
            size_t, bufferLen,
            unsigned, count,
            unsigned, xferWidth,
            unsigned, flags
    ),

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

TRACEPOINT_EVENT(
    kubus,

    netboot_poll,

    TP_ARGS(
            unsigned, hwioType,
            unsigned, devNum,
            unsigned, regionId,
            unsigned long long, offset,
            unsigned, delayLoops,
            unsigned, delayTime,
            unsigned, mask,
            unsigned, value
    ),

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

TRACEPOINT_EVENT(
    kubus,

    netboot_comment,

    TP_ARGS(
            unsigned, hwioType,
            const char*, comment
    ),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_string(comment, comment)
    )
)

TRACEPOINT_EVENT(
    kubus,

    netboot_generic,

    TP_ARGS(
            unsigned, hwioType,
            unsigned, devNum,
            unsigned long long, hwioArg
    ),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned long long, hwioArg, hwioArg)
    )
)

TRACEPOINT_EVENT(
    kubus,

    netboot_fill,

    TP_ARGS(
            unsigned, hwioType,
            unsigned, devNum,
            unsigned, regionId,
            unsigned long long, offset,
            unsigned *, buffer,
            unsigned, bufferLen,
            unsigned, width,
            unsigned, count,
            unsigned, stride,
            unsigned, flags
    ),

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

TRACEPOINT_EVENT(
    kubus,

    netboot_rmw,

    TP_ARGS(
            unsigned, hwioType,
            unsigned, devNum,
            unsigned, regionId,
            long long unsigned, offset,
            long long unsigned*, buffer,
            unsigned, bufferLen,
            unsigned, count,
            unsigned, flags
    ),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned, regionId, regionId)
        ctf_integer(long long unsigned, offset, offset)
        ctf_sequence(long long unsigned, buffer, buffer, unsigned, bufferLen)
        ctf_integer(unsigned, count, count)
        ctf_integer(unsigned, flags, flags)
    )
)

TRACEPOINT_EVENT(
    kubus,

    netboot_sw,

    TP_ARGS(
            unsigned, hwioType,
            unsigned, devNum,
            unsigned, regionId,
            unsigned long long *, buffer,
            unsigned, bufferLen,
            unsigned, length,
            unsigned, flags
    ),

    TP_FIELDS(
        ctf_integer(unsigned, hwioType, hwioType)
        ctf_integer(unsigned, devNum, devNum)
        ctf_integer(unsigned, regionId, regionId)
        ctf_sequence(unsigned long long, buffer, buffer, unsigned, bufferLen)
        ctf_integer(unsigned, length, length)
        ctf_integer(unsigned, flags, flags)
    )
)

TRACEPOINT_EVENT(
    kubus,

    netboot_bbw,

    TP_ARGS(
            unsigned, hwioType,
            unsigned, devNum,
            unsigned, regionId,
            unsigned long long, offset,
            unsigned long long *, buffer,
            unsigned, bufferLen,
            unsigned, length,
            unsigned, nvector,
            unsigned, flags
    ),

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

TRACEPOINT_EVENT(
    kubus,

    netboot_bsw,

    TP_ARGS(
            unsigned, hwioType,
            unsigned, devNum,
            unsigned, regionId,
            unsigned *, addrBuf,
            unsigned long long *, dataBuf,
            unsigned, bufferLen,
            unsigned, length,
            unsigned, nvector,
            unsigned, flags
    ),

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
#endif /* _NETBOOT_TP_H */

#include <lttng/tracepoint-event.h>
