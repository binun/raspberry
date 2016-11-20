
import io 
import os 
import picamera 
import sys
import socket
import time

def outputs():
    stream = io.BytesIO()
    for i in range(10000):
        yield stream

        connection.flush()
        stream.seek(-6404096, io.SEEK_END)
        connection.write(stream.read())
        stream.seek(0)
        stream.truncate()


port=6666
interface="%usb0"
camera = picamera.PiCamera()
width=int(sys.argv[3])
height=int(sys.argv[4])
camera.resolution = (width,height)
camera.framerate = int(sys.argv[2])
pchost=sys.argv[1]
stream=io.BytesIO()
len=6404096

addrinfo = socket.getaddrinfo(pchost+interface, port, socket.AF_INET6, socket.SOCK_STREAM)
(family, socktype, proto, canonname, sockaddr) = addrinfo[0]
s = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
s.connect(sockaddr)

connection = s.makefile('wb')
time.sleep(1)
while True:
	camera.capture_sequence(outputs(),format='rgb',use_video_port=False,burst=True)
#
#while True:
	#camera.capture(stream, format='jpeg', bayer=True)
	#connection.flush()
	#stream.seek(-len, io.SEEK_END)
	#os.write(1,stream.read(len))
	#connection.write(stream.read(len))
	#stream.truncate(0)
	
connection.close()
s.close()
