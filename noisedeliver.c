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
int fptr=0L;
int chain=1;

struct flock lck;
 
//the thread function
void *connection_handler(void *);
 
int main(int argc , char *argv[])
{
    int sock,client_sock;
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
    __sync_fetch_and_and(&fptr, 0);   
    listen(sock , 3);
    
    puts("NOISE Delivery Waiting \n");
    while( (client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t*)&addr_size)) )
    {
        printf("NOISE Client accepted\n");
         
        pthread_t sniffer_thread;
        int *new_sock = malloc(1);
        *new_sock = client_sock;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            printf("could not create thread\n");
            return 1;
        }
    }
     
    if (client_sock < 0)
    {
        printf("accept failed\n");
        return 1;
    }
     
    return 0;
}
 

 
void *connection_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    int send_size;
    int noise;
    int phase=0;
    char noisefile[64] = "";
    sprintf(noisefile,CHAIN,chain);
    while (1) 
    {
		char client_message[10] = "";
		char command[64] = "";
		//printf("Respond phase %s", noisefile);
		int read_size = recv(sock , client_message , 10 , 0);
		if(read_size <= 0)
         {
           continue;
         }
        
        
        memset(server_reply,0,CLIENTBUF);
        
        noise=-1;
        while (1) {
			printf("Opening %s\n",noisefile); 
            noise = open(noisefile, O_RDONLY, 0666);
            if (noise > 0) break;
         
            usleep(10000);
	    }
        printf("Opened %s on %d\n",noisefile,fptr); 
        //if (noise<0) { printf("Opening %s\n", noisefile); continue; }
        //else  
        //
        pthread_mutex_lock(&lock);
        
        
        
        //printf("Respond  2 to client %d\n", sock);
        
        //lck.l_start = fptr;
        //lck.l_whence = SEEK_SET;
        //lck.l_len = CLIENTBUF;
        //lck.l_type = F_RDLCK;
        //fcntl (noise, F_SETLKW, &lck);
    
        //printf("Respond to client %d\n", sock);
        int fsize = (int)(lseek(noise, (size_t)0, SEEK_END)+1);
        int remainder = MIN(CLIENTBUF, fsize - fptr);
        if (remainder > 0)
        {
			read(noise,server_reply,remainder);
			//printf("In the primary chain %d \n", remainder);
		}	
		
        //lck.l_type = F_UNLCK;
        //fcntl (noise, F_SETLKW, &lck);
        
        close(noise);
        
        fptr = fptr + remainder;    
		if (CLIENTBUF - remainder>0) {
		  char rmcommand[64] = "";
		  
		  fptr=0;
		  sprintf(rmcommand,"rm -f noise%d.bin", chain); 
          printf("Removing %s\n", rmcommand);
          chain++;
          //system(rmcommand);
          
          memset(noisefile,0,64);
          sprintf(noisefile,CHAIN,chain);
          printf("Changed chain %d to %s\n", chain,noisefile);
          
	   }
	   
	  pthread_mutex_unlock(&lock);
      write(sock ,server_reply , remainder);   
    }
    return 0;
}
