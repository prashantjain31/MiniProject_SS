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
#include "../Models/enroll_struct.h"
#include "../Models/course_struct.h"
#include "../Helpers/constantStrings.h"
#include "../Helpers/loginHelper.h"
#include "../Helpers/logoutHelper.h"

bool changeStudentPassword(int clientConnectionFD, struct Student *reqStudent) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(studentFD == -1) {
        perror(ERROR_OPEN_STUDENT);
        return false;
    }

    struct Student student;
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        if(strcmp(student.sRollNo, reqStudent->sRollNo) == 0) {
            
            bzero(writeBuf, sizeof(writeBuf));
            strcpy(writeBuf, REQ_NEW_PASSWORD);
            if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return false;
            strcpy(student.sPassword, readBuf);

            lseek(studentFD, -1*sizeof(student), SEEK_CUR);
            writeBytes = write(studentFD, &student, sizeof(student));
            if(writeBytes == -1) {
                perror(ERROR_WRITING_STUDENT_DB);
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
    strcpy(writeBuf, UNABLE_UPDATE_PASSWORD);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }

    close(studentFD);
    return false;
}

void enrollCourse(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    int cid;
    strcpy(writeBuf, REQ_COURSE_ID);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    struct Course course;
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(courseFD == -1) {
        perror(ERROR_OPEN_COURSE);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }

    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_COURSE_GLOBAL, cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
            close(courseFD);
            return;
        }
        close(courseFD);
        return;
    }

    if(course.cCurrentAvailableSeats <= 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, NO_SEATS_AVAILABLE);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
            close(courseFD);
            return;
        }
        close(courseFD);
        return;
    }

    char courseDatabaseFile[50];
    strcpy(courseDatabaseFile, DATABASE_PATH);
    strcat(courseDatabaseFile, course.databasePath);

    int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDWR | O_APPEND, 0777);
    if(courseDbFD == -1) {
        perror(ERROR_OPEN_COURSE);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }

    struct Enroll enrolled;
    while((readBytes = read(courseDbFD, &enrolled, sizeof(enrolled))) != 0) {
        if(enrolled.sid == reqStudent->sId && enrolled.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    

        struct Enroll newEnroll;
        newEnroll.active = 1;
        newEnroll.sid = reqStudent->sId;
        writeBytes = write(courseDbFD, &newEnroll, sizeof(newEnroll));
        if(writeBytes == -1) {
            perror(ERROR_ENROLLING_STUDENT);
            close(courseFD);
            close(courseDbFD);
            return;
        }

        course.cCurrentAvailableSeats-=1;

        lseek(courseFD, -1*sizeof(course), SEEK_CUR);
        writeBytes = write(courseFD, &course, sizeof(course));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_COURSE_DB);
            close(courseFD);
            close(courseDbFD);
            return;
        }

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "\n# Successfully enrolled in the course\n");
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
        }

        close(courseFD);
        close(courseDbFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "\n# Already enrolled in the course.\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
        close(courseDbFD);
        close(courseFD);
        return;
    }
    close(courseFD);
    close(courseDbFD);
}

void viewAllCourses(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    struct Course course;
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(courseFD == -1) {
        perror(ERROR_OPEN_COURSE);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }

    strcpy(writeBuf, COURSE_LIST_HEADING);
    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.active == 1) {
            bzero(tempBuf, sizeof(tempBuf));
            sprintf(tempBuf, COURSE_DETAILS_PRINT, 
                course.cId, course.cName, course.cTotalSeats, course.cCurrentAvailableSeats, course.cDepartment, course.cCredits);
            strcat(writeBuf, tempBuf);
        }
    }

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_TO_CLIENT);
    }

    close(courseFD);
    return;
}

void dropCourse(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    int cid;
    strcpy(writeBuf, REQ_COURSE_ID);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    struct Course course;
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(courseFD == -1) {
        perror(ERROR_OPEN_COURSE);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }

    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_COURSE_GLOBAL, cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
            close(courseFD);
            return;
        }
        close(courseFD);
        return;
    }

    char courseDatabaseFile[50];
    strcpy(courseDatabaseFile, DATABASE_PATH);
    strcat(courseDatabaseFile, course.databasePath);

    int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDWR , 0777);
    if(courseDbFD == -1) {
        perror(ERROR_OPEN_COURSE);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }

    struct Enroll enrolled;
    while((readBytes = read(courseDbFD, &enrolled, sizeof(enrolled))) != 0) {
        if(enrolled.sid == reqStudent->sId && enrolled.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes > 0) {    
        enrolled.sid = -1;
        enrolled.active = 0;
        lseek(courseDbFD, -1*sizeof(enrolled), SEEK_CUR);
        writeBytes = write(courseDbFD, &enrolled, sizeof(enrolled));
        if(writeBytes == -1) {
            perror(ERROR_DROP_COURSE);
            close(courseFD);
            close(courseDbFD);
            return;
        }

        course.cCurrentAvailableSeats+=1;

        lseek(courseFD, -1*sizeof(course), SEEK_CUR);
        writeBytes = write(courseFD, &course, sizeof(course));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_COURSE_DB);
            close(courseFD);
            close(courseDbFD);
            return;
        }

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "\n# Successfully dropped the course\n");
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
        }

        close(courseFD);
        close(courseDbFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "\n# Student not present in the course.\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_TO_CLIENT);
        close(courseDbFD);
        close(courseFD);
        return;
    }
    close(courseFD);
    close(courseDbFD);
}

