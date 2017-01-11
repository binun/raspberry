#define _REENTRANT
#include <stdio.h>
#include <unistd.h>  
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <time.h>
#include <pthread.h>  //for threading , link with lpthread
#include <sys/un.h>
#include <sched.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <sys/file.h>
#include <errno.h>
#include "config.h"

#define LICDAYS 365

typedef unsigned char byte;
typedef struct tagbufferFor {
	char ip[64];
	char filename[64];
    int socket;
    int size;
    int ordNo;
} bufferFor;

pthread_mutex_t noise_mutex;
bufferFor bufFiles[32];
byte binbuffer[RPIBUFFER];
int connectionNo = 0;
int sockmode = SOCK_STREAM;
int addresslen;

long fpos=0;
long MAXFILESIZE = 5368709120;

FILE *fnoise = NULL;
                 
//long MAXFILESIZE = 1073741824;

void *connection_handler(void*);
void *timer_handler(void*);
void *simulation_handler(void*);
void *client_handler(void*);
void *deliver_handler(void*);

struct tm * current_time() {
    time_t rawtime;
    time ( &rawtime );
    
    struct tm *timeinfo = localtime ( &rawtime );

    printf("[%d %d %d %d:%d:%d]\n",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    return timeinfo;
}

long MIN(long x, long y)
{
	if (x > y) return x;
	else return y;
}

int main(int argc, char *argv[])
{
	int 	sockfd;
	int ch1,ch2;
	struct sockaddr_in6 cli_addr, serv_addr;
    struct sched_param params1;
    pthread_t timerthread;
    pthread_t clientthread;
    pthread_t deliverthread;
    
    FILE *datefile;   
   
    /* install*/
    if (argc==2) { 
		printf("    Installed: "); 
		struct tm * now = current_time();
		
		datefile=fopen("timestamp","wb");	
        fwrite(now, sizeof(struct tm),1,datefile);
        fclose(datefile);
        return 0;
	}
	else {
		struct tm install_time;
		
	  	datefile=fopen("timestamp","rb");
	  	if (datefile==NULL) {
			printf("  Illegal execution: no date\n");
			return -1;
		}
		
		fread(&install_time, sizeof(struct tm),1,datefile);
		fclose(datefile);
		
		printf("    Retrieved: "); 
		struct tm * now = current_time();
		
	    int diffSecs = (int)difftime(mktime(now), mktime(&install_time)); 
	    int diffDays = (int)(difftime(mktime(now), mktime(&install_time)) / (60 * 60 * 24));
		printf("    Installed diff %d %d\n",diffSecs,diffDays);
		if (diffDays>LICDAYS)
		    return 0;
	}
	
	fnoise = fopen(NOISEFILE, "r+b");
	
    
    
	if((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
		fprintf(stderr,"server: can't open stream socket\n"), exit(0);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin6_flowinfo = 0;
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(RPIPORT);
   
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
           fprintf(stderr,"server: can't bind local address\n");
           exit(0);
	}
	
	if (SIMULATION) {
		connectionNo = 4;
		for (int i = 0; i < connectionNo; i++) 
		{
		   pthread_t simthread;
           int *new_i = malloc(1);
           *new_i = i;
           pthread_create(&simthread, NULL, simulation_handler, (void*)(new_i));
	    }
	    pthread_create(&timerthread, NULL, timer_handler, NULL);
	    pthread_create(&deliverthread, NULL, deliver_handler, NULL);
	    while (1) {}
	    return 0;
	
	}

	pthread_create(&timerthread, NULL, timer_handler, NULL);
    params1.sched_priority = sched_get_priority_min(SCHED_FIFO);
    pthread_setschedparam(timerthread, SCHED_FIFO, &params1);
    
    pthread_create(&deliverthread, NULL, deliver_handler, NULL);
    
    listen(sockfd, 5);
	//Accept and incoming connection
	printf("Waiting for incoming connections...\n");
        for (;;) {
            pthread_t rpithread;
            struct sched_param params;
            int clilen = sizeof(cli_addr);
            char bufFilename[64] = "";
            char theirIP[100]="";

	        int client_sock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	        if(client_sock < 0)
	        fprintf(stderr,"server: accept error\n"), exit(0);
            
            inet_ntop(AF_INET6, &(cli_addr.sin6_addr),theirIP, 100);
            printf("Incoming connection from client having IPv6 address: %s\n",theirIP);
	    
	        sprintf(bufFilename, "noise%d.bin", connectionNo + 1);
	        strcpy(bufFiles[connectionNo].ip, theirIP);
	        strcpy(bufFiles[connectionNo].filename, bufFilename);
	        bufFiles[connectionNo].socket = client_sock;
	        bufFiles[connectionNo].size = 0;
	        bufFiles[connectionNo].ordNo = connectionNo;
	        printf("Stream from IP %s will be directed to %s\n", bufFiles[connectionNo].ip, bufFiles[connectionNo].filename);
 
            pthread_create(&rpithread, NULL, connection_handler, (void*)(bufFiles+connectionNo));
            params.sched_priority = sched_get_priority_max(SCHED_FIFO);
            pthread_setschedparam(rpithread, SCHED_FIFO, &params); 
            
            pthread_detach(rpithread);
            connectionNo++;
        }	
	return 0;
}

void *connection_handler(void *connection)
{
        
	bufferFor * conn = (bufferFor*)connection;
	int i = conn->ordNo;
           
	char recnoisename[64] = "";
    sprintf(recnoisename,"recentnoise%d.bin",i);
    
    for (;;) 
    {
      int read_size = recv(conn->socket, binbuffer, RPIBUFFER, 0);
       
	  if (read_size<=0) {

           continue;
	  }
	   
	   FILE *frecent = fopen(recnoisename, "a+b");
       fwrite(binbuffer, sizeof(byte), read_size, frecent);
       fclose(frecent);
       
	   pthread_mutex_lock(&noise_mutex);
	   
       int remainder = read_size;
       fpos = ftell(fnoise);
       
       if (MAXFILESIZE - fpos < read_size)
		 remainder = (int)(MAXFILESIZE - fpos+1);
		 
       fwrite(binbuffer, sizeof(byte), remainder, fnoise);
       //printf("% d writing %d bytes at %ld\n",i, remainder ,fpos);
     
       if (fpos>=MAXFILESIZE || remainder==0) {
		 rewind(fnoise);   
	   }
        
       ///fclose(fnoise);
       fflush(fnoise); fsync(fileno(fnoise));
       pthread_mutex_unlock(&noise_mutex);
       
              
	   memset(binbuffer, 0, RPIBUFFER);
	   __sync_fetch_and_add (&(bufFiles[i].size),read_size);
    }

	return 0;
}

void *simulation_handler(void *socket_desc)
{
    int i = *(int*)socket_desc;
    char recnoisename[64] = "";
    sprintf(recnoisename,"recentnoise%d.bin",i);
    for (;;) 
    {
	
	   int read_size = RPIBUFFER;
	   
       //FILE *frecent = fopen(recnoisename, "a+b");
       //fwrite(binbuffer, sizeof(byte), read_size, frecent);
      // fclose(frecent);
       
	   pthread_mutex_lock(&noise_mutex);  
       
       int remainder = read_size;
       fpos = ftell(fnoise);
       
       if (MAXFILESIZE - fpos < read_size)
		 remainder = (int)(MAXFILESIZE - fpos+1);
		 
       fwrite(binbuffer, sizeof(byte), remainder, fnoise);
       //printf("% d writing %d bytes at %ld\n",i, remainder ,fpos);
     
       if (fpos>=MAXFILESIZE || remainder==0) {
	     rewind(fnoise);
	   }
        
       fflush(fnoise); fsync(fileno(fnoise));
       pthread_mutex_unlock(&noise_mutex);
              
	   memset(binbuffer, 0, RPIBUFFER);
	   usleep(33333);
	   __sync_fetch_and_add (&(bufFiles[i].size),read_size);
    }


	return 0;
}

void *timer_handler(void*arg)
{
  int period = 1;
  for (;;) {
        int i=0;
        long fsize=0;
        if (connectionNo>0)
        //printf("%d ", period);
           
        for (i=0; i < connectionNo; i++) 
        {        
          int s = __sync_fetch_and_add( &(bufFiles[i].size), 0 );
          //printf("%d ", FACTOR*s);
          __sync_fetch_and_and(&(bufFiles[i].size), 0);         
        }
      
        if (connectionNo>0)
            //printf("\n");
  	    sleep(1);
        period++;
  }

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
