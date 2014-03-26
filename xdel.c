/*

    xDel is a program to replace the OS/2 del command
    Copyright (C) 1999 D.J. van Enckevort

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    See the file COPYING for the full text of the GNU General Public License
*/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <sys\stat.h>
#include <string.h>
#include <ctype.h>
#include "xdel.h"

void PrintErrorMsg(APIRET rc)
{
switch (rc)
   {
   case NO_ERROR: // 0
      // There is no error, so we don't print an error message :-)
      break;
   case ERROR_INVALID_FUNCTION: // 1
      fprintf(stderr, "Error, invalid function\n");
      break;
   case ERROR_FILE_NOT_FOUND: // 2
      fprintf(stderr, "Error, file not found\n");
      break;
   case ERROR_PATH_NOT_FOUND: // 3
      fprintf(stderr, "Error, path not found\n");
      break;
   case ERROR_TOO_MANY_OPEN_FILES: // 4
      fprintf(stderr, "Error, too many open files\n");
      break;
   case ERROR_ACCESS_DENIED: // 5
      fprintf(stderr, "Error, access denied\n");
      break;
   case ERROR_INVALID_HANDLE: // 6
      fprintf(stderr, "Error, invalid handle\n");
      break;
   case ERROR_NOT_ENOUGH_MEMORY: // 8
      fprintf(stderr, "Error, not enough memory\n");
      break;
   case ERROR_INVALID_ACCESS: // 12
      fprintf(stderr, "Error, invalid access\n");
      break;
   case ERROR_CURRENT_DIRECTORY: // 16
      fprintf(stderr, "Error, current directory\n");
      break;
   case ERROR_NO_MORE_FILES: // 18
      fprintf(stderr, "Error, no more files\n");
      break;
   case ERROR_NOT_DOS_DISK: // 26
      fprintf(stderr, "Error, not a DOS Disk\n");
      break;
   case ERROR_SHARING_VIOLATION: // 32
      fprintf(stderr, "Error, sharing violation\n");
      break;
   case ERROR_SHARING_BUFFER_EXCEEDED: // 36
      fprintf(stderr, "Error, sharing buffer exceeded\n");
      break;
   case ERROR_CANNOT_MAKE: // 82
      fprintf(stderr, "Error, cannot make\n");
      break;
   case ERROR_INVALID_PARAMETER: // 87
      fprintf(stderr, "Error, invalid parameter\n");
      break;
   case ERROR_INTERRUPT: // 95
      fprintf(stderr, "Error, interrupt\n");
      break;
   case ERROR_DEVICE_IN_USE: // 99
      fprintf(stderr, "Error, device in use\n");
      break;
   case ERROR_DRIVE_LOCKED: // 108
      fprintf(stderr, "Error, drive locked\n");
      break;
   case ERROR_OPEN_FAILED: // 110
      fprintf(stderr, "Error, open failed\n");
      break;
   case ERROR_BUFFER_OVERFLOW: // 111
      fprintf(stderr, "Error, buffer overflow\n");
      break;
   case ERROR_DISK_FULL: // 112
      fprintf(stderr, "Error, disk full\n");
      break;
   case ERROR_NO_MORE_SEARCH_HANDLES: // 113
      fprintf(stderr, "Error, no more search handles\n");
      break;
   case ERROR_INSUFFICIENT_BUFFER: // 122
      fprintf(stderr, "Error, insufficient buffer\n");
      break;
   case ERROR_INVALID_NAME: // 123
      fprintf(stderr, "Error, invalid name\n");
      break;
   case ERROR_INVALID_LEVEL: // 124
      fprintf(stderr, "Error, invalid level\n");
      break;
   case ERROR_DIRECT_ACCESS_HANDLE: // 130
      fprintf(stderr, "Error, direct access handle\n");
      break;
   case ERROR_FILENAME_EXCED_RANGE: // 206
      fprintf(stderr, "Error, filename exceeded range\n");
      break;
   case ERROR_META_EXPANSION_TOO_LONG: // 208
      fprintf(stderr, "Error, meta expansion too long\n");
      break;
   case ERROR_PIPE_BUSY: // 231
      fprintf(stderr, "Error, pipe busy\n");
      break;
   case ERROR_INVALID_EA_NAME: // 254
      fprintf(stderr, "Error, invalid ea name\n");
      break;
   case ERROR_EA_LIST_INCONSISTENT: // 255
      fprintf(stderr, "Error, ea list inconsistent\n");
      break;
   case ERROR_EAS_DIDNT_FIT: // 275
      fprintf(stderr, "Error, eas didn't fit\n");
      break;
   default:
      fprintf(stderr, "Unknown error: %i\n", rc);
      break;
   }
}

