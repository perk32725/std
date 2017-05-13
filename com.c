/*% gcc -g -Wall -O0 % -o #
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

char * pgmname;			/* this program's name */

/*--------------------------------------------------------
 * main()
 *--------------------------------------------------------
 */
int
main (int argc, char ** argv)
{
  char inpline[1024];	/* input buffer */
  char cmdline[1024];	/* command line */
  char * s;		/* general char ptr */
  char * t;		/* general char ptr */
  char * u;		/* general char ptr */
  FILE * inpfile;	/* incoming file */

  if ((pgmname = strrchr (argv[0], '/')) == NULL)
    pgmname = argv[0];
  else
    ++pgmname;

  if (argc != 2)
    {
      printf ("%s: usage: %s c-file\n", pgmname, pgmname);
      return 1;
    }

  /* open given file:
   */
  inpfile = fopen (argv[1], "r");
  if (inpfile == NULL)
    {
      printf ("%s: couldn't fopen() %s: %s\n", pgmname, argv[1], strerror (errno));
      return 1;
    }

  /* init:
   */
  memset (inpline, '\0', sizeof (inpline));
  memset (cmdline, '\0', sizeof (cmdline));

  fgets (inpline, sizeof(inpline), inpfile);
  fclose (inpfile);

  /* check that first line:
   */
  if (memcmp (inpline, "/*%", 3) != 0)
    {
      printf ("%s: first line should start with '/*%%'\n", pgmname);
      return 1;
    }

  t = strchr (inpline, '\n');	/* zap NL char */
  if (t != NULL)
    *t = '\0';

  s = inpline + 4;		/* move past the first space */
  t = cmdline;			/* point to cmdline we're building */
  while (*s != '\0')
    {
      if (*s == '%')		/* '%' translates to the file name */
        {
	  strcat (t, argv[1]);
	  t = cmdline + strlen (cmdline);
	  ++s;			/* next char in input line */
	}

      if (*s == '#')		/* '#' translates to file name w/o extension */
        {
	  strcpy (t, argv[1]);
	  u = strchr (t, '.');	/* look for extension */
	  if (u != NULL)
	    {
	      *u = '\0';	/* zap extension */
	      t = u;		/* save ptr */
	    }
	  else
	    t = cmdline + strlen (cmdline);

	  ++s;			/* move source up again */
	}

      *t++ = *s++;		/* copy char to target */
    }

  return system (cmdline);	/* do whatever we've built */
}

/*--------------------------------------------------------
 * 
 *--------------------------------------------------------
 */

/* EOF: com.c
 */
