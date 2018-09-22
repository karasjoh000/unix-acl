//
// Created by John Karasev on 9/20/18.
//

#ifndef CS483_P1_REGEX_H
#define CS483_P1_REGEX_H

#include <regex.h>
#include <stdbool.h>

void freeRegex(regex_t* regex);
bool compileRegex(char* pattern, regex_t *regex);
bool checkMatch(char* buffer, regex_t *regex);
char searchName(char *file, regex_t* regex);

#endif //CS483_P1_REGEX_H
