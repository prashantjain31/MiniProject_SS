#ifndef LOGIN_HELPER
#define LOGIN_HELPER

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "../Models/student_struct.h"
#include "../Models/faculty_struct.h"
#include "../Models/course_struct.h"
#include "../Helpers/constantStrings.h"

bool loginHandler(int connectionFD, int loginType) {
    char readBuf[1000], writeBuf[1000], tempBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Get login message for respective user type
    if(loginType == 1) strcpy(writeBuf, WELCOME_STUDENT);
    else if(loginType == 2) strcpy(writeBuf, WELCOME_PROFESSOR);
    else strcpy(writeBuf, WELCOME_ADMIN);

    strcat(writeBuf, LOGIN_ID_MESSAGE);

    writeBytes = write(connectionFD, writeBuf, strlen(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error writing the welcome message to client !!");
        return false;
    }

    readBytes = read(connectionFD, readBuf, sizeof(readBuf));
    if(readBytes == -1) {
        perror("!! Error reading login ID from client !!");
        return false;
    }
    
    bzero(tempBuf, sizeof(tempBuf));
    strcpy(tempBuf, readBuf);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "*");
    write(connectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error writing the Password message to client !!");
        return false;
    }

    bzero(readBuf, sizeof(readBuf));
    readBytes = read(connectionFD, readBuf, sizeof(readBuf));
    if(readBytes == -1) {
        perror("!! Error reading password from client !!");
        return false;
    }

    if(loginType == 1) {
        printf("Student id: %s and password: %s\n", tempBuf, readBuf);
    } else if(loginType == 2) {
        printf("Professor id: %s and password: %s\n", tempBuf, readBuf);
    } else {
        printf("Admin id: %s and password: %s\n", tempBuf, readBuf);
    }

    return true;
}

#endif