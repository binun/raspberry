#/bin/bash -i

YUM_CMD=$(which yum)
APT_GET_CMD=$(which apt-get)

if [[ ! -z $YUM_CMD ]]; then
	yum -y update && apt-get -y upgrade
	yum -y install netcat ent
elif [[ ! -z $APT_GET_CMD ]]; then
	apt-get -y update && apt-get -y upgrade
	apt-get -y install netcat ent
else
	echo "error can't install package"
	exit 1;
fi

rpi-update

cp ./options/new/sysctl.conf /etc
gcc tcpserver.c -o tcpserver -lpthread

reboot


