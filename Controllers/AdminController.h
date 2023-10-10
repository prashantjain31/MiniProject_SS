#ifndef ADMIN_CONTROLLER
#define ADMIN_CONTROLLER

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
#include "../Models/track_struct.h"
#include "../Helpers/constantStrings.h"
#include "../Helpers/loginHelper.h"
#include "../Helpers/readWriteHelper.h"

void addStudent(int clientConnectionFD) {

    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    struct Student newStudent;

    strcpy(writeBuf, "Enter the student name: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    strcpy(newStudent.sName, readBuf);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Enter the student address: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    strcpy(newStudent.sAddress, readBuf);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Enter the student age: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    newStudent.sAge = atoi(readBuf);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Enter the degree type(MT, IMT, MS etc): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    strcpy(newStudent.sRollNo, readBuf);

    char trackFile[50];
    strcpy(trackFile, "./database/");
    strcat(trackFile, TRACK_FILE);
    int trackFD = open(trackFile, O_CREAT | O_RDWR, 0777);
    if(trackFD == -1) {
        perror("!! Error while opening track database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    struct Track track;
    readBytes = read(trackFD, &track, sizeof(track));
    if(strcmp(track.name, STUDENT) != 0) return;
    newStudent.sId = track.uid;
    track.uid++;

    lseek(trackFD, 0, SEEK_SET);
    write(trackFD, &track, sizeof(track));

    bzero(readBuf, sizeof(readBuf));
    sprintf(readBuf, "%d", newStudent.sId);
    strcat(newStudent.sRollNo, readBuf);

    newStudent.active = true;

    strcpy(newStudent.sPassword, DEFAULT_PASS);

    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDWR | O_APPEND, 0777);
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

    writeBytes = write(studentFD, &newStudent, sizeof(newStudent));
    if(writeBytes == -1) {
        perror("!! Error while writing student to databasse !!");
        close(trackFD);
        close(studentFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, "Student Added Successfully with Roll Number: %s\n", newStudent.sRollNo);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while reporting the response !!");
    }

    close(trackFD);
    close(studentFD);
    return;
}

void viewStudent(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    strcpy(writeBuf, "Enter the student roll number: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char rollnumber[20];
    strcpy(rollnumber, readBuf);
    
    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDWR | O_APPEND, 0777);
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
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        if(strcmp(student.sRollNo, rollnumber) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, "Couldn't find Student with Roll Number: %s\n", rollnumber);
    } else {
        sprintf(writeBuf, "----- Student Details -----\nName: %s\nAddress: %s\nAge: %d\nRoll Number: %s\nActive Status(1 = Active and 0 = Blocked): %d", 
            student.sName, student.sAddress, student.sAge, student.sRollNo, student.active);
    }

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the student details to client !!");
        close(studentFD);
        return;
    }

    close(studentFD);
    return;
}

void activateStudent(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    strcpy(writeBuf, "Enter the student roll number: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char rollnumber[20];
    strcpy(rollnumber, readBuf);
    
    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
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
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        if(strcmp(student.sRollNo, rollnumber) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, "Couldn't find Student with Roll Number: %s\n", rollnumber);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the student details to client !!");
            close(studentFD);
            return;
        }
        close(studentFD);
        return;
    }

    student.active = 1;
    lseek(studentFD, -1*sizeof(student), SEEK_CUR);
    writeBytes = write(studentFD, &student, sizeof(student));
    if(writeBytes == -1) {
        perror("!! Error while writing the student details to database !!");
        close(studentFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Successfully activated the student access\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the student details to client !!");
    }

    close(studentFD);
    return;
}

void blockStudent(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    strcpy(writeBuf, "Enter the student roll number: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char rollnumber[20];
    strcpy(rollnumber, readBuf);
    
    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
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
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        if(strcmp(student.sRollNo, rollnumber) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, "Couldn't find Student with Roll Number: %s\n", rollnumber);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the student details to client !!");
            close(studentFD);
            return;
        }
        close(studentFD);
        return;
    }

    student.active = 0;
    lseek(studentFD, -1*sizeof(student), SEEK_CUR);
    writeBytes = write(studentFD, &student, sizeof(student));
    if(writeBytes == -1) {
        perror("!! Error while writing the student details to database !!");
        close(studentFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Successfully blocked the student access\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the student details to client !!");
    }

    close(studentFD);
    return;
}

void modifyStudent(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    
}

void rootAdminController(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    if(loginHandler(clientConnectionFD, 3)) {
        strcpy(writeBuf, SUCCESS_LOGIN);
        strcat(writeBuf, ADMINPAGE);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the Admin choice Page !!");
            return;
        }

        readBytes = read(clientConnectionFD, readBuf, sizeof(readBuf));
        if(readBytes == -1) {
            perror("!! Error while reading the Admin's choice !!");
            return;
        } else if(readBytes == 0) {
            perror("No data received from the Admin side");
            return;
        }
        int adminChoice = atoi(readBuf);
        switch (adminChoice) {
            case 1:
                addStudent(clientConnectionFD);
                break;
            case 2:
                viewStudent(clientConnectionFD);
                break;
            case 3:
                // addFaculty();
                break;
            case 4:
                // viewFaculty();
                break;
            case 5:
                activateStudent(clientConnectionFD);
                break;
            case 6:
                blockStudent(clientConnectionFD);
                break;
            case 7:
                modifyStudent(clientConnectionFD);
                break;
            case 8:
                // modifyFaculty();
                break;
            case 9: // Logout
            default:
                // Wrong Choice
                write(clientConnectionFD, SUCCESS_LOGOUT, sizeof(SUCCESS_LOGOUT));
                return;
        }

    } else {
        strcpy(writeBuf, FAILED_LOGIN);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    }
}

#endif