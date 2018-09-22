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

    info.effective = geteuid();
    info.real = getuid();
    info.aclFile = createAclPath(argv[1], acl);
    info.sourceFile = argv[1];
    info.destFile = argv[2];

    if (DEBUG) printf("real uid: %d effective uid: %d\n", info.real, info.effective);
 
    if(seteuid(info.real) == -1) {
        euiderr();
        if(DEBUG) perror("ERROR:"); 
        exit(0);
    }

    if (!get(&info)) {
        exit(0);
    }

    if (!copy(&info)) {
        exit(0);
    }

    return 0;
}
