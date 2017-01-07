#!/bin/bash

fuser -k 6666/tcp
fuser -k 8888/tcp
ps -ef | grep 'tcpserver' | grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | grep 'noisedeliver' | grep -v grep | awk '{print $2}' | xargs kill -9
rm *.bin
rm -f tcpserver noiseclient noisedeliver
