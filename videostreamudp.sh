#!/bin/bash
PC=alexpc.local

if [ -f videofifo.500 ]
then
        sudo rm -f videofifo.500
fi

sudo mkfifo videofifo.500
sudo chmod 0777 videofifo.500

cat videofifo.500 | nc -u $PC 6666 &
raspivid -w 1920 -h 1080 -fps 30 -t 0 -o videofifo.500
#sh -c "raspivid -w 2592 -h 1944 -fps 15 -t 0 -o - | mbuffer -m 1M | nc -u $PC 6666" &

