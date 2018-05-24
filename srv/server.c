#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>

#include "shell.h"

#define PORT 9001

int _socket = -1;

void ctrlCHandler(int sig)
{
    if(sig == SIGINT && _socket != -1)
    {
        shutdown(_socket, SHUT_RDWR);
        close(_socket);

        exit(EXIT_SUCCESS);
    }
}

int main()
{
    signal(SIGINT, ctrlCHandler);

    /* this struct describes the connection */
	struct sockaddr_in sa = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    
    /* get a socket from the operating system */
    int socketFD = _socket = socket(AF_INET, SOCK_STREAM, 0);
    
    /* did that work? */
    if(-1 == socketFD) 
    {
        perror("Cannot create socket!");
        exit(EXIT_FAILURE);
    }
    
    /* bind the socket to a port */
    if(-1 == bind(socketFD, (struct sockaddr*)&sa, sizeof(sa)))
    {
        perror("Cannot bind socket!");
        close(socketFD);
        
        exit(EXIT_FAILURE);
    }
    
    /* start listening for connections */
    if(-1 == listen(socketFD, 1))
    {
        // perror("Cannot listen on socket!");
        close(socketFD);
        
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        /* accept an incomming transmission */
        int connectFD = accept(socketFD, NULL, NULL);
        
        /* did that work? */
        if(connectFD < 0)
        {
            // perror("Error while accepting connection!");
                
            close(socketFD);
                
            exit(EXIT_FAILURE);
        }

        pid_t child;
        /* fork into the shell so the server can accept more connections */
        if((child = fork()) == 0)
        {
            int debug = dup(STDERR_FILENO);
            /* map the socket onto the output streams */
            dup2(connectFD, STDOUT_FILENO);
            dup2(connectFD, STDERR_FILENO);

            /* disable buffering */
            setbuf(stdout, NULL);
            setbuf(stderr, NULL);

            /* execute the shell */
            shell(connectFD, debug);

            /* close the connection */
            shutdown(connectFD, SHUT_RDWR);
            close(connectFD);
        }
        else
        {
            close(connectFD);
        }
    }

	return 0;
}
