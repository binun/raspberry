#! /bin/bash

n=1
PERIOD=$1
PORTION=$2

REPORT=entropies.txt
rm -f $REPORT

while true
do
	for f in noise*.bin;do
		#zip temp.zip $f >/dev/null
		#zipinfo -m temp.zip >> $REPORT
		#split --bytes=$PORTION $f
		#lfr=$(find . -maxdepth 1 -type f -name 'x*' | sort -r | head -n 1)
		ENTROPY=$(./entvalue.sh $f)
		echo "Second $n, Entropy $ENTROPY"
		echo "Second $n, Entropy $ENTROPY" >> $REPORT
		#rm -f temp.zip
		#rm -f x*
	done
	n=$(($n+$PERIOD))

	sleep $PERIOD
done
