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


char server_reply[CLIENTBUF]="";
pthread_mutex_t lock;
long fpos=0L;
//long MAXFILESIZE = 1073741824;
long MAXFILESIZE = 5368709120;

struct flock lck;
 
long MIN(long x, long y)
{
	if (x > y) return x;
	else return y;
} 
 
int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in me, client;
    int addr_size = sizeof(client);
    int send_size;
    FILE *fnoise,*ftstamp;
    long fsize=0;
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    printf("NOISE Delivery  Socket created\n");
     
    //Prepare the sockaddr_in structure
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = INADDR_ANY;
    me.sin_port = htons( SERVPORT );
     
    //Bind
    if( bind(sock,(struct sockaddr *)&me , sizeof(me)) < 0)
    {
        //print the error message
        printf("bind failed. Error");
        return 1;
    }
    fpos=0;   
    listen(sock , 3);
    
    puts("NOISE Delivery Waiting \n");
    int client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t*)&addr_size);
    if (client_sock<0) {
		printf("could not create thread\n");
        return 1;
	}
	
    printf("NOISE Client accepted\n");
     
    while( 1 )
    {
       memset(server_reply,0,CLIENTBUF);
       
       FILE *fnoise = fopen(NOISEFILE, "rb");
	         
       fseek(fnoise,fpos, SEEK_SET);
       
       int remainder = fread(server_reply, sizeof(unsigned char), CLIENTBUF, fnoise);
       fpos = fpos + remainder;
       if (fpos>=MAXFILESIZE || remainder<=0) 
         fpos=0;
         
       //printf("             REPLY %d at %ld\n",remainder,fpos);  
       fclose(fnoise);
       write(client_sock ,server_reply , remainder); 
       usleep(10000);
    }
            
    return 0;
}
