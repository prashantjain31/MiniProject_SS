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
#include "../Models/faculty_struct.h"
#include "../Models/enroll_struct.h"
#include "../Models/track_struct.h"
#include "../Helpers/constantStrings.h"
#include "../Helpers/loginHelper.h"
#include "../Helpers/readWriteHelper.h"
#include "../Helpers/listStudentsHelper.h"
#include "../Helpers/listFacultyHelper.h"
#include "../Helpers/readLock.h"
#include "../Helpers/writeLock.h"
#include "../Helpers/releaseLock.h"

/*
* @param clientConnectionFD An file descriptor for the client connection
*
* Adds a new student in the database
*/
void addStudent(int clientConnectionFD) {

    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Takes the new students details as input and populates the structure. ids are generated automatically
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

    // Track file to generate the ids automatically
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
    newStudent.online = false;

    strcpy(newStudent.sPassword, DEFAULT_PASS);

    // Opens the database and stores the data
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDWR | O_APPEND, 0777);
    if(studentFD == -1) {
        perror(ERROR_OPEN_STUDENT);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            release_lock(trackFD);
            return;
        }
        release_lock(trackFD);
        return;
    }
    acquire_write_lock(studentFD);

    writeBytes = write(studentFD, &newStudent, sizeof(newStudent));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_STUDENT_DB);
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
        }
        release_lock(studentFD);
        release_lock(trackFD);
        close(trackFD);
        close(studentFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, STUDENT_ADDED, newStudent.sRollNo);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }
    release_lock(studentFD);
    release_lock(trackFD);
    close(trackFD);
    close(studentFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
*
* View student details in the database
*/
void viewStudent(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Lists all studnets so that admin can choose
    listStudents(clientConnectionFD, writeBuf, sizeof(writeBuf));

    // Request the id from client
    strcat(writeBuf, REQ_STUDENT_ROLLNO);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char rollnumber[20];
    strcpy(rollnumber, readBuf);
    
    // Opens the database and retrieves the student details andp rints it
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, STUDENT_DATABASE);
    
    int studentFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(studentFD == -1) {
        perror(ERROR_OPEN_STUDENT);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }
    acquire_read_lock(studentFD);

    struct Student student;
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        if(strcmp(student.sRollNo, rollnumber) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_STUDENT, rollnumber);
    } else {
        sprintf(writeBuf, STUDENT_DETAILS_PRINT, 
            student.sName, student.sAddress, student.sAge, student.sRollNo, student.active, student.online);
    }

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
        
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
            release_lock(studentFD);
            return;
        }
    }
    release_lock(studentFD);
    close(studentFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
*
* activate student account 
*/
void activateStudent(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Lists all studnets so that admin can choose
    listStudents(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, REQ_STUDENT_ROLLNO);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char rollnumber[20];
    strcpy(rollnumber, readBuf);
    
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, STUDENT_DATABASE);

    // Opens the student database and set the student active status to 1
    int studentFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(studentFD == -1) {
        perror(ERROR_WRITING_STUDENT_DB);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }
    acquire_write_lock(studentFD);

    struct Student student;
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        if(strcmp(student.sRollNo, rollnumber) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_STUDENT, rollnumber);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
        }
        release_lock(studentFD);
        close(studentFD);
        return;
    }

    student.active = 1;
    lseek(studentFD, -1*sizeof(student), SEEK_CUR);
    writeBytes = write(studentFD, &student, sizeof(student));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_STUDENT_DB);
        release_lock(studentFD);
        close(studentFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "\n# Successfully activated the student access\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }
    release_lock(studentFD);
    close(studentFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
*
* block student account 
*/
void blockStudent(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Lists all studnets so that admin can choose
    listStudents(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, REQ_STUDENT_ROLLNO);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char rollnumber[20];
    strcpy(rollnumber, readBuf);
    
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, STUDENT_DATABASE);

    // Opens the student database and set the student active status to 0
    int studentFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(studentFD == -1) {
        perror(ERROR_OPEN_STUDENT);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }
    acquire_write_lock(studentFD);

    struct Student student;
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        if(strcmp(student.sRollNo, rollnumber) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_STUDENT, rollnumber);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
        }
        release_lock(studentFD);
        close(studentFD);
        return;
    }

    student.active = 0;
    lseek(studentFD, -1*sizeof(student), SEEK_CUR);
    writeBytes = write(studentFD, &student, sizeof(student));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_STUDENT_DB);
        release_lock(studentFD);
        close(studentFD);
        return;
    }
    release_lock(studentFD);

    char courseFile[50];
    strcpy(courseFile, DATABASE_PATH);
    strcat(courseFile, COURSE_DATABASE);
    int courseFD = open(courseFile, O_CREAT | O_RDWR, 0777);
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

    struct Course course;
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
        acquire_write_lock(courseDbFD);

        struct Enroll enroll;
        while((readBytes = read(courseDbFD, &enroll, sizeof(enroll))) != 0) {
            if(enroll.sid == student.sId && enroll.active == 1) break;
        }
        if(readBytes > 0) {
            enroll.sid = -1;
            enroll.active = false;
            lseek(courseDbFD, -1*sizeof(enroll), SEEK_CUR);
            write(courseDbFD, &enroll, sizeof(enroll));

            course.cCurrentAvailableSeats++;
            lseek(courseFD, -1*sizeof(course), SEEK_CUR);
            write(courseFD, &course, sizeof(course));
        }
        release_lock(courseDbFD);
        close(courseDbFD);
    }
    release_lock(courseFD);
    close(courseFD);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "\n# Successfully blocked the student access\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }

    close(studentFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
*
* Modify a student's details 
*/
void modifyStudent(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // List all the available students in system
    listStudents(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, REQ_STUDENT_ROLLNO);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char rollnumber[20];
    strcpy(rollnumber, readBuf);
    
    // Open the database file to retireve the students existing details
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(studentFD == -1) {
        perror(ERROR_OPEN_STUDENT);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }
    acquire_write_lock(studentFD);

    struct Student student;
    while((readBytes = read(studentFD, &student, sizeof(student))) != 0) {
        if(strcmp(student.sRollNo, rollnumber) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_STUDENT, rollnumber);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
        }
        release_lock(studentFD);
        close(studentFD);
        return;
    }

    // If student is online cannot modify its details
    if(student.online == 1) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, CANNOT_CHANGE);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
        }
        release_lock(studentFD);
        close(studentFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the Name of student? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new name: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        strcpy(student.sName, readBuf);
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the Address of student? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new address: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        strcpy(student.sAddress, readBuf);
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the Age of student? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new age: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        student.sAge = atoi(readBuf);
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the DEGREE TYPE of student? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new DEGREE TYPE: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        bzero(student.sRollNo, sizeof(student.sRollNo));
        strcpy(student.sRollNo, readBuf);
        bzero(readBuf, sizeof(readBuf));
        sprintf(readBuf, "%d", student.sId);
        strcat(student.sRollNo, readBuf);
    }

    lseek(studentFD, -1*sizeof(student), SEEK_CUR);
    writeBytes = write(studentFD, &student, sizeof(student));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_STUDENT_DB);
        release_lock(studentFD);
        close(studentFD);
        return;
    }
    release_lock(studentFD);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "\n# Successfully updated the student details\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }

    close(studentFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
*
* add's a new faculty into the system 
*/
void addFaculty(int clientConnectionFD) {

    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Take all the faculty details from the client
    struct Faculty newFaculty;

    strcpy(writeBuf, "Enter the faculty name: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    strcpy(newFaculty.fName, readBuf);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Enter the faculty's address: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    strcpy(newFaculty.fAddress, readBuf);

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Enter the department(CSE, ECE etc): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    strcpy(newFaculty.fDepartment, readBuf);

    // Open the track database file to generate the unique id for faculty
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
    lseek(trackFD, 1*sizeof(track), SEEK_SET);
    readBytes = read(trackFD, &track, sizeof(track));
    if(strcmp(track.name, FACULTY) != 0) return;
    newFaculty.fId = track.uid;
    track.uid++;

    lseek(trackFD, 1*sizeof(track), SEEK_SET);
    write(trackFD, &track, sizeof(track));

    bzero(readBuf, sizeof(readBuf));
    sprintf(readBuf, "%d", newFaculty.fId);
    strcpy(newFaculty.fLogin, newFaculty.fDepartment);
    strcat(newFaculty.fLogin, readBuf);

    newFaculty.active = true;
    newFaculty.online = false;

    strcpy(newFaculty.fPassword, DEFAULT_PASS);

    // Open the faculty database file to store the information into the system
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, FACULTY_DATABASE);

    int facultyFD = open(databaseFile, O_CREAT | O_RDWR | O_APPEND, 0777);
    if(facultyFD == -1) {
        perror(ERROR_OPEN_FACULTY);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
        }
        release_lock(trackFD);
        close(trackFD);
        return;
    }
    acquire_write_lock(facultyFD);

    writeBytes = write(facultyFD, &newFaculty, sizeof(newFaculty));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_FACULTY_DB);
        release_lock(facultyFD);
        release_lock(trackFD);
        close(trackFD);
        close(facultyFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, FACULTY_ADDED, newFaculty.fLogin);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }
    release_lock(facultyFD);
    release_lock(trackFD);
    close(trackFD);
    close(facultyFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
*
* View the faculty's details 
*/
void viewFaculty(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Lists all the faculty in system so that admin can choose
    listFaculty(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, REQ_FACULTY_LOGINID);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char login[20];
    strcpy(login, readBuf);
    
    // Opens the file and finds the faculty details
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, FACULTY_DATABASE);

    int facultyFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(facultyFD == -1) {
        perror(ERROR_OPEN_FACULTY);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }
    acquire_read_lock(facultyFD);

    struct Faculty faculty;
    while((readBytes = read(facultyFD, &faculty, sizeof(faculty))) != 0) {
        if(strcmp(faculty.fLogin, login) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_FACULTY, login);
    } else {
        sprintf(writeBuf, FACULTY_DETAILS_PRINT, 
            faculty.fName, faculty.fAddress, faculty.fLogin, faculty.fDepartment, faculty.active, faculty.online);
    }

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }
    release_lock(facultyFD);
    close(facultyFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
*
* Modify the faculty's details 
*/
void modifyFaculty(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Lists all the faculty in system so that admin can choose
    listFaculty(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, REQ_FACULTY_LOGINID);
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char login[20];
    strcpy(login, readBuf);
    
    // Opens database to retireve the existing faculty details in the system
    char databaseFile[50];
    strcpy(databaseFile, DATABASE_PATH);
    strcat(databaseFile, FACULTY_DATABASE);

    int facultyFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(facultyFD == -1) {
        perror(ERROR_WRITING_FACULTY_DB);

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_REPORTING_LOGOUT_MESSAGE);
            return;
        }
        return;
    }
    acquire_write_lock(facultyFD);

    struct Faculty faculty;
    while((readBytes = read(facultyFD, &faculty, sizeof(faculty))) != 0) {
        if(strcmp(faculty.fLogin, login) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, UNABLE_FIND_FACULTY, login);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_RESPONSE);
        }
        release_lock(facultyFD);
        close(facultyFD);
        return;
    }

    // If faculty is online cannot modify its data
    if(faculty.online == 1) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, CANNOT_CHANGE);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror(ERROR_WRITING_TO_CLIENT);
        }
        release_lock(facultyFD);
        close(facultyFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the Name of faculty? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new name: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        strcpy(faculty.fName, readBuf);
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the Address of faculty? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new address: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        strcpy(faculty.fAddress, readBuf);
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Want to modify the Department of faculty? (Enter y or n): ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    if(strcmp(readBuf, "y") == 0) {
        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "Enter the new DEPARTMENT: ");
        if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
        bzero(faculty.fDepartment, sizeof(faculty.fDepartment));
        strcpy(faculty.fDepartment, readBuf);
        bzero(faculty.fLogin, sizeof(faculty.fLogin));
        strcpy(faculty.fLogin, faculty.fDepartment);
        bzero(readBuf, sizeof(readBuf));
        sprintf(readBuf, "%d", faculty.fId);
        strcat(faculty.fLogin, readBuf);
    }

    // Store the updated faculty details back into the system
    lseek(facultyFD, -1*sizeof(faculty), SEEK_CUR);
    writeBytes = write(facultyFD, &faculty, sizeof(faculty));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_FACULTY_DB);
        release_lock(facultyFD);
        close(facultyFD);
        return;
    }
    release_lock(facultyFD);
    
    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "\n# Successfully updated the faculty details\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror(ERROR_WRITING_RESPONSE);
    }

    close(facultyFD);
    return;
}

