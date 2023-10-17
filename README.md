# Academia Portal

The **Academia Portal** project is a user-friendly and multifunctional academic management system developed in C, utilizing Linux system calls for tasks like file locking and socket programming. This project provides functionalities for students, faculty, and administrators to manage academic information efficiently.

## Features

- **User Account Management**: All student and faculty details, along with course information, are stored in files. User accounts are categorized into three roles: Faculty, Student, and Admin.

- **Login System**: Account holders must pass through a secure login system to access their accounts. The administrator manages all user accounts.

- **Administrator Access**: The administrative access is password-protected to prevent unauthorized access to the entire management system.

- **Role-based Menus**:
  - **Admin**: Can perform actions such as adding students, viewing student and faculty details, activating or blocking students, modifying student and faculty details, and logging out.
  - **Student**: Can view all courses, enroll in new courses, drop courses, view enrolled course details, change their password, view all enrolled courses, and log out.
  - **Faculty**: Can view offering courses, add new courses, remove courses from the catalog, update course details, change their password, and log out.

- **Data Locking**: Data locking is implemented to ensure the integrity of academic data. Read locks and write locks are used to protect critical data sections, allowing multiple users to access the system concurrently without data conflicts.

- **Socket Programming**: The project utilizes socket programming, where a server maintains the database and serves multiple clients concurrently. Clients can connect to the server to access their specific academic details.

## Project Structure

The project is organized into several folders and files:

- **Database Folder**: Stores all the database files, including `student_file.txt` and `faculty_file.txt`, etc.

- **Helper Folder**: Contains various helper functions such as `readWriterHelper.h`, `loginHelper.h`, `logoutHelper.h`, `readLock.h`, `writeLock.h`, `releaseLock.h`, `listFacultyHelper.h`, `listStudentHelper.h`, `adminCredentials.h`, etc.

- **Models Folder**: Houses the data structures for student, faculty, and course information, stored in files like `student_struct.h`, `faculty_struct.h`, and `course_struct.h`, etc.

- **Controllers Folder**: Contains three header files (`AdminController.h`, `StudentController.h`, and `FacultyController.h`) that handle the functionality for administrators, students, and faculty members, respectively.

- **Other Files**: 
  - `client.c`: The client-side program that interacts with the server.
  - `server.c`: The server-side program responsible for coordinating and providing the required functionality to clients.

## Usage

To use the Academia Portal, follow these steps:

1. Compile the project using the appropriate build commands for your system.

2. Run the server application on a central server or computer.

3. Run the client application on student, faculty, and admin machines to access the portal. 

4. Log in with the appropriate credentials and access the desired functionality based on your role.

5. Follow the on-screen instructions to perform various academic tasks, including adding students, viewing course details, enrolling in courses, and more.