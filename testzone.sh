#!/bin/bash


test_mz() {
	cat /proc/zoneinfo | grep -A 15 DMA32
	cat /proc/buddyinfo
	grep dma32-kmalloc-8192 /proc/slabinfo
}

if lsmod | grep -q testzone; then
	rmmod testzone
fi

dmesg -C
test_mz
insmod testzone.ko

echo -e "\nkmalloc to DMA32\n"

test_mz
rmmod testzone
