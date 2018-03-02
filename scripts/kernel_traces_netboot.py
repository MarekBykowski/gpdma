#!/usr/bin/env python

import sys
import os
from os.path import basename

KERNEL_PREFIX = 'kern-'
POL_START = '--- pol begin'
POL_END =   '--- pol end'
RESET_START = '--- reset begin'
RESET_END =   '--- reset end'
FUNCTIONS = ('ncpRead', 'ncpWrite', POL_START, POL_END, RESET_START, RESET_END)

def convert(srcname, dstname):
    '''Parse trace file'''
    with open(srcname) as fsrc:
        with open(dstname, 'w') as fdst:
            line_prev = prev_read = prev_write = None
            in_poll = False
            in_reset = False
            for line in fsrc:
                parse = False
                if line.startswith(KERNEL_PREFIX):
                    parse = True
                else:
                    for fun in FUNCTIONS:
                        if line.startswith(fun):
                            parse = True
                            break

                if not parse:
                    fdst.write(line)
                    continue

                if line.startswith(POL_START):
                    in_poll = True
                elif line.startswith(POL_END):
                    in_poll = False

                if line.startswith(RESET_START):
                    in_reset = True
                elif line.startswith(RESET_END):
                    in_reset = False

                if in_poll:
                    if not line.startswith(POL_START) and not line.startswith(KERNEL_PREFIX):
                        fdst.write(line)
                elif in_reset:
                    if not line.startswith(RESET_START) and not line.startswith(KERNEL_PREFIX):
                        fdst.write(line)
                else:
                    if line.startswith('{0}ncpRead'.format(KERNEL_PREFIX)):
                        prev_read = line
                        fdst.write(line[len(KERNEL_PREFIX):])
                    elif line.startswith('ncpRead'):
                        if prev_read:
                            # already written
                            pass
                        else:
                            # write it to file
                            fdst.write(line)
                        prev_read = None

                    elif line.startswith('ncpWrite'):
                        prev_write = line
                        fdst.write(line)

                    elif line.startswith('{0}ncpWrite'.format(KERNEL_PREFIX)):
                        if prev_write:
                            # already written
                            pass
                        else:
                            # write it to file
                            fdst.write(line[len(KERNEL_PREFIX):])
                        prev_write = None

                    elif line.startswith('ncpModify'):
                        prev_write = line
                        fdst.write(line)

                    elif line.startswith('{0}ncpModify'.format(KERNEL_PREFIX)):
                        if prev_write:
                            # already written
                            pass
                        else:
                            # write it to file
                            fdst.write(line[len(KERNEL_PREFIX):])
                        prev_write = None


def split_long_line(srcname, dstname):
    '''Parse trace file'''
    with open(srcname) as fsrc:
        with open(dstname, 'w') as fdst:
            for line in fsrc:
                parse = False
                if line.startswith('#'):
                    parse = False

                parts = line.split()
                if len(parts) > 6 and (line.startswith('ncpRead') or line.startswith('ncpWrite')):
                    parse = True

                if parse:
                    fun, addr = parts[0:2]
                    zero, node, target, offset = '', '', '', ''
                    try:
                        zero, node, target, offset = addr.split('.')
                    except ValueError:
                        print 'Split error: {0}'.format(line)
                        raise
                    offset = int(offset, 16)
                    i = 2
                    while True:
                        args = parts[i:i+4]
                        if not args: break
                        fdst.write('{0} {1}.{2}.{3}.0x{4:010x} {5}\n'.format(fun, zero, node, target, offset, ' '.join(args)))
                        offset += 0x10
                        i += 4
                else:
                    fdst.write(line)


def usage(prog):
    print 'Usage: {0} <input_file>'.format(basename(prog))
    sys.exit(2)

def main(argv):
    if len(argv) != 2:
        usage(argv[0])
    srcname = argv[1]
    dstname = '{0}.converted'.format(srcname)
    tmpname = '{0}.tmp'.format(srcname)
    convert(srcname, tmpname)
    split_long_line(tmpname, dstname)
    os.unlink(tmpname)

if __name__ == '__main__':
    main(sys.argv)
