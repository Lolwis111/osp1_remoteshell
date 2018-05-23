#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>

#include "client.h"
#include "file.h"

#define PORT 9000
#define HOST "127.0.0.1"
#define BUFFER_SIZE 512

void inline CustomSleep()
{
    /* sleeps for 250ms */
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 250000000L; 
    nanosleep(&tim , &tim2);
}

/* removes trailing and leading whitespaces from the given string */
char *strtrim(char* str)
{
    /* from the beginning, skip all the whitespaces */
    while(isspace((unsigned char)*str))
    {
        str++;
    }

    /* if the string consists only of spaces we can end here */
    if(*str == 0)
    {
        return str;
    }

    /* now, starting from the end */
    char* end = str + strlen(str) - 1;
    /* skip all the trailing whitespaces */
    while(end > str && isspace((unsigned char)*end))
    {
        end--;
    }

    /* mark the first non-whitespace as end of string */
    *(end + 1) = 0;

    return str;
}


int main(int argc, char **argv)
{
    struct sockaddr_in sa;
    /* convert the ip-address string into a binary format */
    int res = inet_pton(AF_INET, HOST, &sa.sin_addr);
    if(res <= 0)
    {
        if(res == 0) 
        {
            fprintf(stderr, "'%s' is not a valid IPv4 address!", HOST);
        }
        else
        { 
            perror("inet_pton error!");
        }
        
        exit(EXIT_FAILURE);
    }

    /* get a socket from the operating system */
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    /* did that work? */
    if(-1 == socketFD) 
    {
        perror("Cannot create socket!");
        exit(EXIT_FAILURE);
    }
    
    /* default the connection */
    memset(&sa, 0, sizeof(sa));
    
    /* and set our parameters */
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    
    /* open up a connection */
    if(-1 == connect(socketFD, (struct sockaddr *)&sa, sizeof(sa)))
    {
        perror("Error while connecting!");
        close(socketFD);
        
        exit(EXIT_FAILURE);
    }
    
    /* wait a moment to allow for TCP magic */
    CustomSleep();

    ssize_t bytes;
    char buffer[BUFFER_SIZE];

    /* we start by reading to get the prompt */
    if((bytes = read(socketFD, buffer, BUFFER_SIZE-1)) > 0)
    {
        /* put a \0 byte at the end of the transmission
         * to avoid printing fragments of old buffers */
        buffer[bytes] = '\0';
        printf("%s", buffer);
    }

    while(1)
    {
        /* read a command from the user */
        fgets(buffer, BUFFER_SIZE, stdin);

        /* send the buffer to the server */
        if(write(socketFD, buffer, strlen(buffer) + 1) < 0)
        {
            fprintf(stderr, "%s\n", strerror(errno));
        }

        /* if we entered exit we can stop */
        if(strncmp(buffer, "exit", 4) == 0)
        {
            shutdown(socketFD, SHUT_RDWR);
            exit(EXIT_SUCCESS);
        }
        else if(strncmp(buffer, "put", 3) == 0)
        {
            if(!sendFile(strtrim(buffer+3), socketFD))
            {
                fputs("Error sending file!", stderr);
            }
        }
        else if(strncmp(buffer, "get", 3) == 0)
        {
            if(!recieveFile(strtrim(buffer+3), socketFD))
            {
                fputs("Error recieving file!", stderr);
            }
        }

        /* wait a moment to allow TCP to do some magic */
        CustomSleep();

        /* read back the response */
        do
        {
            if((bytes = read(socketFD, buffer, BUFFER_SIZE - 1)) > 0)
            {
                buffer[bytes] = 0x00;
                printf("%s", buffer);
            }
        }
        /* when the buffer wasnt fully used, 
         * it means everything is transmitted */
        while(bytes == BUFFER_SIZE - 1);
    }
    
    close(socketFD);
    
    return EXIT_SUCCESS;
}
