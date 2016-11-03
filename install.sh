#/bin/bash -i

apt-get -y update && apt-get -y upgrade && rpi-update
apt-get -y install nc ent

cp ./options/new/sysctl.conf /etc

reboot


