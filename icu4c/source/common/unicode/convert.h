/*****************************************************************************
 *
 *   Copyright (C) 1998-2001, International Business Machines
 *   Corporation and others.  All Rights Reserved.
 *
 *
 *   Change history:
 *
 *   06/29/2000  helena      Major rewrite of the callback APIs.
 *****************************************************************************/

#ifndef CONVERT_H
#define CONVERT_H


#include "unicode/unistr.h"
#include "unicode/ucnv.h"

U_NAMESPACE_BEGIN
/**
 * This class is deprecated and will be removed.
 * Use the more powerful C conversion API with the UConverter type and ucnv_... functions.
 *
 * There are also two new functions in ICU 2.0 that convert a UnicodeString
 * and extract a UnicodeString using a UConverter (search unistr.h for UConverter).
 * They replace the fromUnicodeString() and toUnicodeString() functions here.
 * All other UnicodeConverter functions are basically aliases of C API functions.
 *
 * Old documentation:
 *
 * UnicodeConverter is a C++ wrapper class for UConverter.
 * You need one UnicodeConverter object in place of one UConverter object.
 * For details on the API and implementation of the
 * codepage converter interface see ucnv.h.
 *
 * @see UConverter
 * @deprecated To be removed after 2002-sep-30; use the C API with UConverter and ucnv_... functions.
 */
class U_COMMON_API UnicodeConverter
{
 private:
  /*Internal Data representation of the Converter*/
  UConverter* myUnicodeConverter;
  /*Debug method*/
  void printRef(void) const;

 public:

//Constructors and a destructor

  /**
 * Creates Unicode Conversion Object will default to LATIN1 <-> encoding
 * @return the created Unicode converter object
 * @deprecated
 */
 UnicodeConverter();

/**
 * Creates Unicode Conversion Object by specifying the codepage name.  The name
 * string is in ASCII format.
 * @param code_set the pointer to a char[] object containing a codepage name. (I)
 * @param UErrorCode Error status (I/O) IILLEGAL_ARGUMENT_ERROR will be returned if the string is empty.
 * If the internal program does not work correctly, for example, if there's no such codepage,
 * U_INTERNAL_PROGRAM_ERROR will be returned.
 * @return the created Unicode converter object
 * @deprecated
 */
 UnicodeConverter(const char*             name,
             UErrorCode&              err);

 /**
  *Creates a UnicodeConverter object with the names specified as unicode strings. The name should be limited to
  *the ASCII-7 alphanumerics. Dash and underscore characters are allowed for readability, but are ignored in the
  *search.
  *@param code_set name of the uconv table in Unicode string (I)
  *@param err error status (I/O) IILLEGAL_ARGUMENT_ERROR will be returned if the string is empty.  If the internal
  *program does not work correctly, for example, if there's no such codepage, U_INTERNAL_PROGRAM_ERROR will be
  *returned.
  *@return the created Unicode converter object
  * @deprecated
  */
 UnicodeConverter(const UnicodeString&    name,
             UErrorCode&              err);

 /**
  * Creates Unicode Conversion Object using the codepage ID number.
  * @param code_set a codepage # (I)
  * @UErrorCode Error status (I/O) IILLEGAL_ARGUMENT_ERROR will be returned if the string is empty.
  * If the internal program does not work correctly, for example, if there's no such codepage,
  * U_INTERNAL_PROGRAM_ERROR will be returned.
  * @return the Unicode converter object
  * @deprecated
  */
 UnicodeConverter(int32_t                      codepageNumber,
             UConverterPlatform  platform,
             UErrorCode&                   err);

 ~UnicodeConverter();


