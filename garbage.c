byte pickedNoise[BUFSIZE]="";
int numPipes=0;
long curPipeSize=0;
pipeDef pipes[32];

 ftstamp = fopen(TSTAMP,"rb");
        if (ftstamp==NULL)
           fptr=0;
        else {     
          fread((void*)&fptr,sizeof(long),1,ftstamp);
          fclose(ftstamp);
	    }
        
        
        //pthread_mutex_lock(&noise_mutex);
        fnoise = fopen(NOISEFILE, "rb");
        
        fseek(fnoise, 0L, SEEK_END);
        fsize = ftell(fnoise);
        
        send_size = MIN(fsize-fptr,CLIENTBUF);
        fseek(fnoise,fptr,SEEK_SET);
        printf("Noisesize %ld sendsize %ld curoffset %ld\n", fsize,send_size,fptr);
        fread(server_reply,1,send_size,fnoise);
        fptr = MAX(fptr + send_size,fsize);
        fclose(fnoise);
        
        ftstamp = fopen(TSTAMP,"wb");
        fwrite((void*)&fptr,sizeof(long),1,ftstamp); 
        fclose(ftstamp);

typedef struct tagUnixPipe {
	int socket;
	struct sockaddr_un address;
} pipeDef;

  /*pthread_mutex_lock(&client_mutex);
	   
       if (curPipeSize >=BUFSIZE)
         pthread_mutex_unlock(&client_mutex);
       else {
		   int diff = MIN(read_size, BUFSIZE-curPipeSize);
		   int clientHub = conn->udpconn;
	       curPipeSize+=diff;
		   pthread_mutex_unlock(&client_mutex);
		   send(clientHub, binbuffer, diff, 0);
	   }
       
  	   
       //printf("%d bytes were read\n", read_size);    
       pthread_mutex_lock(&noise_mutex);
       
       /*if (fpos+read_size>BUFSIZE) {
		   read_size=BUFSIZE-fpos;
		   fwrite(binbuffer, sizeof(byte), read_size, f);
		   fseek(f, 0, SEEK_SET );
		   fpos=0;
	   }
	   else {       
           fwrite(binbuffer, sizeof(byte), read_size, f);
           fpos = fpos + read_size;
	   }*/

/*void *hub_handler(void *connection)
{    
    int n,s, s2, t, len;
    struct sockaddr_un address,from;
    pthread_t hubthread;

    socklen_t address_length,from_length;	

    if ((s = socket(AF_UNIX, sockmode, 0)) == -1) {
        perror("server socket failed\n");
        exit(1);
    }
    
    unlink(SOCK_PATH);

  
    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCK_PATH);
    address_length = sizeof(address.sun_family) + strlen(address.sun_path);
    if(bind(s, (struct sockaddr *) &address, address_length) != 0)
     {
      printf("bind() failed\n");
      return NULL;
     }
    
     while (1) {
		int decrease=0;
		memset(pickedNoise, 0, BUFSIZE);
        n=recvfrom(s,pickedNoise,BUFSIZE,0,(struct sockaddr *)&from,&from_length);
        
        //pthread_mutex_lock(&client_mutex);
        //curPipeSize = MAX(curPipeSize-n,0);		
  	    //pthread_mutex_unlock(&client_mutex);
        
        printf("         Received %d bytes\n",n);
	}
}*/

/*int initClient(void)
{
	int s;
    struct sockaddr_un address;
  
	s = socket(AF_UNIX, sockmode, 0);

    if (s<0) 
    {
        printf("socket failed");
        return -1;
    }

    printf("Trying to connect...\n");

    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCK_PATH);
    addresslen = strlen(address.sun_path) + sizeof(address.sun_family);
    if(connect(s, (struct sockaddr *) &address, addresslen) != 0)
     {
       printf("connect() failed\n");
       return -1;
     }

    pipes[numPipes].socket = s;
    memmove(&(pipes[numPipes].address), &address, sizeof(struct sockaddr_un));
    
    int r = numPipes;
    numPipes++;
    printf("Connected to %d slot %d\n",s,r);
    return r;
}

  sleep(3);
    
    ch1 = initClient();
    pthread_create(&simthread1, NULL, sim_handler, (void*)&ch1);
    
    ch2 = initClient();
    pthread_create(&simthread2, NULL, sim_handler, (void*)&ch2);
    */
    
    
  void *client_handler(void*arg)
{
  	
  int i = *((int *)arg);
  int loop=0;
  pipeDef *p = pipes+i;
  int s = p->socket;

  int sent;
  struct sockaddr_un address;
  memset(&address,0,sizeof(struct sockaddr_un));
  memmove(&address, &(p->address),sizeof(struct sockaddr_un));
  int toSend = 1024*100; //read_size
  
  for ( loop=0;;loop++) 
	{
	  toSend  =  MIN(toSend,BUFSIZE-curPipeSize);
	  	
	  /*int actual_send = MIN(toSend, BUFSIZE-curPipeSize);
      if (actual_send <=0) {
		   printf("Dropped\n");
		   continue;
	   }
      
	   sent=sendto(s,binbuffer,actual_send,0,(struct sockaddr *)&address, addresslen);  
	   pthread_mutex_unlock(&client_mutex);
	   curPipeSize+=MAX(0, sent);
	   pthread_mutex_unlock(&client_mutex);
	   */
		   
	   sent=sendto(s,binbuffer,toSend,0,(struct sockaddr *)&address, addresslen);   
	   
	   sleep(1);
	   printf("%d sent %d bytes loop %d\n",i, sent,loop);
	   }
   }
   
   /*void *split_handler(void *connection)
{    
  int i=0;
  FILE *ftstamp= NULL,*fnoise = NULL;
  for (;;) {
           
        long fsize = 0,fptr=0;
        long bottom_line = 0;
        pthread_mutex_lock(&noise_mutex);
        for (i=0; i < connectionNo+1; i++) 
        {		       
          char noiseName[64] = "";
          char recent_noiseName[64] = "";
          
          if (i<connectionNo)
            sprintf(noiseName, "noise%d.bin", i + 1);
          else
            strcpy(noiseName, "noise.bin");
          
		  fnoise = fopen(NOISEFILE, "rb");
        
          fseek(fnoise, 0L, SEEK_END);
          fsize = ftell(fnoise);
          fclose(fnoise);
          
          bottom_line = MIN(fptr,(long)(MAXFILESIZE / DISCARD_RATIO));
          bottom_line = (long)(bottom_line / 1048576);
          printf(command, "dd if=%s of=chunk ibs=1M obs=1M skip=%ld status=none && mv chunk %s", noiseName, bottom_line,noiseName);
	      system(command); 
        }
             
  	    pthread_mutex_unlock(&noise_mutex);
  }
}*/

