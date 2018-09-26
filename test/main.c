//
// Created by John Karasev on 9/21/18.
//
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


mode_t getMode(char *file) {
    struct stat buf;
    if (lstat(file, &buf) == -1) {
        return -1;
    }
    return buf.st_mode;
}

int main() {
    int fd = open("../dest", O_CREAT | O_WRONLY);
    printf("%d\n", fd);
}