 /**
  * Transcodes the source UnicodeString to the target string in a codepage encoding
  * with the specified Unicode converter.  For example, if a Unicode to/from JIS
  * converter is specified, the source string in Unicode will be transcoded to JIS
  * encoding.  The result will be stored in JIS encoding.
  *
  * @param source the source Unicode string
  * @param target the target string in codepage encoding
  * @param targetSize Input the number of bytes available in the "target" buffer, Output the number of bytes copied to it
  * @param err the error status code.  U_MEMORY_ALLOCATION_ERROR will be returned if the
  * the internal process buffer cannot be allocated for transcoding.  U_ILLEGAL_ARGUMENT_ERROR
  * is returned if the converter is null or the source or target string is empty.
  * @deprecated
  */
void fromUnicodeString(char*                    target,
               int32_t&                 targetSize,
               const UnicodeString&     source,
               UErrorCode&               err) const;

/**
 * Transcode the source string in codepage encoding to the target string in
 * Unicode encoding.  For example, if a Unicode to/from JIS
 * converter is specified, the source string in JIS encoding will be transcoded
 * to Unicode encoding.  The result will be stored in Unicode encoding.
 * @param source the source string in codepage encoding
 * @param target the target string in Unicode encoding
 * @param targetSize : I/O parameter, Input size buffer, Output # of bytes copied to it
 * @param err the error status code U_MEMORY_ALLOCATION_ERROR will be returned if the
 * the internal process buffer cannot be allocated for transcoding.  U_ILLEGAL_ARGUMENT_ERROR
 * is returned if the converter is null or the source or target string is empty.
 * @deprecated
 */
void  toUnicodeString(UnicodeString&    target,
                      const char*       source,
                      int32_t           sourceSize,
                      UErrorCode&        err) const;

/**
 * Transcodes an array of unicode characters to an array of codepage characters.
 * The source pointer is an I/O parameter, it starts out pointing at the place
 * to begin translating, and ends up pointing after the first sequence of the bytes
 * that it encounters that are semantically invalid.
 * if T_UnicodeConverter_setMissingCharAction is called with an action other than STOP
 * before a call is made to this API, consumed and source should point to the same place
 * (unless target ends with an imcomplete sequence of bytes and flush is FALSE).
 * @param target : I/O parameter. Input : Points to the beginning of the buffer to copy
 *  codepage characters to. Output : points to after the last codepage character copied
 *  to target.
 * @param targetLimit the pointer to the end of the target array
 * @param source the source Unicode character array
 * @param sourceLimit the pointer to the end of the source array
 * @param offsets if NULL is passed, nothing will happen to it, otherwise it needs to have the same number
 * of allocated cells as <TT>target</TT>. Will fill in offsets from target to source pointer
 * e.g: <TT>offsets[3]</TT> is equal to 6, it means that the <TT>target[3]</TT> was a result of transcoding <TT>source[6]</TT>
 * For output data carried across calls, and other data without a specific source character
 * (such as from escape sequences or callbacks)  -1 will be placed for offsets. 
 * @param flush set to <TT>TRUE</TT> if the current source buffer is the last available
 * chunk of the source, <TT>FALSE</TT> otherwise. Note that if a failing status is returned,
 * this function may have to be called multiple times wiht flush set to <TT>TRUE</TT> until
 * the source buffer is consumed.
 * @param flush TRUE if the buffer is the last buffer and the conversion will finish
 * in this call, FALSE otherwise.  (future feature pending)
 * @param UErrorCode the error status.  U_ILLEGAL_ARGUMENT_ERROR will be returned if the
 * converter is null.
 * @deprecated
 */
void fromUnicode(char*&         target,
                 const char*    targetLimit,
                 const UChar*&      source,
                 const UChar* sourceLimit,
                 int32_t * offsets,
                 UBool         flush,
                 UErrorCode&     err);


/**
 * Converts an array of codepage characters into an array of unicode characters.
 * The source pointer is an I/O parameter, it starts out pointing at the place
 * to begin translating, and ends up pointing after the first sequence of the bytes
 * that it encounters that are semantically invalid.
 * if T_UnicodeConverter_setMissingUnicodeAction is called with an action other than STOP
 * before a call is made to this API, consumed and source should point to the same place
 * (unless target ends with an imcomplete sequence of bytes and flush is FALSE).
 * @param target : I/O parameter. Input : Points to the beginning of the buffer to copy
 *  Unicode characters to. Output : points to after the last UChar copied to target.
 * @param targetLimit the pointer to the end of the target array
 * @param source the source codepage character array
 * @param sourceLimit the pointer to the end of the source array
 * @param offsets if NULL is passed, nothing will happen to it, otherwise it needs to have the same number
 * of allocated cells as <TT>target</TT>. Will fill in offsets from target to source pointer
 * e.g: <TT>offsets[3]</TT> is equal to 6, it means that the <TT>target[3]</TT> was a result of transcoding <TT>source[6]</TT>
 * For output data carried across calls, and other data without a specific source character
 * (such as from escape sequences or callbacks)  -1 will be placed for offsets. 
 * @param flush set to <TT>TRUE</TT> if the current source buffer is the last available
 * chunk of the source, <TT>FALSE</TT> otherwise. Note that if a failing status is returned,
 * this function may have to be called multiple times wiht flush set to <TT>TRUE</TT> until
 * the source buffer is consumed.
 * @param flush TRUE if the buffer is the last buffer and the conversion will finish
 * in this call, FALSE otherwise.  (future feature pending)
 * @param err the error code status  U_ILLEGAL_ARGUMENT_ERROR will be returned if the
 * converter is null, targetLimit < target, sourceLimit < source
 * @deprecated
 */
 void toUnicode(UChar*&        target,
        const UChar*   targetLimit,
        const char*&     source,
        const char*      sourceLimit,
        int32_t * offsets,
        UBool           flush,
        UErrorCode&       err);


/**
 * Returns the maximum length of bytes used by a character. This varies between 1 and 4
 * @return the max number of bytes per codepage character  * converter is null, targetLimit < target, sourceLimit < source
 * @deprecated
 */
int8_t getMaxBytesPerChar(void) const;

/**
* Returns the minimum byte length for characters in this codepage. This is either
* 1 or 2 for all supported codepages.
* @return the minimum number of byte per codepage character
* @deprecated
*/
int8_t getMinBytesPerChar(void) const;

/**
 *Gets the type of conversion associated with the converter
 * e.g. SBCS, MBCS, DBCS, UTF8, UTF16_BE, UTF16_LE, ISO_2022, EBCDIC_STATEFUL, LATIN_1
 * @return the type of the converter
 * @deprecated
 */
UConverterType getType(void) const;

/**
 *Gets the "starter" bytes for the converters of type MBCS
 *will fill in an <TT>U_ILLEGAL_ARGUMENT_ERROR</TT> if converter passed in
 *is not MBCS.
 *fills in an array of boolean, with the value of the byte as offset to the array.
 *At return, if TRUE is found in at offset 0x20, it means that the byte 0x20 is a starter byte
 *in this converter.
 * @param starters: an array of size 256 to be filled in
 * @param err: an array of size 256 to be filled in
 * @see ucnv_getType
 * @deprecated
 */
 void getStarters(UBool starters[256],
                  UErrorCode& err) const;
 /**
 * Fills in the output parameter, subChars, with the substitution characters
 * as multiple bytes.
 * @param subChars the subsitution characters
 * @param len the number of bytes of the substitution character array
 * @param  err the error status code.  U_ILLEGAL_ARGUMENT_ERROR will be returned if
 * the converter is null.  If the substitution character array is too small, an
 * U_INDEX_OUTOFBOUNDS_ERROR will be returned.
 * @deprecated
 */
void getSubstitutionChars(char*         subChars,
                          int8_t&       len,
                          UErrorCode&    err) const;
/**
 * Sets the substitution chars when converting from unicode to a codepage. The
 * substitution is specified as a string of 1-4 bytes, and may contain null byte.
 * The fill-in parameter err will get the error status on return.
 * @param cstr the substitution character array to be set with
 * @param len the number of bytes of the substitution character array and upon return will contain the
 * number of bytes copied to that buffer
 * @param err the error status code.  U_ILLEGAL_ARGUMENT_ERROR if the converter is
 * null.   or if the number of bytes provided are not in the codepage's range (e.g length 1 for ucs-2)
 * @deprecated
 */
void setSubstitutionChars(const char*   subChars,
                          int8_t        len,
                          UErrorCode&    err);

/**
 * Resets the state of stateful conversion to the default state. This is used
 * in the case of error to restart a conversion from a known default state.
 * @deprecated
 */
void resetState(void);

/**
 * Gets the name of the converter (zero-terminated).
 * the name will be the internal name of the converter
 * @param converter the Unicode converter
 * @param err the error status code. U_INDEX_OUTOFBOUNDS_ERROR in the converterNameLen is too
 * small to contain the name.
 * @deprecated
 */
const char*  getName( UErrorCode&  err) const;


/**
 * Gets a codepage number associated with the converter. This is not guaranteed
 * to be the one used to create the converter. Some converters do not represent
 * IBM registered codepages and return zero for the codepage number.
 * The error code fill-in parameter indicates if the codepage number is available.
 * @param err the error status code.  U_ILLEGAL_ARGUMENT_ERROR will returned if
 * the converter is null or if converter's data table is null.
 * @return If any error occurrs, null will be returned.
 * @deprecated
 */
 int32_t  getCodepage(UErrorCode& err) const;

