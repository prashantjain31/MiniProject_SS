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

#include "../Models/student_struct.h"
#include "../Helpers/constantStrings.h"
#include "../Helpers/loginHelper.h"

void rootFacultyController(int clientConnectionFD) {
    if(loginHandler(clientConnectionFD, 2)) {
        printf("Login successful\n");
    } else printf("login failed \n");
}

#endif