/*
 * Author John Karasev
 *
 * contains functions that operate on the acl file.
 * includes checking the acl file format and
 * searching for names in the acl file.
 */
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <getregex.h>
#include <debug.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>

#define ACL_EXT ".access"
#define LINE_READ 120


#define ACL_ROW "^[[:blank:]]*[[:alnum:]][[:alnum:]]*[[:blank:]][[:blank:]]*[rwb][[:blank:]]*\n*$"
#define ACL_ROW_B_NAME "^[[:blank:]]*"
#define ACL_ROW_A_NAME "[[:blank:]][[:blank:]]*[rwb][[:blank:]]*\n*$"




/*
 * **************  char *createAclPath(char *source, char *aclFile) *************
 * create path of the acl file.
 *************************************************************/
char *createAclPath(char *source, char *aclFile) {
    strcpy(aclFile, source);
    strcat(aclFile, ACL_EXT);
    return aclFile;
}

/*
 * ***  bool createNameRegex(char* name, regex_t* regex) *************
 * creates a regex to search for a name in the acl file.
 * Return true if successful and false if not.
 *************************************************************/
bool createNameRegex(char* name, regex_t* regex) {
    char nameRegex[strlen(ACL_ROW_B_NAME) + strlen(name) + strlen(ACL_ROW_A_NAME)];
    strcpy(nameRegex, ACL_ROW_B_NAME);
    strcat(nameRegex, name);
    strcat(nameRegex, ACL_ROW_A_NAME);
    return compileRegex(nameRegex, regex);
}

/*
 * **************  createFormatRegex(regex_t *regex) *************
 * creates a format regex to check the format of the acl file.
 * Return true if successful and false if not successful.
 *************************************************************/
bool createFormatRegex(regex_t *regex) {
    return compileRegex(ACL_ROW, regex);
}

bool readline(int fd, char* buffer) {
    int i = 0;
    ssize_t rd;
    do {
        if ( i > LINE_READ) exit(1);
        if ((rd = read(fd, &buffer[i], 1)) == -1) exit(1);
    } while(buffer[i++] != '\n');
    buffer[i] = '\0';
    if (!rd) return true;
    return false;
}


/*
 * ********* bool checkAclFile(int fd, regex_t* regex) ***
 * Checks the format of the acl file. Return true if in correct
 * format and false if not.
 *************************************************************/
bool checkAclFile(int fd, regex_t* regex) {
    char buffer[LINE_READ];
    while (!readline(fd, buffer)) {
        if (buffer[0] == '#') continue;
        if (!checkMatch(buffer, regex)) {
            if (DEBUG) printf("failed on this line:\n%s\n", buffer);
            lseek(fd, 0, SEEK_SET);
            return false;
        }
    }
    lseek(fd, 0, SEEK_SET);
    return true;
}

/*
 * **************char getAccessType(char* buffer) *************
 * Returns r | w | b char that is in a particular line of the acl.
 * buffer contains a line from acl file we want to get access type
 * for.
 *************************************************************/
char getAccessType(char* buffer) {
    int i;
    for ( i = (int) strlen(buffer) - 1; i >= 0 && (isblank(buffer[i]) || buffer[i] == '\n'); i--);
    return buffer[i];
}

/*
 * ******** char searchName(int fd, regex_t* regex) *************
 * searches for regex (compiled with createNameRegex) in the file
 * then returns the access type of the match. If no match return
 * 'f'.
 *************************************************************/
char searchName(int fd, regex_t* regex) {

    char buffer[LINE_READ];
    while (!readline(fd, buffer)) {
        if (buffer[0] == '#') continue;
        if (checkMatch(buffer, regex)) {
            if (DEBUG) printf("match on:\n%s", buffer);
            lseek(fd, 0, SEEK_SET);
            return getAccessType(buffer);
        }
        if (DEBUG) printf("no match:\n%s", buffer);
    }
    lseek(fd, 0, SEEK_SET);
    return 'f';
}

