/*
*******************************************************************************
*
*   Copyright (C) 1999-2011, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  genprops.cpp
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 1999dec08
*   created by: Markus W. Scherer
*
*   This program parses the ppucd.txt preparsed Unicode Character Database file
*   and writes several source and binary files into the ICU source tree.
*/

#include <stdio.h>
#include <stdlib.h>
#include "unicode/utypes.h"
#include "unicode/localpointer.h"
#include "unicode/uniset.h"
#include "unicode/unistr.h"
#include "charstr.h"
#include "genprops.h"
#include "ppucd.h"
#include "toolutil.h"
#include "uoptions.h"

#define LENGTHOF(array) (int32_t)(sizeof(array)/sizeof((array)[0]))

U_NAMESPACE_USE

UBool beVerbose=FALSE;

PropsWriter::~PropsWriter() {}
void PropsWriter::setUnicodeVersion(const UVersionInfo) {}
void PropsWriter::setProps(const UniProps &, const UnicodeSet &, UErrorCode &) {}
void PropsWriter::finalizeData(UErrorCode &) {}
void PropsWriter::writeCSourceFile(const char *, UErrorCode &) {}
void PropsWriter::writeBinaryData(const char *, UBool, UErrorCode &) {}

enum {
    HELP_H,
    HELP_QUESTION_MARK,
    VERBOSE,
    COPYRIGHT
};

/* Keep these values in sync with the above enums */
static UOption options[]={
    UOPTION_HELP_H,
    UOPTION_HELP_QUESTION_MARK,
    UOPTION_VERBOSE,
    UOPTION_COPYRIGHT
};

extern int
main(int argc, char* argv[]) {
    U_MAIN_INIT_ARGS(argc, argv);
    argc=u_parseArgs(argc, argv, LENGTHOF(options), options);

    /* error handling, printing usage message */
    if(argc<0) {
        fprintf(stderr,
            "error in command line argument \"%s\"\n",
            argv[-argc]);
    }
    if(argc<2 || options[HELP_H].doesOccur || options[HELP_QUESTION_MARK].doesOccur) {
        /*
         * Broken into chunks because the C89 standard says the minimum
         * required supported string length is 509 bytes.
         */
        fprintf(stderr,
            "Usage: %s [-options] path/to/ICU/src/root\n"
            "\n"
            "Reads the preparsed UCD file path/to/ICU/src/root/source/data/unidata/ppucd.txt and\n"
            "writes source and binary data files with the character properties.\n"
            "(UCD=Unicode Character Database)\n"
            "\n",
            argv[0]);
        fprintf(stderr,
            "Options:\n"
            "\t-h or -? or --help  this usage text\n"
            "\t-v or --verbose     verbose output\n"
            "\t-c or --copyright   include a copyright notice\n");
        return argc<2 ? U_ILLEGAL_ARGUMENT_ERROR : U_ZERO_ERROR;
    }

    /* get the options values */
    beVerbose=options[VERBOSE].doesOccur;

    /* initialize */
    IcuToolErrorCode errorCode("genprops");
    LocalPointer<PropsWriter> corePropsWriter(createCorePropsWriter(errorCode));
    if(errorCode.isFailure()) {
        fprintf(stderr, "genprops: unable to create PropsWriters - %s\n", errorCode.errorName());
        return errorCode.reset();
    }

    CharString icuSrcRoot(argv[1], errorCode);

    CharString icuSource(icuSrcRoot, errorCode);
    icuSource.appendPathPart("source", errorCode);

    CharString icuSourceData(icuSource, errorCode);
    icuSourceData.appendPathPart("data", errorCode);

    CharString ppucdPath(icuSourceData, errorCode);
    ppucdPath.appendPathPart("unidata", errorCode);
    ppucdPath.appendPathPart("ppucd.txt", errorCode);

    PreparsedUCD ppucd(ppucdPath.data(), errorCode);
    if(errorCode.isFailure()) {
        fprintf(stderr, "genprops: unable to open %s - %s\n",
                ppucdPath.data(), errorCode.errorName());
        return errorCode.reset();
    }
    PreparsedUCD::LineType lineType;
    UnicodeSet newValues;
    while((lineType=ppucd.readLine(errorCode))!=PreparsedUCD::NO_LINE) {
        if(ppucd.lineHasPropertyValues()) {
            const UniProps *props=ppucd.getProps(newValues, errorCode);
            corePropsWriter->setProps(*props, newValues, errorCode);
        } else if(lineType==PreparsedUCD::UNICODE_VERSION_LINE) {
            const UVersionInfo &version=ppucd.getUnicodeVersion();
            corePropsWriter->setUnicodeVersion(version);
        }
        if(errorCode.isFailure()) {
            fprintf(stderr,
                    "genprops: error parsing or setting values from ppucd.txt line %ld - %s\n",
                    (long)ppucd.getLineNumber(), errorCode.errorName());
            return errorCode.reset();
        }
    }

    corePropsWriter->finalizeData(errorCode);
    if(errorCode.isFailure()) {
        fprintf(stderr, "genprops error: failure finalizing the data - %s\n",
                errorCode.errorName());
        return errorCode.reset();
    }

    // Write the files with the generated data.
    CharString sourceCommon(icuSource, errorCode);
    sourceCommon.appendPathPart("common", errorCode);

    CharString sourceDataIn(icuSourceData, errorCode);
    sourceDataIn.appendPathPart("in", errorCode);

    UBool withCopyright=options[COPYRIGHT].doesOccur;

    corePropsWriter->writeCSourceFile(sourceCommon.data(), errorCode);
    corePropsWriter->writeBinaryData(sourceDataIn.data(), withCopyright, errorCode);

    return errorCode;
}

/*
 * Hey, Emacs, please set the following:
 *
 * Local Variables:
 * indent-tabs-mode: nil
 * End:
 *
 */
