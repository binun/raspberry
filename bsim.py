import io 
import os  
import sys
import socket
import time

port=6666
#pchost="::1"
pchost=sys.argv[1]
stream=io.BytesIO(b"1,2,3,4,5,6,7,8,9")

addrinfo = socket.getaddrinfo(pchost, port, socket.AF_INET6, socket.SOCK_STREAM)
(family, socktype, proto, canonname, sockaddr) = addrinfo[0]
print(family)
print(socktype)
print(proto)
print(canonname)
print(sockaddr)
s = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)

s.connect(sockaddr)
connection = s.makefile('wb')
time.sleep(1)
while True:
	connection.flush()
	connection.write(stream.read())
	time.sleep(1)
	stream.seek(0)
	print ("Written\n")

	
connection.close()
s.close()
