#ifndef _FILE_H_
#define _FILE_H_

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

off_t getFileSize(char* filename);
bool recieveFile(char* filename, int socket);
bool sendFile(char* filename, int socket);

#endif