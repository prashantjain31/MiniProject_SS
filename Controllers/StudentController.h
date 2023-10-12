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

void enrollCourse(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    int cid;
    strcpy(writeBuf, "Enter the course id: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    struct Course course;
    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(courseFD == -1) {
        perror("!! Error while opening course database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, "# Couldn't find Course with ID: %d\n", cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the response to client !!");
            close(courseFD);
            return;
        }
        close(courseFD);
        return;
    }

    if(course.cCurrentAvailableSeats <= 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "# No Seats available in course.\n");
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the response to client !!");
            close(courseFD);
            return;
        }
        close(courseFD);
        return;
    }

    char courseDatabaseFile[50];
    strcpy(courseDatabaseFile, "./database/");
    strcat(courseDatabaseFile, course.databasePath);

    int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDWR | O_APPEND, 0777);
    if(courseDbFD == -1) {
        perror("!! Error while opening course database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
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
            perror("!! Error while enrolling the student to course !!");
            close(courseFD);
            close(courseDbFD);
            return;
        }

        course.cCurrentAvailableSeats-=1;

        lseek(courseFD, -1*sizeof(course), SEEK_CUR);
        writeBytes = write(courseFD, &course, sizeof(course));
        if(writeBytes == -1) {
            perror("!! Error while writing the course details to database !!");
            close(courseFD);
            close(courseDbFD);
            return;
        }

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "# Successfully enrolled in the course\n");
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the response to client !!");
        }

        close(courseFD);
        close(courseDbFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "# Already enrolled in the course.\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the response to client !!");
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
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(courseFD == -1) {
        perror("!! Error while opening course database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    strcpy(writeBuf, "# ----- Course List -----\n");
    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.active == 1) {
            bzero(tempBuf, sizeof(tempBuf));
            sprintf(tempBuf, "Course id: %d\nName: %s\nTotal Seats: %d\nAvailable Seats: %d\nDepartment: %s\nCredits: %d\n\n"
            , course.cId, course.cName, course.cTotalSeats, course.cCurrentAvailableSeats, course.cDepartment, course.cCredits);
            strcat(writeBuf, tempBuf);
        }
    }

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while writing response to client !!");
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
    strcpy(writeBuf, "Enter the course id: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    struct Course course;
    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(courseFD == -1) {
        perror("!! Error while opening course database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, "# Couldn't find Course with ID: %d\n", cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the response to client !!");
            close(courseFD);
            return;
        }
        close(courseFD);
        return;
    }

    char courseDatabaseFile[50];
    strcpy(courseDatabaseFile, "./database/");
    strcat(courseDatabaseFile, course.databasePath);

    int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDWR , 0777);
    if(courseDbFD == -1) {
        perror("!! Error while opening course database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
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
            perror("!! Error while droping course !!");
            close(courseFD);
            close(courseDbFD);
            return;
        }

        course.cCurrentAvailableSeats+=1;

        lseek(courseFD, -1*sizeof(course), SEEK_CUR);
        writeBytes = write(courseFD, &course, sizeof(course));
        if(writeBytes == -1) {
            perror("!! Error while writing the course details to database !!");
            close(courseFD);
            close(courseDbFD);
            return;
        }

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "# Successfully dropped the course\n");
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the response to client !!");
        }

        close(courseFD);
        close(courseDbFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "# Student not present in the course.\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the response to client !!");
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
    strcpy(writeBuf, "Enter the course id: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    struct Course course;
    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(courseFD == -1) {
        perror("!! Error while opening course database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, "# Couldn't find Course with ID: %d\n", cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the response to client !!");
            close(courseFD);
            return;
        }
        close(courseFD);
        return;
    }

    char courseDatabaseFile[50];
    strcpy(courseDatabaseFile, "./database/");
    strcat(courseDatabaseFile, course.databasePath);

    int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDONLY , 0777);
    if(courseDbFD == -1) {
        perror("!! Error while opening course database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
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
        strcpy(writeBuf, "# Student is not enrolled in the course\n");
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the response to client !!");
        }

        close(courseFD);
        close(courseDbFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, "\n# Course Details\nCourse id: %d\nName: %s\nTotal Seats: %d\nAvailable Seats: %d\nDepartment: %s\nCredits: %d\n\n"
            , course.cId, course.cName, course.cTotalSeats, course.cCurrentAvailableSeats, course.cDepartment, course.cCredits);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the response to client !!");
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
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(courseFD == -1) {
        perror("!! Error while opening course database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "# List of Courses in which you are enrolled \n\n");
    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.active == 0) continue;

        char courseDatabaseFile[50];
        strcpy(courseDatabaseFile, "./database/");
        strcat(courseDatabaseFile, course.databasePath);
        int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDWR, 0777);
        if(courseDbFD == -1) {
            perror("!! Error while opening course's database file !!");
            continue;
        }

        struct Enroll enroll;
        while((readBytes = read(courseDbFD, &enroll, sizeof(enroll))) != 0) {
            if(enroll.sid == reqStudent->sId && enroll.active == 1) break;
        }
        if(readBytes > 0) {
            bzero(tempBuf, sizeof(tempBuf));
            sprintf(tempBuf, "Course Id: %d\n\n", course.cId);
            strcat(writeBuf, tempBuf);
        }
        close(courseDbFD);
    }

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the response to client !!");
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