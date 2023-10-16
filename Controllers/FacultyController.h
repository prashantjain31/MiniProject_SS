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
#include "../Helpers/readLock.h"
#include "../Helpers/writeLock.h"
#include "../Helpers/releaseLock.h"

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqFaculty An pointer to the current logged in faculty's structure
*                    So that its data can be accessed easily
*
* Handles the change faculty password functionality in the system
*/
bool changeFacultyPassword(int clientConnectionFD, struct Faculty *reqFaculty) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    // Opens the database to store the new password into the system
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
        if(strcmp(faculty.fLogin, reqFaculty->fLogin) == 0) {
            bzero(writeBuf, sizeof(writeBuf));
            strcpy(writeBuf, REQ_NEW_PASSWORD);
            if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return false;
            strcpy(faculty.fPassword, readBuf);

            lseek(facultyFD, -1*sizeof(faculty), SEEK_CUR);
            writeBytes = write(facultyFD, &faculty, sizeof(faculty));
            if(writeBytes == -1) {
                perror(ERROR_WRITING_FACULTY_DB);
                release_lock(facultyFD);
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
    }
    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, UNABLE_UPDATE_PASSWORD);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }
    release_lock(facultyFD);
    close(facultyFD);
    return false;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqFaculty An pointer to the current logged in faculty's structure
