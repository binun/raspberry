#!/bin/bash

apt-get -y update && apt-get -y upgrade
apt-get -y install nc ent sshpass
 
cp ./options/new/sysctl.conf /etc
cp ./options/new/interfaces /etc/network

rm -f tcpserver
gcc tcpserver.c timer.c -o tcpserver -lpthread 
reboot




