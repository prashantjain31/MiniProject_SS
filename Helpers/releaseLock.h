#ifndef RELEASE_LOCK
#define RELEASE_LOCK

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

void release_lock(int file_descriptor) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  // Unlock the entire file

    if (fcntl(file_descriptor, F_SETLK, &lock) == -1) {
        perror("Failed to release lock");
        exit(1);
    }
}

#endif