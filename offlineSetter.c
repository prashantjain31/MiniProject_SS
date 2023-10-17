#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "./Models/student_struct.h"
#include "./Models/faculty_struct.h"
#include "./Helpers/constantStrings.h"
#include "./Helpers/loginHelper.h"
#include "./Helpers/readWriteHelper.h"
#include "./Helpers/writeLock.h"
#include "./Helpers/releaseLock.h"
#include "./Helpers/adminCredentials.h"

int main() {
    char id[10];
    char pass[10];
    bzero(id, sizeof(id));
    bzero(pass, sizeof(pass));
    printf("\nEnter your superadmin id: ");
    scanf("%s", id);
    printf("\nEnter your superadmin password: ");
    scanf("%s", pass);

    if(strcmp(id, SUPERID) == 0 && strcmp(pass, SUPERPASSWORD) == 0) {
        int userType;
        printf("\nEnter the type of user whom you want to set offline(1 for Student and 2 for Faculty): ");
        scanf("%d", &userType);

        char database[50];
        strcpy(database, DATABASE_PATH);
        if(userType == 1) {
            strcat(database, STUDENT_DATABASE);
            int studentFD = open(database, O_RDWR, 0777);
            if(studentFD == -1) {
                printf("\nCouldn't open the database\n");
                return 0;
            }
            acquire_write_lock(studentFD);
            char studentID[20];
            printf("\nEnter the student roll number: ");
            scanf("%s", studentID);

            struct Student student;
            while(read(studentFD, &student, sizeof(student)) > 0) {
                if(strcmp(student.sRollNo, studentID) == 0 && student.online == 1) {
                    student.online = 0;
                    lseek(studentFD, -1*sizeof(student), SEEK_CUR);
                    write(studentFD, &student, sizeof(student));
                    printf("\nSuccessfully set the student as offline\n");
                    release_lock(studentFD);
                    return 0;
                }
            }
            release_lock(studentFD);
            close(studentFD);
            printf("\nCouldn't find the student or already offline\n");
        } else if(userType == 2) {
            strcat(database, FACULTY_DATABASE);
            int facultyFD = open(database, O_RDWR, 0777);
            if(facultyFD == -1) {
                printf("\nCouldn't open the database\n");
                return 0;
            }
            acquire_write_lock(facultyFD);
            char facultyID[20];
            printf("\nEnter the faculty login id: ");
            scanf("%s", facultyID);

            struct Faculty faculty;
            while(read(facultyFD, &faculty, sizeof(faculty)) > 0) {
                if(strcmp(faculty.fLogin, facultyID) == 0 && faculty.online == 1) {
                    faculty.online = 0;
                    lseek(facultyFD, -1*sizeof(faculty), SEEK_CUR);
                    write(facultyFD, &faculty, sizeof(faculty));
                    printf("\nSuccessfully set the faculty as offline\n");
                    release_lock(facultyFD);
                    return 0;
                }
            }
            release_lock(facultyFD);
            close(facultyFD);
            printf("\nCouldn't find the faculty or already offline\n");
        }
    }

    return 0;
}