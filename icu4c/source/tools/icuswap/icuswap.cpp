/*
*******************************************************************************
*
*   Copyright (C) 2003, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  icuswap.cpp
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2003aug08
*   created by: Markus W. Scherer
*
*   This tool takes an ICU data file and "swaps" it, that is, changes its
*   platform properties between big-/little-endianness and ASCII/EBCDIC charset
*   families.
*   The modified data file is written to a new file.
*   Useful as an install-time tool for shipping only one flavor of ICU data
*   and preparing data files for the target platform.
*   Will not work with data DLLs (shared libraries).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unicode/utypes.h"
#include "unicode/udata.h"
#include "cmemory.h"
#include "cstring.h"
#include "uarrsort.h"
#include "ucmndata.h"
#include "udataswp.h"
#include "uoptions.h"

/* swapping implementations in common */

#include "uresdata.h"
#include "ucnv_io.h"
#include "uprops.h"
#include "ucol_swp.h"

/* swapping implementations in i18n */

/* definitions */

#define LENGTHOF(array) (int32_t)(sizeof(array)/sizeof((array)[0]))

static UOption options[]={
    UOPTION_HELP_H,
    UOPTION_HELP_QUESTION_MARK,
    UOPTION_DEF("type", 't', UOPT_REQUIRES_ARG)
};

enum {
    OPT_HELP_H,
    OPT_HELP_QUESTION_MARK,
    OPT_OUT_TYPE
};

