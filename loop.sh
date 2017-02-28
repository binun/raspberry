while true; do 
	if [ -s "host.txt" ]
	then
		echo "host found"
		python3 bsim.py $(cat host.txt)
		break
	fi
	sleep 1;
done
