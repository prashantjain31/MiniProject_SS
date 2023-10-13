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
        perror(ERROR_WRITING_TO_CLIENT);
        return false;
    }

    readBytes = read(clientConnectionFD, readBuf, readSize);
    if(readBytes == -1) {
        perror(ERROR_READING_FROM_CLIENT);
        return false;
    } else if(readBytes == 0) {
        perror(NO_DATA_RECEIVED);
        return false;
    } 

    return true;
}

#endif