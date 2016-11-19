#!/bin/bash

PC=alexpc.local

if [ -f videofifo.500 ]
then
        sudo rm -f videofifo.500
fi

sudo mkfifo videofifo.500
sudo chmod 0777 videofifo.500
cat videofifo.500 | nc $PC 6666 &

