ipv6="::1"
gnome-terminal -x bash -c "./tcpserver"

echo $ipv6 | nc -6 $ipv6 5555
