/*
 ********************************************************************************
 *
 *   Copyright (C) 1998-1999, International Business Machines
 *   Corporation and others.  All Rights Reserved.
 *
 ********************************************************************************
 *
 *
 *  makeconv.c:
 *  tool creating a binary (compressed) representation of the conversion mapping
 *  table (IBM NLTC ucmap format).
 *
 *  05/04/2000    helena     Added fallback mapping into the picture...
 *  06/29/2000  helena      Major rewrite of the callback APIs.
 */
   
#include <stdio.h>
#include "ucmp16.h"
#include "ucmp8.h"
#include "ucnv_io.h"
#include "unicode/ucnv_err.h"
#include "ucnv_bld.h"
#include "ucnv_imp.h"
#include "ucnv_cnv.h"
#include "cstring.h"
#include "cmemory.h"
#include "filestrm.h"
#include "toolutil.h"
#include "uoptions.h"
#include "genmbcs.h"
#include "unicode/udata.h"
#include "unewdata.h"
#include "ucmpwrit.h"

#ifdef XP_MAC_CONSOLE
# include <console.h>
#endif

#define DEBUG 0

/*
 * from ucnvstat.c - static prototypes of data-based converters
 */
extern const UConverterStaticData * ucnv_converterStaticData[UCNV_NUMBER_OF_SUPPORTED_CONVERTER_TYPES];

/*
 * Global - verbosity
 */
UBool VERBOSE = FALSE;


/*Reads the header of the table file and fills in basic knowledge about the converter
 *in "converter"
 */
static void readHeaderFromFile(UConverterSharedData* myConverter, FileStream* convFile, const char* converterName, UErrorCode* err);

/*Reads the rest of the file, and fills up the shared objects if necessary
Returns the UConverterTable. */
static void loadMBCSTableFromFile(FileStream* convFile, UConverterSharedData* sharedData, UErrorCode* err);

/*Reads the rest of the file, and fills up the shared objects if necessary
Returns the UConverterTable. */
static UConverterTable* loadEBCDIC_STATEFULTableFromFile(FileStream* convFile, UConverterStaticData* staticData, UErrorCode* err);

/*Reads the rest of the file, and fills up the shared objects if necessary
Returns the UConverterTable. */
static UConverterTable* loadSBCSTableFromFile(FileStream* convFile, UConverterStaticData* staticData, UErrorCode* err);

/*Reads the rest of the file, and fills up the shared objects if necessary
Returns the UConverterTable. */
static UConverterTable* loadDBCSTableFromFile(FileStream* convFile, UConverterStaticData* staticData, UErrorCode* err);

/* creates a UConverterSharedData from a mapping file.
 * Fills in:  *staticData, *table.  Converter is NOT otherwise useful.
 */
static UConverterSharedData* createConverterFromTableFile(const char* realName, UErrorCode* err);


/*
 * Set up the UNewData and write the converter..
 */
void writeConverterData(UConverterSharedData *mySharedData, const char *cnvName, const char *cnvDir, UErrorCode *status);

/*
 * Writes the StaticData followed by the Table to the udata
 */
static void WriteConverterSharedData(UNewDataMemory *pData, const UConverterSharedData* data);

/*
 * Deletes the static data, table. Ignores any other options in the shareddata.
 */
UBool makeconv_deleteSharedConverterData(UConverterSharedData* deadSharedData);

/*
 * Utility functions
 */
static UConverterPlatform getPlatformFromName(char* name);
static int32_t getCodepageNumberFromName(char* name);


static const char NLTC_SEPARATORS[9] = { '\r', '\n', '\t', ' ', '<', '>' ,'"' , 'U', '\0' };
static const char FALLBACK_SEPARATOR = '|';
static const char PLAIN_SEPARATORS[9] = { '\r', '\n', '\t', ' ', '<', '>' ,'"' ,  '\0' };
static const char STATE_SEPARATORS[9] = { '\r', '\n', '\t', '<', '>' ,'"' , '\0' }; /* do not break on space */
static const char CODEPOINT_SEPARATORS[8] = {  '\r', '>', '\\', 'x', '\n', ' ', '\t', '\0' };
static const char UNICODE_CODEPOINT_SEPARATORS[6] = {  '<', '>', 'U', ' ', '\t', '\0' };



/* Remove all characters followed by '#'
 */
char *
  removeComments (char *line)
{
  char *pound = uprv_strchr (line, '#');
  char *fallback = uprv_strchr(line, '|');
  if (pound != NULL)
  {
      if (fallback != NULL)
      {
          uprv_memset(pound, ' ', fallback-pound);
      }
      else 
      {
          *pound = '\0';
      }
  }
  return line;
}

/*Returns uppercased string */
char *
  strtoupper (char *name)
{
  char *oldPtr = name;

  do {
    *name = (char)uprv_toupper(*name);
  }
  while (*(name++));

  return oldPtr;
}

/* Returns true in c is a in set 'setOfChars', false otherwise
 */
UBool 
  isInSet (char c, const char *setOfChars)
{
  uint8_t i = 0;

  while (setOfChars[i] != '\0')
    {
      if (c == setOfChars[i++])
        return TRUE;
    }

  return FALSE;
}

/* Returns pointer to the next non-whitespace (or non-separator)
 */
int32_t 
  nextTokenOffset (const char *line, const char *separators)
{
  int32_t i = 0;

  while (line[i] && isInSet(line[i], separators))
    i++;

  return i;
}

/* Returns pointer to the next token based on the set of separators
 */
char *
  getToken (char *token, char *line, const char *separators)
{
  int32_t i = nextTokenOffset (line, separators);
  int8_t j = 0;

  while (line[i] && (!isInSet(line[i], separators)))
    token[j++] = line[i++];
  token[j] = '\0';

  return line + i;
}

