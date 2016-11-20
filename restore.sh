#!/bin/bash
fuser -k 6666/tcp

service network-manager restart
sleep 1
service network-manager restart
sleep 1
service network-manager restart
