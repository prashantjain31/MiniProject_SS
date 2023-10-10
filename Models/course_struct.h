#ifndef COURSE_STRUCT
#define COURSE_STRUCT

struct Course {
    int cId;
    char cName[50];
    char cDepartment[10];
    int cTotalSeats;
    int cCurrentAvailableSeats;
    int fId;
    bool active;
    int cCredits;
    char databasePath[50];
};

#endif