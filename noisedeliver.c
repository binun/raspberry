#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <sys/file.h>
#include <fcntl.h>
#include "config.h"

//long MAXFILESIZE = 1073741824;
long MAXFILESIZE = 5368709120;
 
long MIN(long x, long y)
{
	if (x > y) return x;
	else return y;
}

void *deliver_handler(void*arg)
{
    struct sockaddr_in me, client;
    FILE *fnoise;
    int sock,addr_size = sizeof(client);
  
    sock = socket(AF_INET , SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    printf("NOISE Delivery  Socket created\n");
    
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = INADDR_ANY;
    me.sin_port = htons( SERVPORT );
     
    if( bind(sock,(struct sockaddr *)&me , sizeof(me)) < 0)
    {
        printf("bind failed. Error");
        return NULL;
    }
   
    fnoise = fopen(NOISEFILE, "rb");
    listen(sock , 3);
    
    int client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t*)&addr_size);
    if (client_sock<0) {
		printf("could not create thread\n");
        return NULL;
	}
	
    printf("NOISE Client accepted\n");     
    while( 1 )
    {
	   char server_reply[CLIENTBUF]="";
       long fpos = ftell(fnoise);
       int remainder = fread(server_reply, sizeof(unsigned char), CLIENTBUF, fnoise);
       
       if (fpos>=MAXFILESIZE || remainder<=0) {
         rewind(fnoise);
	   }
         
       printf("             REPLY %d at %ld\n",remainder,fpos);  
       
       write(client_sock ,server_reply , remainder); 
       usleep(10000);
    }
    
    fclose(fnoise);         
    return NULL;
}
 
 
int main(int argc , char *argv[])
{  
	pthread_t deliverthread;
	pthread_create(&deliverthread, NULL, deliver_handler, NULL);
    while (1) {}
}
