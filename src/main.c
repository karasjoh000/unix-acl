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


/*
 * At the beginning of the program all the effective users resources that are needed to perform checks
 * are stored into the UIDINFO struct.
 */

 /*
  * The main function performs the first checks like if acl and source exist and if effective user can read them.
  * This is checked by attempting to get a fd from open function on read access.
  */


int main(int argc, char **argv) {

    if (argc != 3) exit(0);


    /*
     *  create a UIDINFO struct and put all the resources there like fd's and stat structs.
     */
    char acl[strlen(argv[1]) + strlen(ACL_EXT)];

    UIDINFO info;
    struct stat aclb, sourceb;


    info.effective = geteuid();
    info.real = getuid();

    if (DEBUG) printf("real uid: %d effective uid: %d\n", info.real, info.effective);

    info.aclFile = createAclPath(argv[1], acl);
    info.sourceFile = argv[1];

    info.euid_source_stat = &sourceb;
    info.euid_acl_stat = &aclb;
    //get the stat structs

     // if error then files might not exist.
    if (lstat(info.aclFile, info.euid_acl_stat) == -1 || lstat(info.sourceFile, info.euid_source_stat) == -1) {
        errormesg("failed to retrieve stat bug");
        exit(1);
    }


    info.aclfd = open(info.aclFile, O_RDONLY);
    info.sourcefd = open(info.sourceFile, O_RDONLY);

    //switch to real.
    if(seteuid(info.real) == -1) {
        euiderr();
        if(DEBUG) perror("ERROR:");
        exit(1);
    }

    // open is successful ONLY if source and acl exist and effective has read access to them.
    if (info.aclfd == -1 || info.sourcefd == -1)
        exit(1);

    info.destFile = argv[2];


    if (!get(&info)) {  // perform all the checks in the get method
        exit(1);
    }

    if (!copy(&info)) { //if checks in get are successful copy the file.
        exit(1);
    }

    //release resources stored in struct.
    close(info.sourcefd);
    close(info.aclfd);

    return 0;
}
