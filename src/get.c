//
// Created by John Karasev on 9/21/18.
//

#include "include/get.h"

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



/***************************
  Checks if ACL file exists,
           has malformed entries,
	   has empty lines,
	   is a symbolic link
	   is avialable to any world or group,
	   or if .ext file is not ordinary.
• source is owned by the effective uid of the
  executing process,
• the effective uid of the executing process has
  read access to source,
• the file source.access exists and indicates read
  access for the real uid of the executing process,
• and the real uid of the executing process can
  write the file destination.
  if any above are true then no access is provided.

**************************************************/
#define LINE_READ 120 //what if there are a lot of whitespaces. Fixed line read will not work need to check char by char.
#define ACL_EXT ".access"
#define ACL_REGEX "^[[:blank:]]*[[:alnum:]]*[[:blank:]]*[rwb][[:blank:]]*\n*$"


uid_t getOwner(char *file) {
    struct stat buf;
    if (stat(file, &buf) == -1) {
        dperror("failed stat on", file); 
        return false; 
    }
    return buf.st_uid;
}


mode_t getMode(char *file) {
    struct stat buf;
    if (lstat(file, &buf) == -1) {
        dperror("failed lstat on", file); 
        return false; 
    }
    return buf.st_mode;
}

bool isReg(char *file) {
    struct stat buf;
    if (lstat(file, &buf) == -1) {
        dperror("failed lstat on", file); 
        return false; 
    }
    return S_ISREG(buf.st_mode);
}

//false if anyone can access
bool checkAccess(char *file) {
    /*
     * removed:
     *     struct stat buf;
     *     if (lstat(file, &buf) == -1) exit(1); */
    if (getMode(file) & (S_IRWXG | S_IRWXO)) return false;
    return true;
}

bool exists(char *file) {
    return (access(file, F_OK) == 0) ? true : false;
}

bool canRead(char *file) {
    return (getMode(file) & S_IRUSR) ? true : false;
}

bool canWrite(char *file) {
    return (getMode(file) & S_IWUSR) ? true : false;
}

bool destAccess(char *dest) {
    if (canWrite(dest)) return true; // if file already exists
    for (int i = (int) strlen(dest) - 1; i <= 0; i--) {
        if (dest[i] == '/') {
            char dir[i];
            strncpy(dir, dest, i);
            dir[i] = '\0';
            return canWrite(dir);
        }
    }
    return canWrite(".");
}

char *nameFromUid(uid_t id) {
    struct passwd *pws;
    pws = getpwuid(id);
    char *name = (char *) malloc(strlen(pws->pw_name));
    return strcpy(name, pws->pw_name);
}

//TODO change this to read write open to enable exec files.
bool copyfile(FILE *source, FILE *dest) {
    char buffer[LINE_READ];
    while (fgets(buffer, LINE_READ, source))
        if (fprintf(dest, "%s", buffer) < 0) return false;
    return true;
}

//TODO catch errors on seteuid and all other system calls.
bool get(UIDINFO *info) {

    if(seteuid(info->effective) == -1) {
        euiderr();
        exit(0);
    }


    if (!exists(info->sourceFile)) {
        accessdeny("source file does not exist");
        return false;
    }

    if (!exists(info->aclFile)) {
        accessdeny("acl file does not exist");
        return false;
    }

    if (getOwner(info->sourceFile) != info->effective) {
        accessdeny("source file not owned by effective user");
        return false;
    }

    if (getOwner(info->aclFile) != info->effective) {
        accessdeny("acl file not owned by effective file");
        return false;
    }

    if (!checkAccess(info->aclFile)) {
        accessdeny("other or group have access to this file");
        return false;
    }

    if (!isReg(info->aclFile)) {
        accessdeny("acl is not a regular file");
        return false;
    }

    if (!isReg(info->sourceFile)) {
        accessdeny("source file is not a regular file");
        return false;
    }

    if (!canRead(info->sourceFile)) {
        accessdeny("effective user cannot read the file");
        return false;
    }

    regex_t regex;

    if (!createFormatRegex(&regex)) {
        accessdeny("failed to compile format regex");
        return false;
    }

    if (!checkAclFile(info->aclFile, &regex)) {
        accessdeny("incorrect acl format");
        return false;
    }

    regfree(&regex);


    if(seteuid(info->real) == -1) {
        euiderr();
        exit(0);
    }


    if (!destAccess(info->destFile)) {
        accessdeny("real user does not have permission to create or write destination file");
        return false;
    }


    char *name = nameFromUid(info->real);

    regex_t regex1;

    if (!createNameRegex(name, &regex1)) {
        accessdeny("failed to compile regex for searching");
        return false;
    }

    if(seteuid(info->effective) == -1) {
        euiderr();
        exit(0);
    }

    char access = searchName(info->aclFile, &regex1);

    if(seteuid(info->real) == -1) {
        euiderr();
        exit(0);
    }

    regfree(&regex1);

    
    switch (access) { 
        case 'f':
        case 'w': 
            if (DEBUG) printf("ACCESS DENIED: user \"%s\" does not have correct permissions to read the file\n", name);
            return false;  
        case 'b':
        case 'r':
            return true; 
        default: 
            if (DEBUG) errormesg("no match on access type from acl file"); 
            return false; 
    }

    free(name);

    return true;
}

//TODO change to read write open to enable execs or test if it works with execs.
bool copy(UIDINFO *info) {

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

    if(seteuid(info->effective) == -1) {
        euiderr();
        exit(0);
    }

    FILE *sfp = fopen(info->sourceFile, "rb");

    if(seteuid(info->real) == -1) {
        euiderr();
        exit(0);
    }

    FILE *dfp = fopen(info->destFile, "wb");

    if (!(sfp && dfp)) {
        errormesg("failed to get file pointer(s)");
        return false;
    }

    if (!copyfile(sfp, dfp)) {
        errormesg("failed to copy source to destination");
        return false;
    }

    fclose(dfp);

    fclose(sfp);

    if (DEBUG) printf("File copied\n");

    return true;

}
