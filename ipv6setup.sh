
#/bin/bash -i

DRY=0

#HEIGHT=2592
#WIDTH=1944

#HEIGHT=3280
#WIDTH=2464
HEIGHT=1920
WIDTH=1080

MEASUREPERIOD=10
MEASUREPORTION=10M

FRAMERATE=60
FORMAT=3

RPIFACE=eno
i=1

RPIS=rpientries.txt
PCIFACE=enp
MAIN_IPV6=$(ifconfig $PCIFACE | grep 'inet6 addr' | grep 'Scope:Link' | awk '{ print $3}')
service network-manager stop

rm -rf noise*.bin
rm -f $RPIS

gnome-terminal -x bash -c "./tcpserver"

#gnome-terminal -x bash -c "nc -6 -u -vv -k -l -p 6666 > noise.bin"

ifconfig -a | sed 's/[ \t].*//;/^$/d' | grep -v 'avahi' | grep $RPIFACE | (while read id; do

	STRLENGTH=$(echo $id | wc -m)
	if (($STRLENGTH >8)); then
		v4ip=169.254.10.$i
		v6ip=fe80::$((10+i))/64
        	echo "  Retrieving $id $v6ip"

		ifconfig $id down
		ifconfig $id $v4ip netmask 255.255.0.0 up
		sleep 5

		echo $id > id.txt

		#ping6 -I $(cat id.txt) ff02::1 -w 30 -c 4
		rpi=$(ping6 -I $(cat id.txt) ff02::1 -w 30 -c 2  | grep 'bytes from' | grep 'DUP' | awk '{print $4}' | sed 's/.$//')
		myipv6=$(ip addr show dev $(cat id.txt) | grep 'link' | sed -e's/^.*inet6 \([^ ]*\)\/.*$/\1/;t;d')

		printf "$id $rpi $myipv6\n"
		printf "$id $rpi $myipv6\n" >> $RPIS
		echo $rpi > rpi.txt
		echo $myipv6 > myipv6.txt

		sleep 1

		#gnome-terminal -x bash -c "sshpass -p 'raspberry' ssh -t -o StrictHostKeyChecking=no pi@$(cat rpi.txt)%$(cat id.txt) './streamv4.sh $HEIGHT $WIDTH $FORMAT $(cat myipv6.txt) none 100 100 100 none; bash -i' "
		#gnome-terminal -x bash -c "sshpass -p 'raspberry' ssh -t -o StrictHostKeyChecking=no pi@$(cat rpi.txt)%$(cat id.txt) './bayer.sh $(cat myipv6.txt) $FRAMERATE $HEIGHT $WIDTH; bash -i' "

		gnome-terminal -x bash -c "sshpass -p 'raspberry' ssh -t -o StrictHostKeyChecking=no pi@$(cat rpi.txt)%$(cat id.txt)"

		i=$((i+1))
	fi
done)

./measurequality.sh $MEASUREPERIOD $MEASUREPORTION &
