#!/bin/sh
row=$1
col=$2
if [ $row -ge 1 -a $col -ge 1 ]
then
	for i in $(seq 1 $row)
	do
		for j in $(seq 1 $col)
		do
			echo -n $i '*' $j '=' `expr $i \* $j` ' '
		done
		echo
	done
else
	echo [Input Range Error] Row and col must be an integer greater than or equal to 1.
fi
exit 0
