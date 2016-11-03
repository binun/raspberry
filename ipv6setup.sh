
#/bin/bash -i

DRY=0

HEIGHT=3280
WIDTH=2464
FRAMERATE=30

IFACE=enp
i=1

RPIS=rpientries.txt
MAIN_IFACE=enp10s0
MAIN_IPV6=$(ifconfig enp10s0 | grep 'inet6 addr' | grep 'Scope:Link' | awk '{ print $3}')
service network-manager stop

rm -rf noise*.bin
rm -f $RPIS

gnome-terminal -x bash -c "./tcpserver"

#touch noise.bin
#./measuresize.sh noise.bin &
#gnome-terminal -x bash -c "nc -6 -u -vv -k -l -p 6666 > noise.bin"

ifconfig -a | sed 's/[ \t].*//;/^$/d' | grep -v 'avahi' | grep $IFACE | (while read id; do

	#rm -f id.txt
	#rm -f rpi.txt
	#rm -f myipv6.txt
	#rm -f v6ip.txt

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

		#echo "sshpass -p 'raspberry' ssh -t -o StrictHostKeyChecking=no pi@$(cat rpi.txt)%$(cat id.txt) './streamv4.sh 1920 1080 4 30 $(cat myipv6.txt); bash -i' " > cmd.txt
		gnome-terminal -x bash -c "sshpass -p 'raspberry' ssh -t -o StrictHostKeyChecking=no pi@$(cat rpi.txt)%$(cat id.txt) './streamv4.sh $HEIGHT $WIDTH 8 $FRAMERATE $(cat myipv6.txt); bash -i' "
		#gnome-terminal -x bash -c "sshpass -p 'raspberry' ssh -t -o StrictHostKeyChecking=no pi@$(cat rpi.txt)%$(cat id.txt)"

		i=$((i+1))
	fi
done)

./measurequality.sh 10 &
