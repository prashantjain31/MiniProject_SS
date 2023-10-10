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
#include "../Helpers/listStudentsHelper.h"
#include "../Helpers/listFacultyHelper.h"

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

    listStudents(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, "Enter the student roll number: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char rollnumber[20];
    strcpy(rollnumber, readBuf);
    
    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, STUDENT_DATABASE);

    int studentFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
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

    listStudents(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, "Enter the student roll number: ");
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

    listStudents(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, "Enter the student roll number: ");
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

    listStudents(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, "Enter the student roll number: ");
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
        perror("!! Error while writing the student details to database !!");
        close(studentFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Successfully updated the student details\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the student details to client !!");
    }

    close(studentFD);
    return;
}

void addFaculty(int clientConnectionFD) {

    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

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

    strcpy(newFaculty.fPassword, DEFAULT_PASS);

    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, FACULTY_DATABASE);

    int facultyFD = open(databaseFile, O_CREAT | O_RDWR | O_APPEND, 0777);
    if(facultyFD == -1) {
        perror("!! Error while opening faculty database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    writeBytes = write(facultyFD, &newFaculty, sizeof(newFaculty));
    if(writeBytes == -1) {
        perror("!! Error while writing faculty to databasse !!");
        close(trackFD);
        close(facultyFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    sprintf(writeBuf, "Faculty Added Successfully with login id: %s\n", newFaculty.fLogin);
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while reporting the response !!");
    }

    close(trackFD);
    close(facultyFD);
    return;
}

void viewFaculty(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    listFaculty(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, "Enter the faculty login id: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char login[20];
    strcpy(login, readBuf);
    
    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, FACULTY_DATABASE);

    int facultyFD = open(databaseFile, O_CREAT | O_RDONLY, 0777);
    if(facultyFD == -1) {
        perror("!! Error while opening faculty database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    struct Faculty faculty;
    while((readBytes = read(facultyFD, &faculty, sizeof(faculty))) != 0) {
        if(strcmp(faculty.fLogin, login) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, "Couldn't find faculty with login: %s\n", login);
    } else {
        sprintf(writeBuf, "----- Faculty Details -----\nName: %s\nAddress: %s\nLogin: %s\nDepartment: %s\nActive Status(1 = Active and 0 = Blocked): %d", 
            faculty.fName, faculty.fAddress, faculty.fLogin, faculty.fDepartment, faculty.active);
    }

    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the faculty details to client !!");
        close(facultyFD);
        return;
    }

    close(facultyFD);
    return;
}

void modifyFaculty(int clientConnectionFD) {
    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;

    bzero(readBuf, sizeof(readBuf));
    bzero(writeBuf, sizeof(writeBuf));

    listFaculty(clientConnectionFD, writeBuf, sizeof(writeBuf));

    strcat(writeBuf, "Enter the faculty login id: ");
    if(!readwrite(clientConnectionFD, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf))) return;
    char login[20];
    strcpy(login, readBuf);
    
    char databaseFile[50];
    strcpy(databaseFile, "./database/");
    strcat(databaseFile, FACULTY_DATABASE);

    int facultyFD = open(databaseFile, O_CREAT | O_RDWR, 0777);
    if(facultyFD == -1) {
        perror("!! Error while opening faculty database file !!");

        bzero(writeBuf, sizeof(writeBuf));
        strcpy(writeBuf, "&");

        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while writing logout message to client !!");
            return;
        }
        return;
    }

    struct Faculty faculty;
    while((readBytes = read(facultyFD, &faculty, sizeof(faculty))) != 0) {
        if(strcmp(faculty.fLogin, login) == 0) break;
    }

    bzero(writeBuf, sizeof(writeBuf));
    if(readBytes == 0) {    
        sprintf(writeBuf, "Couldn't find Faculty with login id: %s\n", login);
        writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
        if(writeBytes == -1) {
            perror("!! Error while sending the faculty details to client !!");
        }
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

    lseek(facultyFD, -1*sizeof(faculty), SEEK_CUR);
    writeBytes = write(facultyFD, &faculty, sizeof(faculty));
    if(writeBytes == -1) {
        perror("!! Error while writing the faculty details to database !!");
        close(facultyFD);
        return;
    }

    bzero(writeBuf, sizeof(writeBuf));
    strcpy(writeBuf, "Successfully updated the faculty details\n");
    writeBytes = write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    if(writeBytes == -1) {
        perror("!! Error while sending the faculty details to client !!");
    }

    close(facultyFD);
    return;
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

    } else {
        strcpy(writeBuf, FAILED_LOGIN);
        write(clientConnectionFD, writeBuf, sizeof(writeBuf));
    }
}

#endif