*                    So that its data can be accessed easily
*
* Add's a new course offered by the logged in faculty
*/
void addCourse(int clientConnectionFD, struct Faculty *reqFaculty) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Takes all the new course's information as input from the client
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

    // Opens the track file to generate the unique id for courses
    char trackFile[50];
    strcpy(trackFile, DATABASE_PATH);
    strcat(trackFile, TRACK_FILE);
    int trackFD = open(trackFile, O_CREAT | O_RDWR, 0777);
    if(trackFD == -1) {
        perror(ERROR_OPEN_TRACK);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }
    acquire_write_lock(trackFD);

    struct Track track;
    lseek(trackFD, 2*sizeof(track), SEEK_SET);
    readBytes = read(trackFD, &track, sizeof(track));
    if(strcmp(track.name, COURSE) != 0) return;
    
    newCourse.cId = track.uid;
    track.uid++;

    newCourse.fId = reqFaculty->fId;
    newCourse.active = 1;
    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, COURSE_PATH, newCourse.cId);
    strcpy(newCourse.databasePath, writeBuf);

    lseek(trackFD, 2*sizeof(track), SEEK_SET);
    write(trackFD, &track, sizeof(track));
    release_lock(trackFD);

    // Opens the database to store the new course information into the system and create its respective database file
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, COURSE_DATABASE);

    int courseFD = open(databaseFile, O_CREAT | O_RDWR | O_APPEND, 0777);
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

    writeBytes = write(courseFD, &newCourse, sizeof(newCourse));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_COURSE_DB);
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
            return;
        }
        release_lock(courseFD);
        close(trackFD);
        close(courseFD);
        return;
    }
    release_lock(courseFD);

    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, COURSE_ADDED, newCourse.cId);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
        close(trackFD);
        close(courseFD);
        return;
    }


    // New database file is created using unique id
    char newDatabaseFile[50];
    strcpy(newDatabaseFile, DATABASE_PATH);
    strcat(newDatabaseFile, newCourse.databasePath);
    int newCourseFD = open(newDatabaseFile, O_CREAT | O_RDWR, 0777);
    if(newCourseFD == -1) {
        perror(ERROR_CREATING_NEW_COURSE_DB);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
        }
        close(trackFD);
        close(courseFD);
        return;
    }

    close(trackFD);
    close(courseFD);
    close(newCourseFD);
}

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqFaculty An pointer to the current logged in faculty's structure
*                    So that its data can be accessed easily
*
* View all courses offered by the faculty 
*/
void viewAllFacultyCourses(int clientConnectionFD, struct Faculty *reqFaculty) {
    char tempBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Opens the course database to retrieve all the course details
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

    // If that course is offered by the currently logged in faculty then it is shown to user
    strcpy(writeBuf, COURSE_LIST_HEADING);
    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.fId == reqFaculty->fId && course.active == 1) {
            bzero(tempBuf, sizeof(tempBuf));
            sprintf(tempBuf, COURSE_DETAILS_PRINT
            , course.cId, course.cName, course.cTotalSeats, course.cCurrentAvailableSeats, course.cDepartment, course.cCredits);
            strcat(writeBuf, tempBuf);
        }
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
* @param *reqFaculty An pointer to the current logged in faculty's structure
*                    So that its data can be accessed easily
*
* Removes course from the database and also removes its file from the system 
*/
void removeCourse(int clientConnectionFD, struct Faculty *reqFaculty) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Takes the course's id as input from client
    int cid;
    strcpy(writeBuf, REQ_COURSE_ID);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    // Opens the database and try to retrieve the information from the database
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
        if(course.cId == atoi(readBuf) && course.fId == reqFaculty->fId && course.active == 1) break;
    }

    // If course is not found reports that to client
    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_COURSE, cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
        }
        release_lock(courseFD);
        close(courseFD);
        return;
    }

    // If course is found then sets it to deactivated.
    course.active = 0;
    lseek(courseFD, -1*sizeof(course), SEEK_CUR);
    writeBytes = write(courseFD, &course, sizeof(course));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_COURSE_DB);
        release_lock(courseFD);
        close(courseFD);
        return;
    }
    release_lock(courseFD);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, SUCCESS_DELETE_COURSE);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }

    // Removes the course's file from the database
    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, DATABASE_PATH);
    strcat(writeBuf, course.databasePath);
    unlink(writeBuf);

    close(courseFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
* @param *reqFaculty An pointer to the current logged in faculty's structure
*                    So that its data can be accessed easily
*
* Update the existing course's details in the system 
*/
void updateCourseDetail(int clientConnectionFD, struct Faculty *reqFaculty) {
    char tempBuf[1000], writeBuf[1000], readBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(tempBuf, sizeof(tempBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Requests the client for the course's id which they want to update
    int cid;
    strcpy(writeBuf, REQ_COURSE_ID);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    cid = atoi(readBuf);

    // Opens the course database to retrieve its information from the system
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

    // Finds the course and checks if its a course offered by same faculty which is logged in or not
    while((readBytes = read(courseFD, &course, sizeof(course))) != 0) {
        if(course.cId == atoi(readBuf) && course.active == 1 && course.fId == reqFaculty->fId) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_COURSE, cid);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
        }
        release_lock(courseFD);
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
            strcpy(writeBuf, "# Total number of seats cannot be zero and in negative\n");
            writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
            if(writeBytes == -1) {
                perror(ERROR_WRITING_RESPONSE);
            }
            release_lock(courseFD);
            close(courseFD);
            return;
        }

        // Checks if the client is increasing or decreasing the seats
        if(newTotalSeats >= course.cTotalSeats) {
            int newlyAdded = newTotalSeats - course.cTotalSeats;
            course.cTotalSeats = newTotalSeats;
            course.cCurrentAvailableSeats += newlyAdded;
        } else {
            // if client is decreasing the seats there might be a chance that we need to de-enroll the last few students from that course
            int newlyRemoved = course.cTotalSeats - newTotalSeats ;
            course.cTotalSeats = newTotalSeats;
            if(course.cCurrentAvailableSeats >= newlyRemoved) {
                course.cCurrentAvailableSeats -= newlyRemoved;
            } else {
                char courseDatabaseFile[50];
                newlyRemoved -= course.cCurrentAvailableSeats;
                course.cCurrentAvailableSeats = 0;

                strcpy(courseDatabaseFile, DATABASE_PATH);
                strcat(courseDatabaseFile, course.databasePath);
                int courseDbFD = open(courseDatabaseFile, O_CREAT | O_RDWR, 0777);
                if(courseDbFD == -1) {
                    perror(ERROR_OPEN_COURSE);

                    bzero(writeBuf, sizeof(writeBuf));
                    strcpy(writeBuf, "&");

                    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
                    if(writeBytes == -1) {
                        perror(ERROR_REPORTING_LOGOUT_MESSAGE);
                    }
                    close(courseFD);
                    return;
                }
                acquire_write_lock(courseDbFD);
                struct Enroll enroll;
                lseek(courseDbFD, -1*sizeof(enroll), SEEK_END);

                bzero(writeBuf, sizeof(writeBuf));
                strcpy(writeBuf, "\n# List of removed student to adjust seats. \n");
                while(newlyRemoved > 0) {
                    read(courseDbFD, &enroll, sizeof(enroll));
                    if(enroll.sid > 0 && enroll.active == 1) {
                        bzero(tempBuf, sizeof(tempBuf));
                        sprintf(tempBuf, "\nStudent id: %d\n", enroll.sid);

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
                    perror(ERROR_WRITING_RESPONSE);
                }
                release_lock(courseDbFD);
                close(courseDbFD);
            }
        }
    }

    lseek(courseFD, -1*sizeof(course), SEEK_CUR);
    writeBytes = write(courseFD, &course, sizeof(course));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_COURSE_DB);
        release_lock(courseFD);
        close(courseFD);
        return;
    }
    release_lock(courseFD);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "\n# Successfully modified the course\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_TO_CLIENT);
    }

    close(courseFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
* 
* Controlls all the faculty functionalities 
*/
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
                perror(ERROR_SENDING_FACULTY_CHOICE);
                return;
            }

            readBytes = read(clientConnectionFD, readBuf, sizeof(readBuf));
            if(readBytes == -1) {
                perror(ERROR_READING_FACULTY_CHOICE);
                return;
            } else if(readBytes == 0) {
                perror(NO_DATA_RECEIVED);
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