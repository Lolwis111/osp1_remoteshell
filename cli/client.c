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
#include <signal.h>

#include <pthread.h>

#include "client.h"
#include "file.h"

#define PORT 9000
#define HOST "127.0.0.1"
#define BUFFER_SIZE 512

bool _fileTransferMode = false;

void* readingThread(void* arg)
{
    int socket = *((int*)arg);
    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    while(1)
    {
        /* file transmissions have to be synchronized
         * to avoid printing file data onto the terminal */ 
        if(!_fileTransferMode)
        {
            /* read back the response */
            do
            {
                if((bytes = read(socket, buffer, BUFFER_SIZE - 1)) > 0)
                {
                    buffer[bytes] = 0x00;
                    printf("%s", buffer);
                }
            }
            /* when the buffer wasnt fully used, 
            * it means everything is transmitted */
            while(bytes == BUFFER_SIZE - 1);
        }
        else
        {
            CustomSleep(250);
        }
    }

    return NULL;
}

void inline CustomSleep(int ms)
{
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = ms * 1000000L; /* convert milliseconds into nanseconds */
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
    CustomSleep(250);
    
    /* create a thread to read asynchronus all the time */
    pthread_t thread;
    int result;
    if((result = pthread_create(&thread, NULL, readingThread, (void*)&socketFD)) != 0)
    {
        fprintf(stderr, "Could not spawn thread: %d\n", result);
        shutdown(socketFD, SHUT_RDWR);
        close(socketFD);
        exit(EXIT_FAILURE);
    }

    CustomSleep(250);

    char buffer[BUFFER_SIZE];

    while(1)
    {
        /* reset the buffer */
        memset(&buffer, 0, BUFFER_SIZE);
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
            /* on exit, the client can end the connection and exit*/
            shutdown(socketFD, SHUT_RDWR);
            close(socketFD);
            exit(EXIT_SUCCESS);
        }
        else if(strncmp(buffer, "put", 3) == 0)
        {
            /* disable the thread for a moment */
            _fileTransferMode = true;
            /* send the file */
            if(!sendFile(strtrim(buffer+3), socketFD))
            {
                fputs("Error sending file!", stderr);
            }
            /* reenable the thread */
            _fileTransferMode = false;
        }
        else if(strncmp(buffer, "get", 3) == 0)
        {
            _fileTransferMode = true;
            if(!recieveFile(strtrim(buffer+3), socketFD))
            {
                fputs("Error recieving file!", stderr);
            }
            _fileTransferMode = false;
        }

        /* wait a moment to allow TCP to do some magic */
        CustomSleep(250);
    }
    
    close(socketFD);
    
    return EXIT_SUCCESS;
}
