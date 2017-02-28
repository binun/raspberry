fuser -k 5555/tcp
rm -f host.txt
sh -c "nc -6 -l 5555 > host.txt" &
./loop.sh &
