/*
**********************************************************************
*   Copyright (C) 1999, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*
*   uconv_cnv.h:
*   defines all the low level conversion functions
*   T_UnicodeConverter_{to,from}Unicode_$ConversionType
*
* Modification History:
*
*   Date        Name        Description
*   05/09/00    helena      Added implementation to handle fallback mappings.
*   06/29/2000  helena      Major rewrite of the callback APIs.
*/

#ifndef UCNV_CNV_H
#define UCNV_CNV_H

#include "unicode/utypes.h"
#include "unicode/ucnv_err.h"
#include "ucnv_bld.h"
#include "ucnvmbcs.h"
#include "ucmp8.h"
#include "ucmp16.h"

/*Table Node Definitions */
typedef struct
  {
    UChar *toUnicode;  /* [256]; */
    CompactByteArray fromUnicode;
    UChar *toUnicodeFallback;
    CompactByteArray fromUnicodeFallback;
  }
UConverterSBCSTable;

typedef struct
  {
    CompactShortArray toUnicode;
    CompactShortArray fromUnicode;
    CompactShortArray toUnicodeFallback;
    CompactShortArray fromUnicodeFallback;
  }
UConverterDBCSTable;

union UConverterTable
  {
    UConverterSBCSTable sbcs;
    UConverterDBCSTable dbcs;
    UConverterMBCSTable mbcs;
  };


U_CDECL_BEGIN

/* this is used in fromUnicode DBCS tables as an "unassigned" marker */
#define missingCharMarker 0xFFFF

/*
 * #define missingUCharMarker 0xfffe
 *
 * there are actually two values used in toUnicode tables:
 * U+fffe "unassigned"
 * U+ffff "illegal"
 */

#define FromU_CALLBACK_MACRO(context, args, codeUnits, length, codePoint, reason, err) \
                { \
                  /*copies current values for the ErrorFunctor to update */ \
                  /*Calls the ErrorFunctor */ \
                  args->converter->fromUCharErrorBehaviour ( context, \
                                                  args, \
                                                  codeUnits, \
                                                  length, \
                                                  codePoint, \
                                                  reason, \
                                                  err); \
                 myTargetIndex = args->target - (char*)myTarget; \
                 mySourceIndex = args->source - mySource; \
                }
/*
*/
#define ToU_CALLBACK_MACRO(context, args, codePoints, length, reason, err) \
                { \
                  /*Calls the ErrorFunctor */ \
                  args->converter->fromCharErrorBehaviour ( \
                                                 context, \
                                                 args, \
                                                 codePoints, \
                                                 length, \
                                                 reason, \
                                                 err); \
                 myTargetIndex = args->target - myTarget; \
                 mySourceIndex = args->source - (const char*)mySource; \
                }
/*
*/
#define FromU_CALLBACK_OFFSETS_LOGIC_MACRO(context, args, codeUnits, length, codePoint, reason, err) \
                { \
                 int32_t My_i = myTargetIndex; \
                  /*copies current values for the ErrorFunctor to update */ \
                  /*Calls the ErrorFunctor */ \
                  args->converter->fromUCharErrorBehaviour ( \
                                                 context, \
                                                 args, \
                                                 codeUnits, \
                                                 length, \
                                                 codePoint, \
                                                 reason, \
                                                 err); \
                  /*Update the local Indexes so that the conversion can restart at the right points */ \
                 myTargetIndex = args->target - (char*)myTarget; \
                 mySourceIndex = args->source - mySource; \
                 args->offsets = saveOffsets; \
                  for (;My_i < myTargetIndex;My_i++) args->offsets[My_i] += currentOffset; \
                }
/*
*/
#define ToU_CALLBACK_OFFSETS_LOGIC_MACRO(context, args, codePoints, length, reason, err) \
                { \
                      args->converter->fromCharErrorBehaviour ( \
                                                 context, \
                                                 args, \
                                                 codePoints, \
                                                 length, \
                                                 reason, \
                                                 err); \
                  /*Update the local Indexes so that the conversion can restart at the right points */ \
                 myTargetIndex = args->target - myTarget; \
                 mySourceIndex = args->source - (const char*)mySource; \
                 args->offsets = saveOffsets; \
                  for (;My_i < myTargetIndex;My_i++) {args->offsets[My_i] += currentOffset;} \
                }


typedef void (*UConverterLoad) (UConverterSharedData *sharedData, const uint8_t *raw, UErrorCode *pErrorCode);
typedef void (*UConverterUnload) (UConverterSharedData *sharedData);

typedef void (*UConverterOpen) (UConverter *cnv, const char *name, const char *locale,uint32_t options, UErrorCode *pErrorCode);
typedef void (*UConverterClose) (UConverter *cnv);

typedef void (*UConverterReset) (UConverter *cnv);

typedef void (*T_ToUnicodeFunction) (UConverterToUnicodeArgs *, UErrorCode *);

typedef void (*T_FromUnicodeFunction) (UConverterFromUnicodeArgs *, UErrorCode *);