UBool haveCopyright=TRUE;

static UDataInfo dataInfo={
    sizeof(UDataInfo),
    0,

    U_IS_BIG_ENDIAN,
    U_CHARSET_FAMILY,
    sizeof(UChar),
    0,

    0x63, 0x6e, 0x76, 0x74,     /* dataFormat="cnvt" */
    6, 0, 0, 0,                 /* formatVersion */
    0, 0, 0, 0                  /* dataVersion (calculated at runtime) */
};


void writeConverterData(UConverterSharedData *mySharedData, 
                        const char *cnvName, 
                        const char *cnvDir, 
                        UErrorCode *status)
{
  UNewDataMemory *mem = NULL;
  uint32_t sz2;
  
  if(U_FAILURE(*status))
    {
      return;
    }

  mem = udata_create(cnvDir, "cnv", cnvName, &dataInfo, haveCopyright ? U_COPYRIGHT_STRING : NULL, status);

  if(U_FAILURE(*status))
    {
      fprintf(stderr, "Couldn't create the udata %s.%s: %s\n",
              cnvName,
              "cnv",
              u_errorName(*status));
      return;
    }

  if(VERBOSE)
    {
      fprintf(stderr, "- Opened udata %s.%s\n", cnvName, "cnv");
    }
  
  WriteConverterSharedData(mem, mySharedData);

  sz2 = udata_finish(mem, status);
  
  if(VERBOSE)
  {
    fprintf(stderr, "- Wrote %d bytes to the udata.\n", sz2); 
  }
}

static UOption options[]={
    UOPTION_HELP_H,              /* 0  Numbers for those who*/ 
    UOPTION_HELP_QUESTION_MARK,  /* 1   can't count. */
    UOPTION_COPYRIGHT,           /* 2 */
    UOPTION_VERSION,             /* 3 */
    UOPTION_DESTDIR,             /* 4 */
    UOPTION_VERBOSE              /* 5 */
};

int main(int argc, char* argv[])
{
    UConverterSharedData* mySharedData = NULL; 
    UErrorCode err = U_ZERO_ERROR;
    char outFileName[UCNV_MAX_FULL_FILE_NAME_LENGTH];
    const char* destdir, *arg;
    size_t destdirlen;
    char* dot = NULL, *outBasename;
    char cnvName[UCNV_MAX_FULL_FILE_NAME_LENGTH];
    UVersionInfo icuVersion;

#ifdef XP_MAC_CONSOLE
    argc = ccommand((char***)&argv);
#endif

    /* Set up the ICU version number */
    u_getVersion(icuVersion);
    uprv_memcpy(&dataInfo.dataVersion, &icuVersion, sizeof(UVersionInfo));

    /* preset then read command line options */
    options[4].value=u_getDataDirectory();
    argc=u_parseArgs(argc, argv, sizeof(options)/sizeof(options[0]), options);

    /* error handling, printing usage message */
    if(argc<0) {
        fprintf(stderr,
            "error in command line argument \"%s\"\n",
            argv[-argc]);
    } else if(argc<2) {
        argc=-1;
    }
    if(argc<0 || options[0].doesOccur || options[1].doesOccur) {
        fprintf(stderr,
            "usage: %s [-options] files...\n"
            "\tread .ucm codepage mapping files and write .cnv files\n"
            "\toptions:\n"
            "\t\t-h or -? or --help  this usage text\n"
            "\t\t-V or --version     show a version message\n"
            "\t\t-c or --copyright   include a copyright notice\n"
            "\t\t-d or --destdir     destination directory, followed by the path\n"
            "\t\t-v or --verbose     Turn on verbose output\n",
            argv[0]);
        return argc<0 ? U_ILLEGAL_ARGUMENT_ERROR : U_ZERO_ERROR;
    }

    if(options[3].doesOccur) {
      fprintf(stderr,"makeconv version %hu.%hu, ICU tool to read .ucm codepage mapping files and write .cnv files\n",
            dataInfo.formatVersion[0], dataInfo.formatVersion[1]);
      fprintf(stderr, "Copyright (C) 1998-2000, International Business Machines\n");
      fprintf(stderr,"Corporation and others.  All Rights Reserved.\n");
        exit(0);
    }

    /* get the options values */
    haveCopyright = options[2].doesOccur;
    destdir = options[4].value;
    VERBOSE = options[5].doesOccur;

    if (destdir != NULL && *destdir != 0) {
        uprv_strcpy(outFileName, destdir);
        destdirlen = uprv_strlen(destdir);
        outBasename = outFileName + destdirlen;
        if (*(outBasename - 1) != U_FILE_SEP_CHAR) {
            *outBasename++ = U_FILE_SEP_CHAR;
            ++destdirlen;
        }
    } else {
        destdirlen = 0;
        outBasename = outFileName;
    }
    
#if DEBUG
    {
      int i;
      printf("makeconv: processing %d files...\n", argc - 1);
      for(i=1; i<argc; ++i) {
        printf("%s ", argv[i]);
      }
      printf("\n");
      fflush(stdout);
    }
#endif

  for (++argv; --argc; ++argv)
    {
      err = U_ZERO_ERROR;
      arg = getLongPathname(*argv);

      /*produces the right destination path for display*/
      if (destdirlen != 0)
        {
          const char *basename;

          /* find the last file sepator */
          basename = uprv_strrchr(arg, U_FILE_SEP_CHAR);
          if (basename == NULL) {
              basename = arg;
          } else {
              ++basename;
          }

          uprv_strcpy(outBasename, basename);
        }
      else
        {
          uprv_strcpy(outFileName, arg);
        }

      /*removes the extension if any is found*/
      dot = uprv_strrchr(outBasename, '.');
      if (dot) 
        {
          *dot = '\0';
        }

      /* the basename without extension is the converter name */
      uprv_strcpy(cnvName, outBasename);

      /*Adds the target extension*/
      uprv_strcat(outBasename, CONVERTER_FILE_EXTENSION);

#if DEBUG
        printf("makeconv: processing %s  ...\n", arg);
        fflush(stdout);
#endif
      mySharedData = createConverterFromTableFile(arg, &err);

      if (U_FAILURE(err) || (mySharedData == NULL))
        {
          /* if an error is found, print out an error msg and keep going */
          fprintf(stderr, "Error creating \"%s\" file for \"%s\" (error code %d - %s)\n", outFileName, arg, err,
                        u_errorName(err));
          err = U_ZERO_ERROR;
        }
      else
        {
          /* Make the static data name equal to the file name */
          if( /*VERBOSE &&  */ uprv_stricmp(cnvName,mySharedData->staticData->name))
          {
            fprintf(stderr, "Warning: %s%s claims to be '%s'\n", 
                    cnvName,
                    CONVERTER_FILE_EXTENSION,
                    mySharedData->staticData->name);
          }

          uprv_strcpy((char*)mySharedData->staticData->name, cnvName);

          writeConverterData(mySharedData, cnvName, destdir, &err);
          makeconv_deleteSharedConverterData(mySharedData);

          if(U_FAILURE(err))
          {
                  /* if an error is found, print out an error msg and keep going*/
            fprintf(stderr, "Error writing \"%s\" file for \"%s\" (error code %d - %s)\n", outFileName, arg, err,
                    u_errorName(err));
          }
          else
          {
              puts(outFileName);
          }
        }
      
    }

  return err;
}


