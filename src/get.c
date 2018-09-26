/*
 * Author: John Karasev
 *
 * This file contains the get method that performs all the checking before
 * copying the source file. It also has the copy method that copies source to destination.
 */


#include "include/get.h" //TODO: fix this.
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <getregex.h>
#include <acl.h>
#include <pwd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pwd.h>
#include <regex.h>
#include <debug.h>
#include <get.h>
#include <fcntl.h>

#define LINE_READ 120

/*
 * **************  uid_t getOwner(char *file) *************
 * Returns the uid_t of owner. If error exit.
 *************************************************************/
uid_t getOwner(char *file) { //take the stat buffer and save it into somewhere, like the info page.
    struct stat buf;
    if (stat(file, &buf) == -1) {
        dperror("failed stat on", file); 
        exit(0);
    }
    return buf.st_uid;
}

/*
 * *************  mode_t getMode(char *file)   *****************
 * Get the permissions of file. If error, exit.
 *************************************************************/
mode_t getMode(char *file) {
    struct stat buf;
    if (lstat(file, &buf) == -1) {
        dperror("failed lstat on", file); 
        exit(1);  //TODO : on all errors exit
    }
    return buf.st_mode;
}

/*
 * ****************  bool isReg(char *file) *****************
 * Check if file is a regular file. If error exit.
 * Return true if regular, false if not.
 *************************************************************/
bool isReg(char *file) {
    struct stat buf;
    if (lstat(file, &buf) == -1) {
        dperror("failed lstat on", file); 
        return false; 
    }
    return S_ISREG(buf.st_mode); //TODO check is this checks for symlinks.
}

/*
 * *************   bool exists(char *file)   *****************
 * return true if file exists. Otherwise return true.
 *************************************************************/
bool exists(char *file) {
    return (access(file, F_OK) == 0);
}

/*
 * **************  canWrite(char *file)   *****************
 * Check if current effective uid can write to file.
 * Return true if can write, else return false.
 *************************************************************/
bool canWrite(char *file) {
    return (access(file, W_OK) == 0);
}


/*
 * *********  bool destAccess(char *dest)   *****************
 * Return true if current effective user of process can write
 * to the dest file. If file does not exist, check if user
 * can write in the dir of the file. Assumes the file paths
 * are in the unix path format.
 *
 * Return true if euid can write to file if it exists,
 *  else return false.
 * Return true if euid can write to directory if
 *  file does not exist, else return false.
 *************************************************************/
bool destAccess(char *dest) {

    // first check if the file exists.
    // if so determine if euid can write to it.
    if (exists(dest)) {
        if (DEBUG) printf("file %s already exists\n", dest);
        return canWrite(dest);
    }

    //find the dir in which the file will be located.
    //loop until a '/' is reached or end of path.
    for (int i = (int) strlen(dest) - 1; i >= 0; i--) {
        if (dest[i] == '/') {                // if '/' copy all contents before including '/'
            char dir[i];                     // assuming os uses the unix form of file paths.
            strncpy(dir, dest, i);
            dir[i] = '\0';
            if (DEBUG) printf("checking if \"%s\" can be written to by ruid\n", dir);
            bool result = canWrite(dir);
            if (DEBUG) printf("%s can be written by ruid?: %d\n", dir, result);
            return canWrite(dir);            // check if user can write to the dir.
        }
    }
    // if looped and no dir specified in path use current working dir.
    return canWrite(".");
}

char *nameFromUid(uid_t id) {
    struct passwd *pws;
    pws = getpwuid(id);
    char *name = (char *) malloc(strlen(pws->pw_name));
    return strcpy(name, pws->pw_name);
}

/*
 * ******************   bool get(UIDINFO*)   *****************
 * Checks if conditions are met for the real uid to read source file.
 * Accepts pointer to UIDINFO that contains source path, destination
 * path, acl path, effective uid, and real uid. Returns True if conditions for
 * reading are satisfied and false if not.
 * Specifically get checks for:
 *   1. source file exists -
 *   2. acl file exists -
 *   3. source is owned by the effective uid of the process (at the start of process) -
 *   4. acl is owned by euid. -
 *   5. acl does not have any group and other privelages -
 *   6. acl file is not a symbolic link
 *   7. source file is an ordinary file (sourceFile)
 *   8. effective uid has read access to source and acl.
 *   9. acl (aclFile) file is in correct format (acl.c and getregex.c)
 *  10. the real uid (ruid) has read access to destination (destFile)
 *  11. acl file indicates read access for ruid.
 **********************************************************************/

