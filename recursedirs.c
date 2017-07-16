/*% gcc -g -Wall -O0 % -o #
 *
 * recursedirs.c- recursive directory list
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

struct thedirptr {
  char name[256];
  char path[256];
  int type;			/* 0=file,1=directory,2=link */

  struct thedirptr * left;
  struct thedirptr * right;
};
typedef struct thedirptr DIRPTR;
typedef DIRPTR * DPTR;

/*------------------------------------------------------------------------------
 * main()-
 * given a directory, do a recursive listing of the directory
 *------------------------------------------------------------------------------
 */
int
main (int argc, char ** argv)
{

  return 0;
}

/*------------------------------------------------------------------------------
 * listdir()
 *------------------------------------------------------------------------------
 */
void
listdir()
{
}

/*------------------------------------------------------------------------------
 * 
 *------------------------------------------------------------------------------
 */

/* EOF: recursedirs.c
 */
