/**************************************************************************
*
*   Copyright (C) 2000, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
***************************************************************************
*   file name:  gmake.c
*   encoding:   ANSI X3.4 (1968)
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2000may17
*   created by: Steven \u24C7 Loomis
*
* Emit a GNU makefile
*/

#include "makefile.h"
#include <stdio.h>
#include <string.h>

char linebuf[2048];

/* Write any setup/initialization stuff */
void
pkg_mak_writeHeader(FileStream *f, const UPKGOptions *o)
{
  sprintf(linebuf, "## Makefile for %s created by pkgdata\n"
                   "## from ICU Version %s\n"
                   "\n",
          o->shortName,
          U_ICU_VERSION);
  T_FileStream_writeLine(f, linebuf);

  sprintf(linebuf, "NAME=%s\n"
          "CNAME=%s\n"
          "TARGETDIR=%s\n"
          "TEMP_DIR=%s\n"
          "srcdir=$(TEMP_DIR)\n"
          "MODE=%s\n"
          "MAKEFILE=%s\n"
          "ENTRYPOINT=%s\n"
          "include %s\n"
          "\n\n\n",
          o->shortName,
          o->cShortName,
          o->targetDir,
          o->tmpDir,
          o->mode,
          o->makeFile,
          o->entryName,
          o->options);
  T_FileStream_writeLine(f, linebuf);

  /* TEMP_PATH  and TARG_PATH will be empty if the respective dir is . */
  /* Avoid //'s and .'s which confuse make ! */
  if(!strcmp(o->tmpDir,"."))
  {
    T_FileStream_writeLine(f, "TEMP_PATH=\n");
  }
  else
  {
    T_FileStream_writeLine(f, "TEMP_PATH=$(TEMP_DIR)/\n");
  }

  if(!strcmp(o->targetDir,"."))
  {
    T_FileStream_writeLine(f, "TARG_PATH=\n");
  }
  else
  {
    T_FileStream_writeLine(f, "TARG_PATH=$(TARGETDIR)/\n");
  }

  sprintf(linebuf, "## List files [%d] containing data files to process (note: - means stdin)\n"
                            "LISTFILES= ",
                         pkg_countCharList(o->fileListFiles));
  T_FileStream_writeLine(f, linebuf);

  pkg_writeCharListWrap(f, o->fileListFiles, " ", " \\\n",0);

  T_FileStream_writeLine(f, "\n\n\n");

  sprintf(linebuf, "## Data Files [%d]\n"
                            "DATAFILES= ",
                         pkg_countCharList(o->files));

  T_FileStream_writeLine(f, linebuf);

  pkg_writeCharListWrap(f, o->files, " ", " \\\n",-1);

  T_FileStream_writeLine(f, "\n\n\n");

  sprintf(linebuf, "## Data File Paths [%d]\n"
                            "DATAFILEPATHS= ",
                         pkg_countCharList(o->filePaths));

  T_FileStream_writeLine(f, linebuf);

  pkg_writeCharListWrap(f, o->filePaths, " ", " \\\n",0);

  T_FileStream_writeLine(f, "\n\n\n");

}

/* Write a stanza in the makefile, with specified   "target: parents...  \n\n\tcommands" [etc] */
void
pkg_mak_writeStanza(FileStream *f, const UPKGOptions *o, 
                    const char *target,
                    CharList* parents,
                    CharList* commands)
{
  T_FileStream_write(f, target, strlen(target));
  T_FileStream_write(f, " : ", 3);
  pkg_writeCharList(f, parents, " ",0);
  T_FileStream_write(f, "\n", 1);

  if(commands)
  {
    T_FileStream_write(f, "\t", 1);
    pkg_writeCharList(f, commands, "\n\t",0);
  }
  T_FileStream_write(f, "\n\n", 2);
}

/* write any cleanup/post stuff */
void
pkg_mak_writeFooter(FileStream *f, const UPKGOptions *o)
{
  /* nothing */
}

