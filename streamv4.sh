#!/bin/bash

WIDTH=$1  # 1920
HEIGHT=$2 # 1080
FORMAT=$3 # 4
SPEED=60  # 60
PC=$4%usb0

ps -ef | grep 'sh -c v4l2-ctl*' | grep 'mbuffer' | awk '{print $2}' | (while read id; do sudo kill -9 $id; done)
sudo modprobe bcm2835-v4l2

if [ "$5" != "none" ]; then
	echo "Brightness: $5"
	v4l2-ctl --set-ctrl brightness=$5 #0
fi

if [ "$6" != "none" ]; then
	echo "Saturation: $6"
        v4l2-ctl --set-ctrl saturation=$6 #-100
fi

if [ "$7" != "none" ]; then
	echo "Sharpness: $7"
        v4l2-ctl --set-ctrl sharpness=$7  # -100
fi

if [ "$8" != "none" ]; then
	echo "Contrast: $8"
        v4l2-ctl --set-ctrl contrast=$8  # -100
fi

if [ "$9" != "none" ]; then
        echo "ISO: $9"
        v4l2-ctl --set-ctrl iso_sensitivity=$9  # 4
fi

v4l2-ctl --set-ctrl video_bitrate_mode=1
v4l2-ctl --set-ctrl video_bitrate=25000000
v4l2-ctl --set-ctrl h264_i_frame_period=10

v4l2-ctl --set-fmt-video=width=$WIDTH,height=$HEIGHT,pixelformat=$FORMAT
v4l2-ctl --set-parm=$SPEED

echo $WIDTH 
echo $HEIGHT 
echo $FORMAT 
echo $SPEED 

sh -c "v4l2-ctl --stream-mmap=5 --stream-to=- | mbuffer -m 4M | nc -6 $PC 6666" &