UConverterPlatform getPlatformFromName(char* name)
{
  char myPlatform[10];
  char mySeparators[2] = { '-', '\0' };
  
  getToken(myPlatform, name, mySeparators);
  strtoupper(myPlatform);

  if (uprv_strcmp(myPlatform, "IBM") == 0)
    return UCNV_IBM;
  else
    return UCNV_UNKNOWN;
}

int32_t getCodepageNumberFromName(char* name)
{
  char myNumber[10];
  char mySeparators[2] = { '-', '\0' };
  char* line = NULL;
 
  line = getToken(myNumber, name, mySeparators);
  getToken(myNumber, line, mySeparators);

  return T_CString_stringToInteger(myNumber, 10);
}

/*Reads the header of the table file and fills in basic knowledge about the converter in "converter"*/
void readHeaderFromFile(UConverterSharedData* mySharedData,
                        FileStream* convFile,
                        const char* converterName,
                        UErrorCode* err)
{
  char storeLine[200];
  char key[16];
  char value[200];
  char* line = storeLine;
  UBool endOfHeader = FALSE;
  UBool hasConvClass = FALSE;
  UBool hasSubChar = FALSE;
  char codepointByte[3];

  UConverterStaticData *myConverter = (UConverterStaticData *)mySharedData->staticData;

  if (U_FAILURE(*err)) return;
  while (T_FileStream_readLine(convFile, line, sizeof(storeLine))) 
    {
      removeComments(line);
      
      /*skip blank lines*/
      if (*(line + nextTokenOffset(line, NLTC_SEPARATORS)) != '\0') 
        {         
          /*gets the key that will qualify adjacent information*/
          /*gets the adjacent value*/
          line = getToken(key, line, NLTC_SEPARATORS);
          if (uprv_strcmp(key, "uconv_class") == 0)
            line = getToken(value, line, PLAIN_SEPARATORS);
          else if (uprv_strcmp(key, "icu:state") == 0)
            line = getToken(value, line, STATE_SEPARATORS);
          else
            line = getToken(value, line, NLTC_SEPARATORS);

          
          /*
             Figure out what key was found and fills in myConverter with the appropriate values
             a switch statement for strings...
             */

          /*Checks for end of header marker*/
          if (uprv_strcmp(key, "CHARMAP") == 0)
            {
              endOfHeader = TRUE;
              break;
            }

          /*get name tag*/
          else if (uprv_strcmp(key, "code_set_name") == 0) 
            {
              if (uprv_strlen(value) != 0) 
              {
                  uprv_strcpy((char*)myConverter->name, value);
                  myConverter->platform = getPlatformFromName(value);
                  myConverter->codepage = getCodepageNumberFromName(value);
              } else {
                  uprv_strcpy((char*)myConverter->name, converterName);
                  myConverter->platform = UCNV_IBM;
              }

            }

          /*get conversion type*/
          else if (uprv_strcmp(key, "uconv_class") == 0)
            {

              hasConvClass = TRUE;
#if DEBUG
                printf("    %s\n", value);
                fflush(stdout);
#endif
              if (uprv_strcmp(value, "DBCS") == 0) 
                {
                  myConverter->conversionType = UCNV_DBCS;
                }
              else if (uprv_strcmp(value, "SBCS") == 0) 
                {
                  myConverter->conversionType = UCNV_SBCS;
                }
              else if (uprv_strcmp(value, "MBCS") == 0) 
                {
                  myConverter->conversionType = UCNV_MBCS;
                }
              else if (uprv_strcmp(value, "EBCDIC_STATEFUL") == 0) 
                {
                  myConverter->conversionType = UCNV_EBCDIC_STATEFUL;
                }
              else 
                {
                  *err = U_INVALID_TABLE_FORMAT;
                  return;
                }

              /* Now that we know the type, copy any 'default' values
                 from the table. */
              {
                const UConverterStaticData *prototype = ucnv_converterStaticData[myConverter->conversionType];
                if ( prototype != NULL )
                {
                  if ( myConverter->name[0] == 0 )
                  {
                    uprv_strcpy((char*)myConverter->name, prototype->name);
                  }

                  if ( myConverter->codepage == 0 )
                  {
                    myConverter->codepage = prototype->codepage;
                  }

                  if ( myConverter->platform == 0 )
                  {
                    myConverter->platform = prototype->platform;
                  }

                  if ( myConverter->minBytesPerChar == 0 )
                  {
                    myConverter->minBytesPerChar = prototype->minBytesPerChar;
                  }

                  if ( myConverter->maxBytesPerChar == 0 )
                  {
                    myConverter->maxBytesPerChar = prototype->maxBytesPerChar;
                  }

                  if ( myConverter->subCharLen == 0 )
                  {
                    myConverter->subCharLen = prototype->subCharLen;
                    uprv_memcpy(myConverter->subChar, prototype->subChar,
                                prototype->subCharLen);
                  }
                }
              }

            }
          
          /*get mb_cur_max amount*/
          else if (uprv_strcmp(key, "mb_cur_max") == 0) 
            myConverter->maxBytesPerChar = (int8_t)T_CString_stringToInteger(value, 10);
          
          /*get mb_cur_max amount*/
          else if (uprv_strcmp(key, "mb_cur_min") == 0)
            myConverter->minBytesPerChar = (int8_t)T_CString_stringToInteger(value, 10);
         
          
          else if (uprv_strcmp(key, "subchar") == 0) 
            {
              hasSubChar = TRUE;
              myConverter->subCharLen = 0;
              
              /*readies value for tokenizing, we want to break each byte of the codepoint into single tokens*/
              line = value;
              while (*line)
                {
                  line = getToken(codepointByte, line, CODEPOINT_SEPARATORS);
                  myConverter->subChar[(myConverter->subCharLen++)] =
                    (unsigned char)T_CString_stringToInteger(codepointByte, 16);
                }
              
              /*Initializes data from the mutable area to that found in the immutable area*/
              
            }
          else if (uprv_strcmp(key, "icu:state") == 0)
            {
              if (myConverter->conversionType != UCNV_MBCS)
                {
                  fprintf(stderr, "error: <icu:state> entry for non-MBCS table or before the <uconv_class> line\n");
                  *err = U_INVALID_TABLE_FORMAT;
                  break;
                }
              if (myConverter->maxBytesPerChar == 0)
                {
                  fprintf(stderr, "error: <icu:state> before the <mb_cur_max> line\n");
                  *err = U_INVALID_TABLE_FORMAT;
                  break;
                }
              if (mySharedData->table == NULL)
                {
                  mySharedData->table = (UConverterTable *)MBCSOpen(myConverter->maxBytesPerChar);
                  if (mySharedData->table == NULL)
                    {
                      *err = U_MEMORY_ALLOCATION_ERROR;
                      break;
                    }
                }
              if (!MBCSAddState((MBCSData *)mySharedData->table, value))
                {
                  *err = U_INVALID_TABLE_FORMAT;
                }
            }
        }
      /*make line point to the beginning of the storage buffer again*/
      line = storeLine;
    }

  if (!endOfHeader || !hasConvClass)
    {
      *err = U_INVALID_TABLE_FORMAT;
    }
  else if (myConverter->conversionType == UCNV_MBCS && mySharedData->table == NULL)
    {
      fprintf(stderr, "error: missing state table information (<icu:state>) for MBCS\n");
      *err = U_INVALID_TABLE_FORMAT;
    }
  return;
}
  
