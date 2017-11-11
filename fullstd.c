/*% gcc -g -Wall -O0 % -o #
 *
 * fullstd.c- full-sized version of std.c
 * this one uses a few of the functions normally found in the utils.c
 * module, they are included here for simplicity.
 *
 */
#include <stdio.h>
#include <string.h>

char * pgmname;			/* this program's name */
char pgmpath[256];		/* where this program lives */
char ininame[64];		/* what the .ini file is called */
char inipath[256];		/* where the .ini file lives */
char logname[64];		/* what the log file is called */
char logpath[256];		/* where the log file lives */

char * getfile (char * filename);

FILE * logfile;

static void process_ini(char * inifilename);
static void get_ini_entry (char * inibuffer, char * ini_entry, char * ini_target);
static void ini_error (char * ini_entry);

char * getfile (char * filename);
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
  if ((pgmname = strrchr (argv[0], '/')) == NULL)
    pgmname = argv[0];
  else
    ++pgmname;

  if ((s = strstr (pgmname, ".exe")) != NULL)
    *s = '\0';			/* for Windows-based systems */

  /* separate out the path:
   */
  strcpy (pgmpath, argv[0]);
  if ((s = strrchr (pgmpath, '/')) != NULL)
    *(s + 1) = '\0';		/* leave the trailing '/' */
  else
    strcpy (pgmpath, "./");	/* set the program path to here */

  strcpy (logname, pgmname);
  strcat (logname, ".log");

  strcpy (ininame, pgmname);
  strcat (ininame, ".ini");
  process_ini (ininame);

  return 0;
} /* end main() */

/*-----------------------------------------------------------------------------
 * getfile()- returns pointer to given file read into memory.
 * Be sure to free this when done!
 *-----------------------------------------------------------------------------
 */
char *
getfile (char * filename)
{
  struct stat fstat;		/* so we can get the file size */
  FILE * inpfile;		/* input file handle */
  int result;			/* for tracking results */
  char * buf;			/* where the file goes */
  
  if ((result = stat (filename, &fstat)) != 0)
    {
      fprintf (stderr, "%s: getfile(): couldn't stat() %s: %s\n", pgmname, filename, strerror (errno));
      return NULL;
    }

  if ((buf = calloc (fstat.st_size + 1, 1)) == NULL)
    {
      fprintf (stderr, "%s: getfile(): couldn't calloc() %ld bytes: %s\n", pgmname, fstat.st_size, strerror (errno));
      return NULL;
    }

  if ((inpfile = fopen (filename, "r")) == NULL)
    {
      fprintf (stderr, "%s: getfile(): couldn't fopen() %s: %s\n", pgmname, filename, strerror (errno));
      free (buf);
      return NULL;
    }

  (void) fread (buf, 1, fstat.st_size, inpfile);
  (void) fclose (inpfile);

  return buf;		     
}

/*-----------------------------------------------------------------------------
 * process_ini()- fill in configurable variables from the given ini file;
 * searches the current directory, then the program's directory, for the
 * given ini filename.
 *
 * USAGE: copy and paste this where needed, customize as needed.
 * Make sure to get the get_ini_entry() and ini_error() functions as well.
 *-----------------------------------------------------------------------------
 */
static void
process_ini(char * inifilename)
{
  char * iniptr;		/* ptr to ini file in memory */
  char * s;			/* general char ptr */
  char * t;			/* general char ptr */

  /* snag the .ini file, if we have it somewhere:
   * .ini file has entries like the following:
   * # comments
   * (blank lines)
   * database_name "host = hostname dbname = database_name"
   * logfilepath "/data/rtca-logs/xasd2db2"
   * key "value string"  # more comments
   */

  /* first, try the current directory:
   */
  strcpy (inipath, inifilename);
  if ((iniptr = getfile (inipath)) == NULL) /* couldn't find it in the current directory, try the program directory */
    {
      strcpy (inipath, pgmpath);
      strcat (inipath, inifilename);
      if ((iniptr = getfile (inipath)) == NULL)	/* couldn't find it where the program lives. Try /etc: */
	{
	  strcpy (inipath, "/etc/");
	  strcat (inipath, inifilename);
	  if ((iniptr = getfile (inipath)) == NULL) /* not in /etc, try /usr/local/etc: */
	    {
	      strcpy (inipath, "/usr/local/etc/");
	      strcat (inipath, inifilename);
	      if ((iniptr = getfile (inipath)) == NULL)	/* not anywhere. Complain, use defaults */
		{
		  fprintf (logfile, "%s: %s unavailable. Using defaults\n", logtime(), inifilename);
		  fflush (logfile);
		  return;
		}
	    }
	}
    }
  
  /* zap all the comments, remove them entirely:
   */
  s = strchr (iniptr, '#');
  while (s != NULL)
    {
      t = strchr (s, '\n');	/* point to end of line */
      memmove (s, t + 1, strlen (t));
      s = strchr (iniptr, '#');
    }

#if 0
  /* SAMPLE: init the entry strings:
   */
  connect_rtca_db[0] = '\0';
  connect_tracks_now[0] = '\0';
  connect_all_etas[0] = '\0';
  logfilepath[0] = '\0';

  /* SAMPLE: how to use get_ini_entry():
   */
  get_ini_entry (iniptr, "rtca_db", connect_rtca_db);
  get_ini_entry (iniptr, "tracks_db_now", connect_tracks_now);
  get_ini_entry (iniptr, "all_etas", connect_all_etas);
  get_ini_entry (iniptr, "logfilepath", logfilepath);
#endif	/* 0 */

  free (iniptr);
} /* end process_ini() */

/*-----------------------------------------------------------------------------
 * get_ini_entry()
 * search for an ini entry in given inibuffer, and copy to ini_target
 *-----------------------------------------------------------------------------
 */
static void
get_ini_entry (char * inibuffer, char * ini_entry, char * ini_target)
{
  char * s;			/* general char ptr */
  char * t;			/* general char ptr */

  if (!inibuffer || !ini_entry || !ini_target)
    return;			/* error, missing a pointer */

  if ((s = strstr (inibuffer, ini_entry)) == NULL)
    return;			/* not here, do nothing */

  if ((s = strchr (s, '"')) == NULL)
    {
      ini_error (ini_entry);
      return;
    }

  ++s;
  if ((t = strchr (s, '"')) == NULL)
    {
      ini_error (ini_entry);
      return;
    }
	  
  *t = '\0';
  strcpy (ini_target, s);
  *t = '"';
} /* end get_ini_entry() */

/*-----------------------------------------------------------------------------
 * ini_error()- complain bitterly about missing double-quotes in .ini file
 *-----------------------------------------------------------------------------
 */
static void
ini_error (char * ini_entry)
{
  fprintf (stderr, "%s: malformed %s.ini file- at %s\n", pgmname, pgmname, ini_entry);
  fprintf (stderr, "entries should be in the form:\n");
  fprintf (stderr, "key \"value\"\n");
  fprintf (stderr, "(missing a '\"')\n");
  fprintf (stderr, "ignoring entry\n");
}

/*-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 */

/* EOF: fullstd.c
 */
