#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>

#include "./Helpers/constantStrings.h"

// Handles all repetitive writing and reading until the disconnection.
void connectionHandler(int socketFD) {
    char readBuf[1000];
    char writeBuf[1000];
    ssize_t readBytes, writeBytes;
    char tempBuf[1000];

    do {
        bzero(readBuf, sizeof(readBuf));
        bzero(writeBuf, sizeof(writeBuf));
        readBytes = read(socketFD, readBuf, sizeof(readBuf));
        if(readBytes == -1)
            perror("!! Error while reading from connection !!");
        else if(readBytes == 0)
            printf("Closing the connection to the server!\n");
        else {
            if(strchr(readBuf, '*') != NULL) {
                strcpy(writeBuf, getpass(LOGIN_PASS_MESSAGE));
            } else if(strchr(readBuf, '$') != NULL) {
                strcpy(writeBuf, getpass(readBuf));
            } else if(strchr(readBuf, '&') != NULL) {
                printf("%s", WRONG);
                continue;
            } else if(strchr(readBuf, '#') != NULL) {
                printf("%s", readBuf);
                continue;
            } else {
                printf("%s", readBuf);
                scanf("%[^\n]%*c", writeBuf);
            }

            writeBytes = write(socketFD, writeBuf, strlen(writeBuf));
            if(writeBytes == -1) {
                perror("!! Error while writing to client socket !!");
                printf("Closing the connection to the server now!\n");
                break;
            }
        }
    } while(readBytes > 0);

    close(socketFD);
}

// Creates the socket and connects to the server
void main() {
    int socketFD, connectionStatus;
    struct sockaddr_in serverAddress;

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD == -1) {
        perror("!! Error while creating server socket !!");
        _exit(0);
    }

    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    connectionStatus = connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if(connectionStatus == -1) {
        perror("!! Error while connecting to server !!");
        close(socketFD);
        _exit(0);
    }

    connectionHandler(socketFD);

    close(socketFD);
}