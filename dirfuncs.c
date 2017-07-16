/*% gcc -g -Wall -O0 % -o #
 *
 * dirfuncs.c: some simple directory functions for finding xasd files
 * - part of rtca/newasd project
 *
 * Usage: dirfuncs <bdir> <[YY]YY> <[M]M> <[D]D>\n", pgmname, pgmname);
 *  where:
 *  bdir is starting directory
 *  [YY]YY [M]M [D]D is year, month, and day
 *
 * REPURPOSED:
 *  given path now something like /var/www/html/data/asd2/YYYY/[M]M/[D]D/HH/mmss.raw
 *  alternate path is /home/data/asd2/YYYY/[M]M/[D]D/HH/mmss.raw
 *  we want to grab all the filenames in /var/www/html/data/asd2/YYYY/[M]M/[D]D/,
 *  visiting each HH directory.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

int listfiles (char * list, int list_size, char * sdir);
static int   procdir (char * list, int list_size, char * dirname);
static int   sortlist (char * list);
static int   getdircontents (char * list, int list_size, DIR * dirp);
static DIR * getdir (char * name, char * dirbuf, int dirbuf_sz);

/* set PRODUCTION to 1 to make this file into the xasd2db2 project.
 * Set to 0 for testing new functionality.
 */
#define PRODUCTION 0

#if PRODUCTION

extern char * pgmname;
extern FILE * logfile;
extern char * logtime(void);

#endif	/* PRODUCTION */

#define MAIN

#ifdef MAIN

char * pgmname;			/* this program's name */

/*-----------------------------------------------------------------------------
 * main()- for testing and debugging
 *-----------------------------------------------------------------------------
 */
int
main (int argc, char ** argv)
{
  char list[16384];		/* 16K of space */
  int result;			/* for tracking results */

  pgmname = strrchr (argv[0], '/');
  if (pgmname == NULL)
    pgmname = argv[0];
  else
    ++pgmname;

  if (argc != 2)
    {
      fprintf (stderr, "%s: usage: %s directory\n", pgmname, pgmname);
      return 1;
    }

  result = listfiles (list, sizeof (list), argv[1]);
  if (result != 0)
    {
      fprintf (stderr, "%s: %s\n", pgmname, list);
      return 1;
    }

  printf ("%s\n", list);
  
  return 0;
} /* end main() */
#endif	/* MAIN */

/*-----------------------------------------------------------------------------
 * listfiles()- returns a list of files in the form of:
 * name\n
 * ENTRY:
 * char * list: space for the list of file entries
 * int list_size: how large the list space is
 * char * sdir: starting directory name
 *
 * RETURNS:
 * 0 if all OK
 * 1 for general errors
 * 2 if the list buffer is too small.
 * list buffer filled in as above
 *-----------------------------------------------------------------------------
 */
int
listfiles (char * list, int list_size, char * sdir)
{
  int result;		 /* for tracking results */
  char dirname[256];	 /* if we have a link */
  char * s;		 /* general char ptr */

  if (!list || !sdir || list_size == 0)
    return 1;

  strcpy (dirname, sdir);
  s = strrchr (dirname, '/');
  if (!s || *(s + 1) != '\0')
    strcat (dirname, "/");

  /* read everything, put usable entries into the given list,
   * make sure we don't overflow the list.
   */
  result = procdir (list, list_size, dirname);

  if (result != 0)
    return result;

  sortlist (list);
  return 0;
} /* end listfiles() */

/*-----------------------------------------------------------------------------
 * procdir()-
 *
 * ENTRY:
 * char * list- buffer for the list of files
 * int list_size- size of the list buffer
 * char * dirname- name of the directory we're processing.
 *
 * EXIT:
 * char * list filled in with subdirectories and filenames
 * returns 0 if all OK, 2 if the list is too small
 *-----------------------------------------------------------------------------
 */
static int
procdir (char * list, int list_size, char * dirname)
{
  DIR * dirp;			/* for when we opendir() */
  char linkdir[256];		/* in case there are symlinks */
  int result = 0;		/* for tracking results */

  if (!list || !dirname || list_size < 1)
    return 1;

  /* get a directory pointer, follow symlinks if necessary:
   */
  dirp = getdir (dirname, linkdir, sizeof (linkdir));
  if (dirp == NULL)
    return 1;

  /* get the contents of the directory:
   */
  result = getdircontents (list, list_size, dirp);
  return result;
} /* end procdir() */

