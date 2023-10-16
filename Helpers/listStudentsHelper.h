#ifndef LIST_STUDENT_HELPER
#define LIST_STUDENT_HELPER

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
#include "../Models/student_struct.h"
#include "./readLock.h"
#include "./releaseLock.h"

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *writeBuf A buffer pointer for writing data
* @param writeSize Size of writeBuffer
*
* Creates list of all students present in system in format Roll Number -> Name
*/
void listStudents(int clientConnectionFD, char *writeBuf, int writeSize) {
    ssize_t readBytes, writeBytes;
    char tempBuf[1000];

    // Opening database
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(studentFD == -1) {
        perror(ERROR_OPEN_STUDENT);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }
    acquire_read_lock(studentFD);

    // Reading database and creating the student list and storing the data into the writeBuffer
    struct Student student;
    strcpy(writeBuf, STUDENT_LIST_HEADING);
    strcat(writeBuf, "\nRoll Number -> Name\n");
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        bzero(tempBuf, sizeof(tempBuf));
        sprintf(tempBuf, "%s -> %s\n", student.sRollNo, student.sName);
        strcat(writeBuf, tempBuf);
    }
    release_lock(studentFD);
    close(studentFD);
    return;
}

#endif