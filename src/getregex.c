/*
 * Author: John Karasev
 *
 * contains functions that deal with compiling regex that are
 * used when checking acl file and searching for names in
 * the acl file.
 */

#include <stdio.h>
#include <stdbool.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>


/*
 * ****** bool compileRegex(char* pattern, regex_t *regex) *************
 * compiles the pattern into a regex. Return true on success and false on
 * failure.
 *************************************************************/
bool compileRegex(char* pattern, regex_t *regex) {
    int res;
    res = regcomp(regex, pattern, REG_ICASE);
    if (res) {
        return false;
    }
    if (DEBUG) printf("compiled %s successfully\n", pattern);
    return true;
}

/*
 * ********  bool checkMatch(char* buffer, regex_t *regex) *************
 * check match of regex in the line that is located in buffer.
 * Return true if match and false if no match.
 *************************************************************/
bool checkMatch(char* buffer, regex_t *regex) {
    if (regexec(regex, buffer, 0, NULL, 0) == REG_NOMATCH) return false;
    return true;
}