UConverterTable *loadSBCSTableFromFile(FileStream* convFile, UConverterStaticData* myConverter, UErrorCode* err)
{
  char storageLine[200];
  char* line = NULL;
  UConverterTable* myUConverterTable = NULL;
  UChar unicodeValue = 0xfffe;
  int32_t sbcsCodepageValue = 0, fallback = 0;
  UBool seenFallback = FALSE;
  char codepointBytes[5];
  int32_t i = 0;
  CompactByteArray *myFromUnicode = NULL, *myFromUnicodeFallback = NULL;

  
  if (U_FAILURE(*err)) return NULL;
  myUConverterTable = (UConverterTable*)uprv_malloc(sizeof(UConverterSBCSTable));

  if (myUConverterTable == NULL) 
    {
      *err = U_MEMORY_ALLOCATION_ERROR;
      return NULL;
    }

  uprv_memset(myUConverterTable, 0, sizeof(UConverterSBCSTable));
  myConverter->hasFromUnicodeFallback = myConverter->hasToUnicodeFallback = FALSE;
  /*create a compact array with replacement chars as default chars*/
  ucmp8_init(&myUConverterTable->sbcs.fromUnicode, 0);
  myFromUnicode = &myUConverterTable->sbcs.fromUnicode;
  /*create a bogus compact array */
  ucmp8_initBogus(&myUConverterTable->sbcs.fromUnicodeFallback);
  myFromUnicodeFallback = &myUConverterTable->sbcs.fromUnicodeFallback;
  if (myFromUnicode == NULL)  
    {
      uprv_free(myUConverterTable);
      *err = U_MEMORY_ALLOCATION_ERROR;
      return NULL;
    } 

  myUConverterTable->sbcs.toUnicode = (UChar*)malloc(sizeof(UChar)*256);
  myUConverterTable->sbcs.toUnicodeFallback = (UChar*)malloc(sizeof(UChar)*256);
  /*fills in the toUnicode array with the Unicode Replacement Char*/
  for (i=0;i<=0xFF;i++) 
  {
      myUConverterTable->sbcs.toUnicode[i] = unicodeValue;
      myUConverterTable->sbcs.toUnicodeFallback[i] = unicodeValue;
  }
  while (T_FileStream_readLine(convFile, storageLine, sizeof(storageLine)))
    {
      /*removes comments*/
      removeComments(storageLine);

      /*set alias pointer back to the beginning of the buffer*/
      line = storageLine;
      
      /*skips empty lines*/
      if (line[nextTokenOffset(line, NLTC_SEPARATORS)] != '\0')
        {
          line = getToken(codepointBytes, line, UNICODE_CODEPOINT_SEPARATORS);
          if (!uprv_strcmp(codepointBytes, "END")) break;
          unicodeValue = (UChar)T_CString_stringToInteger(codepointBytes, 16);
          line = getToken(codepointBytes, line, CODEPOINT_SEPARATORS);
          sbcsCodepageValue = T_CString_stringToInteger(codepointBytes, 16);
          /* hsys: check fallback value here... */
          line = uprv_strchr(line, FALLBACK_SEPARATOR);
          uprv_memset(codepointBytes, 0, 5);
          if (line != NULL)
          {
              uprv_memcpy(codepointBytes, line+1, 1);
          }
          fallback = T_CString_stringToInteger(codepointBytes, 10);
          if (fallback == 0) {
              /*Store in the toUnicode array*/
              myUConverterTable->sbcs.toUnicode[sbcsCodepageValue] = unicodeValue;
              /*Store in the fromUnicode compact array*/
              ucmp8_set(myFromUnicode, unicodeValue, (int8_t)sbcsCodepageValue);
          } else if (fallback == 1) {
              /* Check if this fallback is in the toUnicode or fromUnicode table */
              if (seenFallback == FALSE) 
              {
                  myConverter->hasToUnicodeFallback = myConverter->hasFromUnicodeFallback = seenFallback = TRUE;
                  ucmp8_init(myFromUnicodeFallback, 0);
              }
              myUConverterTable->sbcs.toUnicodeFallback[sbcsCodepageValue] = unicodeValue;
              ucmp8_set(myFromUnicodeFallback, unicodeValue, (int8_t)sbcsCodepageValue);
          }
      }
  }
  seenFallback = FALSE;
  for (i = 0; i < 256; i++) 
  {
      if ((myUConverterTable->sbcs.toUnicode[i] >= 0xfffe) &&
          (myUConverterTable->sbcs.toUnicodeFallback[i] < 0xfffe))
          
      {
          seenFallback = TRUE;
          break;
      }
  }
  if (seenFallback == FALSE)
  {
      free(myUConverterTable->sbcs.toUnicodeFallback);
      myUConverterTable->sbcs.toUnicodeFallback = NULL;
      myConverter->hasToUnicodeFallback = FALSE;
  } 
  else if (myConverter->hasFromUnicodeFallback == TRUE)
  {
      ucmp8_compact(myFromUnicodeFallback, 1);
  }
  ucmp8_compact(myFromUnicode, 1);
  /*Initially sets the referenceCounter to 1*/
  
  return myUConverterTable;
}

