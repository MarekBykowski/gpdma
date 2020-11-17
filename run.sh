#!/bin/sh

PATH=${PWD}:$PATH
exec taskset -c 18 dmatest_full.sh $1
