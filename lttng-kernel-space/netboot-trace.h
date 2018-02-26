#undef TRACE_SYSTEM
#define TRACE_SYSTEM netboot

#if !defined(_NETBOOT_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _NETBOOT_TRACE_H

#include <linux/tracepoint.h>

TRACE_EVENT(
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

    TP_STRUCT__entry(
        __field(unsigned, hwioType)
        __field(unsigned, devNum)
        __field(unsigned, regionId)
        __field(unsigned long long, offset)
        __dynamic_array(unsigned char, buffer, bufferLen)
        __field(unsigned, count)
        __field(unsigned, xferWidth)
        __field(unsigned, flags)
    ),

    TP_fast_assign(
        __entry->hwioType = hwioType;
        __entry->devNum = devNum;
        __entry->regionId = regionId;
        __entry->offset = offset;
        memcpy(__get_dynamic_array(buffer), buffer, bufferLen);
        __entry->count = count;
        __entry->xferWidth = xferWidth;
        __entry->flags = flags;
    ),

    TP_printk("%d", 0)
);

TRACE_EVENT(
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

    TP_STRUCT__entry(
        __field(unsigned, hwioType)
        __field(unsigned, devNum)
        __field(unsigned, regionId)
        __field(unsigned long long, offset)
        __field(unsigned, delayLoops)
        __field(unsigned, delayTime)
        __field(unsigned, mask)
        __field(unsigned, value)
    ),

    TP_fast_assign(
        __entry->hwioType = hwioType;
        __entry->devNum = devNum;
        __entry->regionId = regionId;
        __entry->offset = offset;
        __entry->delayLoops = delayLoops;
        __entry->delayTime = delayTime;
        __entry->mask = mask;
        __entry->value = value;
    ),

    TP_printk("%d", 0)
);

TRACE_EVENT(
    netboot_kernel_comment,

    TP_PROTO(
        unsigned hwioType,
        const char* comment
    ),

    TP_ARGS(hwioType, comment),

    TP_STRUCT__entry(
        __field(unsigned, hwioType)
        __string(comment, comment)
    ),

    TP_fast_assign(
        __entry->hwioType = hwioType;
        __assign_str(comment, comment);
    ),

    TP_printk("%d", 0)
);

TRACE_EVENT(
    netboot_kernel_generic,

    TP_PROTO(
        unsigned hwioType,
        unsigned devNum,
        unsigned long long hwioArg
    ),

    TP_ARGS(hwioType, devNum, hwioArg),

    TP_STRUCT__entry(
        __field(unsigned, hwioType)
        __field(unsigned, devNum)
        __field(unsigned long long, hwioArg)
    ),

    TP_fast_assign(
        __entry->hwioType = hwioType;
        __entry->devNum = devNum;
        __entry->hwioArg = hwioArg;
    ),

    TP_printk("%d", 0)
);

TRACE_EVENT(
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

    TP_STRUCT__entry(
        __field(unsigned, hwioType)
        __field(unsigned, devNum)
        __field(unsigned, regionId)
        __field(unsigned long long, offset)
        __dynamic_array(unsigned, buffer, bufferLen)
        __field(unsigned, width)
        __field(unsigned, count)
        __field(unsigned, stride)
        __field(unsigned, flags)
    ),

    TP_fast_assign(
        __entry->hwioType = hwioType;
        __entry->devNum = devNum;
        __entry->regionId = regionId;
        __entry->offset = offset;
        memcpy(__get_dynamic_array(buffer), buffer, bufferLen * sizeof(unsigned));
        __entry->width = width;
        __entry->count = count;
        __entry->stride = stride;
        __entry->flags = flags;
    ),

    TP_printk("%d", 0)
);

TRACE_EVENT(
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

    TP_STRUCT__entry(
        __field(unsigned, hwioType)
        __field(unsigned, devNum)
        __field(unsigned, regionId)
        __field(unsigned long long, offset)
        __dynamic_array(unsigned long long, buffer, bufferLen)
        __field(unsigned, count)
        __field(unsigned, flags)
    ),

    TP_fast_assign(
        __entry->hwioType = hwioType;
        __entry->devNum = devNum;
        __entry->regionId = regionId;
        __entry->offset = offset;
        memcpy(__get_dynamic_array(buffer), buffer, bufferLen * sizeof(unsigned long long));
        __entry->count = count;
        __entry->flags = flags;
    ),

    TP_printk("%d", 0)
);

TRACE_EVENT(
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

    TP_STRUCT__entry(
        __field(unsigned, hwioType)
        __field(unsigned, devNum)
        __field(unsigned, regionId)
        __dynamic_array(unsigned long long, buffer, bufferLen)
        __field(unsigned, length)
        __field(unsigned, flags)
    ),

    TP_fast_assign(
        __entry->hwioType = hwioType;
        __entry->devNum = devNum;
        __entry->regionId = regionId;
        memcpy(__get_dynamic_array(buffer), buffer, bufferLen * sizeof(unsigned long long));
        __entry->length = length;
        __entry->flags = flags;
    ),

    TP_printk("%d", 0)
);

TRACE_EVENT(
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

    TP_STRUCT__entry(
        __field(unsigned, hwioType)
        __field(unsigned, devNum)
        __field(unsigned, regionId)
        __field(unsigned long long, offset)
        __dynamic_array(unsigned long long, buffer, bufferLen)
        __field(unsigned, length)
        __field(unsigned, nvector)
        __field(unsigned, flags)
    ),

    TP_fast_assign(
        __entry->hwioType = hwioType;
        __entry->devNum = devNum;
        __entry->regionId = regionId;
        __entry->offset = offset;
        memcpy(__get_dynamic_array(buffer), buffer, bufferLen * sizeof(unsigned long long));
        __entry->length = length;
        __entry->nvector = nvector;
        __entry->flags = flags;
    ),

    TP_printk("%d", 0)
);

TRACE_EVENT(
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

    TP_STRUCT__entry(
        __field(unsigned, hwioType)
        __field(unsigned, devNum)
        __field(unsigned, regionId)
        __dynamic_array(unsigned, addrBuf, bufferLen)
        __dynamic_array(unsigned long long, dataBuf, bufferLen)
        __field(unsigned, length)
        __field(unsigned, nvector)
        __field(unsigned, flags)
    ),

    TP_fast_assign(
        __entry->hwioType = hwioType;
        __entry->devNum = devNum;
        __entry->regionId = regionId;
        memcpy(__get_dynamic_array(addrBuf), addrBuf, bufferLen * sizeof(unsigned));
        memcpy(__get_dynamic_array(dataBuf), dataBuf, bufferLen * sizeof(unsigned long long));
        __entry->length = length;
        __entry->nvector = nvector;
        __entry->flags = flags;
    ),

    TP_printk("%d", 0)
);
#endif /* _NETBOOT_TRACE_H */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE netboot-trace
#include <trace/define_trace.h>