//TODO catch errors on seteuid and all other system calls.
bool get(UIDINFO *info) {



    // The first set of checks need effective uid privileges
    if(seteuid(info->effective) == -1) {
        euiderr();
        exit(0);
    }

    /*
     * 1. Source file exists?
     */
    if (!exists(info->sourceFile)) {
        accessdeny("source file does not exist");
        return false;
    }

    /*
     * 2. Acl file exists?
     */
    if (!exists(info->aclFile)) {
        accessdeny("acl file does not exist");
        return false;
    }

    if(seteuid(info->real) == -1) {
        euiderr();
        exit(0);
    }


    /*
     * 3. Source is owned by effective uid of process?
     */
    if (info->euid_source_stat->st_uid != info->effective) {
        accessdeny("source file not owned by effective user");
        return false;
    }

    /*
     * 4. Acl file is owned by effective uid?
     */
    if (info->euid_acl_stat->st_uid != info->effective) {
        accessdeny("acl file not owned by effective file");
        return false;
    }

    /*
     * 5. No acl file privileges are granted to group or other?
     */
    if (info->euid_acl_stat->st_mode & (S_IRWXG | S_IRWXO)) {
        accessdeny("other or group have access to this file");
        return false;
    }

    /*
     * 6. Acl is not a symlink?
     */
    if (!S_ISREG(info->euid_acl_stat->st_mode)) {
        accessdeny("acl is not a regular file");
        return false;
    }

    /*
     * 7. Source file is a ordinary file?
     */
    if (!S_ISREG(info->euid_source_stat->st_mode)) {
        accessdeny("source file is not a regular file");
        return false;
    }

    /*
     * 8. Effective uid can read source?
     */
    if (!(info->euid_source_stat->st_mode & S_IRUSR)) {
        accessdeny("effective user cannot read the file");
        return false;
    }

    /*
     * 8. Effective uid can read acl?
     */
    if (!(info->euid_acl_stat->st_mode & S_IRUSR)) {
        accessdeny("effective user cannot read the file");
        return false;
    }

    // Create a regex to check the format of acl file.
    regex_t regex;

    if (!createFormatRegex(&regex)) {
        accessdeny("failed to compile format regex");
        return false;
    }

    /*
     * 9. Acl file in correct format?
     */
    if (!checkAclFile(info->aclfd, &regex)) {
        accessdeny("incorrect acl format");
        return false;
    }

    regfree(&regex);

    /*
     * 10. ruid has read access to destination file?
     */
    if (!destAccess(info->destFile)) {
        accessdeny("real user does not have permission to create or write destination file");
        return false;
    }
    //get username of ruid.
    char *name = nameFromUid(info->real);

    //create regex to search for name in acl file.
    if (!createNameRegex(name, &regex)) {
        free(name);
        accessdeny("failed to compile regex for searching");
        return false;
    }
    free(name);

    /*
     * 11. acl indicates read access for ruid?
     */
    char access = searchName(info->aclfd, &regex);

    regfree(&regex);

    
    switch (access) { 
        case 'f':
        case 'w': 
            if (DEBUG) printf("ACCESS DENIED: user \"%s\" does not have correct permissions to read the file\n", name);
            return false;  
        case 'b':
        case 'r':
            break;
        default: 
            if (DEBUG) errormesg("no match on access type from acl file"); 
            return false; 
    }

    return true;
}


/*
 * ******************   bool copy(UIDINFO*)   *****************
 * Given two file pointers, copies one stream into the other.
 * If fail return false.
 *************************************************************/
void copyfile(int sfd, int dfd) { //TODO use read, write, open functions.
    char buffer[LINE_READ];
    ssize_t rr, wr;
    while ((rr = read(sfd, buffer, LINE_READ))) {
        if (rr == -1) exit(1);
        wr = write(dfd, buffer, rr);
        if (wr == -1) exit(1);
    }
}


/*
 * ******************   bool copy(UIDINFO*)   *****************
 * Copies source file to destination. Returns true on success
 * and false on failure. If destination file already exists, user
 * is queried before overwriting the file.
 *************************************************************/
bool copy(UIDINFO *info) {

    // if file exists, query user if it is okay to overwrite it.
    if (exists(info->destFile)) {
        char response[2];
        while (true) {
            printf("File \"%s\" already exists. Do you want to overwrite it? [y/n] ", info->destFile);
            if (scanf("%s", response) == 1 && (!strcmp(response, "y") || !strcmp(response, "n"))) {
                break;
            }
        }
        if (!strcmp(response, "n")) exit(0);
    }

    int dfd = open(info->destFile, O_WRONLY | O_CREAT | O_TRUNC, info->euid_source_stat->st_mode);

    if (dfd == -1) { //check if there was a success opening both file pointers.
        errormesg("failed to get file descriptor");
        return false;
    }

    // copy the file over.
    copyfile(info->sourcefd, dfd);

    close(dfd);

    if (DEBUG) printf("File copied\n");

    return true;

}
