#!/bin/bash
PC=10.10.11.32
IFACE=enp10s0
RPIS=(10.10.11.10)
ifconfig $IFACE down
ifconfig $IFACE $PC netmask 255.255.255.0 up # $PCADDR=10.10.11.32 is PC
for rpi in ${RPIS[@]};
do
	route add default gw $rpi $IFACE;
done

ifconfig $IFACE txqueuelen 10000
sysctl -p ./sysctl.conf


