#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

#include "./Models/track_struct.h"
#include "./Helpers/constantStrings.h"

int main() {

    char track_file[50];
    strcpy(track_file, "./database/");
    strcat(track_file, TRACK_FILE);

    printf("filename: %s\n", track_file);

    int trackFD = open(track_file, O_CREAT | O_EXCL | O_RDWR, 0777);
    if(trackFD == -1) {
        perror("!! Error occurred while opening track file !!");
        _exit(0);
    }

    struct Track student, course, faculty;

    bzero(student.name, sizeof(student.name));
    bzero(course.name, sizeof(course.name));
    bzero(faculty.name, sizeof(faculty.name));

    strcpy(student.name, STUDENT);
    student.uid = 1;

    strcpy(course.name, COURSE);
    course.uid = 1;

    strcpy(faculty.name, FACULTY);
    faculty.uid = 1;

    write(trackFD, &student, sizeof(student));
    write(trackFD, &faculty, sizeof(faculty));
    write(trackFD, &course, sizeof(course));

    lseek(trackFD, 0, SEEK_SET);
    
    struct Track temp;
    read(trackFD, &temp, sizeof(temp));
    printf("name: %s and id: %d\n", temp.name, temp.uid);
    read(trackFD, &temp, sizeof(temp));
    printf("name: %s and id: %d\n", temp.name, temp.uid);
    read(trackFD, &temp, sizeof(temp));
    printf("name: %s and id: %d\n", temp.name, temp.uid);

    return 0;
}