void loadMBCSTableFromFile(FileStream* convFile, UConverterSharedData* sharedData, UErrorCode* err)
{
  char storageLine[200];
  char* line = NULL;
  MBCSData *mbcsData = (MBCSData *)sharedData->table;
  UChar32 unicodeValue;
  uint8_t mbcsBytes[8];
  int32_t mbcsLength;
  char codepointBytes[20];
  int8_t isFallback;
  UBool isOK = TRUE;
  uint8_t precisionMask = 0;
  char endOfLine;

  /* before this, the MBCSData is allocated and initialized with the state table info */
  MBCSProcessStates(mbcsData);

  while (T_FileStream_readLine(convFile, storageLine, sizeof(storageLine)))
    {
      removeComments(storageLine);
      line = storageLine;
      if (line[nextTokenOffset(line, NLTC_SEPARATORS)] != '\0')
        {
          /* get the Unicode code point */
          line = getToken(codepointBytes, line, UNICODE_CODEPOINT_SEPARATORS);
          if (uprv_strcmp(codepointBytes, "END") == 0)
            {
              break;
            }
          unicodeValue = (UChar32)T_CString_stringToInteger(codepointBytes, 16);

          /* get the codepage bytes */
          mbcsLength = 0;
          do
            {
              line = getToken(codepointBytes, line, CODEPOINT_SEPARATORS);
              mbcsBytes[mbcsLength++] = (uint8_t)T_CString_stringToInteger(codepointBytes, 16);

              /* End of line could be \0 or | (if fallback) */
              endOfLine= line[nextTokenOffset(line, CODEPOINT_SEPARATORS)];
            } while((endOfLine != '\0') && (endOfLine != FALLBACK_SEPARATOR));
          if(endOfLine == FALLBACK_SEPARATOR)
            {
              /* we know that there is a fallback separator */
              precisionMask |= 1;
              line = uprv_strchr(line, FALLBACK_SEPARATOR) + 1;
              if(*line == '2')
                {
                  /* skip subchar mappings */
                  continue;
                }
              else if(*line == '3')
              {
                  /* set only a fallback mapping from codepage to Unicode */
                  isOK &= MBCSAddToUnicode(mbcsData, mbcsBytes, mbcsLength, unicodeValue, 1);
                  continue;
              }
              else
                {
                  isFallback = (int8_t)(*line == '1');
                }
            }
          else
            {
              precisionMask |= 2;
              isFallback = -1;
            }

          /* set the mappings */
          isOK &= MBCSAddToUnicode(mbcsData, mbcsBytes, mbcsLength, unicodeValue, isFallback) &&
                  MBCSAddFromUnicode(mbcsData, mbcsBytes, mbcsLength, unicodeValue, isFallback);
        }
    }

  MBCSPostprocess(mbcsData);
  if(!isOK)
    {
      *err = U_INVALID_TABLE_FORMAT;
    }
  else if(precisionMask == 0 || precisionMask == 3)
    {
      fprintf(stderr, "error: some entries have the mapping precision (with '|'), some do not\n");
      *err = U_INVALID_TABLE_FORMAT;
    }
}

