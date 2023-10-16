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
#include "../Helpers/readLock.h"
#include "../Helpers/writeLock.h"
#include "../Helpers/releaseLock.h"

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqStudent An pointer to the current logged in student's structure
*                    So that its data can be accessed easily
*
* Controlls the change password functionality for the student 
*/
bool changeStudentPassword(int clientConnectionFD, struct Student *reqStudent) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    // Opens student database to store the new changed password
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
        if(strcmp(student.sRollNo, reqStudent->sRollNo) == 0) {
            
            bzero(writeBuf, sizeof(writeBuf));
            strcpy(writeBuf, REQ_NEW_PASSWORD);
            if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return false;
            strcpy(student.sPassword, readBuf);

            lseek(studentFD, -1*sizeof(student), SEEK_CUR);
            writeBytes = write(studentFD, &student, sizeof(student));
            if(writeBytes == -1) {
                perror(ERROR_WRITING_STUDENT_DB);
                release_lock(studentFD);
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
    }
    release_lock(studentFD);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, UNABLE_UPDATE_PASSWORD);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }

    close(studentFD);
    return false;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqStudent An pointer to the current logged in student's structure
*                    So that its data can be accessed easily
*
* Enroll the studnet into a course. 
*/
void enrollCourse(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Request client for the course id 
    int cid;
    strcpy(writeBuf, REQ_COURSE_ID);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    // Open the course database file to know whether that course exists or not
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
    acquire_read_lock(courseFD);

    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_COURSE_GLOBAL, cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
        }
        release_lock(courseFD);
        close(courseFD);
        return;
    }

    // If available seats in course is zero cannot enroll
    if(course.cCurrentAvailableSeats <= 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, NO_SEATS_AVAILABLE);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
        }
        release_lock(courseFD);
        close(courseFD);
        return;
    }

    // If seats are available then opens the course's file to enroll the student into it
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
    acquire_write_lock(courseDbFD);

    struct Enroll enrolled;
    while((readBytes = read(courseDbFD, &enrolled, sizeof(enrolled))) != 0) {
        if(enrolled.sid == reqStudent->sId && enrolled.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    // Enrolls the student into the course if not enrolled previously
    if(readBytes == 0) {    

        struct Enroll newEnroll;
        newEnroll.active = 1;
        newEnroll.sid = reqStudent->sId;
        writeBytes = write(courseDbFD, &newEnroll, sizeof(newEnroll));
        if(writeBytes == -1) {
            perror(ERROR_ENROLLING_STUDENT);
            release_lock(courseDbFD);
            release_lock(courseFD);
            close(courseFD);
            close(courseDbFD);
            return;
        }

        course.cCurrentAvailableSeats-=1;

        lseek(courseFD, -1*sizeof(course), SEEK_CUR);
        writeBytes = write(courseFD, &course, sizeof(course));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_COURSE_DB);
            release_lock(courseDbFD);
            release_lock(courseFD);
            close(courseFD);
            close(courseDbFD);
            return;
        }
        release_lock(courseDbFD);
        release_lock(courseFD);

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
    release_lock(courseDbFD);
    release_lock(courseFD);

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

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqStudent An pointer to the current logged in student's structure
*                    So that its data can be accessed easily
*
* View All available courses into system 
*/
void viewAllCourses(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Opens the course database file if course is active then shows that to client
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
    acquire_read_lock(courseFD);

    strcpy(writeBuf, COURSE_LIST_HEADING);
    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.active == 1) {
            bzero(tempBuf, sizeof(tempBuf));
            sprintf(tempBuf, COURSE_DETAILS_PRINT, 
                course.cId, course.cName, course.cTotalSeats, course.cCurrentAvailableSeats, course.cDepartment, course.cCredits);
            strcat(writeBuf, tempBuf);
        }
    }
    release_lock(courseFD);

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_TO_CLIENT);
    }

    close(courseFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqStudent An pointer to the current logged in student's structure
*                    So that its data can be accessed easily
*
* Allows student to drop a course 
*/
void dropCourse(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Request the student the course id to de-enroll
    int cid;
    strcpy(writeBuf, REQ_COURSE_ID);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    // Open the course database file to check if that course exists
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
    acquire_write_lock(courseFD);

    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_COURSE_GLOBAL, cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
        }
        release_lock(courseFD);
        close(courseFD);
        return;
    }

    // If course exists then searches the studnet in course to de-enroll
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
        }
        release_lock(courseFD);
        return;
    }
    acquire_write_lock(courseDbFD);

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
            release_lock(courseDbFD);
            release_lock(courseFD);
            close(courseFD);
            close(courseDbFD);
            return;
        }

        course.cCurrentAvailableSeats+=1;

        lseek(courseFD, -1*sizeof(course), SEEK_CUR);
        writeBytes = write(courseFD, &course, sizeof(course));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_COURSE_DB);
            release_lock(courseDbFD);
            release_lock(courseFD);
            close(courseFD);
            close(courseDbFD);
            return;
        }
        release_lock(courseDbFD);
        release_lock(courseFD);

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
    }
    release_lock(courseDbFD);
    release_lock(courseFD);
    close(courseFD);
    close(courseDbFD);
}

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqStudent An pointer to the current logged in student's structure
*                    So that its data can be accessed easily
*
* View details of course in which the student is enrolled 
*/
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
    acquire_read_lock(courseFD);

    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {
        sprintf(writeBuf, UNABLE_FIND_COURSE_GLOBAL, cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
        }
        release_lock(courseFD);
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
        }
        release_lock(courseFD);
        return;
    }
    acquire_read_lock(courseDbFD);

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
        release_lock(courseDbFD);
        release_lock(courseFD);
        close(courseFD);
        close(courseDbFD);
        return;
    }
    release_lock(courseDbFD);
    release_lock(courseFD);

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

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqStudent An pointer to the current logged in student's structure
*                    So that its data can be accessed easily
*
* Shows all courses in which the student is enrolled 
*/
void viewAllEnrolledCourses(int clientConnectionFD, struct Student *reqStudent) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
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
    acquire_read_lock(courseFD);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, COURSE_LIST_HEADING);
    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.active == 0) continue;

        char courseDatabaseFile[50];
        strcpy(courseDatabaseFile, DATABASE_PATH);
        strcat(courseDatabaseFile, course.databasePath);
        int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDONLY, 0777);
        if(courseDbFD == -1) {
            perror(ERROR_OPEN_COURSE);
            continue;
        }
        acquire_read_lock(courseDbFD);

        struct Enroll enroll;
        while((readBytes = read(courseDbFD, &enroll, sizeof(enroll))) != 0) {
            if(enroll.sid == reqStudent->sId && enroll.active == 1) break;
        }
        if(readBytes > 0) {
            bzero(tempBuf, sizeof(tempBuf));
            sprintf(tempBuf, "Course Id: %d\n", course.cId);
            strcat(writeBuf, tempBuf);
        }
        release_lock(courseDbFD);
        close(courseDbFD);
    }
    release_lock(courseFD);

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }

    close(courseFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
* 
* Handles all the student functionalities 
*/
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