#ifndef CONSTANT_STRINGS
#define CONSTANT_STRINGS

// Port number for client and server
#define PORT 8080

// Default passwords
#define DEFAULT_PASS "1234"

// Database file names and basic path
#define TRACK_FILE "track_file.txt"
#define DATABASE_PATH "./database/"
#define COURSE_PATH "course_%d.txt"
#define STUDENT_DATABASE "student_file.txt"
#define FACULTY_DATABASE "faculty_file.txt"
#define COURSE_DATABASE "course_file.txt"

// Default Roles and Structures
#define ADMIN "ADMIN"
#define STUDENT "STUDENT"
#define FACULTY "FACULTY"
#define COURSE "COURSE"

// Welcome Strings
#define WELCOME_STUDENT "\n---------- WELCOME BACK STUDENT ----------\n"
#define WELCOME_ADMIN "\n---------- WELCOME BACK ADMIN ----------\n"
#define WELCOME_PROFESSOR "\n---------- WELCOME BACK PROFESSOR ----------\n"


// Pages of portal
#define HOMEPAGE "\n---------- WELCOME BACK TO THE ACADEMIA PORTAL ----------\n1.STUDENT \n2.PROFESSOR \n3.ADMINISTRATOR \nENTER LOGIN TYPE (1, 2, OR 3): "
#define ADMINPAGE "\n---------- WELCOME TO ADMIN PAGE ----------\n1.ADD STUDENT \n2.VIEW STUDENT DETAILS \n3.ADD FACULTY \n4.VIEW FACULTY DETAILS \n5.ACTIVATE STUDENT \n6.BLOCK STUDENT \n7.MODIFY STUDENT DETAILS \n8.MODIFY FACULTY DETAILS \n9.LOGOUT AND EXIT \nENTER YOUR CHOICE: "
#define FACULTYPAGE "\n---------- WELCOME TO FACULTY PAGE ----------\n1.VIEW OFFERING COURSES \n2.ADD NEW COURSE \n3.REMOVE COURSE FROM CATALOG \n4.UPDATE COURSE DETAILS \n5.CHANGE PASSWORD \n6.LOGOUT AND EXIT \nENTER YOUR CHOICE: "
#define STUDENTYPAGE "\n---------- WELCOME TO STUDENT PAGE ----------\n1.VIEW ALL COURSES \n2.ENROLL INTO NEW COURSE \n3.DROP COURSE \n4.VIEW ENROLLED COURSE DETAILS \n5.CHANGE PASSWORD \n6.VIEW ALL ENROLLED COURSES \n7.LOGOUT AND EXIT \nENTER YOUR CHOICE: "


// Login related strings
#define LOGIN_ID_MESSAGE "\nEnter your Login id: "
#define LOGIN_PASS_MESSAGE "\nEnter your Password: "
#define FAILED_LOGIN "\nYou entered wrong ID or Password\n"
#define SUCCESS_LOGIN "\n--- Login Successful ---\n"
#define SUCCESS_LOGOUT "\n--- Logout Successful ---\n"
#define FAILED_LOGOUT "\n--- Logout Successful ---\n"
#define BLOCKED "\n# This account is blocked please talk to administrator\n"
#define ALREADY_LOGGED_IN "\n# This account is already logged in\n"
#define CANNOT_CHANGE "\n# This account is online cannot modify it currently\n"