APIRET DeleteDir(char *directory, POPTIONS options)
{
APIRET rc;
char answer;
if (options->query)
   {
   printf("%s: Are you sure? (Y/N)\n", directory);
   answer=toupper(_getch());
   if (answer!='Y')
      return NO_ERROR;
   }
else
   {
   if (options->verbose)
      printf("%s\n", directory);
   }
rc=DosDeleteDir(directory);
return rc;
}

APIRET DeleteFile(char *file, POPTIONS options)
{
APIRET rc;
HFILE hf;
ULONG action;
FILESTATUS3 fs3;
char answer;
if (options->query)
   {
   printf("%s: Are you sure? (Y/N)\n", file);
   answer=toupper(_getch());
   if (answer!='Y')
      return NO_ERROR;
   }
else
   {
   if (options->verbose)
      printf("%s\n", file);
   }
if (options->force)
   chmod(file, S_IWRITE);
if (options->unrecoverable)
   {
   rc=DosForceDelete(file);
   }
else
   {
   rc=DosDelete(file);
   }
return rc;
}

APIRET ProcessFiles(char *file, POPTIONS options)
{
APIRET rc, rcd;
HDIR hdirFindHandle = HDIR_CREATE;
FILEFINDBUF3 FindBuffer = {0};
ULONG ulResultBufLen = sizeof(FILEFINDBUF3);
ULONG ulFindCount = 1;
char fullname[CCHMAXPATH], *filepart;
char dir[CCHMAXPATHCOMP], filespec[CCHMAXPATHCOMP];

// Zoek het einde van het directory gedeelte.
filepart=strrchr(file, '\\');
if (filepart!=NULL)
   {
   // laat filepart verwijzen naar de bestandsnaam
   filepart++;
   // kopieer de bestandsnaam naar filespec
   strcpy(filespec,filepart);
   // kopieer het directorygedeelte naar dir
   strncpy(dir, file, filepart-file);
   dir[filepart-file]=0;
   }
else
   {
   strcpy(filespec, file);
   dir[0]=0;
   }
// file bevat nu de volledige naam, inclusief pad, met evt. wildcards
// filespec bevat nu de bestandsnaam, zonder het pad, met evt. wildcards
// dir bevat nu het pad

if (options->force)
   {
   rc = DosFindFirst(file, &hdirFindHandle, FILE_HIDDEN | FILE_SYSTEM | FILE_NORMAL, &FindBuffer, ulResultBufLen, &ulFindCount, FIL_STANDARD);
   }
else
   rc = DosFindFirst(file, &hdirFindHandle, FILE_NORMAL, &FindBuffer, ulResultBufLen, &ulFindCount, FIL_STANDARD);
if (rc!=NO_ERROR)
   if (rc!=ERROR_NO_MORE_FILES)
      return rc;
   else
      return NO_ERROR;
else
   {
   strcpy(fullname, dir);
   strcat(fullname, FindBuffer.achName);
   rcd=DeleteFile(fullname, options);
   if (rcd!=NO_ERROR)
      PrintErrorMsg(rcd);
   }
while (rc!=ERROR_NO_MORE_FILES)
   {
   ulFindCount = 1;
   rc = DosFindNext(hdirFindHandle, &FindBuffer, ulResultBufLen, &ulFindCount);
   if ((rc != NO_ERROR) && (rc != ERROR_NO_MORE_FILES))
      return rc;
   else
      {
      if (rc!=ERROR_NO_MORE_FILES)
         {
         strcpy(fullname, dir);
         strcat(fullname, FindBuffer.achName);
         rcd=DeleteFile(fullname, options);
         if (rcd!=NO_ERROR)
            PrintErrorMsg(rcd);
         }
      }
   }
rc = DosFindClose(hdirFindHandle);
return rc;
}

