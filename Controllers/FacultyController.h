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

#include "../Models/faculty_struct.h"
#include "../Helpers/constantStrings.h"
#include "../Helpers/loginHelper.h"
#include "../Helpers/logoutHelper.h"

bool changeFacultyPassword(int clientConnectionFD, struct Faculty *reqFaculty) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

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
        if(strcmp(faculty.fLogin, reqFaculty->fLogin) == 0) {
            bzero(writeBuf, sizeof(writeBuf));
            strcpy(writeBuf, "$ Enter the new password: ");
            if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return false;
            strcpy(faculty.fPassword, readBuf);

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
    }
    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "# Failed to update the password\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the faculty details to client !!");
    }

    close(facultyFD);
    return false;
}

void rootFacultyController(int clientConnectionFD) {

    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));
    
    struct Faculty faculty;
    if(loginHandler(clientConnectionFD, 2, NULL, &faculty)) {
        strcpy(writeBuf, SUCCESS_LOGIN);
        int facultyChoice;
        do {
            strcat(writeBuf, FACULTYPAGE);
            writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
            if(writeBytes == -1) {
                perror("!! Error while sending the Faculty choice Page !!");
                return;
            }

            readBytes = read(clientConnectionFD, readBuf, sizeof(readBuf));
            if(readBytes == -1) {
                perror("!! Error while reading the Faculty's choice !!");
                return;
            } else if(readBytes == 0) {
                perror("No data received from the Faculty side");
                return;
            }
            facultyChoice = atoi(readBuf);
            switch (facultyChoice) {
                case 1:
                    // viewAllCourses(clientConnectionFD);
                    break;
                case 2:
                    // addCourse(clientConnectionFD);
                    break;
                case 3:
                    // removeCourse(clientConnectionFD);
                    break;
                case 4:
                    // updateCourseDetail(clientConnectionFD);
                    break;
                case 5:
                    changeFacultyPassword(clientConnectionFD, &faculty);
                    break;
                case 6: // Logout
                default:
                    // Wrong Choice
                    logoutHandler(2, NULL, &faculty);
                    write(clientConnectionFD, SUCCESS_LOGOUT, sizeof(SUCCESS_LOGOUT));
                    return;
            }
            bzero(writeBuf, sizeof(writeBuf));
        } while(facultyChoice > 0 && facultyChoice < 6);

    } else {
        strcpy(writeBuf, FAILED_LOGIN);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    }
}

#endif