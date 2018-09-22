//
// Created by John Karasev on 9/21/18.
//

#include <debug.h>
#include <stdio.h>
#include <string.h>

void accessdeny(char* mesg) {
    if (DEBUG) printf("ACCESS DENIED: %s\n", mesg);
}

void errormesg(char* mesg) {
    if (DEBUG) printf("ERROR: %s\n", mesg);
}

void euiderr() {
    if (DEBUG) printf("ERROR: failed to set euid\n");
}

void dperror(char* mesg, char* file) { 
    if (DEBUG) { 
        char errorm[strlen(file)+20];
        sprintf(errorm, "%s%s", mesg, file); 
        perror(errorm);
    }
}
