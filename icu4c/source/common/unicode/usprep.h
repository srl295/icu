/*
 *******************************************************************************
 *
 *   Copyright (C) 2003, International Business Machines
 *   Corporation and others.  All Rights Reserved.
 *
 *******************************************************************************
 *   file name:  usprep.h
 *   encoding:   US-ASCII
 *   tab size:   8 (not used)
 *   indentation:4
 *
 *   created on: 2003jul2
 *   created by: Ram Viswanadha
 */

#ifndef __USPREP_H__
#define __USPREP_H__

#include "unicode/utypes.h"

#if !UCONFIG_NO_IDNA

#include "unicode/parseerr.h"
/**
 * The StringPrep profile
 * @draft ICU 2.8
 */
typedef struct UStringPrepProfile UStringPrepProfile;


/** 
 * Option to prohibit processing of unassigned code points in the input
 * 
 * @see  usprep_prepare
 * @draft ICU 2.8
 */
#define USPREP_NONE 0x0000

/** 
 * Option to allow processing of unassigned code points in the input
 * 
 * @see  usprep_prepare
 * @draft ICU 2.8
 */
#define USPREP_ALLOW_UNASSIGNED 0x0001



/**
 * Creates a StringPrep profile from the data file.
 *
 * @param path      string containing the full path pointing to the directory
 *                  where the profile reside followed by the package name
 *                  e.g. "/usr/resource/my_app/profiles/mydata" on a Unix system.
 *                  if NULL, ICU default data files will be used.
 * @param fileName  name of the profile file to be opened
 * @param status    ICU error code in/out parameter. Must not be NULL.
 *                  Must fulfill U_SUCCESS before the function call.
 * @return Pointer to UStringPrepProfile that is opened. Should be closed by
 * calling usprep_close()
 * @see usprep_close()
 * @draft ICU 2.8
 */
U_CAPI UStringPrepProfile* U_EXPORT2
usprep_open(const char* path, 
            const char* fileName,
            UErrorCode* status);


/**
 * Closes the profile
 * @param profile The profile to close
 * @draft ICU 2.8
 */
U_CAPI void U_EXPORT2
usprep_close(UStringPrepProfile* profile);


/**
 * Prepare the input buffer for use in applications with the given profile. This operation maps, normalizes(NFKC),
 * checks for prohited and BiDi characters in the order defined by RFC 3454
 * depending on the options specified in the profile.
 *
 * @param prep          The profile to use 
 * @param src           Pointer to UChar buffer containing the string to prepare
 * @param srcLength     Number of characters in the source string
 * @param dest          Pointer to the destination buffer to receive the output
 * @param destCapacity  The capacity of destination array
 * @param options       A bit set of options:
 *
 *  - USPREP_NONE               Prohibit processing of unassigned code points in the input
 *
 *  - USPREP_ALLOW_UNASSIGNED   Treat the unassigned code points are in the input 
 *                              as normal Unicode code points.
 *
 * @param parseError        Pointer to UParseError struct to receive information on position 
 *                          of error if an error is encountered. Can be NULL.
 * @param status            ICU in/out error code parameter.
 *                          U_INVALID_CHAR_FOUND if src contains
 *                          unmatched single surrogates.
 *                          U_INDEX_OUTOFBOUNDS_ERROR if src contains
 *                          too many code points.
 *                          U_BUFFER_OVERFLOW_ERROR if destCapacity is not enough
 * @return The number of UChars in the destination buffer
 * @draft ICU 2.8
 */

U_CAPI int32_t U_EXPORT2
usprep_prepare(   const UStringPrepProfile* prep,
                  const UChar* src, int32_t srcLength, 
                  UChar* dest, int32_t destCapacity,
                  int32_t options,
                  UParseError* parseError,
                  UErrorCode* status );


#endif /* #if !UCONFIG_NO_IDNA */

#endif
