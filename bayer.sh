#!/bin/bash
PC=$1
SPEED=$2
W=$3
H=$4
FR=$5

#sh -c "python bayer.py $SPEED | mbuffer -m 16M | nc -6 $PC 6666" &
sh -c "python3 bayer.py $PC $SPEED $W $H $FR" &
