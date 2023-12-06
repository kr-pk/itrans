/*
 *========================================================================== 
 * Copyright 1991, 1992 Avinash Chopde, All Rights Reserved.
 */

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/utils.c,v 1.4 1998/09/13 02:13:29 avinash Exp $";

#include "itrans.h"

/* =================================================================== */

/*
 *   The search routine takes a directory list, separated by :, and
 *   tries to open a file.  Null directory components indicate current
 *   directory.
 *   Returns the open file descriptor if ok, else NULL.
 *   If fopened is non-null, copies the given file name (PATH_MAX) into it.
 */

FILE* search_fopen(char* path, char* file, char* mode, char* fopened)
{
   register char *nam ;                 /* index into fname */
   register FILE *fd ;                  /* file desc of file */
   char fname[PATH_MAX] ;             /* to store file name */
   static char *home = 0 ;              /* home is where the heart is */
   int slen;

   slen = strlen(file);
   if (slen < 1) return NULL;

			/* check for full path name */
   if (   (file[0] == DIRSEP)
       || (slen > 1 && file[0] == '.' && file[1] == DIRSEP)
       || (slen > 2 && file[0] == '.' && file[1] == '.' && file[2] == DIRSEP)) {

      if ((fd=fopen(file,mode)) != NULL) {
	 if (fopened) strcpy(fopened, file);
         return(fd) ;
      } else
         return(NULL) ;
   }

#if defined(MSDOS) || defined(WIN32)
   if ( isalpha(file[0]) && file[1]==':' ) {   /* if full path name */
      if ((fd=fopen(file,mode)) != NULL) {
	 if (fopened) strcpy(fopened, file);
         return(fd) ;
      } else
         return(NULL) ;
   }
#endif

   do {
      /* copy the current directory into fname */
      nam = fname;
      /* copy till : */
      if (*path == '~') {
         char *p = nam ;
         path++ ;
         while (*path && *path != PATHSEP && *path != DIRSEP)
            *p++ = *path++ ;
         *p = 0 ;
         if (*nam == 0) {
            if (home == 0) {
               home = (char*) getenv("HOME");
               if (home) {
		  /*EMPTY*/;
               } else
                  home = "." ;
            }
            strcpy(fname, home) ;
         } else {
#if defined(MSDOS) || defined(WIN32)
            /* error("! ~username in path???") ; */
            fprintf(stderr, "! ~username in path???") ;
#else
            struct passwd *pw = getpwnam(fname) ;
            if (pw)
               strcpy(fname, pw->pw_dir) ;
            else {
	       fprintf(stderr, "No such user!\n");
	    }
#endif
         }
         nam = fname + strlen(fname) ;
      }
      while (*path != PATHSEP && *path) *nam++ = *path++;
      *nam = 0 ;

      if (nam == fname) *nam++ = '.';   /* null component is current dir */

      if (*file != '\0') {
         *nam++ = DIRSEP;                  /* add separator */
         (void)strcpy(nam,file);                   /* tack the file on */
      }
      else
         *nam = '\0' ;

      /* belated check -- bah! */
      if ((nam - fname) + strlen(file) + 1 > PATH_MAX) {
         fprintf(stderr, "! overran allocated storage in search_fopen()");
	 exit(1);
      }

#ifdef DEBUG
         (void)fprintf(stderr,"Trying to open %s\n", fname) ;
#endif
      if ((fd=fopen(fname,mode)) != NULL) {
	 if (fopened) strcpy(fopened, fname);
         return(fd);
      }

   /* skip over PATHSEP and try again */
   } while (*(path++));

   return(NULL);

}               /* end search */

/* =================================================================== */
/* compare extensions, "." must be first char of right_ext[].. */
int chk_ext(char *path, char *right_ext)
{
  char *p;
  int err = 0;

  /* Find the last '.' in the path */
  p = strrchr(path, '.');

  if(!p) {
    err++; /* extension missing.... */
  } else if(strcmp(p, right_ext)) {
    err++; /* incorrect extension */
  }

  return(err);
}

/* =================================================================== */

#ifdef NEED_IDIV1000_A
/* divide the given integer by 1000, convert the result into a string */
/* was written since on the PC, using DJG's GNU compiler, no floating
 * point arith was possible.
 * And, anyway, this may even be faster than float/1000 and call to
 * print...
 * [abc] 5 sept 1991, don't need this anymore, got the DJG GCC compiler
 * working -- instead of saying EMU in the GO32 env var, I had to
 * say emu! (small instead of caps!)
 */
int idiv1000_a(int n, char str[])
{
    int i, j;

    i = n / 1000; /* get leading digits */
    n = n - i * 1000; /* this left over, guaranteed < 1000 */

    /* itoa(i, str, 10); */
    sprintf(str, "%d", i);
    
    j = i = strlen(str);

    str[i++] = '.';
    /* add leading zeros */
    if (n < 10) { /* single digit */
        str[i++] = '0';
        str[i++] = '0';
    } else if (n < 100) { /* two digits */
        str[i++] = '0';
    }

    /* itoa(n, &str[i], 10); */
    sprintf(&str[i], "%d", n);

    return (j + 4);
}
#endif /*NEED_IDIV1000_A*/
/* =================================================================== */
#if defined(MSDOS) || defined(WIN32)
/* support functions that may be missing on a PC/MSDOS system. */

#if 0
/* getopt()
   Only simple versions supported:
   options may not be combined (as in the real getopt()), but there may
   not be a space between the arg letter and the option argument.
 */

/* externally visible */
char* optarg =0;	int optind =1;	int opterr =1;

int getopt(int argc, char* argv[], char* optstring)
{
    int i, j, found;
    char c, *curr;
    char tmp[256];

    found = FALSE;
    optarg = NULL;

    while (!found) {

	curr = argv[optind];

        if (!curr || optind >= argc) return -1; /* end of arguments */

        if (curr[0] != '-') {
	    /* end of arguments */
	    return -1;
        }

        if (curr[0] == '-' && curr[1] == '-') {
	    /* end of arguments */
	    return -1;
        }

	    /* got an argument */
        j = strlen(optstring);
        c = curr[1];
        for (i = 0; i < j; i ++) {
	    if (optstring[i] == ':') continue;
    
	    if (c == optstring[i]) {
	        /* check if needs arguments */
	        if (optstring[i+1] == ':') {
		    strcpy(tmp, argv[optind]);
		    optarg = strtok(tmp+2, " 	"); /* space, TAB chars */
		    if (!optarg) { /* arg for option is at next location */
		        optind++;
		        optarg = argv[optind];
		    }
	        }
	        found = TRUE;
	        break;
	    }
        }

	if (!found && opterr)  {
	    fprintf(stderr, "Unrecognized option: %c -- ignoring it\n",c);
	}
    
        optind++;
    }

    return c;
}
#endif
#endif

/* =================================================================== */
/* on SunOS, realloc does not accept the NULL pointer, so make my own reallo */
void* my_realloc(void* ptr, int size)
{
    if (ptr) return realloc(ptr, size);
    else return malloc(size);
}
/* =================================================================== */