UConverterTable *loadEBCDIC_STATEFULTableFromFile(FileStream* convFile, UConverterStaticData* myConverter, UErrorCode* err)
{
  char storageLine[200];
  char* line = NULL;
  UConverterTable* myUConverterTable = NULL;
  UChar unicodeValue = 0xfffe;
  int32_t mbcsCodepageValue = '\0';
  char codepointBytes[6];
  int32_t replacementChar = 0x0000, fallback = 0;
  uint32_t i = 0;
  char endOfLine;
  UBool seenFallback = FALSE;
  CompactShortArray* myFromUnicode = NULL;
  CompactShortArray* myToUnicode = NULL;
  CompactShortArray* myFromUnicodeFallback = NULL;
  CompactShortArray* myToUnicodeFallback = NULL;

  /*Evaluates the replacement codepoint*/
  replacementChar = 0xFFFF;

  myUConverterTable = (UConverterTable*)uprv_malloc(sizeof(UConverterDBCSTable));
  if (myUConverterTable == NULL) 
    {
      *err = U_MEMORY_ALLOCATION_ERROR;
      return NULL;
    }
  
  uprv_memset(myUConverterTable, 0, sizeof(UConverterDBCSTable));
  
  myFromUnicode = &myUConverterTable->dbcs.fromUnicode;
  ucmp16_init(myFromUnicode, (uint16_t)replacementChar);
  myToUnicode = &myUConverterTable->dbcs.toUnicode;
  ucmp16_init(myToUnicode, (uint16_t)0xfffe);  

  myFromUnicodeFallback = &myUConverterTable->dbcs.fromUnicodeFallback;
  ucmp16_initBogus(myFromUnicodeFallback);
  myToUnicodeFallback = &myUConverterTable->dbcs.toUnicodeFallback;
  ucmp16_initBogus(myToUnicodeFallback);  

  while (T_FileStream_readLine(convFile, storageLine, sizeof(storageLine)))
    {
      removeComments(storageLine);
      line = storageLine;
      if (line[nextTokenOffset(line, NLTC_SEPARATORS)] != '\0')
        {
          line = getToken(codepointBytes, line, UNICODE_CODEPOINT_SEPARATORS);
          if (!uprv_strcmp(codepointBytes, "END")) break;
          unicodeValue = (UChar)T_CString_stringToInteger(codepointBytes, 16);
          line = getToken(codepointBytes, line, CODEPOINT_SEPARATORS);
          endOfLine= line[nextTokenOffset(line, CODEPOINT_SEPARATORS)];
          if ( (endOfLine!= '\0') && (endOfLine != FALLBACK_SEPARATOR) )
            {
              /*two-byter!*/
              line = getToken(codepointBytes+2, line, CODEPOINT_SEPARATORS);
            }
          
          mbcsCodepageValue = T_CString_stringToInteger(codepointBytes, 16);
          
          line = uprv_strchr(line, FALLBACK_SEPARATOR);
          uprv_memset(codepointBytes, 0, 6);
          if (line != NULL)
          {
              uprv_memcpy(codepointBytes, line+1, 1);
          }
          fallback = T_CString_stringToInteger(codepointBytes, 10);
          if (fallback == 0) 
          {
              ucmp16_set(myToUnicode, (int16_t)mbcsCodepageValue, unicodeValue);
              ucmp16_set(myFromUnicode, unicodeValue, (int16_t)mbcsCodepageValue);
          } 
          else if (fallback == 1)
          {
              /* Check if this fallback is in the toUnicode or fromUnicode table */
              if (seenFallback == FALSE) 
              {
                  myConverter->hasFromUnicodeFallback = myConverter->hasToUnicodeFallback = seenFallback = TRUE;
                  ucmp16_init(myFromUnicodeFallback, (uint16_t)replacementChar);
                  ucmp16_init(myToUnicodeFallback, (uint16_t)0xfffe);
              }
              ucmp16_set(myToUnicodeFallback, (int16_t)mbcsCodepageValue, unicodeValue);
              ucmp16_set(myFromUnicodeFallback, unicodeValue, (int16_t)mbcsCodepageValue);
          }
      }
  }
  seenFallback = FALSE;
  if (myConverter->hasToUnicodeFallback == TRUE)
  {
      for (i = 0; i < (uint32_t)ucmp16_getkUnicodeCount(); i++) 
      {
        if ((ucmp16_getu(myToUnicode, i) >= 0xfffe) &&
            (ucmp16_getu(myToUnicodeFallback, i) < 0xfffe))
        {
            seenFallback = TRUE;
            break;
        }
      }
      if (seenFallback == FALSE)
      {
          ucmp16_close(myToUnicodeFallback);
          myConverter->hasToUnicodeFallback = FALSE;
      } 
      else if (myConverter->hasFromUnicodeFallback == TRUE)
      {
          ucmp16_compact(myFromUnicodeFallback);
          ucmp16_compact(myToUnicodeFallback);
      }
  }

  ucmp16_compact(myFromUnicode);
  ucmp16_compact(myToUnicode);

  return myUConverterTable;
}


