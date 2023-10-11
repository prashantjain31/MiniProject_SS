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

bool loginHandler(int clientConnectionFD, int loginType, struct Student *reqStudent, struct Faculty *reqFaculty) {
    char readBuf[1000], writeBuf[1000], tempBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    if(loginType == 1) strcpy(writeBuf, WELCOME_STUDENT);
    else if(loginType == 2) strcpy(writeBuf, WELCOME_PROFESSOR);
    else strcpy(writeBuf, WELCOME_ADMIN);

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
            if(strcmp(student.sRollNo, tempBuf) == 0) {
                if(student.active == 0) {
                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, "# ");
                    strcat(writeBuf, BLOCKED);
                    write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror("!! Error writing the message to client !!");
                        return false;
                    }
                    return false;
                }
                if(student.online == 1) {
                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, "# ");
                    strcat(writeBuf, ALREADY_LOGGED_IN);
                    write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror("!! Error writing the message to client !!");
                        return false;
                    }
                    return false;
                }
                if(strcmp(student.sPassword, readBuf) == 0) {
                    student.online = 1;
                    if(strcmp(student.sPassword, DEFAULT_PASS) == 0) {
                        bzero(writeBuf, sizeof(writeBuf));
                        strcpy(writeBuf, "$ Enter the new password (because your current password is default password): ");
                        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return false;
                        strcpy(student.sPassword, readBuf);
                    }

                    lseek(studentFD, -1*sizeof(student), SEEK_CUR);
                    writeBytes = write(studentFD, &student, sizeof(student));
                    if(writeBytes == -1) {
                        perror("!! Error while writing the student details to database !!");
                        close(studentFD);
                        return false;
                    }
                    
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
                close(studentFD);
                return false;
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
            if(strcmp(faculty.fLogin, tempBuf) == 0) {
                if(faculty.active == 0) {
                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, "# ");
                    strcat(writeBuf, BLOCKED);
                    write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror("!! Error writing the message to client !!");
                        return false;
                    }
                    return false;
                }
                if(faculty.online == 1) {
                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, "# ");
                    strcat(writeBuf, ALREADY_LOGGED_IN);
                    write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror("!! Error writing the message to client !!");
                        return false;
                    }
                    return false;
                }
                if(strcmp(faculty.fPassword, readBuf) == 0) {
                    faculty.online = 1;
                    if(strcmp(faculty.fPassword, DEFAULT_PASS) == 0) {
                        bzero(writeBuf, sizeof(writeBuf));
                        strcpy(writeBuf, "$ Enter the new password (because your current password is default password): ");
                        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return false;
                        strcpy(faculty.fPassword, readBuf);
                    }

                    lseek(facultyFD, -1*sizeof(faculty), SEEK_CUR);
                    writeBytes = write(facultyFD, &faculty, sizeof(faculty));
                    if(writeBytes == -1) {
                        perror("!! Error while writing the faculty details to database !!");
                        close(facultyFD);
                        return false;
                    }
                    
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
                close(facultyFD);
                return false;
            }
        }
        close(facultyFD);
        return false;
    } else {
        if(strcmp(tempBuf, SUPERID) == 0 && strcmp(readBuf, SUPERPASSWORD) == 0) {
            return true;
        }
        else return false;
    }

    return true;
}

#endif