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
} bufferFor;

pthread_mutex_t noise_mutex;
pthread_mutex_t client_mutex;

bufferFor bufFiles[32];
byte binbuffer[RPIBUFFER];
int connectionNo = 0;
int snapshot=1;
int sockmode = SOCK_STREAM;
int addresslen;
int chain=1;

void *connection_handler(void*);
void *timer_handler(void*);
void *simulation_handler(void*);
void *client_handler(void*);

struct tm * current_time() {
    time_t rawtime;
    time ( &rawtime );
    
    struct tm *timeinfo = localtime ( &rawtime );

    printf("[%d %d %d %d:%d:%d]\n",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    return timeinfo;
}

int main(int argc, char *argv[])
{
	int 	sockfd;
	int ch1,ch2;
	struct sockaddr_in6 cli_addr, serv_addr;
    struct sched_param params1;
    pthread_t timerthread;
    //pthread_t splitthread;
    pthread_t clientthread;
    
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
	
	
    if (SIMULATION) {
		for (int i = 0; i < 2; i++) {
		   pthread_t simthread;
           int *new_i = malloc(1);
           *new_i = i;
           pthread_create(&simthread, NULL, simulation_handler, (void*)(new_i));
	   }
	   while (1) {}
	   return 0;
	
	}
    
	if((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
		fprintf(stderr,"server: can't open stream socket\n"), exit(0);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin6_flowinfo = 0;
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(RPIPORT);
   
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
           fprintf(stderr,"server: can't bind local address\n"), exit(0);

	pthread_create(&timerthread, NULL, timer_handler, NULL);
    params1.sched_priority = sched_get_priority_min(SCHED_FIFO);
    pthread_setschedparam(timerthread, SCHED_FIFO, &params1);
    
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
           
        
	int read_size;
	char noisename[64] = "";
    char recnoisename[64] = "recent";
    
    long fpos = 0;
    
    char jointnoisename[64] = NOISEFILE;
    strcpy(noisename,conn->filename);
    strcat(recnoisename,noisename);
    
    FILE * fnoise =NULL;
    FILE * frecent = NULL;
    FILE * fjoint = NULL;
    
    for (;;) {
	  long sz=0;
      read_size = recv(conn->socket, binbuffer, RPIBUFFER, 0);
       
	  if (read_size<=0) {

           continue;
	  }
	   
	   pthread_mutex_lock(&noise_mutex);
	   
	   fnoise = fopen(noisename, "a+b");
       frecent = fopen(recnoisename, "a+b");
       fjoint = fopen(jointnoisename, "a+b");
       
       flock(fileno(fnoise), LOCK_EX);
       flock(fileno(frecent), LOCK_EX);
       flock(fileno(fjoint), LOCK_EX);
       
       fseek(fnoise,0L,SEEK_END);
       fseek(frecent,0L,SEEK_END);
       fseek(fjoint,0L,SEEK_END);
       
       fwrite(binbuffer, sizeof(byte), read_size, frecent);
       
       if (ftell(fnoise) < MAXFILESIZE)
	      fwrite(binbuffer, sizeof(byte), read_size, fnoise);
	   
	   //if (ftell(fjoint) < MAXFILESIZE)
	      //fwrite(binbuffer, sizeof(byte), read_size, fjoint);
	      
	   if (read_size < MAXFILESIZE-ftell(fjoint))
			fwrite(binbuffer, sizeof(byte), read_size, fjoint);
	    else {
			int remainder1 = MAXFILESIZE-ftell(fjoint);
			int remainder2 = read_size-remainder1;
			FILE *fnext = NULL;
			
			char nextchain[64] = "";
			fwrite(binbuffer, sizeof(byte), remainder1, fjoint);
			chain++;
			sprintf(nextchain,CHAIN,chain);
			fnext = fopen(nextchain,"a+b");
			flock(fileno(fnext),LOCK_EX);
			fwrite(binbuffer+remainder1, sizeof(byte), remainder2, fnext);
			flock(fileno(fnext),LOCK_UN);
			fclose(fnext);
		}
	   
	   fflush(fnoise);
	   fflush(frecent);
	   fflush(fjoint);
	   
	   pthread_mutex_unlock(&noise_mutex);
	        
	   memset(binbuffer, 0, RPIBUFFER);
	   
	   flock(fileno(fnoise), LOCK_UN);
       flock(fileno(frecent), LOCK_UN);
       flock(fileno(fjoint), LOCK_UN);
	   
	   fclose(fnoise);
	   fclose(frecent);
	   fclose(fjoint);
    }

	return 0;
}

void *simulation_handler(void *socket_desc)
{
        
	int read_size;
    int i = *(int*)socket_desc;
    
    FILE * fnoise =NULL;
   
    for (;;) {
	
	   read_size = RPIBUFFER;
     
	   char noisename[64] = "";
	   sprintf(noisename,CHAIN,chain);
	   fnoise = fopen(noisename, "a+b");
	   flock(fileno(fnoise), LOCK_EX);
	   fseek(fnoise,0L,SEEK_END);
	   int remainder1 = MIN(read_size, MAXFILESIZE-ftell(fnoise));
	   int remainder2 = read_size - remainder1;
	   
	   if (remainder1>0) {
		  fwrite(binbuffer, sizeof(byte), read_size, fnoise);
		  //printf("Written %d in %s\n", remainder1,noisename);
	  }
	   
	   flock(fileno(fnoise), LOCK_UN);
	   fclose(fnoise);
	   
	   if (remainder2>0)
	   {
		   char nextchain[64] = "";
		   chain++;
		   sprintf(nextchain,CHAIN,chain);
		   FILE *fnext = fopen(nextchain,"a+b");
		   flock(fileno(fnext),LOCK_EX);
		   fwrite(binbuffer+remainder1, sizeof(byte), remainder2, fnext);
		   flock(fileno(fnext),LOCK_UN);
		   fclose(fnext);
		   //printf("Remainder %d in %s\n", remainder2,nextchain);
	   }
			
	   usleep(33333);     
	   memset(binbuffer, 0, RPIBUFFER);
	   __sync_fetch_and_add (&(bufFiles[i].size),read_size);
    }

	return 0;
}

void *timer_handler(void*arg)
{
  int period = 0;
  FILE *fnoise;
  for (;;) {
        int i=0,fd=0;
        long fsize=0;
        if (connectionNo>0)
           printf("%d ", snapshot++);
           
        //pthread_mutex_lock(&noise_mutex);
        for (i=0; i < connectionNo; i++) {
   
          //int noise;
          //int size=0;
          //char noiseName[64] = "";
          //char recent_noiseName[64] = "";
          //sprintf(noiseName, "noise%d.bin", i + 1);
          //sprintf(recent_noiseName, "recentnoise%d.bin", i + 1);
          
          //if (period % RECENT_NOISE_TIME ==0) {
             //remove(recent_noiseName);
		  //}
		  
		  //fnoise = fopen(NOISEFILE,"rb");
          //fseek(fnoise, 0L, SEEK_END);
          //fsize = ftell(fnoise);
          //fclose(fnoise);
          
          //printf("%ld ", FACTOR*(fsize - bufFiles[i].prevsize));
          //bufFiles[i].prevsize = fsize;
          
          int s = __sync_fetch_and_add( &(bufFiles[i].size), 0 );
          printf("%d ", FACTOR*s);
          __sync_fetch_and_and(&(bufFiles[i].size), 0);
          
        }
        
  	    //pthread_mutex_unlock(&noise_mutex);
        if (connectionNo>0)
            printf("\n");
  	    sleep(1);
        period++;
  }

}
