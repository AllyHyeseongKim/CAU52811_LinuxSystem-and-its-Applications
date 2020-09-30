#!/bin/sh
row=$1
col=$2
for i in $(seq 1 $row)
do
	for j in $(seq 1 $col)
	do
		echo -n $i '*' $j '=' `expr $i \* $j` ' '
	done
	echo
done
exit 0
