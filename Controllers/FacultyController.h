#ifndef FACULTY_CONTROLLER
#define FACULTY_CONTROLLER

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
#include "../Helpers/constantStrings.h"
#include "../Helpers/loginHelper.h"

void rootFacultyController(int clientConnectionFD) {

    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    if(loginHandler(clientConnectionFD, 2)) {
        strcpy(writeBuf, SUCCESS_LOGIN);
        strcat(writeBuf, FACULTYPAGE);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    } else {
        strcpy(writeBuf, FAILED_LOGIN);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    }
}

#endif