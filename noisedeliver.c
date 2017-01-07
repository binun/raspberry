#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <sys/file.h>
#include "config.h"


char server_reply[CLIENTBUF]="";
pthread_mutex_t noise_mutex;
long fptr=0L;
int chain=0;
 
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
      
    
    
    /*while(1)
    {
      char client_message[10] = "";
      int read_size = recvfrom(sock , client_message , 10 , 0, (struct sockaddr *)&other, &addr_size);
      if(read_size <= 0)
         {
           continue;
         }
        
        //printf("Respond to client %d\n", sock);
        memset(server_reply,0,CLIENTBUF);
        
        fnoise = fopen(NOISEFILE, "rb");    
        flock(fileno(fnoise), LOCK_EX);
        fseek(fnoise, 0L, SEEK_END);
        fsize = ftell(fnoise);
        send_size = MIN(fsize-fptr,CLIENTBUF);
        fseek(fnoise,fptr,SEEK_SET);
        printf("Noisesize %ld sendsize %d curoffset %ld\n", fsize,send_size,fptr);
        if (send_size>=MIN_REPLY) 
        {      
          fread(server_reply,1,send_size,fnoise);
          fptr = fptr + send_size;
          //write(sock ,server_reply , send_size);
          sendto(sock,server_reply,send_size,0,(struct sockaddr *)&other,addr_size);
	    }
	    
	    flock(fileno(fnoise), LOCK_UN);
        fclose(fnoise);
          
        if (fsize > (long)(MAXFILESIZE*CLEAN_FACTOR)) {
			long bottom_line = (long)(fptr / MBYTE) - 1;
			char command[128] = "";
			
			sprintf(command, "dd if=%s of=chunk ibs=1M obs=1M skip=%ld status=none;mv chunk %s; rm -f chunk", NOISEFILE, bottom_line,NOISEFILE);
	        printf("Cutting file %s\n", command);
	        system(command);  
		}   
      }*/
      
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
    FILE *fnoise,*ftstamp;
    long fsize=0;
    int phase=0;
 
    while (1) 
    {
		char client_message[10] = "";
		char command[64] = "";
		char noisefile[64] = "";
		int read_size = recv(sock , client_message , 10 , 0);
		if(read_size <= 0)
         {
           continue;
         }
        
        printf("Respond  1 to client %d\n", sock);
        memset(server_reply,0,CLIENTBUF);
        
        //pthread_mutex_lock(&noise_mutex);
        
        //strcpy(noisefile,NOISEFILE);
        sprintf(noisefile,CHAIN,chain);
        fnoise = fopen(noisefile, "rb"); 
        printf(noisefile); 
        if (fnoise==NULL) continue;  
        
        printf("Respond  2 to client %d\n", sock);
        
        flock(fileno(fnoise), LOCK_EX);
        fseek(fnoise, 0L, SEEK_END);
        fsize = ftell(fnoise);
        printf("Respond  3 to client %d\n", sock);
        
        if (fsize-fptr > CLIENTBUF) 
        {
		  fread(server_reply,1,CLIENTBUF,fnoise);
          fptr = fptr + CLIENTBUF;
          flock(fileno(fnoise), LOCK_UN);
          printf("Working through the primary chain %d \n", chain); 
          fclose(fnoise);
		}
		else {
		  int remainder1 = fsize-fptr;
		  int remainder2 = CLIENTBUF - remainder1;
		  FILE *fnext = NULL;	
		  char rmcommand[64] = "";	
		  char nextchain[64] = "";
		  fread(server_reply, sizeof(unsigned char), remainder1, fnoise);
		  
		  chain++;
		  sprintf(nextchain,CHAIN,chain);
		  fnext = fopen(nextchain,"rb");
		  if (fnext!=NULL) {
		    flock(fileno(fnext),LOCK_EX);
		    fread(server_reply+remainder1, sizeof(unsigned char), remainder2, fnoise);
		  
		    flock(fileno(fnext),LOCK_UN);
		    fclose(fnext);
	       }
	      fptr = fptr + remainder1+remainder2;flock(fileno(fnoise), LOCK_UN);
          fclose(fnoise);
          sprintf(rmcommand,"rm -f noise%d.bin", chain-1); 
          printf("Removing %s\n", rmcommand); 
          system(rmcommand);
          //remove(noisefile);
		}
		
        /*fptr = fptr + send_size;
        send_size = MIN(fsize-fptr,CLIENTBUF);
        fseek(fnoise,fptr,SEEK_SET);
        printf("Noisesize %ld sendsize %d curoffset %ld\n", fsize,send_size,fptr);
        if (send_size>=MIN_REPLY) 
        {      
          fread(server_reply,1,send_size,fnoise);
          fptr = fptr + send_size;
          write(sock ,server_reply , send_size);
	    }
	    
	    flock(fileno(fnoise), LOCK_UN);
        fclose(fnoise);
          
        if (fsize > (long)(MAXFILESIZE*CLEAN_FACTOR)) {
			long bottom_line = (long)(fptr / MBYTE) - 1;
			char command[128] = "";
			
			sprintf(command, "dd if=%s of=chunk ibs=1M obs=1M skip=%ld status=none;mv chunk %s; rm -f chunk", NOISEFILE, bottom_line,NOISEFILE);
	        printf("Cutting file %s\n", command);
	        system(command);  
		}
		*/
		
		
		//pthread_mutex_unlock(&noise_mutex);
		
	}
    //Free the socket pointer
    free(socket_desc);
     
    return 0;
}