 /**
  * Returns the current setting action taken when a character from a codepage
  * is missing or a byte sequence is illegal etc.
  * @param action the callback function pointer
  * @param context the callback function state
  * @deprecated
  */
 void getMissingCharAction(UConverterToUCallback *action,
                           const void **context) const;

/**
 * Return the current setting action taken when a unicode character is missing
 * or there is an unpaired surrogate etc.
 * @param action the callback function pointer
 * @param context the callback function state
 * @deprecated
 */
 void getMissingUnicodeAction(UConverterFromUCallback *action,
                              const void **context) const;

 /**
  * Sets the current setting action taken when a character from a codepage is
  * missing. (Currently STOP or SUBSTITUTE).
  * @param newAction the action constant if an equivalent codepage character is missing
  * @param newContext the new toUnicode callback function state
  * @param oldAction the original action constant, saved for later restoration.
  * @param oldContext the old toUnicode callback function state
  * @param err the error status code
  * @deprecated
  */
 void  setMissingCharAction(UConverterToUCallback     newAction,
                const void* newContext,
                UConverterToUCallback *oldAction, 
                const void** oldContext,
                UErrorCode&            err);

/**
 * Sets the current setting action taken when a unicode character is missing.
 * (currently T_UnicodeConverter_MissingUnicodeAction is either STOP or SUBSTITUTE,
 *  SKIP, CLOSEST_MATCH, ESCAPE_SEQ may be added in the future).
 * @param newAction the action constant if an equivalent Unicode character is missing
 * @param newContext the new fromUnicode callback function state
 * @param oldAction the original action constant, saved for later restoration.
 * @param oldContext the old fromUnicode callback function state
 * @param err the error status code
 * @deprecated
 */
 void  setMissingUnicodeAction(UConverterFromUCallback  newAction,
                   const void* newContext,
                   UConverterFromUCallback *oldAction,
                   const void** oldContext,
                   UErrorCode&            err);
/**
 * Returns the localized name of the UnicodeConverter, if for any reason it is
 * available, the internal name will be returned instead.
 * @param displayLocale the valid Locale, from which we want to localize
 * @param displayString a UnicodeString that is going to be filled in.
 * @deprecated
 */
void getDisplayName(const Locale&   displayLocale,
                    UnicodeString&  displayName) const;

/**
 * Returns the T_UnicodeConverter_platform (ICU defined enum) of a UnicodeConverter
 * available, the internal name will be returned instead.
 * @param err the error code status
 * @return the codepages platform
 * @deprecated
 */
UConverterPlatform  getCodepagePlatform(UErrorCode& err) const;


