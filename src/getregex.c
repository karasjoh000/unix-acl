//
// Created by John Karasev on 9/20/18.
//

#include <stdio.h>
#include <stdbool.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>



void freeRegex(regex_t* regex) {
    regfree(regex);
    free(regex);
}

bool compileRegex(char* pattern, regex_t *regex) {
    int res;
    res = regcomp(regex, pattern, REG_ICASE);
    if (res) {
        return false;
    }
    if (DEBUG) printf("compiled %s successfully\n", pattern);
    return true;
}

bool checkMatch(char* buffer, regex_t *regex) {
    if (regexec(regex, buffer, 0, NULL, 0) == REG_NOMATCH) return false;
    return true;
}