UConverterTable * loadDBCSTableFromFile(FileStream* convFile, UConverterStaticData* myConverter, UErrorCode* err)
{
  char storageLine[200];
  char* line = NULL;
  UConverterTable* myUConverterTable = NULL;
  UChar unicodeValue = 0xfffe;
  int32_t dbcsCodepageValue = '\0';
  char codepointBytes[6];
  int32_t replacementChar = 0x0000, fallback = 0;
  uint32_t i = 0;
  UBool seenFallback = FALSE;
  CompactShortArray* myFromUnicode = NULL;
  CompactShortArray* myToUnicode = NULL;
  CompactShortArray* myFromUnicodeFallback = NULL;
  CompactShortArray* myToUnicodeFallback = NULL;
  
  /*Evaluates the replacement codepoint*/
  replacementChar = 0xFFFF;

  myUConverterTable = (UConverterTable*)uprv_malloc(sizeof(UConverterDBCSTable));
  if (myUConverterTable == NULL) 
    {
      *err = U_MEMORY_ALLOCATION_ERROR;
      return NULL;
    }

  uprv_memset(myUConverterTable, 0, sizeof(UConverterDBCSTable));

  myFromUnicode = &(myUConverterTable->dbcs.fromUnicode);
  ucmp16_init(myFromUnicode, (int16_t)replacementChar);
  myToUnicode = &(myUConverterTable->dbcs.toUnicode);
  ucmp16_init(myToUnicode, (uint16_t)0xfffe);
  
  myFromUnicodeFallback = &(myUConverterTable->dbcs.fromUnicodeFallback);
  ucmp16_initBogus(myFromUnicodeFallback);
  myToUnicodeFallback = &(myUConverterTable->dbcs.toUnicodeFallback);
  ucmp16_initBogus(myToUnicodeFallback);

  while (T_FileStream_readLine(convFile, storageLine, sizeof(storageLine)))
    {
      removeComments(storageLine);
      line = storageLine;
      if (line[nextTokenOffset(line, NLTC_SEPARATORS)] != '\0')
        {
          line = getToken(codepointBytes, line, UNICODE_CODEPOINT_SEPARATORS);
          if (!uprv_strcmp(codepointBytes, "END")) break;
          unicodeValue = (UChar)T_CString_stringToInteger(codepointBytes, 16);
          
          /*first byte*/
          line = getToken(codepointBytes, line, CODEPOINT_SEPARATORS);
          
          /*second byte*/
          line = getToken(codepointBytes+2, line, CODEPOINT_SEPARATORS);
        }
      
      dbcsCodepageValue = T_CString_stringToInteger(codepointBytes, 16);
          line = uprv_strchr(line, FALLBACK_SEPARATOR);
          uprv_memset(codepointBytes, 0, 6);
          if (line != NULL)
          {
              uprv_memcpy(codepointBytes, line+1, 1);
          }
      fallback = T_CString_stringToInteger(codepointBytes, 10);
      if (fallback == 0) 
      {
          ucmp16_set(myToUnicode, (int16_t)dbcsCodepageValue, unicodeValue);
          ucmp16_set(myFromUnicode, unicodeValue, (int16_t)dbcsCodepageValue);
      }
      else if (fallback == 1)
      {
          /* Check if this fallback is in the toUnicode or fromUnicode table */
          if (seenFallback == FALSE) 
          {
              myConverter->hasFromUnicodeFallback = myConverter->hasToUnicodeFallback = seenFallback = TRUE;
              ucmp16_init(myFromUnicodeFallback, (uint16_t)replacementChar);
              ucmp16_init(myToUnicodeFallback, (uint16_t)0xfffe);
          }
          ucmp16_set(myToUnicodeFallback, (int16_t)dbcsCodepageValue, unicodeValue);
          ucmp16_set(myFromUnicodeFallback, unicodeValue, (int16_t)dbcsCodepageValue);
      }
  }
  seenFallback = FALSE;
  if (myConverter->hasToUnicodeFallback == TRUE)
  {
      for (i = 0; i < (uint32_t)ucmp16_getkUnicodeCount(); i++) 
      {
        if ((ucmp16_getu(myToUnicode, i) >= 0xfffe) &&
            (ucmp16_getu(myToUnicodeFallback, i) < 0xfffe))
        {
            seenFallback = TRUE;
            break;
        }
      }
      if (seenFallback == FALSE)
      {
          ucmp16_close(myToUnicodeFallback);
          myConverter->hasToUnicodeFallback = FALSE;
      } 
      else if (myConverter->hasFromUnicodeFallback == TRUE)
      {
          ucmp16_compact(myFromUnicodeFallback);
          ucmp16_compact(myToUnicodeFallback);
      }
  }
  
  ucmp16_compact(myFromUnicode);
  ucmp16_compact(myToUnicode);
  
  return myUConverterTable;
}

/*deletes the "shared" type object*/
UBool makeconv_deleteSharedConverterData(UConverterSharedData* deadSharedData)
{
  if (deadSharedData->staticData->conversionType == UCNV_SBCS)
    {
      ucmp8_close(&(deadSharedData->table->sbcs.fromUnicode));
      if (deadSharedData->staticData->hasFromUnicodeFallback == TRUE)
          ucmp8_close(&(deadSharedData->table->sbcs.fromUnicodeFallback));
      uprv_free(deadSharedData->table);
      uprv_free(deadSharedData);
    }
  else if (deadSharedData->staticData->conversionType == UCNV_MBCS)
    {
      MBCSClose((MBCSData *)deadSharedData->table);
      uprv_free((UConverterStaticData*)deadSharedData->staticData);
      uprv_free(deadSharedData);
    }
  else if ((deadSharedData->staticData->conversionType == UCNV_DBCS) || (deadSharedData->staticData->conversionType == UCNV_EBCDIC_STATEFUL))
    {
      ucmp16_close(&(deadSharedData->table->dbcs.fromUnicode));
      ucmp16_close(&(deadSharedData->table->dbcs.toUnicode));
      if (deadSharedData->staticData->hasFromUnicodeFallback == TRUE)
          ucmp16_close(&(deadSharedData->table->dbcs.fromUnicodeFallback));
      if (deadSharedData->staticData->hasToUnicodeFallback == TRUE)
          ucmp16_close(&(deadSharedData->table->dbcs.toUnicodeFallback));
      uprv_free(deadSharedData->table);
      uprv_free((UConverterStaticData*)deadSharedData->staticData);
      uprv_free(deadSharedData);
    }
  else
    { /* ? */
      uprv_free(deadSharedData->table);
      uprv_free((UConverterStaticData*)deadSharedData->staticData);
      uprv_free(deadSharedData);
    }
  return TRUE;
}



