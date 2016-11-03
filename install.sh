#/bin/bash -i

apt-get -y update && apt-get -y upgrade && rpi-update
apt-get -y install nc ent

cp /etc/network/interfaces ./options/old
cp /etc/sysctl.conf ./options/old

cp ./options/new/sysctl.conf /etc
cp ./options/new/interfaces /etc/network

reboot


