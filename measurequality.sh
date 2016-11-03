#! /bin/bash

n=1
PERIOD=$1

REPORT=entropies.txt
rm -f $REPORT

while true
do
	for f in noise*.bin;do
		zip temp.zip $f >/dev/null
		zipinfo -m temp.zip >> $REPORT
		ENTROPY=$(./entvalue.sh $f)
		echo "Second $n, Entropy $ENTROPY"
		echo "Second $n, Entropy $ENTROPY" >> $REPORT
		rm -f temp.zip
	done
	n=$(($n+$PERIOD))
	sleep $PERIOD
done
