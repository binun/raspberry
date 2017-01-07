#!/bin/bash


rm -f tcpserver
rm -f noiseclient
rm -f noisedeliver

gcc tcpserver.c -o tcpserver -lpthread 
gcc noiseclient.c -o noiseclient
gcc noisedeliver.c -o noisedeliver -lpthread






