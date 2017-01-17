all:
	clean tcpserver noiseclient noisedeliver

clean:
	rm -f tcpserver noiseclient noisedeliver

tcpserver:
	gcc tcpserver.c -o tcpserver -lpthread 

noiseclient:
	gcc noiseclient.c -o noiseclient

noisedeliver:
	gcc noisedeliver.c -o noisedeliver -lpthread
