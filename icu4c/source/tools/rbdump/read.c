/*
*******************************************************************************
*                                                                             *
* COPYRIGHT:                                                                  *
*   (C) Copyright International Business Machines Corporation, 1998, 1999     *
*   Licensed Material - Program-Property of IBM - All Rights Reserved.        *
*   US Government Users Restricted Rights - Use, duplication, or disclosure   *
*   restricted by GSA ADP Schedule Contract with IBM Corp.                    *
*                                                                             *
*******************************************************************************
*
* File main.c
*
* Modification History:
*
*   Date        Name        Description
*   06/10/99    stephen	    Creation.
*******************************************************************************
*/

#include <stdio.h>

#include "utypes.h"
#include "cmemory.h"
#include "cstring.h"
#include "filestrm.h"
#include "parse.h"


/* Protos */
static void usage();
static void version();
static void processFile(const char *filename, UErrorCode *status);
static const char* errorName(UErrorCode status);
int main(int argc, char **argv);

/* The version of rbdump */
#define RBDUMP_VERSION "1.0"


int
main(int argc,
     char **argv)
{
  int printUsage = 0;
  int printVersion = 0;
  int optind = 1;
  int i;
  char *arg;
  UErrorCode status;


  /* parse the options */
  for(optind = 1; optind < argc; ++optind) {
    arg = argv[optind];
    
    /* version info */
    if(icu_strcmp(arg, "-v") == 0 || icu_strcmp(arg, "--version") == 0) {
      printVersion = 1;
    }
    /* usage info */
    else if(icu_strcmp(arg, "-h") == 0 || icu_strcmp(arg, "--help") == 0) {
      printUsage = 1;
    }
    /* POSIX.1 says all arguments after -- are not options */
    else if(icu_strcmp(arg, "--") == 0) {
      /* skip the -- */
      ++optind;
      break;
    }
    /* unrecognized option */
    else if(icu_strncmp(arg, "-", icu_strlen("-")) == 0) {
      printf("rbdump: invalid option -- %s\n", arg+1);
      printUsage = 1;
    }
    /* done with options, start file processing */
    else {
      break;
    }
  }

  /* print usage info */
  if(printUsage) {
    usage();
    return 0;
  }

  /* print version info */
  if(printVersion) {
    version();
    return 0;
  }

  /* print out the files */
  for(i = optind; i < argc; ++i) {
    status = U_ZERO_ERROR;
    processFile(argv[i], &status);
    if(FAILURE(status)) {
      printf("rbdump: %s processing file \"%s\"\n", errorName(status), argv[i]);
    }
  }

  return 0;
}

/* Usage information */
static void
usage()
{  
  puts("Usage: rbdump [OPTIONS] [FILES]");
  puts("Options:");
  puts("  -h, --help        Print this message and exit.");
  puts("  -v, --version     Print the version number of rbdump and exit.");
}

/* Version information */
static void
version()
{
  printf("rbdump version %s (ICU version %s), by Stephen F. Booth.\n", 
	 RBDUMP_VERSION, ICU_VERSION);
  puts("(C) Copyright International Business Machines Corporation, 1998, 1999");
  puts("Licensed Material - Program-Property of IBM - All Rights Reserved.");
  puts("US Government Users Restricted Rights - Use, duplication, or disclosure");
  puts("restricted by GSA ADP Schedule Contract with IBM Corp.");
}

/* Process a file */
static void
processFile(const char *filename,
	    UErrorCode *status)
{
  FileStream *in;

  if(FAILURE(*status)) return;

  /* Setup */
  in =  0;

  /* Open the input file for reading */
  in = T_FileStream_open(filename, "rb");
  if(in == 0) {
    *status = U_FILE_ACCESS_ERROR;
    return;
  }

  /* Parse the data and print it out */
  parse(in, status);

  /* Clean up */
  T_FileStream_close(in);
}
