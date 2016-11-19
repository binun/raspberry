#!/bin/bash

PREVSIZE=0
n=1
touch noise.bin
./generate.sh noise.bin &
while true
do
	FILESIZE=$(stat -c%s noise.bin)
	INCREMENT=$(($FILESIZE-$PREVSIZE))
	PREVSIZE=$FILESIZE
	echo "Second $n, $INCREMENT bytes passed"
	n=$(($n+1))
	sleep 1
done

