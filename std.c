/*% gcc -g -Wall -O0 % -o #
 *
 */

#include <stdio.h>
#include <string.h>

char * pgmname;			/* this program's name */

/*--------------------------------------------------------
 * main()
 *--------------------------------------------------------
 */
int
main (int argc, char ** argv)
{
  if ((pgmname = strrchr (argv[0], '/')) == NULL)
    pgmname = argv[0];
  else
    ++pgmname;

  /* your code here:
   */

  /* say all OK:
   */
  return 0;
}

/*--------------------------------------------------------
 * 
 *--------------------------------------------------------
 */

/* EOF: std.c
 */
