#ifndef WRITE_LOCK
#define WRITE_LOCK

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

void acquire_write_lock(int file_descriptor) {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  // Assuming the record size is an int

    if (fcntl(file_descriptor, F_SETLKW, &lock) == -1) {
        perror("Failed to acquire write lock");
        exit(1);
    }
}

#endif