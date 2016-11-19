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

while True:
	camera.capture(stream,format='rgb');
	buff=numpy.fromstring(stream.getvalue(),dtype=numpy.uint8);
	print buff.nbytes;
	
	bitmap=bitarray(int(buff.nbytes/8)+1);
	bitmap.setall(0);
	index=0;
	print ('Begin');
	for b in buff:
		bitmap[index] = b & 1;
		index+=1;
		print(index);
	#print(bitmap[4]);
	#time.sleep(delay);	
 	stream.seek(0);
	stream.truncate();

