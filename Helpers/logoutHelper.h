#ifndef LOGOUT_HELPER
#define LOGOUT_HELPER

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
#include "../Helpers/adminCredentials.h"

bool logoutHandler(int loginType, struct Student *reqStudent, struct Faculty *reqFaculty) {

    char readBuf[1000], writeBuf[1000], tempBuf[1000];
    ssize_t readBytes, writeBytes;

    if(loginType == 1) {
        char databaseFile[50];
        strcpy(databaseFile, "./database/");
        strcat(databaseFile, STUDENT_DATABASE);

        int studentFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
        if(studentFD == -1) {
            perror("!! Error while opening student database file !!");
            return false;
        }

        struct Student student;
        while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
            if(student.sId == reqStudent->sId) {
                student.online = 0;
                lseek(studentFD, -1*sizeof(student), SEEK_CUR);
                writeBytes = write(studentFD, &student, sizeof(student));
                if(writeBytes == -1) {
                    perror("!! Error while writing the student details to database !!");
                    close(studentFD);
                    return false;
                }

                close(studentFD);
                return true;
            }
        }
        close(studentFD);
        return false;
    } else if(loginType == 2) {
        char databaseFile[50];
        strcpy(databaseFile, "./database/");
        strcat(databaseFile, FACULTY_DATABASE);

        int facultyFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
        if(facultyFD == -1) {
            perror("!! Error while opening faculty database file !!");
            return false;
        }

        struct Faculty faculty;
        while((readBytes = read(facultyFD, &faculty, sizeof(faculty))) != 0) {
            if(faculty.fId == reqFaculty->fId) {
                faculty.online = 0;
                lseek(facultyFD, -1*sizeof(faculty), SEEK_CUR);
                writeBytes = write(facultyFD, &faculty, sizeof(faculty));
                if(writeBytes == -1) {
                    perror("!! Error while writing the faculty details to database !!");
                    close(facultyFD);
                    return false;
                }

                close(facultyFD);
                return true;
            }
        }
        close(facultyFD);
        return false;
    } 

    return true;
}

#endif