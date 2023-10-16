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
#include "../Helpers/adminCredentials.h"
#include "../Helpers/readWriteHelper.h"
#include "../Helpers/writeLock.h"
#include "../Helpers/releaseLock.h"

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param loginType Tracks whether the user is admin, student, or faculty.
* @param *reqStudent If loginType is student then finds the student and stores
*                    its data in this object for further use.
* @param *reqFaculty If loginType is faculty then finds the faculty and stores
*                    its data in this object for further use.
*
* Handles the login functionality for every user and also stores it in their
* respective objects for use in their respective controllers. Also set the 
* users online if login is successful.
*/
bool loginHandler(int clientConnectionFD, int loginType, struct Student *reqStudent, struct Faculty *reqFaculty) {
    char readBuf[1000], writeBuf[1000], tempBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Welcome Messages
    if(loginType == 1) strcpy(writeBuf, WELCOME_STUDENT);
    else if(loginType == 2) strcpy(writeBuf, WELCOME_PROFESSOR);
    else strcpy(writeBuf, WELCOME_ADMIN);

    // Take login id from client
    strcat(writeBuf, LOGIN_ID_MESSAGE);

    writeBytes = write(clientConnectionFD, writeBuf, strlen(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error writing the welcome message to client !!");
        return false;
    }

    readBytes = read(clientConnectionFD, readBuf, sizeof(readBuf));
    if(readBytes == -1) {
        perror("!! Error reading login ID from client !!");
        return false;
    }
    

    // Take password from client
    bzero(tempBuf, sizeof(tempBuf));
    strcpy(tempBuf, readBuf);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "*");
    write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error writing the Password message to client !!");
        return false;
    }

    bzero(readBuf, sizeof(readBuf));
    readBytes = read(clientConnectionFD, readBuf, sizeof(readBuf));
    if(readBytes == -1) {
        perror("!! Error reading password from client !!");
        return false;
    }

    // Checks the loginType to handle the respective users
    if(loginType == 1) {
        // Student Login
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
            // Compares roll numbers to find the student
            if(strcmp(student.sRollNo, tempBuf) == 0) {
                // If student is blocked cannot login
                if(student.active == 0) {
                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, BLOCKED);
                    write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror(ERROR_WRITING_TO_CLIENT);
                    }
                    release_lock(studentFD);
                    close(studentFD);
                    return false;
                }
                // if student is online cannot login again
                if(student.online == 1) {
                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, ALREADY_LOGGED_IN);
                    write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror(ERROR_WRITING_TO_CLIENT);
                    }
                    release_lock(studentFD);
                    close(studentFD);
                    return false;
                }
                // Checks if password is correct if yes then set that student online and writes it data in respective object.
                if(strcmp(student.sPassword, readBuf) == 0) {
                    student.online = 1;
                    if(strcmp(student.sPassword, DEFAULT_PASS) == 0) {
                        bzero(writeBuf, sizeof(writeBuf));
                        strcpy(writeBuf, REQ_DEFAULT_TO_NEW_PASS);
                        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return false;
                        strcpy(student.sPassword, readBuf);
                    }

                    lseek(studentFD, -1*sizeof(student), SEEK_CUR);
                    writeBytes = write(studentFD, &student, sizeof(student));
                    if(writeBytes == -1) {
                        perror(ERROR_WRITING_STUDENT_DB);
                        close(studentFD);
                        return false;
                    }
                    release_lock(studentFD);

                    reqStudent->sId = student.sId;
                    reqStudent->active = student.active;
                    reqStudent->online = student.online;
                    reqStudent->sAge = student.sAge;
                    strcpy(reqStudent->sRollNo, student.sRollNo);
                    strcpy(reqStudent->sAddress, student.sAddress);
                    strcpy(reqStudent->sName, student.sName);
                    strcpy(reqStudent->sPassword, student.sPassword);

                    close(studentFD);
                    return true;
                }
                release_lock(studentFD);
                close(studentFD);
                return false;
            }
        }
        release_lock(studentFD);
        close(studentFD);
        return false;
    } else if(loginType == 2) {
        // Faculty login
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
            // Compares login id to find the faculty
            if(strcmp(faculty.fLogin, tempBuf) == 0) {
                // If faculty is blocked cannot login
                if(faculty.active == 0) {
                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, BLOCKED);
                    write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror(ERROR_WRITING_TO_CLIENT);
                    }
                    release_lock(facultyFD);
                    close(facultyFD);
                    return false;
                }

                // If faculty is online cannot login
                if(faculty.online == 1) {
                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, ALREADY_LOGGED_IN);
                    write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror(ERROR_WRITING_TO_CLIENT);
                    }
                    release_lock(facultyFD);
                    close(facultyFD);
                    return false;
                }

                // Checks if password is correct if yes then set that faculty online and writes it data in respective object.
                if(strcmp(faculty.fPassword, readBuf) == 0) {
                    faculty.online = 1;
                    if(strcmp(faculty.fPassword, DEFAULT_PASS) == 0) {
                        bzero(writeBuf, sizeof(writeBuf));
                        strcpy(writeBuf, REQ_DEFAULT_TO_NEW_PASS);
                        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return false;
                        strcpy(faculty.fPassword, readBuf);
                    }

                    lseek(facultyFD, -1*sizeof(faculty), SEEK_CUR);
                    writeBytes = write(facultyFD, &faculty, sizeof(faculty));
                    if(writeBytes == -1) {
                        perror(ERROR_WRITING_FACULTY_DB);
                        close(facultyFD);
                        return false;
                    }
                    release_lock(facultyFD);
                    
                    reqFaculty->fId = faculty.fId;
                    reqFaculty->active = faculty.active;
                    reqFaculty->online = faculty.online;
                    strcpy(reqFaculty->fDepartment, faculty.fDepartment);
                    strcpy(reqFaculty->fLogin, faculty.fLogin);
                    strcpy(reqFaculty->fAddress, faculty.fAddress);
                    strcpy(reqFaculty->fName, faculty.fName);
                    strcpy(reqFaculty->fPassword, faculty.fPassword);

                    close(facultyFD);
                    return true;
                }
                release_lock(facultyFD);
                close(facultyFD);
                return false;
            }
        }
        release_lock(facultyFD);
        close(facultyFD);
        return false;
    } else {
        // Admin login
        if(strcmp(tempBuf, SUPERID) == 0 && strcmp(readBuf, SUPERPASSWORD) == 0) {
            return true;
        }
        else return false;
    }

    return false;
}

#endif