/*
*******************************************************************************
*
*   Copyright (C) 2001-2010, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  gennorm.c
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2001may25
*   created by: Markus W. Scherer
*
*   This program reads the Unicode character database text file,
*   parses it, and extracts the data for normalization.
*   It then preprocesses it and writes a binary file for efficient use
*   in various Unicode text normalization processes.
*/

#include <stdio.h>
#include <stdlib.h>
#include "unicode/utypes.h"
#include "unicode/uchar.h"
#include "unicode/ustring.h"
#include "unicode/putil.h"
#include "unicode/uclean.h"
#include "unicode/udata.h"
#include "unicode/uset.h"
#include "cmemory.h"
#include "cstring.h"
#include "unewdata.h"
#include "uoptions.h"
#include "uparse.h"
#include "unormimp.h"

U_CDECL_BEGIN
#include "gennorm.h"
U_CDECL_END

UBool beVerbose=FALSE;

/* prototypes --------------------------------------------------------------- */

static void
parseDerivedNormalizationProperties(const char *filename, UErrorCode *pErrorCode, UBool reportError);

static void
parseDB(const char *filename, UErrorCode *pErrorCode);

/* -------------------------------------------------------------------------- */

enum {
    HELP_H,
    HELP_QUESTION_MARK,
    VERBOSE,
    DESTDIR,
    SOURCEDIR,
    ICUDATADIR
};

static UOption options[]={
    UOPTION_HELP_H,
    UOPTION_HELP_QUESTION_MARK,
    UOPTION_VERBOSE,
    UOPTION_DESTDIR,
    UOPTION_SOURCEDIR,
    UOPTION_ICUDATADIR
};

extern int
main(int argc, char* argv[]) {
#if !UCONFIG_NO_NORMALIZATION
    char filename[300];
#endif
    const char *srcDir=NULL, *destDir=NULL, *suffix=NULL;
    char *basename=NULL;
    UErrorCode errorCode=U_ZERO_ERROR;

    U_MAIN_INIT_ARGS(argc, argv);

    /* preset then read command line options */
    options[DESTDIR].value=u_getDataDirectory();
    options[SOURCEDIR].value="";
    options[ICUDATADIR].value=u_getDataDirectory();
    argc=u_parseArgs(argc, argv, sizeof(options)/sizeof(options[0]), options);

    /* error handling, printing usage message */
    if(argc<0) {
        fprintf(stderr,
            "error in command line argument \"%s\"\n",
            argv[-argc]);
    }
    if(argc<0 || options[HELP_H].doesOccur || options[HELP_QUESTION_MARK].doesOccur) {
        /*
         * Broken into chucks because the C89 standard says the minimum
         * required supported string length is 509 bytes.
         */
        fprintf(stderr,
            "Usage: %s [-options] [suffix]\n"
            "\n"
            "Read the UnicodeData.txt file and other Unicode properties files and\n"
            "write nfc.txt and nfkc.txt files for gennorm2\n"
            "\n",
            argv[0]);
        fprintf(stderr,
            "Options:\n"
            "\t-h or -? or --help  this usage text\n"
            "\t-v or --verbose     verbose output\n");
        fprintf(stderr,
            "\t-d or --destdir     destination directory, followed by the path\n"
            "\t-s or --sourcedir   source directory, followed by the path\n"
            "\t-i or --icudatadir  directory for locating any needed intermediate data files,\n"
            "\t                    followed by path, defaults to <%s>\n"
            "\tsuffix              suffix that is to be appended with a '-'\n"
            "\t                    to the source file basenames before opening;\n"
            "\t                    'gennorm new' will read UnicodeData-new.txt etc.\n",
            u_getDataDirectory());
        return argc<0 ? U_ILLEGAL_ARGUMENT_ERROR : U_ZERO_ERROR;
    }

    /* get the options values */
    beVerbose=options[VERBOSE].doesOccur;
    srcDir=options[SOURCEDIR].value;
    destDir=options[DESTDIR].value;

    if(argc>=2) {
        suffix=argv[1];
    } else {
        suffix=NULL;
    }

#if !UCONFIG_NO_NORMALIZATION

    if (options[ICUDATADIR].doesOccur) {
        u_setDataDirectory(options[ICUDATADIR].value);
    }

    /*
     * Verify that we can work with properties
     * but don't call u_init() because that needs unorm.icu which we are just
     * going to build here.
     */
    {
        U_STRING_DECL(ideo, "[:Ideographic:]", 15);
        USet *set;

        U_STRING_INIT(ideo, "[:Ideographic:]", 15);
        set=uset_openPattern(ideo, -1, &errorCode);
        if(U_FAILURE(errorCode) || !uset_contains(set, 0xf900)) {
            fprintf(stderr, "gennorm is unable to work with properties (uprops.icu): %s\n", u_errorName(errorCode));
            exit(errorCode);
        }
        uset_close(set);
    }

    /* prepare the filename beginning with the source dir */
    uprv_strcpy(filename, srcDir);
    basename=filename+uprv_strlen(filename);
    if(basename>filename && *(basename-1)!=U_FILE_SEP_CHAR && *(basename-1)!=U_FILE_ALT_SEP_CHAR) {
        *basename++=U_FILE_SEP_CHAR;
    }

    /* initialize */
    init();

    /* process DerivedNormalizationProps.txt (name changed for Unicode 3.2, to <=31 characters) */
    if(suffix==NULL) {
        uprv_strcpy(basename, "DerivedNormalizationProps.txt");
    } else {
        uprv_strcpy(basename, "DerivedNormalizationProps");
        basename[30]='-';
        uprv_strcpy(basename+31, suffix);
        uprv_strcat(basename+31, ".txt");
    }
    parseDerivedNormalizationProperties(filename, &errorCode, FALSE);
    if(U_FAILURE(errorCode)) {
        /* can be only U_FILE_ACCESS_ERROR - try filename from before Unicode 3.2 */
        if(suffix==NULL) {
            uprv_strcpy(basename, "DerivedNormalizationProperties.txt");
        } else {
            uprv_strcpy(basename, "DerivedNormalizationProperties");
            basename[30]='-';
            uprv_strcpy(basename+31, suffix);
            uprv_strcat(basename+31, ".txt");
        }
        parseDerivedNormalizationProperties(filename, &errorCode, TRUE);
    }

    /* process UnicodeData.txt */
    if(suffix==NULL) {
        uprv_strcpy(basename, "UnicodeData.txt");
    } else {
        uprv_strcpy(basename, "UnicodeData");
        basename[11]='-';
        uprv_strcpy(basename+12, suffix);
        uprv_strcat(basename+12, ".txt");
    }
    parseDB(filename, &errorCode);

    /* process parsed data */
    if(U_SUCCESS(errorCode)) {
        writeNorm2(destDir);

        cleanUpData();
    }

#endif

    return errorCode;
}

