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
#include "./readLock.h"
#include "./releaseLock.h"

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *writeBuf A buffer pointer for writing data
* @param writeSize Size of writeBuffer
*
* Creates list of all faculty present in system in format Login ID -> Name
*/
void listFaculty(int clientConnectionFD, char *writeBuf, int writeSize) {
    ssize_t readBytes, writeBytes;
    char tempBuf[1000];

    // Opening database
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, FACULTY_DATABASE);

    int facultyFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(facultyFD == -1) {
        perror(ERROR_OPEN_FACULTY);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }
    acquire_read_lock(facultyFD);

    // Reading database and creating the faculty list and storing the data into the writeBuffer
    struct Faculty faculty;
    strcpy(writeBuf, FACULTY_LIST_HEADING);
    strcat(writeBuf, "Login ID -> Name\n");
    while((readBytes = read(facultyFD, &faculty, sizeof(faculty))) != 0) {
        bzero(tempBuf, sizeof(tempBuf));
        sprintf(tempBuf, "%s -> %s\n", faculty.fLogin, faculty.fName);
        strcat(writeBuf, tempBuf);
    }
    release_lock(facultyFD);
    close(facultyFD);
    return;
}

#endif