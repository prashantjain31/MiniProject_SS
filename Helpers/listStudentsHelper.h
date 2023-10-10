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

void listStudents(int clientConnectionFD, char *writeBuf, int writeSize) {
    ssize_t readBytes, writeBytes;
    char tempBuf[1000];

    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(studentFD == -1) {
        perror("!! Error while opening student database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    struct Student student;
    strcpy(writeBuf, "----- Student List -----\n");
    strcat(writeBuf, "Roll Number -> Name\n");
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        bzero(tempBuf, sizeof(tempBuf));
        sprintf(tempBuf, "%s -> %s\n", student.sRollNo, student.sName);
        strcat(writeBuf, tempBuf);
    }

    return;
}

#endif