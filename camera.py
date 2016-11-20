import io
import socket
import sys
import time
import cv2 
import picamera
import numpy
from bitarray import bitarray

host='10.0.0.3';
port= 7500;
resol=(2592,1644);

if len(sys.argv)>1:
	host=sys.argv[1]	

# client_socket=socket.socket();
# client_socket.connect((host,port));
# connection=client_socket.makefile('wb')

camera=picamera.PiCamera();
camera.resolution=resol;
camera.framerate=10;
delay=1/camera.framerate;
# stream=PiRGBArray(camera,size=resol);
stream=io.BytesIO();
time.sleep(2);
print ('Warming up ...');

for frame in camera.capture_continuous(stream, format=frmat):
	stream.seek(0)
	stream.truncate()

while True:
	camera.capture(stream,format='rgb')

 	stream.seek(0);
	stream.truncate();

