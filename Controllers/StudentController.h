#ifndef STUDENT_CONTROLLER
#define STUDENT_CONTROLLER

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
#include "../Helpers/logoutHelper.h"

bool changeStudentPassword(int clientConnectionFD, struct Student *reqStudent) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

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
        if(strcmp(student.sRollNo, reqStudent->sRollNo) == 0) {
            
            bzero(writeBuf, sizeof(writeBuf));
            strcpy(writeBuf, "$ Enter the new password: ");
            if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return false;
            strcpy(student.sPassword, readBuf);

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
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "# Failed to update the password\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the student details to client !!");
    }

    close(studentFD);
    return false;
}

void rootStudentController(int clientConnectionFD) {
    
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));
    
    struct Student student;
    if(loginHandler(clientConnectionFD, 1, &student, NULL)) {
        strcpy(writeBuf, SUCCESS_LOGIN);
        int studentChoice;
        do {
            strcat(writeBuf, STUDENTYPAGE);
            writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
            if(writeBytes == -1) {
                perror("!! Error while sending the Student choice Page !!");
                return;
            }

            readBytes = read(clientConnectionFD, readBuf, sizeof(readBuf));
            if(readBytes == -1) {
                perror("!! Error while reading the Student's choice !!");
                return;
            } else if(readBytes == 0) {
                perror("No data received from the Student side");
                return;
            }
            studentChoice = atoi(readBuf);
            switch (studentChoice) {
                case 1:
                    // viewAllCourses(clientConnectionFD);
                    break;
                case 2:
                    // enrollCourse(clientConnectionFD);
                    break;
                case 3:
                    // dropCourse(clientConnectionFD);
                    break;
                case 4:
                    // viewEnrolledCourseDetail(clientConnectionFD);
                    break;
                case 5:
                    changeStudentPassword(clientConnectionFD, &student);
                    break;
                case 6: // Logout
                default:
                    // Wrong Choice
                    logoutHandler(1, &student, NULL);
                    write(clientConnectionFD, SUCCESS_LOGOUT, sizeof(SUCCESS_LOGOUT));
                    return;
            }
            bzero(writeBuf, sizeof(writeBuf));
        } while(studentChoice > 0 && studentChoice < 6);

    } else {
        strcpy(writeBuf, FAILED_LOGIN);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    }
}

#endif