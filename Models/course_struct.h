#ifndef COURSE_STRUCT
#define COURSE_STRUCT

struct Course {
    int cId;
    char cName[50];
    char cDepartment[10];
    int cTotalSeats;
    int cCurrentAvailableSeats;
    int fId;
    int cCredits;
};

#endif