/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
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
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket\n");
    }
    
    printf("Socket to connect to %s\n",argv[1] );
    memset((char *) &server, 0, sizeof(server));
            
    //if (inet_aton(argv[1] , &server.sin_addr) == 0) 
    //{
        //printf("inet_aton() failed\n");
        //return 1;
    //}
    
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons( SERVPORT ); 
 
  
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("connect failed. Error\n");
        return 1;
    }
     
    printf("Noise Client Connected\n");
     
    //keep communicating with server
    while(1)
    {      
		char message[10] = ASK;
        
        //sendto(sock , message, strlen(message) , 0,(struct sockaddr *)&server,addr_size);
        send(sock , message, strlen(message) , 0);
        memset(server_reply,0,CLIENTBUF);
        //Receive a reply from the server
        //int n = recvfrom(sock , server_reply , CLIENTBUF , 0,(struct sockaddr *) &server, &addr_size);
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