// Error Messages
#define WRONG "\nSomething went wrong at server side. \n"
#define ERROR_OPEN_TRACK "!! Error while opening track database file !!"
#define ERROR_REPORTING_LOGOUT_MESSAGE "!! Error while writing logout message to client !!"
#define ERROR_OPEN_STUDENT "!! Error while opening student database file !!"
#define ERROR_OPEN_COURSE "!! Error while opening course database file !!"
#define ERROR_OPEN_FACULTY "!! Error while opening faculty database file !!"
#define ERROR_WRITING_STUDENT_DB "!! Error while writing student to database !!"
#define ERROR_WRITING_FACULTY_DB "!! Error while writing faculty to database !!"
#define ERROR_WRITING_COURSE_DB "!! Error while writing course to database !!"
#define ERROR_CREATING_NEW_COURSE_DB "!! Error while creating new course database file !!" 
#define ERROR_WRITING_TO_CLIENT "!! Error while writing to client !!"
#define ERROR_READING_FROM_CLIENT "!! Error while reading from client !!"
#define ERROR_WRITING_RESPONSE "!! Error while reporting the response !!"
#define ERROR_SENDING_ADMIN_CHOICE "!! Error while sending the Admin choice Page !!"
#define ERROR_READING_ADMIN_CHOICE "!! Error while reading the Admin's choice !!"
#define ERROR_SENDING_FACULTY_CHOICE "!! Error while sending the Faculty choice Page !!"
#define ERROR_READING_FACULTY_CHOICE "!! Error while reading the Faculty's choice !!"
#define ERROR_ENROLLING_STUDENT "!! Error while enrolling the student to course !!"
#define ERROR_SENDING_STUDENT_CHOICE "!! Error while sending the Student choice Page !!"
#define ERROR_READING_STUDENT_CHOICE "!! Error while reading the Student's choice Page !!"
#define ERROR_DROP_COURSE "!! Error while droping course !!"
#define NO_DATA_RECEIVED "\nNo data received from the client side"


// Request data strings
#define REQ_STUDENT_ROLLNO "\nEnter the student roll number: "
#define REQ_FACULTY_LOGINID "\nEnter the faculty login id: "
#define REQ_DEFAULT_TO_NEW_PASS "\n$ Enter the new password (because your current password is default password): "
#define REQ_NEW_PASSWORD "\n$ Enter the new password: "
#define REQ_COURSE_ID "\nEnter the course id: "


// Unable to find data strings
#define UNABLE_FIND_STUDENT "\n# Couldn't find Student with Roll Number: %s\n"
#define UNABLE_FIND_FACULTY "\n# Couldn't find faculty with Login: %s\n"
#define UNABLE_UPDATE_PASSWORD "\n# Failed to update the password.\n"
#define UNABLE_FIND_COURSE "\n# Couldn't find Course with ID: %d under current faculty profile.\n"
#define UNABLE_FIND_COURSE_GLOBAL "\n# Couldn't find Course with ID: %d\n"

// Data printing structures
#define STUDENT_ADDED "\n# Student added Successfully with Roll Number: %s\n"
#define FACULTY_ADDED "\n# Faculty added Successfully with login id: %s\n"
#define COURSE_ADDED "\n# Course added Successfully with id: %d\n" 
#define STUDENT_DETAILS_PRINT "\n# STUDENT DETAILS \nNAME: %s\nADDRESS: %s\nAGE: %d\nROLL NUMBER: %s\nACTIVE STATUS(1=ACTIVE and 0=BLOCKED): %d\nONLINE STATUS(1=ONLINE and 0=OFFLINE): %d\n"
#define FACULTY_DETAILS_PRINT "\n# FACULTY DETAILS \nNAME: %s\nADDRESS: %s\nLOGIN: %s\nDEPARTMENT: %s\nACTIVE STATUS(1=ACTIVE and 0=BLOCKED): %d\nONLINE STATUS(1=ONLINE and 0=OFFLINE): %d\n"
#define COURSE_LIST_HEADING "\n# ----- COURSE LIST -----\n"
#define STUDENT_LIST_HEADING "\n----- STUDENT LIST -----\n"
#define FACULTY_LIST_HEADING "\n----- FACULTY LIST -----\n"
#define COURSE_DETAILS_PRINT "\nCOURSE ID: %d\nNAME: %s\nTOTAL SEATS: %d\nAVAILABLE SEATS: %d\nDEPARTMENT: %s\nCREDITS: %d\n"
#define SUCCESS_DELETE_COURSE "\n# Successfully deleted the course\n"
#define NO_SEATS_AVAILABLE "\n# No Seats available in course.\n" 

#endif