APIRET ProcessRecursing(char *file, POPTIONS options)
{
APIRET rc, rcd;
HDIR hdirFindHandle = HDIR_CREATE;
FILEFINDBUF3 FindBuffer = {0};
ULONG ulResultBufLen = sizeof(FILEFINDBUF3);
ULONG ulFindCount = 1;
char fullname[CCHMAXPATH], *filepart;
char dir[CCHMAXPATHCOMP], filespec[CCHMAXPATHCOMP];

// Zoek het einde van het directory gedeelte.
filepart=strrchr(file, '\\');
if (filepart!=NULL)
   {
   // laat filepart verwijzen naar de bestandsnaam
   filepart++;
   // kopieer de bestandsnaam naar filespec
   strcpy(filespec,filepart);
   // kopieer het directorygedeelte naar dir
   strncpy(dir, file, filepart-file);
   dir[filepart-file]=0;
   }
else
   {
   strcpy(filespec, file);
   dir[0]=0;
   }

// file bevat nu de volledige naam, inclusief pad, met evt. wildcards
// filespec bevat nu de bestandsnaam, zonder het pad, met evt. wildcards
// dir bevat nu het pad

strcpy(fullname, dir);
strcat(fullname, "*");
if (options->force)
   {
   rc = DosFindFirst(fullname, &hdirFindHandle, FILE_HIDDEN | FILE_SYSTEM | MUST_HAVE_DIRECTORY, &FindBuffer, ulResultBufLen, &ulFindCount, FIL_STANDARD);
   }
else
   rc = DosFindFirst(fullname, &hdirFindHandle, MUST_HAVE_DIRECTORY, &FindBuffer, ulResultBufLen, &ulFindCount, FIL_STANDARD);

if (rc!=NO_ERROR)
   if (rc!=ERROR_NO_MORE_FILES)
      return rc;
   else
      return NO_ERROR;
else
   {
   if ((strcmp(FindBuffer.achName, ".")!=0) && (strcmp(FindBuffer.achName, "..")!=0))
      {  // We moeten de . en .. directory negeren
      // construeer het volledige pad voor de subdirectory
      strcpy(fullname, dir);
      strcat(fullname, FindBuffer.achName);
      // voeg de slash eraan toe
      strcat(fullname, "\\");
      // en voeg de filespecs eraan toe
      strcat(fullname, filespec);
      // en roep deze functie opnieuw aan
      rcd=Delete(fullname, options);
      if (rcd!=NO_ERROR)
         PrintErrorMsg(rcd);
      }
   }
while (rc != ERROR_NO_MORE_FILES)
   {
   ulFindCount = 1;
   rc = DosFindNext(hdirFindHandle, &FindBuffer, ulResultBufLen, &ulFindCount);
   if ((rc != NO_ERROR) && (rc != ERROR_NO_MORE_FILES))
      return rc;
   else
      {
      if (rc!=ERROR_NO_MORE_FILES)
         {
         if ((strcmp(FindBuffer.achName, ".")!=0) && (strcmp(FindBuffer.achName, "..")!=0))
            {  // We moeten de . en .. directory negeren
            // construeer het volledige pad voor de subdirectory
            strcpy(fullname, dir);
            strcat(fullname, FindBuffer.achName);
            // voeg de slash eraan toe
            strcat(fullname, "\\");
            strcat(fullname, filespec);
            // en roep deze functie opnieuw aan
            rcd=Delete(fullname, options);
            if (rcd!=NO_ERROR)
               PrintErrorMsg(rcd);
            }
         }
      }
   }
rc = DosFindClose(hdirFindHandle);
return rc;
}

