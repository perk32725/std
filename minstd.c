/*% gcc -g -Wall -O0 % -o #
 *
 * minstd.c- minimal, standard C program
 *
 */
#include <stdio.h>
#include <string.h>

char * pgmname;			/* this program's name */
char pgmpath[256];		/* where this program lives */

/*-----------------------------------------------------------------------------
 * main()
 *-----------------------------------------------------------------------------
 */
int
main (int argc, char ** argv)
{
  char * s;			/* general char ptr */

  /* get this program's name:
   */
  pgmname = strrchr (argv[0], '/');
  if (pgmname == NULL)
    pgmname = argv[0];
  else
    ++pgmname;

  if ((s = strchr (pgmname, ".exe")) != NULL)
    *s = '\0';			/* for Windows-based systems */

  /* separate out the path:
   */
  strcpy (pgmpath, argv[0]);
  s = strrchr (pgmpath, '/');
  if (s)
    *(s + 1) = '\0';		/* leave the trailing '/' */
  else
    strcpy (pgmpath, "./");	/* set the program path to here */

} /* end main() */

/*-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 */

/* EOF: minstd.c
 */