#if !UCONFIG_NO_NORMALIZATION

/* parser for DerivedNormalizationProperties.txt ---------------------------- */

static void U_CALLCONV
derivedNormalizationPropertiesLineFn(void *context,
                                     char *fields[][2], int32_t fieldCount,
                                     UErrorCode *pErrorCode) {
    UChar string[32];
    char *s;
    uint32_t start, end;
    int32_t count;

    /* get code point range */
    count=u_parseCodePointRange(fields[0][0], &start, &end, pErrorCode);
    if(U_FAILURE(*pErrorCode)) {
        fprintf(stderr, "gennorm: error parsing DerivedNormalizationProperties.txt mapping at %s\n", fields[0][0]);
        exit(*pErrorCode);
    }

    /* ignore hangul - handle explicitly */
    if(start==0xac00) {
        return;
    }

    /* get property - ignore unrecognized ones */
    s=(char *)u_skipWhitespace(fields[1][0]);
    if(0==uprv_memcmp(s, "Comp_Ex", 7) || 0==uprv_memcmp(s, "Full_Composition_Exclusion", 26)) {
        /* full composition exclusion */
        while(start<=end) {
            setCompositionExclusion(start++);
        }
    }
}

static void
parseDerivedNormalizationProperties(const char *filename, UErrorCode *pErrorCode, UBool reportError) {
    char *fields[2][2];

    if(pErrorCode==NULL || U_FAILURE(*pErrorCode)) {
        return;
    }

    u_parseDelimitedFile(filename, ';', fields, 2, derivedNormalizationPropertiesLineFn, NULL, pErrorCode);
    if(U_FAILURE(*pErrorCode) && (reportError || *pErrorCode!=U_FILE_ACCESS_ERROR)) {
        fprintf(stderr, "gennorm error: u_parseDelimitedFile(\"%s\") failed - %s\n", filename, u_errorName(*pErrorCode));
        exit(*pErrorCode);
    }
}

/* parser for UnicodeData.txt ----------------------------------------------- */