/*-----------------------------------------------------------------------------
 * getdircontents()- retrieve the contents of a directory
 *-----------------------------------------------------------------------------
 */
static int
getdircontents (char * list, int list_size, DIR * dirp)
{
  struct dirent * dirent;

  /* read everything, put usable entries into the given list,
   * make sure we don't overflow the list.
   */
  while ((dirent = readdir (dirp)) != NULL)
    {
      if ((strcmp (dirent->d_name, ".") == 0) ||
	  (strcmp (dirent->d_name, "..") == 0))
	continue;		/* skip '.' and '..' */
      
      if ((strlen (list) + strlen (dirent->d_name) + 4) > list_size)
	return 2;		/* all OK, just ran out of room */

      strcat (list, dirent->d_name);
      strcat (list, "\n");
    }

  return 0;
}

/*-----------------------------------------------------------------------------
 * getdir()- do all the processing necessary to get a DIR *.
 * This function checks for a directory which is actually a symlink, so it gets
 * the directory the symlink points to.
 *
 * ENTRY:
 * char * name- name of the directory
 * char * dirbuf- space for a symlinked directory name
 * int dirbuf_sz- size of dirbuf
 *
 * RETURNS:
 * an open DIR *, or NULL if error occurs.
 * Spits out an error message to stderr if needed.
 *-----------------------------------------------------------------------------
 */
static DIR *
getdir (char * name, char * dirbuf, int dirbuf_sz)
{
  DIR * dirp;			/* so we can give them an open directory */
  struct stat st;		/* so we can stat the given name */
  int result;			/* for tracking results */

  result = stat (name, &st);
  if (result == -1)
    {
      fprintf (stderr, "%s: getdir(): couldn't stat %s: %s\n",
	       pgmname, name, strerror (errno));
      return NULL;
    }

  memset (dirbuf, 0, dirbuf_sz);
  if (S_ISLNK(st.st_mode) != 0) /* it's a link, get where it goes */
    {
      result = readlink (name, dirbuf, dirbuf_sz);
      if (result == -1)
	{
	  fprintf (stderr, "%s: getdir(): readlink() error: %s\n",
		   pgmname, strerror (errno));
	  return NULL;
	}
    }
  else				/* not a link... */
    strcpy (dirbuf, name);	/* just copy name to dirbuf */

  dirp = opendir (dirbuf);
  if (dirp == NULL)
    {
      fprintf (stderr, "%s: getdir(): couldn't opendir() %s: %s\n",
	       pgmname, dirbuf, strerror (errno));
      return NULL;
    }

  return dirp;
}

/*-----------------------------------------------------------------------------
 * sortlist()- simple hack to sort the given list of directory/filenames,
 * using the given list buffer.
 * ENTRY:
 * char * list- list of directory/filenames to sort
 *
 * EXIT:
 * list is newly sorted, if no errors, otherwise unchanged.
 *
 * USES:
 * /tmp/newasd.tmp, /tmp/newasd.sort, both removed when done
 *-----------------------------------------------------------------------------
 */
static int
sortlist (char * list)
{
  int length = strlen (list);
  char tempn[64];		/* temp name */
  char sortn[64];		/* temp sort name */
  pid_t pid;			/* so we can use the pid */
  FILE * iofile;		/* the file we're writing */
  char cmd[256];		/* the sort command */

  pid = getpid();
  sprintf (tempn, "/tmp/%s%d.tmp",  pgmname, pid);
  sprintf (sortn, "/tmp/%s%d.sort", pgmname, pid);
  
  iofile = fopen (tempn, "w");
  if (iofile == NULL)
    {
      fprintf (stderr, "%s: sortlist(): couldn't fopen() %s for writing: %s\n",
	       pgmname, tempn, strerror (errno));
      return 1;
    }

  /* write out the list, use local sort program on it:
   */
  fwrite (list, 1, length, iofile);
  fclose (iofile);
  sprintf (cmd, "/bin/sort %s >%s", tempn, sortn);
  system (cmd);

  /* bring in the sorted file:
   */
  iofile = fopen (sortn, "r");
  if (iofile == NULL)
    {
      fprintf (stderr, "%s: sortlist(): couldn't fopen() %s for reading: %s\n",
	       pgmname, sortn, strerror (errno));
      unlink (tempn);
      return 1;
    }

  fread (list, 1, length, iofile);
  fclose (iofile);

  /* clean up the mess, say we're good:
   */
  unlink (tempn);
  unlink (sortn);
  return 0;
}
 
/*-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 */

/* EOF: dirfuncs.c
 */
