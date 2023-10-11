#ifndef STUDENT_STRUCT
#define STUDENT_STRUCT

struct Student {
    int sId;
    char sRollNo[20];
    char sName[50];
    char sAddress[100];
    char sPassword[50];
    int sAge;
    bool active;
    bool online;
};

#endif