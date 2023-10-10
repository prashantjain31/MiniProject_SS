#ifndef READ_WRITE_HELPER
#define READ_WRITE_HELPER

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

bool readwrite(int clientConnectionFD, char *writeBuf, int writeSize, char *readBuf, int readSize) {
    ssize_t readBytes, writeBytes;

    bzero(readBuf, readSize);
    writeBytes = write(clientConnectionFD, writeBuf, writeSize);
    if(writeBytes == -1) {
        perror("!! Error while sending the request to client !!");
        return false;
    }

    readBytes = read(clientConnectionFD, readBuf, readSize);
    if(readBytes == -1) {
        perror("!! Error while reading the response of client !!");
        return false;
    } else if(readBytes == 0) {
        perror("No data received from the client side");
        return false;
    } 

    return true;
}

#endif