 UnicodeConverter&   operator=(const UnicodeConverter& that);
 UBool              operator==(const UnicodeConverter& that) const;
 UBool              operator!=(const UnicodeConverter& that) const;
 UnicodeConverter(const UnicodeConverter&  that);

/**
 * Returns the available names. Lazy evaluated, Library owns the storage
 * @param num the number of available converters
 * @param err the error code status
 * @return the name array
 * @deprecated
 */
static  const char* const* getAvailableNames(int32_t&   num,
                         UErrorCode&  err);

/**
 * Iterates through every cached converter and frees all the unused ones
 * @return the number of cached converters successfully deleted
 * @deprecated
 */
static   int32_t flushCache(void);
/**
 * Fixes the backslash character mismapping.  For example, in SJIS, the backslash 
 * character in the ASCII portion is also used to represent the yen currency sign.  
 * When mapping from Unicode character 0x005C, it's unclear whether to map the 
 * character back to yen or backslash in SJIS.  This function will take the input
 * buffer and replace all the yen sign characters with backslash.  This is necessary
 * when the user tries to open a file with the input buffer on Windows.
 * @param source the input buffer to be fixed
 * @deprecated
 */
void fixFileSeparator(UnicodeString& source) const;

/**
 * Determines if the converter contains ambiguous mappings of the same
 * character or not.
 * @return TRUE if the converter contains ambiguous mapping of the same 
 * character, FALSE otherwise.
 * @deprecated
 */
UBool isAmbiguous(void) const;

};

/**
 * Typedef for backward compatibility
 * @deprecated Remove in 2.0 release
 */
typedef UnicodeConverter UnicodeConverterCPP;   /* Backwards compatibility. */

U_NAMESPACE_END
#endif
