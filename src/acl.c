//
// Created by John Karasev on 9/20/18.
//
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <getregex.h>
#include <debug.h>

#define ACL_EXT ".access"
#define LINE_READ 120


#define ACL_ROW "^[[:blank:]]*[[:alnum:]][[:alnum:]]*[[:blank:]][[:blank:]]*[rwb][[:blank:]]*\n*$"
#define ACL_ROW_B_NAME "^[[:blank:]]*"
#define ACL_ROW_A_NAME "[[:blank:]][[:blank:]]*[rwb][[:blank:]]*\n*$"


char *createAclPath(char *source, char *aclFile) {
    strcpy(aclFile, source);
    strcat(aclFile, ACL_EXT);
    return aclFile;
}

bool createNameRegex(char* name, regex_t* regex) {
    char nameRegex[strlen(ACL_ROW_B_NAME) + strlen(name) + strlen(ACL_ROW_A_NAME)];
    strcpy(nameRegex, ACL_ROW_B_NAME);
    strcat(nameRegex, name);
    strcat(nameRegex, ACL_ROW_A_NAME);
    return compileRegex(nameRegex, regex);
}

bool createFormatRegex(regex_t *regex) {
    return compileRegex(ACL_ROW, regex);
}

//put the two functions together?

bool checkAclFile(char *file, regex_t* regex) {
    FILE *fp;
    if ((fp = fopen(file, "r")) == NULL) {
        if (DEBUG) printf("ERROR: failed to open file %s\n", file);
        exit(0);
    }

    char buffer[LINE_READ];
    while (fgets(buffer, LINE_READ, fp) != NULL) {
        if (buffer[0] == '#') continue;
        if (!checkMatch(buffer, regex)) {
            if (DEBUG) printf("failed on this line:\n%s\n", buffer);
            fclose(fp);
            return false;
        }
    }
    fclose(fp);
    return true;
}

char getAccessType(char* buffer) {
    int i;
    for ( i = (int) strlen(buffer) - 1; i >= 0 && (isblank(buffer[i]) || buffer[i] == '\n'); i--);
    return buffer[i];
}

char searchName(char *file, regex_t* regex) {
    FILE *fp;
    if ((fp = fopen(file, "r")) == NULL) {
        if (DEBUG) printf("ERROR: failed to open file %s\n", file);
        exit(0);
    }
    char buffer[LINE_READ];
    while (fgets(buffer, LINE_READ, fp) != NULL) {
        if (buffer[0] == '#') continue;
        if (checkMatch(buffer, regex)) {
            if (DEBUG) printf("match on:\n%s", buffer);
            fclose(fp);
            return getAccessType(buffer);
        }
        if (DEBUG) printf("no match:\n%s", buffer);
    }
    fclose(fp);
    return 'f';
}

