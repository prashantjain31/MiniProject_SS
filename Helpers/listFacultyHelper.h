#ifndef LIST_FACULTY_HELPER
#define LIST_FACULTY_HELPER

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "./constantStrings.h"
#include "../Models/faculty_struct.h"

void listFaculty(int clientConnectionFD, char *writeBuf, int writeSize) {
    ssize_t readBytes, writeBytes;
    char tempBuf[1000];

    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, FACULTY_DATABASE);

    int facultyFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(facultyFD == -1) {
        perror("!! Error while opening faculty database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    struct Faculty faculty;
    strcpy(writeBuf, "----- Faculty List -----\n");
    strcat(writeBuf, "Login ID -> Name\n");
    while((readBytes = read(facultyFD, &faculty, sizeof(faculty))) != 0) {
        bzero(tempBuf, sizeof(tempBuf));
        sprintf(tempBuf, "%s -> %s\n", faculty.fLogin, faculty.fName);
        strcat(writeBuf, tempBuf);
    }

    return;
}

#endif