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

#define PORT 6666
#define BUFSIZE 8*1024*1024
#define LOG "report.csv"
#define FACTOR 4
#define RECENT_TIME 2
#define LICDAYS 365
#define FSIZE 1073741824

typedef unsigned char byte;
typedef struct tagbufferFor {
	char ip[64];
	char filename[64];
        int socket;
        long prevsize;
} bufferFor;

pthread_mutex_t noise_mutex;
bufferFor bufFiles[32];
byte binbuffer[BUFSIZE];
int connectionNo = 0;
int snapshot=1;

void *connection_handler(void*);
void *timer_handler(void*);

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
	struct sockaddr_in6 cli_addr, serv_addr;
    struct sched_param params1;
    pthread_t timerthread;
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
		
	    long diffSecs = (long)difftime(mktime(now), mktime(&install_time)); 
	    long diffDays = (long)(difftime(mktime(now), mktime(&install_time)) / (60 * 60 * 24));
		printf("    Installed diff %ul %ul\n",diffSecs,diffDays);
		if (diffDays>LICDAYS)
		    return 0;
	}
    
    
	if((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
		fprintf(stderr,"server: can't open stream socket\n"), exit(0);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin6_flowinfo = 0;
        serv_addr.sin6_family = AF_INET6;
        serv_addr.sin6_addr = in6addr_any;
        serv_addr.sin6_port = htons(PORT);
   
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
	    printf("Stream from IP %s will be directed to %s\n", bufFiles[connectionNo].ip, bufFiles[connectionNo].filename);
 
            pthread_create(&rpithread, NULL, connection_handler, (void*)(bufFiles+connectionNo));
            params.sched_priority = sched_get_priority_max(SCHED_FIFO);
            pthread_setschedparam(rpithread, SCHED_FIFO, &params); 
            
            pthread_detach(rpithread);
            connectionNo++;
        }	
	return 0;
}

/*
* This will handle connection for each client
* */
void *connection_handler(void *connection)
{
        
	bufferFor * conn = (bufferFor*)connection;
           
        
	int read_size;
	char noisename[64] = "";
    char recnoisename[64] = "recent";

    strcpy(noisename,conn->filename);
    strcat(recnoisename,noisename);
    
    FILE * f = fopen(noisename, "a+b");
    FILE * fr = fopen(recnoisename, "a+b");
    
    for (;;) {
	  long sz=0;
      read_size = recv(conn->socket, binbuffer, BUFSIZE, 0);
       
	      if (read_size<=0) {

                continue;
	      }
       //printf("%d bytes were read\n", read_size);    
       pthread_mutex_lock(&noise_mutex);
       
       fwrite(binbuffer, sizeof(byte), read_size, fr);
       
       
       sz = ftell(f);
       if (sz+read_size>FSIZE) {
		   read_size=FSIZE-sz;
		   fwrite(binbuffer, sizeof(byte), read_size, f);
		   rewind(f);
	   }
	   else
	       fwrite(binbuffer, sizeof(byte), read_size, f);
	      
	       
	   fflush(f);
	   fflush(fr);
       pthread_mutex_unlock(&noise_mutex);
	   memset(binbuffer, 0, BUFSIZE);
	   
	   fclose(f);
	   fclose(fr);
    }

	return 0;
}

void *timer_handler(void*arg)
{
  int period = 0;
  for (;;) {
        int i=0,fd=0;
        long diff=0;
        if (connectionNo>0)
           printf("%d ", snapshot++);
           
        pthread_mutex_lock(&noise_mutex);
        for (i=0; i < connectionNo; i++) {
          struct stat buf;
          int noise;
          int size=0;
          char noiseName[64] = "";
          char recent_noiseName[64] = "";

          sprintf(noiseName, "noise%d.bin", i + 1);
          sprintf(recent_noiseName, "recentnoise%d.bin", i + 1);
          
          if (period % RECENT_TIME ==0) {
             remove(recent_noiseName);
		  }
          noise=open(noiseName,O_RDONLY);
          fstat(noise, &buf);
          close(noise);
          diff = (long)buf.st_size - bufFiles[i].prevsize;
          printf("%d ", FACTOR*diff);
          bufFiles[i].prevsize = buf.st_size;
        }
        
  	    pthread_mutex_unlock(&noise_mutex);
        if (connectionNo>0)
            printf("\n");
  	    sleep(1);
        period++;
  }

}