static int32_t
fileSize(FILE *f) {
    int32_t size;

    fseek(f, 0, SEEK_END);
    size=(int32_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

/**
 * Identifies and then transforms the ICU data piece in-place, or determines
 * its length. See UDataSwapFn.
 * This function handles .dat data packages as well as single data pieces
 * and internally dispatches to per-type swap functions.
 * Sets a U_UNSUPPORTED_ERROR if the data format is not recognized.
 *
 * @see UDataSwapFn
 * @see udata_openSwapper
 * @see udata_openSwapperForInputData
 * @draft ICU 2.8
 */
static int32_t
udata_swap(const UDataSwapper *ds,
           const void *inData, int32_t length, void *outData,
           UErrorCode *pErrorCode);

/**
 * Swap an ICU .dat package, including swapping of enclosed items.
 */
U_CAPI int32_t U_EXPORT2
udata_swapPackage(const UDataSwapper *ds,
                  const void *inData, int32_t length, void *outData,
                  UErrorCode *pErrorCode);

static void U_CALLCONV
printError(void *context, const char *fmt, va_list args) {
    vfprintf((FILE *)context, fmt, args);
}

static int
printUsage(const char *pname, UBool ishelp) {
    fprintf(stderr,
            "%csage: %s [ -h, -?, --help ] -tl|-tb|-te|--type=b|... infilename outfilename\n",
            ishelp ? 'U' : 'u', pname);
    if(ishelp) {
        fprintf(stderr,
              "\nOptions: -h, -?, --help    print this message and exit\n"
                "         Read the input file, swap its platform properties according\n"
                "         to the -t or --type option, and write the result to the output file.\n"
                "         -tl               change to little-endian/ASCII charset family\n"
                "         -tb               change to big-endian/ASCII charset family\n"
                "         -te               change to little-endian/EBCDIC charset family\n");
    }

    return !ishelp;
}

extern int
main(int argc, char *argv[]) {
    FILE *in, *out;
    const char *pname;
    char *data;
    int32_t length;
    UBool ishelp;
    int rc;

    UDataSwapper *ds;
    UErrorCode errorCode;
    uint8_t outCharset;
    UBool outIsBigEndian;

    U_MAIN_INIT_ARGS(argc, argv);

    /* get the program basename */
    pname=strrchr(argv[0], U_FILE_SEP_CHAR);
    if(pname==NULL) {
        pname=strrchr(argv[0], '/');
    }
    if(pname!=NULL) {
        ++pname;
    } else {
        pname=argv[0];
    }

    argc=u_parseArgs(argc, argv, LENGTHOF(options), options);
    ishelp=options[OPT_HELP_H].doesOccur || options[OPT_HELP_QUESTION_MARK].doesOccur;
    if(ishelp || argc!=3) {
        return printUsage(pname, ishelp);
    }

    /* parse the output type option */
    data=(char *)options[OPT_OUT_TYPE].value;
    if(data[0]==0 || data[1]!=0) {
        /* the type must be exactly one letter */
        return printUsage(pname, FALSE);
    }
    switch(data[0]) {
    case 'l':
        outIsBigEndian=FALSE;
        outCharset=U_ASCII_FAMILY;
        break;
    case 'b':
        outIsBigEndian=TRUE;
        outCharset=U_ASCII_FAMILY;
        break;
    case 'e':
        outIsBigEndian=TRUE;
        outCharset=U_EBCDIC_FAMILY;
        break;
    default:
        return printUsage(pname, FALSE);
    }

    in=out=NULL;
    data=NULL;

    /* open the input file, get its length, allocate memory for it, read the file */
    in=fopen(argv[1], "rb");
    if(in==NULL) {
        fprintf(stderr, "%s: unable to open input file \"%s\"\n", pname, argv[1]);
        rc=2;
        goto done;
    }

    length=fileSize(in);
    if(length<=0) {
        fprintf(stderr, "%s: empty input file \"%s\"\n", pname, argv[1]);
        rc=2;
        goto done;
    }

    /*
     * +15: udata_swapPackage() may need to add a few padding bytes to the
     * last item if charset swapping is done,
     * because the last item may be resorted into the middle and then needs
     * additional padding bytes
     */
    data=(char *)malloc(length+15);
    if(data==NULL) {
        fprintf(stderr, "%s: error allocating memory for \"%s\"\n", pname, argv[1]);
        rc=2;
        goto done;
    }

    /* set the last 15 bytes to the usual padding byte, see udata_swapPackage() */
    uprv_memset(data+length-15, 0xaa, 15);

    if(length!=(int32_t)fread(data, 1, length, in)) {
        fprintf(stderr, "%s: error reading \"%s\"\n", pname, argv[1]);
        rc=3;
        goto done;
    }

    fclose(in);
    in=NULL;

    /* swap the data in-place */
    errorCode=U_ZERO_ERROR;
    ds=udata_openSwapperForInputData(data, length, outIsBigEndian, outCharset, &errorCode);
    if(U_FAILURE(errorCode)) {
        fprintf(stderr, "%s: udata_openSwapperForInputData(\"%s\") failed - %s\n",
                pname, argv[1], u_errorName(errorCode));
        rc=4;
        goto done;
    }

    ds->printError=printError;
    ds->printErrorContext=stderr;

    length=udata_swap(ds, data, length, data, &errorCode);
    udata_closeSwapper(ds);
    if(U_FAILURE(errorCode)) {
        fprintf(stderr, "%s: udata_swap(\"%s\") failed - %s\n",
                pname, argv[1], u_errorName(errorCode));
        rc=4;
        goto done;
    }

    out=fopen(argv[2], "wb");
    if(out==NULL) {
        fprintf(stderr, "%s: unable to open output file \"%s\"\n", pname, argv[2]);
        rc=5;
        goto done;
    }

    if(length!=(int32_t)fwrite(data, 1, length, out)) {
        fprintf(stderr, "%s: error writing \"%s\"\n", pname, argv[2]);
        rc=6;
        goto done;
    }

    fclose(out);
    out=NULL;

    /* all done */
    rc=0;

done:
    if(in!=NULL) {
        fclose(in);
    }
    if(out!=NULL) {
        fclose(out);
    }
    if(data!=NULL) {
        free(data);
    }
    return rc;
}

/* swap the data ------------------------------------------------------------ */

static const struct {
    uint8_t dataFormat[4];
    UDataSwapFn *swapFn;
} swapFns[]={
    { { 0x52, 0x65, 0x73, 0x42 }, ures_swap },          /* dataFormat="ResB" */
    { { 0x43, 0x76, 0x41, 0x6c }, ucnv_swapAliases },   /* dataFormat="CvAl" */
    { { 0x43, 0x6d, 0x6e, 0x44 }, udata_swapPackage },  /* dataFormat="CmnD" */
    /* insert data formats here, descending by expected frequency of occurrence */
    { { 0x55, 0x50, 0x72, 0x6f }, uprops_swap },        /* dataFormat="UPro" */
    { { 0x55, 0x43, 0x6f, 0x6c }, ucol_swap },          /* dataFormat="UCol" */
    { { 0x49, 0x6e, 0x76, 0x43 }, ucol_swapInverseUCA },/* dataFormat="InvC" */
    { { 0x75, 0x6e, 0x61, 0x6d }, uchar_swapNames }     /* dataFormat="unam" */
};

static int32_t
udata_swap(const UDataSwapper *ds,
           const void *inData, int32_t length, void *outData,
           UErrorCode *pErrorCode) {
    const UDataInfo *pInfo;
    int32_t headerSize, i;

    if(pErrorCode==NULL || U_FAILURE(*pErrorCode)) {
        return 0;
    }

    /*
     * Preflight the header first; checks for illegal arguments, too.
     * Do not swap the header right away because the format-specific swapper
     * will swap it, get the headerSize again, and also use the header
     * information. Otherwise we would have to pass some of the information
     * and not be able to use the UDataSwapFn signature.
     */
    headerSize=udata_swapDataHeader(ds, inData, -1, NULL, pErrorCode);

    /*
     * If we wanted udata_swap() to also handle non-loadable data like a UTrie,
     * then we could check here for further known magic values and structures.
     */
    if(U_FAILURE(*pErrorCode)) {
        return 0; /* the data format was not recognized */
    }

    /* dispatch to the swap function for the dataFormat */
    pInfo=(const UDataInfo *)((const char *)inData+4);
    for(i=0; i<LENGTHOF(swapFns); ++i) {
        if(0==memcmp(swapFns[i].dataFormat, pInfo->dataFormat, 4)) {
            length=swapFns[i].swapFn(ds, inData, length, outData, pErrorCode);

            if(U_FAILURE(*pErrorCode)) {
                /* data formats are chosen to be ASCII-readable mnemonics */
#               if(U_CHARSET_FAMILY==U_ASCII_FAMILY)
                    udata_printError(ds, "udata_swap(): failure swapping data format %02x.%02x.%02x.%02x (\"%c%c%c%c\") - %s\n",
                                     pInfo->dataFormat[0], pInfo->dataFormat[1],
                                     pInfo->dataFormat[2], pInfo->dataFormat[3],
                                     (char)pInfo->dataFormat[0], (char)pInfo->dataFormat[1],
                                     (char)pInfo->dataFormat[2], (char)pInfo->dataFormat[3],
                                     u_errorName(*pErrorCode));
#               else
                    udata_printError(ds, "udata_swap(): failure swapping data format %02x.%02x.%02x.%02x - %s\n",
                                     pInfo->dataFormat[0], pInfo->dataFormat[1],
                                     pInfo->dataFormat[2], pInfo->dataFormat[3],
                                     u_errorName(*pErrorCode));
#               endif
            }

            return length;
        }
    }

    /* the dataFormat was not recognized */
    if(ds->inCharset==U_CHARSET_FAMILY) {
        udata_printError(ds, "udata_swap(): unknown data format %02x.%02x.%02x.%02x (\"%c%c%c%c\")\n",
                         pInfo->dataFormat[0], pInfo->dataFormat[1],
                         pInfo->dataFormat[2], pInfo->dataFormat[3],
                         (char)pInfo->dataFormat[0], (char)pInfo->dataFormat[1],
                         (char)pInfo->dataFormat[2], (char)pInfo->dataFormat[3]);
    } else {
        udata_printError(ds, "udata_swap(): unknown data format %02x.%02x.%02x.%02x\n",
                         pInfo->dataFormat[0], pInfo->dataFormat[1],
                         pInfo->dataFormat[2], pInfo->dataFormat[3]);
    }

    *pErrorCode=U_UNSUPPORTED_ERROR;
    return 0;
}

struct ToCEntry {
    uint32_t nameOffset, inOffset, outOffset, length;
};

static int32_t
compareToCEntries(const void *context, const void *left, const void *right) {
    const char *chars=(const char *)context;
    return (int32_t)uprv_strcmp(chars+((const ToCEntry *)left)->nameOffset,
                                chars+((const ToCEntry *)right)->nameOffset);
}

U_CAPI int32_t U_EXPORT2
udata_swapPackage(const UDataSwapper *ds,
                  const void *inData, int32_t length, void *outData,
                  UErrorCode *pErrorCode) {
    const UDataInfo *pInfo;
    int32_t headerSize;

    const uint8_t *inBytes;
    uint8_t *outBytes;

    uint32_t itemCount, offset, i;
    int32_t itemLength;

    const UDataOffsetTOCEntry *inEntries;
    UDataOffsetTOCEntry *outEntries;

    ToCEntry *table;

    /* udata_swapDataHeader checks the arguments */
    headerSize=udata_swapDataHeader(ds, inData, length, outData, pErrorCode);
    if(pErrorCode==NULL || U_FAILURE(*pErrorCode)) {
        return 0;
    }

    /* check data format and format version */
    pInfo=(const UDataInfo *)((const char *)inData+4);
    if(!(
        pInfo->dataFormat[0]==0x43 &&   /* dataFormat="CmnD" */
        pInfo->dataFormat[1]==0x6d &&
        pInfo->dataFormat[2]==0x6e &&
        pInfo->dataFormat[3]==0x44 &&
        pInfo->formatVersion[0]==1
    )) {
        udata_printError(ds, "udata_swapPackage(): data format %02x.%02x.%02x.%02x (format version %02x) is not recognized as an ICU .dat package\n",
                         pInfo->dataFormat[0], pInfo->dataFormat[1],
                         pInfo->dataFormat[2], pInfo->dataFormat[3],
                         pInfo->formatVersion[0]);
        *pErrorCode=U_UNSUPPORTED_ERROR;
        return 0;
    }

    inBytes=(const uint8_t *)inData+headerSize;
    inEntries=(const UDataOffsetTOCEntry *)(inBytes+4);

    if(length<0) {
        /* preflighting */
        itemCount=ds->readUInt32(*(const uint32_t *)inBytes);
        if(itemCount==0) {
            /* no items: count only the item count and return */
            return headerSize+4;
        }

        /* read the last item's offset and preflight it */
        offset=ds->readUInt32(inEntries[itemCount-1].dataOffset);
        itemLength=udata_swap(ds, inBytes+offset, -1, NULL, pErrorCode);

        if(U_SUCCESS(*pErrorCode)) {
            return headerSize+offset+(uint32_t)itemLength;
        } else {
            return 0;
        }
    } else {
        /* check that the itemCount fits, then the ToC table, then at least the header of the last item */
        length-=headerSize;
        if(length<4) {
            /* itemCount does not fit */
            offset=0xffffffff;
            itemCount=0; /* make compilers happy */
        } else {
            itemCount=ds->readUInt32(*(const uint32_t *)inBytes);
            if(itemCount==0) {
                offset=4;
            } else if((uint32_t)length<(4+8*itemCount)) {
                /* ToC table does not fit */
                offset=0xffffffff;
            } else {
                /* offset of the last item plus at least 20 bytes for its header */
                offset=20+ds->readUInt32(inEntries[itemCount-1].dataOffset);
            }
        }
        if((uint32_t)length<offset) {
            udata_printError(ds, "udata_swapPackage(): too few bytes (%d after header) for unames.icu\n",
                             length);
            *pErrorCode=U_INDEX_OUTOFBOUNDS_ERROR;
            return 0;
        }

        outBytes=(uint8_t *)outData+headerSize;

        /* swap the item count */
        ds->swapArray32(ds, inBytes, 4, outBytes, pErrorCode);

        if(itemCount==0) {
            /* no items: just return now */
            return headerSize+4;
        }

        /* swap the item name strings */
        offset=4+8*itemCount;
        itemLength=(int32_t)(ds->readUInt32(inEntries[0].dataOffset)-offset);
        udata_swapInvStringBlock(ds, inBytes+offset, itemLength, outBytes+offset, pErrorCode);
        if(U_FAILURE(*pErrorCode)) {
            udata_printError(ds, "udata_swapPackage() failed to swap the data item name strings\n");
            return 0;
        }
        /* keep offset and itemLength in case we allocate and copy the strings below */

        /*
         * Allocate the ToC table and, if necessary, a temporary buffer for
         * pseudo-in-place swapping.
         *
         * We cannot swap in-place because:
         *
         * 1. If the swapping of an item fails mid-way, then in-place swapping
         * has destroyed its data.
         * Out-of-place swapping allows us to then copy its original data.
         *
         * 2. If swapping changes the charset family, then we must resort
         * not only the ToC table but also the data items themselves.
         * This requires a permutation and is best done with separate in/out
         * buffers.
         *
         * We swapped the strings above to avoid the malloc below if string swapping fails.
         */
        if(inData==outData) {
            /* +15: prepare for extra padding of a newly-last item */
            table=(ToCEntry *)uprv_malloc(itemCount*sizeof(ToCEntry)+length+15);
            if(table!=NULL) {
                outBytes=(uint8_t *)(table+itemCount);

                /* copy the item count and the swapped strings */
                uprv_memcpy(outBytes, inBytes, 4);
                uprv_memcpy(outBytes+offset, inBytes+offset, itemLength);
            }
        } else {
            table=(ToCEntry *)uprv_malloc(itemCount*sizeof(ToCEntry));
        }
        if(table==NULL) {
            udata_printError(ds, "udata_swapPackage(): out of memory allocating %d bytes\n",
                             inData==outData ?
                                 itemCount*sizeof(ToCEntry)+length+15 :
                                 itemCount*sizeof(ToCEntry));
            *pErrorCode=U_MEMORY_ALLOCATION_ERROR;
            return 0;
        }
        outEntries=(UDataOffsetTOCEntry *)(outBytes+4);

        /* read the ToC table */
        for(i=0; i<itemCount; ++i) {
            table[i].nameOffset=ds->readUInt32(inEntries[i].nameOffset);
            table[i].inOffset=ds->readUInt32(inEntries[i].dataOffset);
            if(i>0) {
                table[i-1].length=table[i].inOffset-table[i-1].inOffset;
            }
        }
        table[itemCount-1].length=(uint32_t)length-table[itemCount-1].inOffset;

        if(ds->inCharset==ds->outCharset) {
            /* no charset swapping, no resorting: keep item offsets the same */
            for(i=0; i<itemCount; ++i) {
                table[i].outOffset=table[i].inOffset;
            }
        } else {
            /* charset swapping: resort items by their swapped names */

            /*
             * Before the actual sorting, we need to make sure that each item
             * has a length that is a multiple of 16 bytes so that all items
             * are 16-aligned.
             * Only the old last item may be missing up to 15 padding bytes.
             * Add padding bytes for it.
             * Since the icuswap main() function has already allocated enough
             * input buffer space and set the last 15 bytes there to 0xaa,
             * we only need to increase the total data length and the length
             * of the last item here.
             */
            if((length&0xf)!=0) {
                int32_t delta=16-(length&0xf);
                length+=delta;
                table[itemCount-1].length+=(uint32_t)delta;
            }

            uprv_sortArray(table, (int32_t)itemCount, (int32_t)sizeof(ToCEntry),
                           compareToCEntries, outBytes, FALSE, pErrorCode);

            /*
             * Note: Before sorting, the inOffset values were in order.
             * Now the outOffset values are in order.
             */

            /* assign outOffset values */
            offset=table[0].inOffset;
            for(i=0; i<itemCount; ++i) {
                table[i].outOffset=offset;
                offset+=table[i].length;
            }
        }

        /* write the output ToC table */
        for(i=0; i<itemCount; ++i) {
            ds->writeUInt32(&outEntries[i].nameOffset, table[i].nameOffset);
            ds->writeUInt32(&outEntries[i].dataOffset, table[i].outOffset);
        }

        /* swap each data item */
        for(i=0; i<itemCount; ++i) {
            /* first copy the item bytes to make sure that unreachable bytes are copied */ 
            uprv_memcpy(outBytes+table[i].outOffset, inBytes+table[i].inOffset, table[i].length);

            /* swap the item */
            udata_swap(ds, inBytes+table[i].inOffset, (int32_t)table[i].length,
                          outBytes+table[i].outOffset, pErrorCode);

            if(U_FAILURE(*pErrorCode)) {
                if(ds->outCharset==U_CHARSET_FAMILY) {
                    udata_printError(ds, "warning: udata_swapPackage() failed to swap item \"%s\"\n"
                                         "    at inOffset 0x%x length 0x%x - %s\n"
                                         "    the data item will be copied, not swapped\n\n",
                                     (char *)outBytes+table[i].nameOffset,
                                     table[i].inOffset, table[i].length, u_errorName(*pErrorCode));
                } else {
                    udata_printError(ds, "warning: udata_swapPackage() failed to swap an item\n"
                                         "    at inOffset 0x%x length 0x%x - %s\n"
                                         "    the data item will be copied, not swapped\n\n",
                                     table[i].inOffset, table[i].length, u_errorName(*pErrorCode));
                }
                /* reset the error code, copy the data item, and continue */
                *pErrorCode=U_ZERO_ERROR;
                uprv_memcpy(outBytes+table[i].outOffset, inBytes+table[i].inOffset, table[i].length);
            }
        }

        if(inData==outData) {
            /* copy the data from the temporary buffer to the in-place buffer */
            uprv_memcpy((uint8_t *)outData+headerSize, outBytes, length);
        }
        uprv_free(table);

        return headerSize+length;
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
