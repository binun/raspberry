#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>

#define BUF_SIZE 10
#define SERVPATH "ud_ucase"
#define CLIPATH "ud_client"

struct sockaddr_un svaddr, claddr;
int sfd, j;
size_t msgLen;
ssize_t numBytes;
char resp[BUF_SIZE];


sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
if (sfd == -1)
    errExit("socket");

memset(&claddr, 0, sizeof(struct sockaddr_un));
claddr.sun_family = AF_UNIX;
strncpy(claddr.sun_path,  SV_SOCK_PATH, (long) getpid());

if (bind(sfd, (struct sockaddr *) &claddr, sizeof(struct sockaddr_un)) == -1)
        errExit("bind");

    /* Construct address of server */

 memset(&svaddr, 0, sizeof(struct sockaddr_un));
 svaddr.sun_family = AF_UNIX;
 strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    /* Send messages to server; echo responses on stdout */

    for (j = 1; j < argc; j++) {
        msgLen = strlen(argv[j]);       /* May be longer than BUF_SIZE */
        if (sendto(sfd, argv[j], msgLen, 0, (struct sockaddr *) &svaddr,
                sizeof(struct sockaddr_un)) != msgLen)
            fatal("sendto");

        numBytes = recvfrom(sfd, resp, BUF_SIZE, 0, NULL, NULL);
        /* Or equivalently: numBytes = recv(sfd, resp, BUF_SIZE, 0);
                        or: numBytes = read(sfd, resp, BUF_SIZE); */
        if (numBytes == -1)
            errExit("recvfrom");
        printf("Response %d: %.*s\n", j, (int) numBytes, resp);
    }

    remove(claddr.sun_path);            /* Remove client socket pathname */
    exit(EXIT_SUCCESS);
