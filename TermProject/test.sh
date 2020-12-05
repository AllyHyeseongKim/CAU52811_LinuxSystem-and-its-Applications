#!/bin/bash
make
rmmod testing.ko
insmod testing.ko
dmesg -w