void viewEnrolledCourseDetail(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    int cid;
    strcpy(writeBuf, REQ_COURSE_ID);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    struct Course course;
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(courseFD == -1) {
        perror(ERROR_OPEN_COURSE);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
            return;
        }
        return;
    }

    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_COURSE_GLOBAL, cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
            close(courseFD);
            return;
        }
        close(courseFD);
        return;
    }

    char courseDatabaseFile[50];
    strcpy(courseDatabaseFile, DATABASE_PATH);
    strcat(courseDatabaseFile, course.databasePath);

    int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDONLY , 0777);
    if(courseDbFD == -1) {
        perror(ERROR_OPEN_COURSE);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }

    struct Enroll enrolled;
    while((readBytes = read(courseDbFD, &enrolled, sizeof(enrolled))) != 0) {
        if(enrolled.sid == reqStudent->sId && enrolled.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "\n# Student is not enrolled in the course\n");
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
        }

        close(courseFD);
        close(courseDbFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, COURSE_DETAILS_PRINT
            , course.cId, course.cName, course.cTotalSeats, course.cCurrentAvailableSeats, course.cDepartment, course.cCredits);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
        close(courseDbFD);
        close(courseFD);
        return;
    }
    close(courseFD);
    close(courseDbFD);
}

void viewAllEnrolledCourses(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    struct Course course;
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(courseFD == -1) {
        perror(ERROR_OPEN_COURSE);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, COURSE_LIST_HEADING);
    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.active == 0) continue;

        char courseDatabaseFile[50];
        strcpy(courseDatabaseFile, DATABASE_PATH);
        strcat(courseDatabaseFile, course.databasePath);
        int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDWR, 0777);
        if(courseDbFD == -1) {
            perror(ERROR_OPEN_COURSE);
            continue;
        }

        struct Enroll enroll;
        while((readBytes = read(courseDbFD, &enroll, sizeof(enroll))) != 0) {
            if(enroll.sid == reqStudent->sId && enroll.active == 1) break;
        }
        if(readBytes > 0) {
            bzero(tempBuf, sizeof(tempBuf));
            sprintf(tempBuf, "Course Id: %d\n", course.cId);
            strcat(writeBuf, tempBuf);
        }
        close(courseDbFD);
    }

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }

    close(courseFD);
    return;
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
                perror(ERROR_SENDING_STUDENT_CHOICE);
                return;
            }

            readBytes = read(clientConnectionFD, readBuf, sizeof(readBuf));
            if(readBytes == -1) {
                perror(ERROR_READING_STUDENT_CHOICE);
                return;
            } else if(readBytes == 0) {
                perror(NO_DATA_RECEIVED);
                return;
            }
            studentChoice = atoi(readBuf);
            switch (studentChoice) {
                case 1:
                    viewAllCourses(clientConnectionFD, &student);
                    break;
                case 2:
                    enrollCourse(clientConnectionFD, &student);
                    break;
                case 3:
                    dropCourse(clientConnectionFD, &student);
                    break;
                case 4:
                    viewEnrolledCourseDetail(clientConnectionFD, &student);
                    break;
                case 5:
                    changeStudentPassword(clientConnectionFD, &student);
                    break;
                case 6:
                    viewAllEnrolledCourses(clientConnectionFD, &student);
                    break;
                case 7: // Logout
                default:
                    // Wrong Choice
                    logoutHandler(1, &student, NULL);
                    write(clientConnectionFD, SUCCESS_LOGOUT, sizeof(SUCCESS_LOGOUT));
                    return;
            }
            bzero(writeBuf, sizeof(writeBuf));
        } while(studentChoice > 0 && studentChoice < 7);

    } else {
        strcpy(writeBuf, FAILED_LOGIN);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    }
}

#endif