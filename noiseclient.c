
#include <stdio.h> 
#include <string.h>    
#include <sys/socket.h>   
#include <arpa/inet.h> 
#include <memory.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
char server_reply[CLIENTBUF] = "";

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server,client;
    int addr_size = sizeof(server);
     
    sock = socket(AF_INET , SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket\n");
    }
    
    printf("Socket to connect to %s\n",argv[1] );
    memset((char *) &server, 0, sizeof(server));
            
   
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons( SERVPORT ); 
 
  
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("connect failed. Error\n");
        return 1;
    }
     
    printf("Noise Client Connected\n");
 
    while(1)
    {      
		char message[10] = ASK;
        
        memset(server_reply,0,CLIENTBUF);
        
        int n = recv(sock , server_reply , CLIENTBUF , 0);
        if( n <= 0)
            printf("recv failed\n");
        
        else
          printf("%d bytes obtained \n", n); 
         
         memset(server_reply,0,CLIENTBUF);
    
    }
     
    close(sock);
    return 0;
}