/*creates a UConverterStaticData, fills in necessary links to it the appropriate function pointers*/
UConverterSharedData* createConverterFromTableFile(const char* converterName, UErrorCode* err)
{
  FileStream* convFile = NULL;
  UConverterSharedData* mySharedData = NULL;
  UConverterStaticData* myStaticData = NULL;

  if (U_FAILURE(*err)) return NULL;
  
  convFile = T_FileStream_open(converterName, "r");
  if (convFile == NULL) 
    {
      *err = U_FILE_ACCESS_ERROR;
      return NULL;
    }
  
  
  mySharedData = (UConverterSharedData*) uprv_malloc(sizeof(UConverterSharedData));
  if (mySharedData == NULL)
    {
      *err = U_MEMORY_ALLOCATION_ERROR;
      T_FileStream_close(convFile);
      return NULL;
    }
  
  uprv_memset(mySharedData, 0, sizeof(UConverterSharedData));
  
  mySharedData->structSize = sizeof(UConverterSharedData);

  myStaticData =  (UConverterStaticData*) uprv_malloc(sizeof(UConverterStaticData));
  if (myStaticData == NULL)
    {
      *err = U_MEMORY_ALLOCATION_ERROR;
      T_FileStream_close(convFile);
      return NULL;
    }  
  uprv_memset(myStaticData, 0, sizeof(UConverterStaticData));
  mySharedData->staticData = myStaticData;
  myStaticData->structSize = sizeof(UConverterStaticData);
  mySharedData->staticDataOwned = TRUE;

  uprv_strcpy(myStaticData->name, converterName);

  mySharedData->dataMemory = NULL; /* for init */

  readHeaderFromFile(mySharedData, convFile, converterName, err);

  if (U_FAILURE(*err)) return NULL;
  
  switch (myStaticData->conversionType)
    {
    case UCNV_SBCS: 
      {
        mySharedData->table = loadSBCSTableFromFile(convFile, myStaticData, err);
        break;
      }
    case UCNV_MBCS: 
      {
        loadMBCSTableFromFile(convFile, mySharedData, err);
        break;
      }
    case UCNV_EBCDIC_STATEFUL: 
      {
        mySharedData->table = loadEBCDIC_STATEFULTableFromFile(convFile, myStaticData, err);
        break;
      }
    case UCNV_DBCS: 
      {
        mySharedData->table = loadDBCSTableFromFile(convFile, myStaticData, err);
        break;
      }

    default : 
      mySharedData->table = NULL;
      break;
    };

  T_FileStream_close(convFile);
  
  return mySharedData;
}



static void WriteConverterSharedData(UNewDataMemory *pData, const UConverterSharedData* data)
{
    uint32_t size = 0;

    /* all read only, clean, platform independent data.  Mmmm. :)  */
    udata_writeBlock(pData, data->staticData, sizeof(UConverterStaticData));
    size += sizeof(UConverterStaticData); /* Is 4-aligned  - by size */
    /* Now, write the table .. Please note, the size of this table is
     * */
    switch (data->staticData->conversionType)
    {
    case UCNV_SBCS:    {
        udata_writeBlock(pData, (void*)data->table->sbcs.toUnicode, sizeof(uint16_t)*256);
        size += sizeof(uint16_t)*256;
        size += udata_write_ucmp8(pData, &data->table->sbcs.fromUnicode);
        if (data->staticData->hasFromUnicodeFallback == TRUE)
        {
            if (size%4)
            {
                udata_writePadding(pData, 4-(size%4));
                size+= 4-(size%4);
            }
            size += udata_write_ucmp8(pData, &data->table->sbcs.fromUnicodeFallback);
        }
        if (data->staticData->hasToUnicodeFallback == TRUE)
        {
            if (size%4)
            {
                udata_writePadding(pData, 4-(size%4));
                size+= 4-(size%4);
            }
            udata_writeBlock(pData, (void*)data->table->sbcs.toUnicodeFallback, sizeof(uint16_t)*256);
            /* don't care about alignment anymore */
        }
      }
    break;
 
    case UCNV_DBCS:
    case UCNV_EBCDIC_STATEFUL:
      {
        size += udata_write_ucmp16(pData,&data->table->dbcs.toUnicode);
        if(size%4)
        {
            udata_writePadding(pData, 4-(size%4) );
            size+= 4-(size%4);
        }
        size += udata_write_ucmp16(pData,&data->table->dbcs.fromUnicode);
        if (data->staticData->hasFromUnicodeFallback == TRUE)
        {
            if(size%4)
            {
                udata_writePadding(pData, 4-(size%4) );
                size+= 4-(size%4);
            }
            size += udata_write_ucmp16(pData,&data->table->dbcs.fromUnicodeFallback);
        }
        if (data->staticData->hasToUnicodeFallback == TRUE)
        {

            if(size%4)
            {
                udata_writePadding(pData, 4-(size%4) );
                size+= 4-(size%4);
            }
            size += udata_write_ucmp16(pData,&data->table->dbcs.toUnicodeFallback);
        }
      }
      break;

    case UCNV_MBCS:
      {
        size += MBCSWrite((MBCSData *)data->table, pData);
      }
      break;

    default:
      {
        /*If it isn't any of the above, the file is invalid */
        fprintf(stderr, "Error: bad converter type, can't write!!\n");
        exit(1);
        return; /* 0; */
      }
    }
  
}

/*
 * Hey, Emacs, please set the following:
 *
 * Local Variables:
 * indent-tabs-mode: nil
 * End:
 *
 */