static void U_CALLCONV
unicodeDataLineFn(void *context,
                  char *fields[][2], int32_t fieldCount,
                  UErrorCode *pErrorCode) {
    uint32_t decomp[40];
    Norm norm;
    const char *s;
    char *end;
    uint32_t code, value;
    int32_t length;
    UBool isCompat, something=FALSE;

    /* ignore First and Last entries for ranges */
    if( *fields[1][0]=='<' &&
        (length=(int32_t)(fields[1][1]-fields[1][0]))>=9 &&
        (0==uprv_memcmp(", First>", fields[1][1]-8, 8) || 0==uprv_memcmp(", Last>", fields[1][1]-7, 7))
    ) {
        return;
    }

    /* reset the properties */
    uprv_memset(&norm, 0, sizeof(Norm));

    /* get the character code, field 0 */
    code=(uint32_t)uprv_strtoul(fields[0][0], &end, 16);
    if(end<=fields[0][0] || end!=fields[0][1]) {
        fprintf(stderr, "gennorm: syntax error in field 0 at %s\n", fields[0][0]);
        *pErrorCode=U_PARSE_ERROR;
        exit(U_PARSE_ERROR);
    }

    /* get canonical combining class, field 3 */
    value=(uint32_t)uprv_strtoul(fields[3][0], &end, 10);
    if(end<=fields[3][0] || end!=fields[3][1] || value>0xff) {
        fprintf(stderr, "gennorm: syntax error in field 3 at %s\n", fields[0][0]);
        *pErrorCode=U_PARSE_ERROR;
        exit(U_PARSE_ERROR);
    }
    if(value>0) {
        norm.udataCC=(uint8_t)value;
        something=TRUE;
    }

    /* get the decomposition, field 5 */
    if(fields[5][0]<fields[5][1]) {
        if(*(s=fields[5][0])=='<') {
            ++s;
            isCompat=TRUE;

            /* skip and ignore the compatibility type name */
            do {
                if(s==fields[5][1]) {
                    /* missing '>' */
                    fprintf(stderr, "gennorm: syntax error in field 5 at %s\n", fields[0][0]);
                    *pErrorCode=U_PARSE_ERROR;
                    exit(U_PARSE_ERROR);
                }
            } while(*s++!='>');
        } else {
            isCompat=FALSE;
        }

        /* parse the decomposition string */
        length=u_parseCodePoints(s, decomp, sizeof(decomp)/4, pErrorCode);
        if(U_FAILURE(*pErrorCode)) {
            fprintf(stderr, "gennorm error parsing UnicodeData.txt decomposition of U+%04lx - %s\n",
                    (long)code, u_errorName(*pErrorCode));
            exit(*pErrorCode);
        }

        /* store the string */
        if(length>0) {
            something=TRUE;
            if(isCompat) {
                norm.lenNFKD=(uint8_t)length;
                norm.nfkd=decomp;
            } else {
                if(length>2) {
                    fprintf(stderr, "gennorm: error - length of NFD(U+%04lx) = %ld >2 in UnicodeData - illegal\n",
                            (long)code, (long)length);
                    *pErrorCode=U_PARSE_ERROR;
                    exit(U_PARSE_ERROR);
                }
                norm.lenNFD=(uint8_t)length;
                norm.nfd=decomp;
            }
        }
    }

    /* check for non-character code points */
    if((code&0xfffe)==0xfffe || (uint32_t)(code-0xfdd0)<0x20 || code>0x10ffff) {
        fprintf(stderr, "gennorm: error - properties for non-character code point U+%04lx\n",
                (long)code);
        *pErrorCode=U_PARSE_ERROR;
        exit(U_PARSE_ERROR);
    }

    if(something) {
        /* there are normalization values, so store them */
#if 0
        if(beVerbose) {
            printf("store values for U+%04lx: cc=%d, lenNFD=%ld, lenNFKD=%ld\n",
                   (long)code, norm.udataCC, (long)norm.lenNFD, (long)norm.lenNFKD);
        }
#endif
        storeNorm(code, &norm);
    }
}

static void
parseDB(const char *filename, UErrorCode *pErrorCode) {
    char *fields[15][2];

    if(pErrorCode==NULL || U_FAILURE(*pErrorCode)) {
        return;
    }

    u_parseDelimitedFile(filename, ';', fields, 15, unicodeDataLineFn, NULL, pErrorCode);
    if(U_FAILURE(*pErrorCode)) {
        fprintf(stderr, "gennorm error: u_parseDelimitedFile(\"%s\") failed - %s\n", filename, u_errorName(*pErrorCode));
        exit(*pErrorCode);
    }
}

#endif /* #if !UCONFIG_NO_NORMALIZATION */

/*
 * Hey, Emacs, please set the following:
 *
 * Local Variables:
 * indent-tabs-mode: nil
 * End:
 *
 */
