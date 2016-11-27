
import io 
import os 
import picamera 
import sys
import socket
import time

period=600
port=6666
interface="%usb0"
iso_=int(sys.argv[6])
frmat=sys.argv[5]
camera = picamera.PiCamera()
width=int(sys.argv[3])
height=int(sys.argv[4])
camera.resolution = (width,height)
camera.framerate = int(sys.argv[2])
#camera.sharpness=-100
#camera.iso=iso_
pchost=sys.argv[1]

addrinfo = socket.getaddrinfo(pchost+interface, port, socket.AF_INET6, socket.SOCK_STREAM)
(family, socktype, proto, canonname, sockaddr) = addrinfo[0]
s = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
s.connect(sockaddr)

connection = s.makefile('wb')
time.sleep(1)

camera.start_recording(connection, format=frmat,bitrate=25000000)

camera.wait_recording(period)
camera.stop_recording()

connection.close()
s.close()