APIRET ProcessDirectories(char *file, POPTIONS options)
{
APIRET rc, rcd;
HDIR hdirFindHandle = HDIR_CREATE;
FILEFINDBUF3 FindBuffer = {0};
ULONG ulResultBufLen = sizeof(FILEFINDBUF3);
ULONG ulFindCount = 1;
char fullname[CCHMAXPATH], *filepart;
char dir[CCHMAXPATHCOMP], filespec[CCHMAXPATHCOMP];

// Zoek het einde van het directory gedeelte.
filepart=strrchr(file, '\\');
if (filepart!=NULL)
   {
   // laat filepart verwijzen naar de bestandsnaam
   filepart++;
   // kopieer de bestandsnaam naar filespec
   strcpy(filespec,filepart);
   // kopieer het directorygedeelte naar dir
   strncpy(dir, file, filepart-file);
   dir[filepart-file]=0;
   }
else
   {
   strcpy(filespec, file);
   dir[0]=0;
   }
// file bevat nu de volledige naam, inclusief pad, met evt. wildcards
// filespec bevat nu de bestandsnaam, zonder het pad, met evt. wildcards
// dir bevat nu het pad
if (options->force)
   {
   rc = DosFindFirst(file, &hdirFindHandle, MUST_HAVE_DIRECTORY | FILE_HIDDEN | FILE_SYSTEM, &FindBuffer, ulResultBufLen, &ulFindCount, FIL_STANDARD);
   }
else
   rc = DosFindFirst(file, &hdirFindHandle, MUST_HAVE_DIRECTORY, &FindBuffer, ulResultBufLen, &ulFindCount, FIL_STANDARD);
if (rc != NO_ERROR)
   if (rc!=ERROR_NO_MORE_FILES)
      return rc;
   else
      return NO_ERROR;
else
   {
   if ((strcmp(FindBuffer.achName, ".")!=0) && (strcmp(FindBuffer.achName, "..")!=0))
      { // We moeten de . en .. directory negeren
      // construeer het volledige pad voor de directory
      strcpy(fullname, dir);
      strcat(fullname, FindBuffer.achName);
      // en wis de directory
      rcd=DeleteDir(fullname, options);
      if (rcd!=NO_ERROR)
         PrintErrorMsg(rcd);
      }
   }
while (rc != ERROR_NO_MORE_FILES)
   {
   ulFindCount = 1;
   rc = DosFindNext(hdirFindHandle, &FindBuffer, ulResultBufLen, &ulFindCount);
   if ((rc != NO_ERROR) && (rc != ERROR_NO_MORE_FILES))
      return rc;
   else
      {
      if (rc!=ERROR_NO_MORE_FILES)
         {
         if ((strcmp(FindBuffer.achName, ".")!=0) && (strcmp(FindBuffer.achName, "..")!=0))
            { // We moeten de . en .. directory negeren
            strcpy(fullname, dir);
            strcat(fullname, FindBuffer.achName);
            rcd=DeleteDir(fullname, options);
            if (rcd!=NO_ERROR)
               PrintErrorMsg(rcd);
            }
         }
      }
   }
rc = DosFindClose(hdirFindHandle);
return rc;
}

APIRET Delete(char *file, POPTIONS options)
{
APIRET rc;
rc=ProcessFiles(file, options);
PrintErrorMsg(rc);
if (options->recurse)
   {
   rc=ProcessRecursing(file, options);
   PrintErrorMsg(rc);
   }
if (options->dirs)
   {
   rc=ProcessDirectories(file, options);
   PrintErrorMsg(rc);
   }
return NO_ERROR;
}


