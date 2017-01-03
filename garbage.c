byte pickedNoise[BUFSIZE]="";
int numPipes=0;
long curPipeSize=0;
pipeDef pipes[32];


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
