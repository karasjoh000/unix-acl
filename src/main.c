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



//TODO catch errors on seteuid and all other system calls.
int main(int argc, char **argv) {

    if (argc != 3) exit(0);



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

    if (lstat(info.aclFile, info.euid_acl_stat) == -1 || lstat(info.sourceFile, info.euid_source_stat) == -1)
        exit(1);

    info.aclfd = open(info.aclFile, O_RDONLY);
    info.sourcefd = open(info.sourceFile, O_RDONLY);

    //switch to real.
    if(seteuid(info.real) == -1) {
        euiderr();
        if(DEBUG) perror("ERROR:");
        exit(0);
    }

    if (info.aclfd == -1 || info.sourcefd == -1)
        exit(1);

    info.destFile = argv[2];


    if (!get(&info)) {
        exit(0);
    }

    if (!copy(&info)) {
        exit(0);
    }

    close(info.sourcefd);
    close(info.aclfd);

    return 0;
}
