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
#include "../Models/track_struct.h"
#include "../Models/course_struct.h"
#include "../Helpers/constantStrings.h"
#include "../Helpers/loginHelper.h"
#include "../Helpers/logoutHelper.h"
#include "../Helpers/readWriteHelper.h"

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

void addCourse(int clientConnectionFD, struct Faculty *reqFaculty) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    struct Course newCourse;

    strcpy(writeBuf, "Enter the course name: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    strcpy(newCourse.cName, readBuf);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Enter the department: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    strcpy(newCourse.cDepartment, readBuf);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Enter the total seats: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    newCourse.cTotalSeats = atoi(readBuf);
    
    newCourse.cCurrentAvailableSeats = newCourse.cTotalSeats;

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Enter the credits: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    newCourse.cCredits = atoi(readBuf);

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
    lseek(trackFD, 2*sizeof(track), SEEK_SET);
    readBytes = read(trackFD, &track, sizeof(track));
    if(strcmp(track.name, COURSE) != 0) return;
    
    newCourse.cId = track.uid;
    track.uid++;

    newCourse.fId = reqFaculty->fId;
    newCourse.active = 1;
    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, "course_%d.txt", newCourse.cId);
    strcpy(newCourse.databasePath, writeBuf);

    lseek(trackFD, 2*sizeof(track), SEEK_SET);
    write(trackFD, &track, sizeof(track));

    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDWR | O_APPEND, 0777);
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

    writeBytes = write(courseFD, &newCourse, sizeof(newCourse));
    if(writeBytes == -1) {
        perror("!! Error while writing course to databasse !!");
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing to client !!");
            return;
        }
        close(trackFD);
        close(courseFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, "# Course Added Successfully with id: %d\n", newCourse.cId);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while reporting the response !!");
        close(trackFD);
        close(courseFD);
        return;
    }

    char newDatabaseFile[50];
    strcpy(newDatabaseFile, "./database/");
    strcat(newDatabaseFile, newCourse.databasePath);
    int newCourseFD = open(newDatabaseFile, O_CREAT | O_RDWR, 0777);
    if(newCourseFD == -1) {
        perror("!! Error while creating new course database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
        }
        close(trackFD);
        close(courseFD);
        return;
    }

    close(trackFD);
    close(courseFD);
    close(newCourseFD);
}

void viewAllFacultyCourses(int clientConnectionFD, struct Faculty *reqFaculty) {
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
        if(course.fId == reqFaculty->fId && course.active == 1) {
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

void removeCourse(int clientConnectionFD, struct Faculty *reqFaculty) {
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
        if(course.cId == atoi(readBuf) && course.fId == reqFaculty->fId && course.active == 1) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, "# Couldn't find Course with ID: %d under current faculty profile\n", cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the course details to client !!");
            close(courseFD);
            return;
        }
        close(courseFD);
        return;
    }

    course.active = 0;
    lseek(courseFD, -1*sizeof(course), SEEK_CUR);
    writeBytes = write(courseFD, &course, sizeof(course));
    if(writeBytes == -1) {
        perror("!! Error while writing the course details to database !!");
        close(courseFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "# Successfully deleted the course\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the response to client !!");
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "./database/");
    strcat(writeBuf, course.databasePath);
    unlink(writeBuf);

    close(courseFD);
    return;
}

void updateCourseDetail(int clientConnectionFD, struct Faculty *reqFaculty) {
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
        if(course.cId == atoi(readBuf) && course.active == 1 && course.fId == reqFaculty->fId) break;
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

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the Name of course? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new name: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        strcpy(course.cName, readBuf);
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the department of course? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new department: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        strcpy(course.cDepartment, readBuf);
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the course credits? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new credits: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        course.cCredits = atoi(readBuf);
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the total number of seats? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new total number of seats: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        int newTotalSeats = atoi(readBuf);
        if(newTotalSeats <= 0) {
            bzero(writeBuf, sizeof(writeBuf));
            strcpy(writeBuf, "# Total number of seats cannot be xero and in negative\n");
            writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
            if(writeBytes == -1) {
                perror("!! Error while sending the response to client !!");
            }
            close(courseFD);
            return;
        }

        if(newTotalSeats >= course.cTotalSeats) {
            int newlyAdded = newTotalSeats - course.cTotalSeats;
            course.cTotalSeats = newTotalSeats;
            course.cCurrentAvailableSeats += newlyAdded;
        } else {
            int newlyRemoved = course.cTotalSeats - newTotalSeats ;
            course.cTotalSeats = newTotalSeats;
            if(course.cCurrentAvailableSeats >= newlyRemoved) {
                course.cCurrentAvailableSeats -= newlyRemoved;
            } else {
                char courseDatabaseFile[50];
                newlyRemoved -= course.cCurrentAvailableSeats;
                course.cCurrentAvailableSeats = 0;

                strcpy(courseDatabaseFile, "./database/");
                strcat(courseDatabaseFile, course.databasePath);
                int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDWR, 0777);
                if(courseDbFD == -1) {
                    perror("!! Error while opening course's database file !!");

                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, "&");

                    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror("!! Error while writing logout message to client !!");
                    }
                    close(courseFD);
                    return;
                }
                struct Enroll enroll;
                lseek(courseDbFD, -1*sizeof(enroll), SEEK_END);

                bzero(writeBuf, sizeof(writeBuf));
                strcpy(writeBuf, "# List of removed student to adjust seats. \n\n");
                while(newlyRemoved > 0) {
                    read(courseDbFD, &enroll, sizeof(enroll));
                    if(enroll.sid > 0 && enroll.active == 1) {
                        bzero(tempBuf, sizeof(tempBuf));
                        sprintf(tempBuf, "Student id: %d\n", enroll.sid);

                        enroll.sid = -1;
                        enroll.active = false;
                        newlyRemoved--;
                        lseek(courseDbFD, -1*sizeof(enroll), SEEK_CUR);
                        write(courseDbFD, &enroll, sizeof(enroll));

                        strcat(writeBuf, tempBuf);
                    }
                    lseek(courseDbFD, -2*sizeof(enroll), SEEK_CUR);
                }

                writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                if(writeBytes == -1) {
                    perror("!! Error while writing response to client !!");
                }

                close(courseDbFD);
            }
        }
    }

    lseek(courseFD, -1*sizeof(course), SEEK_CUR);
    writeBytes = write(courseFD, &course, sizeof(course));
    if(writeBytes == -1) {
        perror("!! Error while writing the course details to database !!");
        close(courseFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "# Successfully modified the course\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the response to client !!");
    }

    close(courseFD);
    return;
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
                    viewAllFacultyCourses(clientConnectionFD, &faculty);
                    break;
                case 2:
                    addCourse(clientConnectionFD, &faculty);
                    break;
                case 3:
                    removeCourse(clientConnectionFD, &faculty);
                    break;
                case 4:
                    updateCourseDetail(clientConnectionFD, &faculty);
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