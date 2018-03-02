#!/bin/bash

echo "babeltrace'ing the traces. May take a while..."
./parse_netboot.py > out
#if happened there are duplicates for user and kernel doing the same 
# operations, supress 
./kernel_traces_netboot.py out
#convert to Uboot traces
echo "Converting to Uboot traces"
./convert.py --input=./out.converted --architecture=all
#change the PA to VA addresses
patch -p6 < VA2PA.patch
