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
#include "../Helpers/writeLock.h"
#include "../Helpers/releaseLock.h"

/*
* @param loginType Tracks whether the user is admin, student, or faculty.
* @param *reqStudent If loginType is student then use the reqStudent
*                    to logout the student and set it to offline
* @param *reqFaculty If loginType is faculty then use the reqFaculty
*                    to logout the faculty and set it to offline
*
* Handles the logout functionality for every user. Also set the 
* users offline if logout is successful.
*/
bool logoutHandler(int loginType, struct Student *reqStudent, struct Faculty *reqFaculty) {

    char readBuf[1000], writeBuf[1000], tempBuf[1000];
    ssize_t readBytes, writeBytes;

    if(loginType == 1) {
        // Student logout
        // Open database
        char databaseFile[50];
        strcpy(databaseFile, DATABASE_PATH);
        strcat(databaseFile, STUDENT_DATABASE);

        int studentFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
        if(studentFD == -1) {
            perror(ERROR_OPEN_STUDENT);
            return false;
        }

        acquire_write_lock(studentFD);

        struct Student student;
        while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
            // Compares the student id in database to find the student
            if(student.sId == reqStudent->sId) {
                student.online = 0;
                lseek(studentFD, -1*sizeof(student), SEEK_CUR);
                writeBytes = write(studentFD, &student, sizeof(student));
                if(writeBytes == -1) {
                    perror(ERROR_WRITING_STUDENT_DB);
                    release_lock(studentFD);
                    close(studentFD);
                    return false;
                }
                release_lock(studentFD);
                close(studentFD);
                return true;
            }
        }
        release_lock(studentFD);
        close(studentFD);
        return false;
    } else if(loginType == 2) {
        // Faculty logout
        // Open database
        char databaseFile[50];
        strcpy(databaseFile, DATABASE_PATH);
        strcat(databaseFile, FACULTY_DATABASE);

        int facultyFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
        if(facultyFD == -1) {
            perror(ERROR_OPEN_FACULTY);
            return false;
        }
        acquire_write_lock(facultyFD);
        struct Faculty faculty;
        while((readBytes = read(facultyFD, &faculty, sizeof(faculty))) != 0) {
            // Compares the faculty id in database to find the faculty
            if(faculty.fId == reqFaculty->fId) {
                faculty.online = 0;
                lseek(facultyFD, -1*sizeof(faculty), SEEK_CUR);
                writeBytes = write(facultyFD, &faculty, sizeof(faculty));
                if(writeBytes == -1) {
                    perror(ERROR_WRITING_FACULTY_DB);
                    release_lock(facultyFD);
                    close(facultyFD);
                    return false;
                }
                release_lock(facultyFD);
                close(facultyFD);
                return true;
            }
        }
        release_lock(facultyFD);
        close(facultyFD);
        return false;
    } 

    return false;
}

#endif