#ifndef READ_LOCK
#define READ_LOCK

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

// Function to acquire a read lock for a record
void acquire_read_lock(int file_descriptor) {
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  

    if (fcntl(file_descriptor, F_SETLKW, &lock) == -1) {
        perror("Failed to acquire read lock");
        _exit(0);
    }
}

#endif