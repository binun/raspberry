
#/bin/bash -i

#HEIGHT=2592
#WIDTH=1944

#HEIGHT=3280
#WIDTH=2464
HEIGHT=1920
WIDTH=1088

#HEIGHT=640
#WIDTH=480
MEASUREPERIOD=15
MEASUREPORTION=60M

FRAMERATE=60
ISO=1200

#FORMAT=3
FORMAT=rgb
#FORMAT=h264

PCIFACE=enp10s0
RPIFACE=enp

RPIS=rpientries.txt

fuser -k 6666/tcp
fuser -k 8888/tcp

MAIN_IPV6=$(ifconfig $PCIFACE | grep 'inet6 addr' | grep 'Scope:Link' | awk '{ print $3}')
service network-manager stop

rm -f $RPIS
rm -rf *.bin

fuser -k 6666/tcp
fuser -k 8888/tcp
ps -ef | grep 'tcpserver' | grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | grep 'noisedeliver' | grep -v grep | awk '{print $2}' | xargs kill -9
fallocate -l 5G noise.bin
/etc/init.d/avahi-daemon stop

sleep 1
#gnome-terminal -x bash -c "./tcpserver"
sleep 1
#gnome-terminal -x bash -c "./noiseclient 127.0.0.1"

i=1
ifconfig -a | sed 's/[ \t].*//;/^$/d' | grep -v 'avahi' | grep $RPIFACE | (while read id; do

	STRLENGTH=$(echo $id | wc -m)
	if (($STRLENGTH >8)); then
		v4ip=169.254.10.$i
		v6ip=fe80::$((10+i))/64
        	echo "  Retrieving $id $v6ip"

		ifconfig $id down
		ifconfig $id $v4ip netmask 255.255.0.0 up
		sleep 2

		echo $id > id.txt

		#ping6 -I $(cat id.txt) ff02::1 -w 30 -c 4
		rpi=$(ping6 -I $(cat id.txt) ff02::1 -w 30 -c 1  | grep 'bytes from' | awk '{print $4}' | sed 's/.$//')
		myipv6=$(ip addr show dev $(cat id.txt) | grep 'link' | sed -e's/^.*inet6 \([^ ]*\)\/.*$/\1/;t;d')

		printf "$id $rpi $myipv6\n"
		#printf "$id $rpi $myipv6\n" >> $RPIS
		echo $rpi > rpi.txt
		echo $myipv6 > myipv6.txt

		echo "ssh pi@$(cat rpi.txt)%$(cat id.txt)"
		#gnome-terminal -x bash -c "sshpass -p 'raspberry' ssh -t -o StrictHostKeyChecking=no pi@$(cat rpi.txt)%$(cat id.txt) './streamv4.sh $HEIGHT $WIDTH $FORMAT $(cat myipv6.txt) none 100 100 100 none; bash -i' "
		#gnome-terminal -x bash -c "sshpass -p 'raspberry' ssh -t -o StrictHostKeyChecking=no pi@$(cat rpi.txt)%$(cat id.txt) './bayer.sh $(cat myipv6.txt) $FRAMERATE $HEIGHT $WIDTH $FORMAT $ISO; bash -i' "

		#gnome-terminal -x bash -c "sshpass -p 'raspberry' ssh -t -o StrictHostKeyChecking=no pi@$(cat rpi.txt)%$(cat id.txt)"

		i=$((i+1))
		sleep 10
	fi
done)

#./measurequality.sh $MEASUREPERIOD $MEASUREPORTION &