/*
* @param clientConnectionFD An file descriptor for the client connection
*
* Controlls all the admin functionalities 
*/
void rootAdminController(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    // Attempts to login into the system otherwise returns
    if(loginHandler(clientConnectionFD, 3, NULL, NULL)) {
        strcpy(writeBuf, SUCCESS_LOGIN);
        int adminChoice;
        do {
            strcat(writeBuf, ADMINPAGE);
            writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
            if(writeBytes == -1) {
                perror(ERROR_SENDING_ADMIN_CHOICE);
                return;
            }

            readBytes = read(clientConnectionFD, readBuf, sizeof(readBuf));
            if(readBytes == -1) {
                perror(ERROR_READING_ADMIN_CHOICE);
                return;
            } else if(readBytes == 0) {
                perror(NO_DATA_RECEIVED);
                return;
            }
            adminChoice = atoi(readBuf);
            switch (adminChoice) {
                case 1:
                    addStudent(clientConnectionFD);
                    break;
                case 2:
                    viewStudent(clientConnectionFD);
                    break;
                case 3:
                    addFaculty(clientConnectionFD);
                    break;
                case 4:
                    viewFaculty(clientConnectionFD);
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
                    modifyFaculty(clientConnectionFD);
                    break;
                case 9: // Logout
                default:
                    // Wrong Choice
                    write(clientConnectionFD, SUCCESS_LOGOUT, sizeof(SUCCESS_LOGOUT));
                    return;
            }
            bzero(writeBuf, sizeof(writeBuf));
        } while(adminChoice > 0 && adminChoice < 9);

    } else {
        strcpy(writeBuf, FAILED_LOGIN);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    }
}

#endif