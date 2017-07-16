# std
some standard programs and functions I use

com.c: executes the first line of a C source file.
 Looks for a '/*% ' as the first four characters on the first line of the file;
 substitutes the file's name for any '%' seen in the line, substitutes the
 file's name without the extension for any '#' on the line.

 Example:
 com.c:
 /*% gcc -g -Wall -O0 % -o #
 translates into:
 gcc -g -Wall -O0 com.c -o com

minstd.c: a minimal starting point for a C program.
 includes stdio.h and string.h
 has globals char * pgmname (program name) and char pgmpath[256] for the 
 executable's path.
 main() sets pgmname and fills in pgmpath.

fullstd.c: a more filled-out minstd.c
 Set up for .ini file processing, logging, has getfile()

utils.c: includes functions logtime(), getTime(), mkEastern(), getfile(),
 process_ini(), get_ini_entry(), and ini_error().

dirfuncs.c: some simple directory functions for finding xasd files
 part of rtca/newasd project

 Usage: dirfuncs <bdir> <[YY]YY> <[M]M> <[D]D>\n", pgmname, pgmname);
  where:
   bdir is starting directory
   [YY]YY [M]M [D]D is year, month, and day

  dirfuncs REPURPOSED:
   given path now something like /var/www/html/data/asd2/YYYY/[M]M/[D]D/HH/mmss.raw
   alternate path is /home/data/asd2/YYYY/[M]M/[D]D/HH/mmss.raw
   we want to grab all the filenames in /var/www/html/data/asd2/YYYY/[M]M/[D]D/,
   visiting each HH directory.

# EOF:
