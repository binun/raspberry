#!/bin/bash
fuser -k 6666/tcp

service network-manager stop
sleep 1
service network-manager start
sleep 1
service network-manager stop
sleep 1
service network-manager start
