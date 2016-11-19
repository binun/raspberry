#!/bin/bash

v4l2-ctl --set-fmt-video=width=1920,height=1088,pixelformat=4
v4l2-ctl --set-parm=30
v4l2-ctl --stream-mmap=3 --silent --stream-to==$1