typedef UChar32 (*T_GetNextUCharFunction) (UConverterToUnicodeArgs *, UErrorCode *);

typedef void (*UConverterGetStarters)(const UConverter* converter,
                                      UBool starters[256],
                                      UErrorCode *pErrorCode);
/* If this function pointer is null or if the function returns null
 * the name field in static data struct should be returned by 
 * ucnv_getName() API function
 */
typedef const char * (*UConverterGetName) (const UConverter *cnv);

UBool CONVERSION_U_SUCCESS (UErrorCode err);

void flushInternalUnicodeBuffer (UConverter * _this,
                                 UChar * myTarget,
                                 int32_t * myTargetIndex,
                                 int32_t targetLength,
                                 int32_t** offsets,
                                 UErrorCode * err);

void flushInternalCharBuffer (UConverter * _this,
                              char *myTarget,
                              int32_t * myTargetIndex,
                              int32_t targetLength,
                              int32_t** offsets,
                              UErrorCode * err);

/**
 * UConverterImpl contains all the data and functions for a converter type.
 * Its function pointers work much like a C++ vtable.
 * Many converter types need to define only a subset of the functions;
 * when a function pointer is NULL, then a default action will be performed.
 *
 * Every converter type must implement toUnicode, fromUnicode, and getNextUChar,
 * otherwise the converter may crash.
 * Every converter type that has variable-length codepage sequences should
 * also implement toUnicodeWithOffsets and fromUnicodeWithOffsets for
 * correct offset handling.
 * All other functions may or may not be implemented - it depends only on
 * whether the converter type needs them.
 *
 * When open() fails, then close() will be called, if present.
 */
struct UConverterImpl {
    UConverterType type;

    UConverterLoad load;
    UConverterUnload unload;

    UConverterOpen open;
    UConverterClose close;
    UConverterReset reset;

    T_ToUnicodeFunction toUnicode;
    T_ToUnicodeFunction toUnicodeWithOffsets;
    T_FromUnicodeFunction fromUnicode;
    T_FromUnicodeFunction fromUnicodeWithOffsets;
    T_GetNextUCharFunction getNextUChar;

    UConverterGetStarters getStarters;
    UConverterGetName getName;
};

extern const UConverterSharedData
    _SBCSData, _DBCSData, _MBCSData, _Latin1Data,
    _UTF8Data, _UTF16BEData, _UTF16LEData, _UTF32BEData, _UTF32LEData,
    _EBCDICStatefulData, _ISO2022Data, 
    _LMBCSData1,_LMBCSData2, _LMBCSData3, _LMBCSData4, _LMBCSData5, _LMBCSData6,
    _LMBCSData8,_LMBCSData11,_LMBCSData16,_LMBCSData17,_LMBCSData18,_LMBCSData19,_HZData;

U_CDECL_END

/**
 * This function is useful for implementations of getNextUChar().
 * After a call to a callback function or to toUnicode(), an output buffer
 * begins with a Unicode code point that needs to be returned as UChar32,
 * and all following code units must be prepended to the - potentially
 * prefilled - overflow buffer in the UConverter.
 * The buffer should be at least of capacity UTF_MAX_CHAR_LENGTH so that a
 * complete UChar32's UChars fit into it.
 *
 * @param cnv    The converter that will get remaining UChars copied to its overflow area.
 * @param buffer An array of UChars that was passed into a callback function
 *               or a toUnicode() function.
 * @param length The number of code units (UChars) that are actually in the buffer.
 *               This must be >0.
 * @return The code point from the first UChars in the buffer.
 */
U_CFUNC UChar32
ucnv_getUChar32KeepOverflow(UConverter *cnv, const UChar *buffer, int32_t length);

/**
 * This helper function updates the offsets array after a callback function call.
 * It adds the sourceIndex to each offsets item, or sets each of them to -1 if
 * sourceIndex==-1.
 *
 * @param offsets The pointer to offsets entry that corresponds to the first target
 *                unit that the callback wrote.
 * @param length  The number of output units that the callback wrote.
 * @param sourceIndex The sourceIndex of the input sequence that the callback
 *                    function was called for.
 * @return offsets+length if offsets!=NULL, otherwise NULL
 */
U_CFUNC int32_t *
ucnv_updateCallbackOffsets(int32_t *offsets, int32_t length, int32_t sourceIndex);

/** Always use fallbacks from codepage to Unicode */
#define TO_U_USE_FALLBACK(useFallback) TRUE
#define UCNV_TO_U_USE_FALLBACK(cnv) TRUE

/** Use fallbacks from Unicode to codepage when cnv->useFallback or for private-use code points */
#define FROM_U_USE_FALLBACK(useFallback, c) ((useFallback) || (uint32_t)((c)-0xe000)<0x1900 || (uint32_t)((c)-0xf0000)<0x20000)
#define UCNV_FROM_U_USE_FALLBACK(cnv, c) FROM_U_USE_FALLBACK((cnv)->useFallback, c)

#endif /* UCNV_CNV */
