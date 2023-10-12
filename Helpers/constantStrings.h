#ifndef CONSTANT_STRINGS
#define CONSTANT_STRINGS

#define PORT 8086

#define TRACK_FILE "track_file.txt"
#define STUDENT_DATABASE "student_file.txt"
#define FACULTY_DATABASE "faculty_file.txt"
#define COURSE_DATABASE "course_file.txt"

#define ADMIN "ADMIN"
#define STUDENT "STUDENT"
#define FACULTY "FACULTY"
#define COURSE "COURSE"

#define WELCOME_STUDENT "---------- Welcome back Student ----------\n"
#define WELCOME_ADMIN "---------- Welcome back Admin ----------\n"
#define WELCOME_PROFESSOR "---------- Welcome back Professor ----------\n"

#define HOMEPAGE "---------- Welcome back to the Academia Portal ----------\n1.Student \n2.Professor \n3.Administrator \nEnter Login Type (1, 2, or 3): "
#define ADMINPAGE "---------- Welcome to Admin Page ----------\n1.Add Student \n2.View Student Details \n3.Add Faculty \n4.View Faculty Details \n5.Activate Student \n6.Block Student \n7.Modify Student Details \n8.Modify Faculty Details \n9.Logout and Exit \nEnter your choice: "
#define FACULTYPAGE "---------- Welcome to Faculty Page ----------\n1.View Offering Courses \n2.Add new Course \n3.Remove Course from Catalog \n4.UpdateCourse Details \n5.Change Password \n6.Logout and Exit \nEnter your choice: "
#define STUDENTYPAGE "---------- Welcome to Student Page ----------\n1.View all Courses \n2.Enroll into new Course \n3.Drop Course \n4.View Enrolled Course Details \n5.Change Password \n6.View All enrolled courses \n7.Logout and Exit \nEnter your choice: "

#define LOGIN_ID_MESSAGE "Enter your Login id: "
#define LOGIN_PASS_MESSAGE "Enter your Password: "
#define FAILED_LOGIN "You entered wrong ID or Password\n"
#define SUCCESS_LOGIN "--- Login Successful ---\n"
#define SUCCESS_LOGOUT "--- Logout Successful ---\n"
#define FAILED_LOGOUT "--- Logout Successful ---\n"
#define BLOCKED "--- This account is blocked please talk to administrator ---\n"
#define ALREADY_LOGGED_IN "--- This account is already logged in ---\n"
#define CANNOT_CHANGE "--- This account is online cannot modify it currently ---\n"

#define WRONG "Something went wrong at server side. \n"

#define DEFAULT_PASS "1234" 


#endif