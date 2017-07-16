/* utils.c- for the xasd2db2 project
 * use with minstd.c, not with fullstd.c
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern char * pgmname;		/* this program's name */

/* prototypes:
 */
char * logtime (void);
time_t getTime (char * in_time);
char * mkEastern (char * in_time);
char * getfile (char * filename);

void process_ini(char * inifilename);
static void get_ini_entry (char * inibuffer, char * ini_entry, char * ini_target);
static void ini_error (char * ini_entry);

/*-----------------------------------------------------------------------------
 * logtime()- returns "YYYYMMDD HHMMSS" from current time
 *-----------------------------------------------------------------------------
 */
char *
logtime (void)
{
  time_t t;			/* for the current time */
  struct tm theTime;		/* for logging */
  static char logtime[17];	/* space for the current date and time */

  t = time (NULL);
  memcpy ((char *) &theTime, localtime (&t), sizeof (theTime));
  (void) sprintf (logtime, "%d%02d%02d %02d%02d%02d",
		  theTime.tm_year +1900,
		  theTime.tm_mon + 1,
		  theTime.tm_mday,
		  theTime.tm_hour,
		  theTime.tm_min,
		  theTime.tm_sec);
  return logtime;
}

/*-----------------------------------------------------------------------------
 * getTime()- convert a timestring (YYYY-MM-DD HH:MM:SS-05) to time in seconds
 *-----------------------------------------------------------------------------
 */
time_t
getTime (char * in_time)
{
  struct tm gtime;		/* given time structure */
  static time_t timeinsecs;	/* time in seconds */
  
  gtime.tm_isdst = 0;
  if (strchr (in_time, 'Z') == NULL)
    {
      putenv("TZ=US/Eastern");
      if (strcmp (in_time + 19, "-04") == 0)
	gtime.tm_isdst = 1;
    }
  else
    putenv("TZ=UTC");
  
  gtime.tm_year = atoi (in_time) - 1900;
  gtime.tm_mon  = atoi (in_time + 5) - 1;
  gtime.tm_mday = atoi (in_time + 8);
  gtime.tm_hour = atoi (in_time + 11);
  gtime.tm_min  = atoi (in_time + 14);
  gtime.tm_sec  = atoi (in_time + 17);

  timeinsecs = mktime (&gtime);
  if (timeinsecs == -1)
    {
      fprintf (stderr, "%s: getTime() error: %s\n", pgmname, strerror (errno));
      return 0;
    }

    timeinsecs = mktime (&gtime);
  return timeinsecs;
} /* end getTime() */

/*-----------------------------------------------------------------------------
 * mkEastern()- convert given modified Zulu timestring (YYYY-MM-DD HH:MM:SS-00)
 * to EST (YYYY-MM-DD HH:MM:SS-05)
 *-----------------------------------------------------------------------------
 */
char *
mkEastern (char * in_time)
{
  struct tm gtime;		/* given time structure */
  struct tm * rtime;		/* result time structure */
  time_t timeinsecs;		/* time in seconds */
  static char outbuf[32];	/* new time string */
  
  gtime.tm_isdst = 0;
  if (strchr (in_time, 'Z') == NULL)
    {
      putenv("TZ=US/Eastern");
      if (strcmp (in_time + 19, "-04") == 0)
	gtime.tm_isdst = 1;
    }
  else
    putenv("TZ=UTC");

  gtime.tm_year = atoi (in_time) - 1900;
  gtime.tm_mon  = atoi (in_time + 5) - 1;
  gtime.tm_mday = atoi (in_time + 8);
  gtime.tm_hour = atoi (in_time + 11);
  gtime.tm_min  = atoi (in_time + 14);
  gtime.tm_sec  = atoi (in_time + 17);

  timeinsecs = mktime (&gtime);
  if (timeinsecs == -1)
    {
      fprintf (stderr, "%s: getTime() error: %s\n", pgmname, strerror (errno));
      return 0;
    }

  putenv("TZ=US/Eastern");
  rtime = localtime (&timeinsecs);
  (void) snprintf (outbuf, sizeof (outbuf), "%d-%02d-%02d %02d:%02d:%02d-%02d",
		   rtime->tm_year + 1900,
		   rtime->tm_mon + 1,
		   rtime->tm_mday,
		   rtime->tm_hour,
		   rtime->tm_min,
		   rtime->tm_sec,
		   (int) (abs (rtime->tm_gmtoff/3600)));
  return outbuf;
} /* end mkEastern() */

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
  
  result = stat (filename, &fstat);
  if (result != 0)
    {
      fprintf (stderr, "%s: getfile(): couldn't stat() %s: %s\n",
	       pgmname, filename, strerror (errno));
      return NULL;
    }

  buf = malloc (fstat.st_size + 1);
  if (buf == NULL)
    {
      fprintf (stderr, "%s: getfile(): couldn't malloc() %ld bytes: %s\n",
	       pgmname, fstat.st_size, strerror (errno));
      return NULL;
    }

  inpfile = fopen (filename, "r");
  if (inpfile == NULL)
    {
      fprintf (stderr, "%s: getfile(): couldn't fopen() %s: %s\n",
	       pgmname, filename, strerror (errno));
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
void
process_ini(char * inifilename)
{
  char lclini[256];		/* local copy of the ini file name */
  char * iniptr;		/* ptr to ini file in memory */
  char * s;			/* general char ptr */
  char * t;			/* general char ptr */

  /* snag the .ini file, if we have it somewhere:
   * .ini file has entries like the following:
   * # comments
   * (blank lines)
   * key "value"
   *
   * example:

   # comment
   database_name "host = hostname dbname = database_name"
   logfilepath "/data/rtca-logs/xasd2db2"

   */

  /* first, try the current directory:
   */
  strcpy (lclini, inifilename);
  iniptr = getfile (lclini);
  if (iniptr == NULL)		/* couldn't find it in current directory, try pgm directory */
    {
      strcpy (inipath, pgmpath);
      strcat (inipath, inifilename);
      iniptr = getfile (inipath);
      if (iniptr == NULL)	/* couldn't find it where the program lives. Try /etc: */
	{
	  strcpy (inipath, "/etc/");
	  strcat (inipath, inifilename);
	  iniptr = getfile (inipath);
	  if (iniptr == NULL)	/* not in /etc, try /usr/local/etc: */
	    {
	      strcpy (inipath, "/usr/local/etc/");
	      strcat (inipath, inifilename);
	      iniptr = getfile (inipath);
	      if (iniptr == NULL)	/* not in /usr/local/etc: */
		{
		  fprintf (logfile, "%s: %s unavailable. Using defaults\n",
			   logtime(), inifilename);
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
      memmove (s, t, strlen (t) + 1);
      s = strchr (iniptr, '#');
    }

#if 0
  /* SAMPLE: init the connection strings:
   */
  connect_rtca_db[0] = '\0';
  connect_tracks_now[0] = '\0';
  connect_all_etas[0] = '\0';  

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

  s = strchr (s, '"');
  if (s == NULL)
    {
      ini_error (ini_entry);
      return;
    }

  ++s;
  t = strchr (s, '"');
  if (t == NULL)
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

/* EOF: utils.c
 */
