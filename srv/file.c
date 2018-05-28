#include "file.h"

off_t getFileSize(char* filename)
{
    struct stat st;
    if (stat(filename, &st) == 0)
    {
        return st.st_size;
    }

    return -1;
}

bool recieveFile(char* filename, int socket)
{
    /* read the filesize */
    size_t length, ret;
    if(read(socket, &length, sizeof(size_t)) < 0)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return false;
    }

    /* calculate how many segments we should recieve */
    size_t segments = length / 512;
    size_t rest = length % 512;

    /* create/override the file */
    FILE* output;
    if((output = fopen(filename, "w+")) == NULL)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return false;
    }

    char buffer[512];

    for(size_t i = 0; i < segments; i++)
    {
        /* read a 512 byte segment */
        if(read(socket, buffer, 512) < 0)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            return false;
        }

        /* write a 512 byte segment */
        ret = fwrite(buffer, 512, 1, output);
        if(ret != 1 || ferror(output) != 0)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            return false;
        }
    }

    /* read the rest block */
    if(read(socket, buffer, rest) < 0)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return false;
    }

    /* write the rest block */
    ret = fwrite(buffer, rest, 1, output);
    if(ret != 1 || ferror(output) != 0)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return false;
    }

    fflush(output);
    fclose(output);

    return true;
}

bool sendFile(char* filename, int socket)
{
    off_t size = getFileSize(filename);
    size_t ret;
    if(size == -1)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return false;
    }

    /* get the filesize */
    size_t length = (size_t)size;

    /* send the filesize */
    if(write(STDOUT_FILENO, &length, sizeof(size_t)) < 0)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return false;
    }

    /* open the file */
    FILE* input;
    if((input = fopen(filename, "r")) == NULL)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return false;
    }

    /* we send the file in blocks of 512 */
    size_t segments = length / 512;
    /* which means, there is a non 512-byte block of size rest */
    size_t rest = length % 512;
    char buffer[512];

    for(size_t i = 0; i < segments; i++)
    {
        /* read a 512 byte block */
        ret = fread(buffer, 512, 1, input);
        if(ret != 1 || ferror(input) != 0)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            return false;
        }

        /* and send it */
        if(write(STDOUT_FILENO, buffer, 512) < 0)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            return false;
        }
    }

    /* read the rest block */
    ret = fread(buffer, rest, 1, input);
    if(ret != 1 || ferror(input) != 0)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return false;
    }
    
    /* send the rest block */
    if(write(STDOUT_FILENO, buffer, rest) < 0)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return false;
    }

    fclose(input);

    return true;
}