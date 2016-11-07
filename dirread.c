#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>
#include <stdio.h>

typedef unsigned char byte;

int main(int argc, char **argv) {
    int fd,wfd,rd;
    
    int HEIGHT=atoi(argv[1]); //3280
    int WIDTH=atoi(argv[2]);  //2464
    int BYTES_PER_PIXEL=atoi(argv[3]); //3
    int SIZE=BYTES_PER_PIXEL*WIDTH*HEIGHT;
    
    byte buffer[SIZE];
    memset(buffer,0,SIZE);
    fd=open("/dev/video0",O_RDONLY);
    wfd=open("image.raw",O_RDWR|O_CREAT|O_APPEND,S_IRWXU);
    
    if(fd==-1)
        printf("open");
        
    
    rd=read(fd,buffer,SIZE);
    write(wfd,buffer,rd);
    
    close(fd);
    close(wfd);
    return 0;
}