void Help(void)
{
printf("xDel 1.01\tCopyright 1999 by D.J. van Enckevort\n\n");
printf("xDel comes with ABSOLUTELY NO WARRANTY\n");
printf("This is free software, and you are welcome to redistribute it under certain\n");
printf("conditions. See the file COPYING for more information.\n\n");
printf("Syntaxt: xDel [opion1 option2] <filespec1> [option3 option4] <filespec2>\n\n");
printf("Options: /f Forced deletion\n");
printf("         /h Help\n");
printf("         /? help\n");
printf("         /i Interactive\n");
printf("         /r Remove directories\n");
printf("         /s recurse into Subdirectories\n");
printf("         /u unrecoverable delete\n");
printf("         /v Verbose\n");
printf("         /- Read filenames from stdin\n\n");
printf("Each option must be separated from the next by a whitespace.\n");
printf("filespec can be any valid filename, with or without wildcards * and ?.\n\n");
printf("Options can be set in the XDEL_OPTIONS environment variable too.\n\n");
printf("Sample: SET XDEL_OPTIONS=/i /f\n");
}

APIRET setoption(char option, POPTIONS options)
{
APIRET rc;
rc=NO_ERROR;
switch (option)
   {
   case 'S':
      options->recurse=TRUE;
      break;
   case 'F':
      options->force=TRUE;
      break;
   case 'R':
      options->dirs=TRUE;
      break;
   case 'I':
      options->query=TRUE;
      break;
   case 'V':
      options->verbose=TRUE;
      break;
   case '?':
      Help();
      break;
   case 'H':
      Help();
      break;
   case 'U':
      options->unrecoverable=TRUE;
      break;
   case '-':
      options->parsestdin=TRUE;
      break;
   default:
      rc=ERROR_INVALID_PARAMETER;
   }
return rc;
}

APIRET main(int argc, char *argv[])
{
APIRET rc;
char file[CCHMAXPATH+1], pattern[10], swchar, *env, *xdel_options;
ULONG i;
OPTIONS options;
// Put the max length in the pattern string
sprintf(pattern, "%%%is", CCHMAXPATH);
options.query=FALSE;
options.dirs=FALSE;
options.force=FALSE;
options.recurse=FALSE;
options.verbose=FALSE;
options.parsestdin=FALSE;
options.unrecoverable=FALSE;
rc=NO_ERROR;

if (argc==1)
   {
   Help();
   }
else
   {
   env=getenv(XDEL_OPTIONS);
   if (env!=NULL)
      {
      i=strlen(env)+1;
      rc=DosAllocMem((void **)&xdel_options, i, PAG_COMMIT | PAG_READ | PAG_WRITE);
      if (rc!=NO_ERROR)
         {
         PrintErrorMsg(rc);
         return rc;
         }
      strcpy(xdel_options, env);
      for (i=0; i<strlen(xdel_options); i++)
         {
         if (xdel_options[i]=='/')
            {
            swchar=toupper(xdel_options[i+1]);
            rc=setoption(swchar, &options);
            if (rc!=NO_ERROR)
               return rc;
            }
         }
      }
   for (i=1; i<argc; i++)
      {
      if (argv[i][0]=='/')
         {
         swchar=toupper(argv[i][1]);
         rc=setoption(swchar, &options);
         if (rc!=NO_ERROR)
            break;
         }
      if (argv[i][0]!='/')
         {
         rc=Delete(argv[i], &options);
         if (rc!=NO_ERROR)
            break;
         }
      }
   }
if ((options.parsestdin==TRUE) && (rc==NO_ERROR))
   {
   // read from stdin
   while (!feof(stdin))
      {
      file[0]=0;
      if (fscanf(stdin, pattern, file)!=0)
         {
         if (strlen(file)!=0)
            {
            rc=Delete(file, &options);
            if (rc!=NO_ERROR)
               break;
            }
         }
      }
   }
PrintErrorMsg(rc);
return rc;
}

