//
// Created by John Karasev on 9/21/18.
//

#ifndef CS_483_P1_GET_H
#define CS_483_P1_GET_H

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct uidInfo {
    uid_t effective;
    uid_t real;
    char *aclFile;
    char *sourceFile;
    char *destFile;
} UIDINFO;

bool copy(UIDINFO *info);

bool get(UIDINFO *info);

#endif //CS_483_P1_GET_H
