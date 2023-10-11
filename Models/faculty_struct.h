#ifndef FACULTY_STRUCT
#define FACULTY_STRUCT

struct Faculty {
    int fId;
    char fName[50];
    char fLogin[20];
    char fDepartment[10];
    char fAddress[100];
    char fPassword[50];
    bool active;
    bool online;
};

#endif