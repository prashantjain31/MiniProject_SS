#include <stdio.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "./Helpers/constantStrings.h"
#include "./Helpers/loginHelper.h"
#include "./Controllers/StudentController.h"
#include "./Controllers/FacultyController.h"
#include "./Controllers/AdminController.h"

// Handles the connection for a specific client
void connectionHandler(int clientConnectionFD) {
    printf("Client has connected to the server!\n");

    char readBuf[1000], writeBuf[1000];
    ssize_t readBytes, writeBytes;
    int userChoice;

    writeBytes = write(clientConnectionFD, HOMEPAGE, strlen(HOMEPAGE));
    if(writeBytes == -1)
        perror("!! Error while sending first prompt to the user !!");
    else {
        bzero(readBuf, sizeof(readBuf));
        readBytes = read(clientConnectionFD, readBuf, sizeof(readBuf));
        if(readBytes == -1)
            perror(ERROR_READING_FROM_CLIENT);
        else if(readBytes == 0)
            printf(NO_DATA_RECEIVED);
        else {
            userChoice = atoi(readBuf);
            switch(userChoice) {
            case 1:
                // Student
                rootStudentController(clientConnectionFD);
                break;
            case 2:
                // Professor
                rootFacultyController(clientConnectionFD);
                break;
            case 3:
                // Administrator
                rootAdminController(clientConnectionFD);
                break;
            default:
                // Exit
                break;
            }
        }
    }
    printf("Terminating connection to client!\n");
}

void main() {
    // Creates the socket, binds it, set it to listen mode and then wait for connection acceptance
    int socketFD, socketBindStatus, socketListenStatus, clientConnectionFD, clientSize;
    struct sockaddr_in serverAddress, clientAddress;

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1) {
        perror("!! Error while creating server socket !!");
        _exit(0);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    socketBindStatus = bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if(socketBindStatus == -1) {
        perror("!! Error while binding to server socket !!");
        close(socketFD);
        _exit(0);
    }

    socketListenStatus = listen(socketFD, 100);
    if(socketListenStatus == -1) {
        perror("!! Error while listening for connections on the server socket !!");
        close(socketFD);
        _exit(0);
    }

    while(1) {
        clientSize = (int)sizeof(clientAddress);
        clientConnectionFD = accept(socketFD, (struct sockaddr *)&clientAddress, &clientSize);
        if(clientConnectionFD == -1) {
            perror("!! Error while connecting to client !!");
        } else {
            if(!fork()) {
                // Child connection to handle the client
                close(socketFD);
                connectionHandler(clientConnectionFD);
                _exit(0);
            } else {
                // Parent closes the file descriptor and waits for new connection.
                close(clientConnectionFD);
            }
        }
    }

    close(socketFD);
}