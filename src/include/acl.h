//
// Created by John Karasev on 9/20/18.
//

#ifndef CS483_P1_ACL_H
#define CS483_P1_ACL_H

#include <regex.h>
#include <stdbool.h>

#define ACL_EXT ".access"

char *createAclPath(char *source, char *aclFile);
bool createNameRegex(char* name, regex_t* regex);
bool createFormatRegex(regex_t *regex);
bool checkAclFile(int fd, regex_t* regex);
char searchName(int fd, regex_t* regex);


#endif //CS483_P1_ACL_H
