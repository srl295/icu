/*
**********************************************************************
*   Copyright (C) 1998-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*
* File ustring.h
*
* Modification History:
*
*   Date        Name        Description
*   12/07/98    bertrand    Creation.
******************************************************************************
*/

#ifndef USTRING_H
#define USTRING_H
#include "unicode/utypes.h"

/** Simple declaration for u_strToTitle() to avoid including unicode/ubrk.h. */
#ifndef UBRK_TYPEDEF_UBREAK_ITERATOR
#   define UBRK_TYPEDEF_UBREAK_ITERATOR
    typedef void *UBreakIterator;
#endif

/**
 * \file
 * \brief C API: Unicode string handling functions
 *
 * These C API functions provide Unicode string handling.
 *
 * Some functions are equivalent in name, signature, and behavior to the ANSI C <string.h>
 * functions. (For example, they do not check for bad arguments like NULL string pointers.)
 * In some cases, only the thread-safe variant of such a function is implemented here
 * (see u_strtok_r()).
 *
 * Other functions provide more Unicode-specific functionality like locale-specific
 * upper/lower-casing and string comparison in code point order.
 *
 * ICU uses 16-bit Unicode (UTF-16) in the form of arrays of UChar code units.
 * UTF-16 encodes each Unicode code point with either one or two UChar code units.
 * Some APIs accept a 32-bit UChar32 value for a single code point.
 * (This is the default form of Unicode, and a forward-compatible extension of the original,
 * fixed-width form that was known as UCS-2. UTF-16 superseded UCS-2 with Unicode 2.0
 * in 1996.)
 *
 * Although UTF-16 is a variable-width encoding form (like some legacy multi-byte encodings),
 * it is much more efficient even for random access because the code unit values
 * for single-unit characters vs. lead units vs. trail units are completely disjoint.
 * This means that it is easy to determine character (code point) boundaries from
 * random offsets in the string.
 * (It also means, e.g., that u_strstr() does not need to verify that a match was
 * found on actual character boundaries; with some legacy encodings, strstr() may need to
 * scan back to the start of the text to verify this.)
 *
 * Unicode (UTF-16) string processing is optimized for the single-unit case.
 * Although it is important to support supplementary characters
 * (which use pairs of lead/trail code units called "surrogates"),
 * their occurrence is rare. Almost all characters in modern use require only
 * a single UChar code unit (i.e., their code point values are <=0xffff).
 */

/**
 * Determine the length of an array of UChar.
 *
 * @param s The array of UChars, NULL (U+0000) terminated.
 * @return The number of UChars in <TT>chars</TT>, minus the terminator.
 * @stable
 */
U_CAPI int32_t U_EXPORT2
u_strlen(const UChar *s);

/**
 * Count Unicode code points in the length UChar code units of the string.
 * A code point may occupy either one or two UChar code units.
 * Counting code points involves reading all code units.
 *
 * This functions is basically the inverse of the UTF_FWD_N() macro (see utf.h).
 *
 * @param s The input string.
 * @param length The number of UChar code units to be checked, or -1 to count all
 *               code points before the first NUL (U+0000).
 * @return The number of code points in the specified code units.
 * @draft ICU 2.0
 */
U_CAPI int32_t U_EXPORT2
u_countChar32(const UChar *s, int32_t length);

/**
 * Concatenate two ustrings.  Appends a copy of <TT>src</TT>,
 * including the null terminator, to <TT>dst</TT>. The initial copied
 * character from <TT>src</TT> overwrites the null terminator in <TT>dst</TT>.
 *
 * @param dst The destination string.
 * @param src The source string.
 * @return A pointer to <TT>dst</TT>.
 * @stable
 */
U_CAPI UChar* U_EXPORT2
u_strcat(UChar     *dst, 
    const UChar     *src);

/**
 * Concatenate two ustrings.  
 * Appends at most <TT>n</TT> characters from <TT>src</TT> to <TT>dst</TT>.
 * Adds a null terminator.
 *
 * @param dst The destination string.
 * @param src The source string.
 * @param n The maximum number of characters to compare.
 * @return A pointer to <TT>dst</TT>.
 * @stable
 */
U_CAPI UChar* U_EXPORT2
u_strncat(UChar     *dst, 
     const UChar     *src, 
     int32_t     n);

/**
 * Find the first occurrence of a specified character in a ustring.
 *
 * @param s The string to search.
 * @param c The character to find.
 * @return A pointer to the first occurrence of <TT>c</TT> in <TT>s</TT>,
 * or a null pointer if <TT>s</TT> does not contain <TT>c</TT>.
 * @stable
 */
U_CAPI UChar*  U_EXPORT2
u_strchr(const UChar     *s, 
    UChar     c);

/**
 * Find the first occurrence of a substring in a string.
 *
 * @param s The string to search.
 * @return A pointer to the first occurrence of <TT>substring</TT> in 
 * <TT>s</TT>, or a null pointer if <TT>substring</TT>
 * is not in <TT>s</TT>.
 * @stable
 */
U_CAPI UChar * U_EXPORT2
u_strstr(const UChar *s, const UChar *substring);

/**
 * Find the first occurence of a specified code point in a string.
 *
 * @param s The string to search.
 * @param c The code point (0..0x10ffff) to find.
 * @return A pointer to the first occurrence of <TT>c</TT> in <TT>s</TT>,
 * or a null pointer if there is no such character.
 * If <TT>c</TT> is represented with several UChars, then the returned
 * pointer will point to the first of them.
 * @stable
 */
U_CAPI UChar * U_EXPORT2
u_strchr32(const UChar *s, UChar32 c);

/**
 * Locates the first occurrence in the string str of any of the characters
 * in the string accept.
 * Works just like C's strpbrk but with Unicode.
 *
 * @return A pointer to the  character in str that matches one of the
 *         characters in accept, or NULL if no such character is found.
 * @stable
 */
U_CAPI UChar * U_EXPORT2
u_strpbrk(const UChar *string, const UChar *matchSet);

/**
 * Returns the number of consecutive characters in string1,
 * beginning with the first, that do not occur somewhere in string2.
 * Works just like C's strcspn but with Unicode.
 *
 * @see u_strspn
 * @stable
 */
U_CAPI int32_t U_EXPORT2
u_strcspn(const UChar *string, const UChar *matchSet);

/**
 * Returns the number of consecutive characters in string1,
 * beginning with the first, that occur somewhere in string2.
 * Works just like C's strspn but with Unicode.
 *
 * @see u_strcspn
 * @stable
 */
U_CAPI int32_t U_EXPORT2
u_strspn(const UChar *string, const UChar *matchSet);

/**
 * The string tokenizer API allows an application to break a string into
 * tokens. Unlike strtok(), the saveState (the current pointer within the
 * original string) is maintained in saveState. In the first call, the
 * argument src is a pointer to the string. In subsequent calls to
 * return successive tokens of that string, src must be specified as
 * NULL. The value saveState is set by this function to maintain the
 * function's position within the string, and on each subsequent call
 * you must give this argument the same variable. This function does
 * handle surrogate pairs. This function is similar to the strtok_r()
 * the POSIX Threads Extension (1003.1c-1995) version.
 *
 * @param src String containing token(s). This string will be modified.
 *            After the first call to u_strtok_r(), this argument must
 *            be NULL to get to the next token.
 * @param delim Set of delimiter characters (Unicode code points).
 * @param saveState The current pointer within the original string,
 *                which is set by this function.
 * @return A pointer to the next token found in src, or NULL
 *         when there are no more tokens.
 * @stable
 */
U_CAPI UChar * U_EXPORT2
u_strtok_r(UChar    *src, 
     const UChar    *delim,
           UChar   **saveState);

/**
 * Compare two Unicode strings for bitwise equality (code unit order).
 *
 * @param s1 A string to compare.
 * @param s2 A string to compare.
 * @return 0 if <TT>s1</TT> and <TT>s2</TT> are bitwise equal; a negative
 * value if <TT>s1</TT> is bitwise less than <TT>s2,/TT>; a positive
 * value if <TT>s1</TT> is bitwise greater than <TT>s2,/TT>.
 * @stable
 */
U_CAPI int32_t  U_EXPORT2
u_strcmp(const UChar     *s1, 
    const UChar     *s2);

/**
 * Compare two Unicode strings in code point order.
 * This is different in UTF-16 from u_strcmp() if supplementary characters are present:
 * In UTF-16, supplementary characters (with code points U+10000 and above) are
 * stored with pairs of surrogate code units. These have values from 0xd800 to
 * 0xdfff, which means that they compare as less than some other BMP characters
 * like U+feff. This function compares Unicode strings in code point order.
 * If eihter of the UTF-16 strings is malformed (i.e., it contains unpaired
 * surrogates), then the result is not defined.
 *
 * @param s1 A string to compare.
 * @param s2 A string to compare.
 * @return a negative/zero/positive integer corresponding to whether
 * the first string is less than/equal to/greater than the second one
 * in code point order
 * @draft ICU 1.8
 */
U_CAPI int32_t U_EXPORT2
u_strcmpCodePointOrder(const UChar *s1, const UChar *s2);

/**
 * Compare two ustrings for bitwise equality. 
 * Compares at most <TT>n</TT> characters.
 *
 * @param s1 A string to compare.
 * @param s2 A string to compare.
 * @param n The maximum number of characters to compare.
 * @return 0 if <TT>s1</TT> and <TT>s2</TT> are bitwise equal; a negative
 * value if <TT>s1</TT> is bitwise less than <TT>s2,/TT>; a positive
 * value if <TT>s1</TT> is bitwise greater than <TT>s2,/TT>.
 * @stable
 */
U_CAPI int32_t U_EXPORT2
u_strncmp(const UChar     *ucs1, 
     const UChar     *ucs2, 
     int32_t     n);

/**
 * Compare two Unicode strings in code point order.
 * This is different in UTF-16 from u_strncmp() if supplementary characters are present.
 * For details, see u_strcmpCodePointOrder().
 *
 * @param s1 A string to compare.
 * @param s2 A string to compare.
 * @param n The maximum number of characters to compare.
 * @return a negative/zero/positive integer corresponding to whether
 * the first string is less than/equal to/greater than the second one
 * in code point order
 * @draft ICU 1.8
 */
U_CAPI int32_t U_EXPORT2
u_strncmpCodePointOrder(const UChar *s1, const UChar *s2, int32_t n);

/**
 * Compare two strings case-insensitively using full case folding.
 * This is equivalent to u_strcmp(u_strFoldCase(s1, options), u_strFoldCase(s2, options)).
 *
 * @param s1 A string to compare.
 * @param s2 A string to compare.
 * @param options Either U_FOLD_CASE_DEFAULT or U_FOLD_CASE_EXCLUDE_SPECIAL_I
 * @return A negative, zero, or positive integer indicating the comparison result.
 * @draft ICU 1.8
 */
U_CAPI int32_t U_EXPORT2
u_strcasecmp(const UChar *s1, const UChar *s2, uint32_t options);

/**
 * Compare two strings case-insensitively using full case folding.
 * This is equivalent to u_strcmp(u_strFoldCase(s1, at most n, options),
 * u_strFoldCase(s2, at most n, options)).
 *
 * @param s1 A string to compare.
 * @param s2 A string to compare.
 * @param n The maximum number of characters each string to case-fold and then compare.
 * @param options Either U_FOLD_CASE_DEFAULT or U_FOLD_CASE_EXCLUDE_SPECIAL_I
 * @return A negative, zero, or positive integer indicating the comparison result.
 * @draft ICU 1.8
 */
U_CAPI int32_t U_EXPORT2
u_strncasecmp(const UChar *s1, const UChar *s2, int32_t n, uint32_t options);

/**
 * Compare two strings case-insensitively using full case folding.
 * This is equivalent to u_strcmp(u_strFoldCase(s1, n, options),
 * u_strFoldCase(s2, n, options)).
 *
 * @param s1 A string to compare.
 * @param s2 A string to compare.
 * @param n The number of characters in each string to case-fold and then compare.
 * @param options Either U_FOLD_CASE_DEFAULT or U_FOLD_CASE_EXCLUDE_SPECIAL_I
 * @return A negative, zero, or positive integer indicating the comparison result.
 * @draft ICU 2.0
 */
U_CAPI int32_t U_EXPORT2
u_memcasecmp(const UChar *s1, const UChar *s2, int32_t length, uint32_t options);

/**
 * Copy a ustring. Adds a null terminator.
 *
 * @param dst The destination string.
 * @param src The source string.
 * @return A pointer to <TT>dst</TT>.
 * @stable
 */
U_CAPI UChar* U_EXPORT2
u_strcpy(UChar     *dst, 
    const UChar     *src);

/**
 * Copy a ustring.
 * Copies at most <TT>n</TT> characters.  The result will be null terminated
 * if the length of <TT>src</TT> is less than <TT>n</TT>.
 *
 * @param dst The destination string.
 * @param src The source string.
 * @param n The maximum number of characters to copy.
 * @return A pointer to <TT>dst</TT>.
 * @stable
 */
U_CAPI UChar* U_EXPORT2
u_strncpy(UChar     *dst, 
     const UChar     *src, 
     int32_t     n);

/**
 * Copy a byte string encoded in the default codepage to a ustring.
 * Adds a null terminator.
 * Performs a host byte to UChar conversion
 *
 * @param dst The destination string.
 * @param src The source string.
 * @return A pointer to <TT>dst</TT>.
 * @stable
 */
U_CAPI UChar* U_EXPORT2 u_uastrcpy(UChar *dst,
               const char *src );

/**
 * Copy a byte string encoded in the default codepage to a ustring.
 * Copies at most <TT>n</TT> characters.  The result will be null terminated
 * if the length of <TT>src</TT> is less than <TT>n</TT>.
 * Performs a host byte to UChar conversion
 *
 * @param dst The destination string.
 * @param src The source string.
 * @param n The maximum number of characters to copy.
 * @return A pointer to <TT>dst</TT>.
 * @stable
 */
U_CAPI UChar* U_EXPORT2 u_uastrncpy(UChar *dst,
            const char *src,
            int32_t n);

/**
 * Copy ustring to a byte string encoded in the default codepage.
 * Adds a null terminator.
 * Performs a UChar to host byte conversion
 *
 * @param dst The destination string.
 * @param src The source string.
 * @return A pointer to <TT>dst</TT>.
 * @stable
 */
U_CAPI char* U_EXPORT2 u_austrcpy(char *dst,
            const UChar *src );

/**
 * Copy ustring to a byte string encoded in the default codepage.
 * Copies at most <TT>n</TT> characters.  The result will be null terminated
 * if the length of <TT>src</TT> is less than <TT>n</TT>.
 * Performs a UChar to host byte conversion
 *
 * @param dst The destination string.
 * @param src The source string.
 * @param n The maximum number of characters to copy.
 * @return A pointer to <TT>dst</TT>.
 * @stable
 */
U_CAPI char* U_EXPORT2 u_austrncpy(char *dst,
            const UChar *src,
            int32_t n );

/**
 * Synonym for memcpy(), but with UChars only.
 * @stable
 */
U_CAPI UChar* U_EXPORT2
u_memcpy(UChar *dest, const UChar *src, int32_t count);

/**
 * Synonym for memmove(), but with UChars only.
 * @stable
 */
U_CAPI UChar* U_EXPORT2
u_memmove(UChar *dest, const UChar *src, int32_t count);

/**
 * Initialize <TT>count</TT> characters of <TT>dest</TT> to <TT>c</TT>.
 *
 * @param dest The destination string.
 * @param c The character to initialize the string.
 * @param count The maximum number of characters to set.
 * @return A pointer to <TT>dest</TT>.
 * @stable
 */
U_CAPI UChar* U_EXPORT2
u_memset(UChar *dest, UChar c, int32_t count);

/**
 * Compare the first <TT>count</TT> UChars of each buffer.
 *
 * @param buf1 The first string to compare.
 * @param buf2 The second string to compare.
 * @param count The maximum number of UChars to compare.
 * @return When buf1 < buf2, a negative number is returned.
 *      When buf1 == buf2, 0 is returned.
 *      When buf1 > buf2, a positive number is returned.
 * @stable
 */
U_CAPI int32_t U_EXPORT2
u_memcmp(UChar *buf1, UChar *buf2, int32_t count);

/**
 * Compare two Unicode strings in code point order.
 * This is different in UTF-16 from u_memcmp() if supplementary characters are present.
 * For details, see u_strcmpCodePointOrder().
 *
 * @param s1 A string to compare.
 * @param s2 A string to compare.
 * @param n The maximum number of characters to compare.
 * @return a negative/zero/positive integer corresponding to whether
 * the first string is less than/equal to/greater than the second one
 * in code point order
 * @draft ICU 1.8
 */
U_CAPI int32_t U_EXPORT2
u_memcmpCodePointOrder(const UChar *s1, const UChar *s2, int32_t count);

/**
 * Search for a UChar within a Unicode string until <TT>count</TT>
 * is reached.
 *
 * @param src string to search in
 * @param ch character to find
 * @param count maximum number of UChars in <TT>src</TT>to search for
 *      <TT>ch</TT>.
 * @return A pointer within src, pointing to <TT>ch</TT>, or NULL if it
 *      was not found.
 * @stable
 */
U_CAPI UChar* U_EXPORT2
u_memchr(UChar *src, UChar ch, int32_t count);

/**
 * Search for a UChar32 within a Unicode string until <TT>count</TT>
 * is reached. This also includes surrogates in UTF-16.
 *
 * @param src string to search in
 * @param ch character to find
 * @param count maximum number of UChars in <TT>src</TT>to search for
 *      <TT>ch</TT>.
 * @return A pointer within src, pointing to <TT>ch</TT>, or NULL if it
 *      was not found.
 * @stable
 */
U_CAPI UChar* U_EXPORT2
u_memchr32(UChar *src, UChar32 ch, int32_t count);

/**
 * Unicode String literals in C.
 * We need one macro to declare a variable for the string
 * and to statically preinitialize it if possible,
 * and a second macro to dynamically intialize such a string variable if necessary.
 *
 * The macros are defined for maximum performance.
 * They work only for strings that contain "invariant characters", i.e.,
 * only latin letters, digits, and some punctuation.
 * See utypes.h for details.
 *
 * A pair of macros for a single string must be used with the same
 * parameters.
 * The string parameter must be a C string literal.
 * The length of the string, not including the terminating
 * <code>NUL</code>, must be specified as a constant.
 * The U_STRING_DECL macro should be invoked exactly once for one
 * such string variable before it is used.
 *
 * Usage:
 * <pre>
 * &#32;   U_STRING_DECL(ustringVar1, "Quick-Fox 2", 11);
 * &#32;   U_STRING_DECL(ustringVar2, "jumps 5%", 8);
 * &#32;   static UBool didInit=FALSE;
 * &#32;
 * &#32;   int32_t function() {
 * &#32;       if(!didInit) {
 * &#32;           U_STRING_INIT(ustringVar1, "Quick-Fox 2", 11);
 * &#32;           U_STRING_INIT(ustringVar2, "jumps 5%", 8);
 * &#32;           didInit=TRUE;
 * &#32;       }
 * &#32;       return u_strcmp(ustringVar1, ustringVar2);
 * &#32;   }
 * </pre>
 * @stable
 */
#if U_SIZEOF_WCHAR_T==U_SIZEOF_UCHAR && U_CHARSET_FAMILY==U_ASCII_FAMILY
#   define U_STRING_DECL(var, cs, length) static const wchar_t var[(length)+1]={ L ## cs }
#   define U_STRING_INIT(var, cs, length)
#elif U_SIZEOF_UCHAR==1 && U_CHARSET_FAMILY==U_ASCII_FAMILY
#   define U_STRING_DECL(var, cs, length) static const UChar var[(length)+1]={ (const UChar *)cs }
#   define U_STRING_INIT(var, cs, length)
#else
#   define U_STRING_DECL(var, cs, length) static UChar var[(length)+1]
#   define U_STRING_INIT(var, cs, length) u_charsToUChars(cs, var, length+1)
#endif

/**
 * Unescape a string of characters and write the resulting
 * Unicode characters to the destination buffer.  The following escape
 * sequences are recognized:
 *
 * \uhhhh       4 hex digits; h in [0-9A-Fa-f]
 * \Uhhhhhhhh   8 hex digits
 * \xhh         1-2 hex digits
 * \ooo         1-3 octal digits; o in [0-7]
 *
 * as well as the standard ANSI C escapes:
 *
 * \a => U+0007, \b => U+0008, \t => U+0009, \n => U+000A,
 * \v => U+000B, \f => U+000C, \r => U+000D,
 * \" => U+0022, \' => U+0027, \? => U+003F, \\ => U+005C
 *
 * Anything else following a backslash is generically escaped.  For
 * example, "[a\-z]" returns "[a-z]".
 *
 * If an escape sequence is ill-formed, this method returns an empty
 * string.  An example of an ill-formed sequence is "\u" followed by
 * fewer than 4 hex digits.
 *
 * The above characters are recognized in the compiler's codepage,
 * that is, they are coded as 'u', '\\', etc.  Characters that are
 * not parts of escape sequences are converted using u_charsToUChars().
 *
 * This function is similar to UnicodeString::unescape() but not
 * identical to it.  The latter takes a source UnicodeString, so it
 * does escape recognition but no conversion.
 *
 * @param src a zero-terminated string of invariant characters
 * @param dest pointer to buffer to receive converted and unescaped
 * text and, if there is room, a zero terminator.  May be NULL for
 * preflighting, in which case no UChars will be written, but the
 * return value will still be valid.  On error, an empty string is
 * stored here (if possible).
 * @param destCapacity the number of UChars that may be written at
 * dest.  Ignored if dest == NULL.
 * @return the capacity required to fully convert all of the source
 * text, including the zero terminator, or 0 on error.
 * @see u_unescapeAt
 * @see UnicodeString#unescape()
 * @see UnicodeString#unescapeAt()
 * @stable
 */
U_CAPI int32_t U_EXPORT2
u_unescape(const char *src,
           UChar *dest, int32_t destCapacity);

/**
 * Callback function for u_unescapeAt() that returns a character of
 * the source text given an offset and a context pointer.  The context
 * pointer will be whatever is passed into u_unescapeAt().
 *
 * @see u_unescapeAt
 * @stable
 */
U_CDECL_BEGIN
typedef UChar (*UNESCAPE_CHAR_AT)(int32_t offset, void *context);
U_CDECL_END

/**
 * Unescape a single sequence. The character at offset-1 is assumed
 * (without checking) to be a backslash.  This method takes a callback
 * pointer to a function that returns the UChar at a given offset.  By
 * varying this callback, ICU functions are able to unescape char*
 * strings, UnicodeString objects, and UFILE pointers.
 *
 * If offset is out of range, or if the escape sequence is ill-formed,
 * (UChar32)0xFFFFFFFF is returned.  See documentation of u_unescape()
 * for a list of recognized sequences.
 *
 * @param charAt callback function that returns a UChar of the source
 * text given an offset and a context pointer.
 * @param offset pointer to the offset that will be passed to charAt.
 * The offset value will be updated upon return to point after the
 * last parsed character of the escape sequence.  On error the offset
 * is unchanged.
 * @param length the number of characters in the source text.  The
 * last character of the source text is considered to be at offset
 * length-1.
 * @param context an opaque pointer passed directly into charAt.
 * @return the character represented by the escape sequence at
 * offset, or (UChar32)0xFFFFFFFF on error.
 * @see u_unescape()
 * @see UnicodeString#unescape()
 * @see UnicodeString#unescapeAt()
 * @stable
 */
U_CAPI UChar32 U_EXPORT2
u_unescapeAt(UNESCAPE_CHAR_AT charAt,
             int32_t *offset,
             int32_t length,
             void *context);

/**
 * Uppercase the characters in a string.
 * Casing is locale-dependent and context-sensitive.
 * The result may be longer or shorter than the original.
 * The source string and the destination buffer are allowed to overlap.
 *
 * @param dest      A buffer for the result string. The result will be zero-terminated if
 *                  the buffer is large enough.
 * @param destCapacity The size of the buffer (number of UChars). If it is 0, then
 *                  dest may be NULL and the function will only return the length of the result
 *                  without writing any of the result string.
 * @param src       The original string
 * @param srcLength The length of the original string. If -1, then src must be zero-terminated.
 * @param locale    The locale to consider, or "" for the root locale or NULL for the default locale.
 * @param pErrorCode Must be a valid pointer to an error code value,
 *                  which must not indicate a failure before the function call.
 * @return The length of the result string. It may be greater than destCapacity. In that case,
 *         only some of the result was written to the destination buffer.
 * @draft ICU 1.8
 */
U_CAPI int32_t U_EXPORT2
u_strToUpper(UChar *dest, int32_t destCapacity,
             const UChar *src, int32_t srcLength,
             const char *locale,
             UErrorCode *pErrorCode);

/**
 * Lowercase the characters in a string.
 * Casing is locale-dependent and context-sensitive.
 * The result may be longer or shorter than the original.
 * The source string and the destination buffer are allowed to overlap.
 *
 * @param dest      A buffer for the result string. The result will be zero-terminated if
 *                  the buffer is large enough.
 * @param destCapacity The size of the buffer (number of UChars). If it is 0, then
 *                  dest may be NULL and the function will only return the length of the result
 *                  without writing any of the result string.
 * @param src       The original string
 * @param srcLength The length of the original string. If -1, then src must be zero-terminated.
 * @param locale    The locale to consider, or "" for the root locale or NULL for the default locale.
 * @param pErrorCode Must be a valid pointer to an error code value,
 *                  which must not indicate a failure before the function call.
 * @return The length of the result string. It may be greater than destCapacity. In that case,
 *         only some of the result was written to the destination buffer.
 * @draft ICU 1.8
 */
U_CAPI int32_t U_EXPORT2
u_strToLower(UChar *dest, int32_t destCapacity,
             const UChar *src, int32_t srcLength,
             const char *locale,
             UErrorCode *pErrorCode);

/**
 * Titlecase a string.
 * Casing is locale-dependent and context-sensitive.
 * Titlecasing uses a break iterator to find the first characters of words
 * that are to be titlecased. It titlecases those characters and lowercases
 * all others.
 *
 * The titlecase break iterator can be provided to customize for arbitrary
 * styles, using rules and dictionaries beyond the standard iterators.
 * It may be more efficient to always provide an iterator to avoid
 * opening and closing one for each string.
 * The standard titlecase iterator for the root locale implements the
 * algorithm of Unicode TR 21.
 *
 * This function uses only the first() and next() methods of the
 * provided break iterator.
 *
 * The result may be longer or shorter than the original.
 * The source string and the destination buffer are allowed to overlap.
 *
 * @param dest      A buffer for the result string. The result will be zero-terminated if
 *                  the buffer is large enough.
 * @param destCapacity The size of the buffer (number of UChars). If it is 0, then
 *                  dest may be NULL and the function will only return the length of the result
 *                  without writing any of the result string.
 * @param src       The original string
 * @param srcLength The length of the original string. If -1, then src must be zero-terminated.
 * @param titleIter A break iterator to find the first characters of words
 *                  that are to be titlecased.
 *                  If none is provided (NULL), then a standard titlecase
 *                  break iterator is opened.
 * @param locale    The locale to consider, or "" for the root locale or NULL for the default locale.
 * @param pErrorCode Must be a valid pointer to an error code value,
 *                  which must not indicate a failure before the function call.
 * @return The length of the result string. It may be greater than destCapacity. In that case,
 *         only some of the result was written to the destination buffer.
 * @draft ICU 2.1
 */
U_CAPI int32_t U_EXPORT2
u_strToTitle(UChar *dest, int32_t destCapacity,
             const UChar *src, int32_t srcLength,
             UBreakIterator *titleIter,
             const char *locale,
             UErrorCode *pErrorCode);

/**
 * Case-fold the characters in a string.
 * Case-folding is locale-independent and not context-sensitive,
 * but there is an option for whether to include or exclude mappings for dotted I
 * and dotless i that are marked with 'I' in CaseFolding.txt.
 * The result may be longer or shorter than the original.
 * The source string and the destination buffer are allowed to overlap.
 *
 * @param dest      A buffer for the result string. The result will be zero-terminated if
 *                  the buffer is large enough.
 * @param destCapacity The size of the buffer (number of UChars). If it is 0, then
 *                  dest may be NULL and the function will only return the length of the result
 *                  without writing any of the result string.
 * @param src       The original string
 * @param srcLength The length of the original string. If -1, then src must be zero-terminated.
 * @param options   Either U_FOLD_CASE_DEFAULT or U_FOLD_CASE_EXCLUDE_SPECIAL_I
 * @param pErrorCode Must be a valid pointer to an error code value,
 *                  which must not indicate a failure before the function call.
 * @return The length of the result string. It may be greater than destCapacity. In that case,
 *         only some of the result was written to the destination buffer.
 * @draft ICU 1.8
 */
U_CAPI int32_t U_EXPORT2
u_strFoldCase(UChar *dest, int32_t destCapacity,
              const UChar *src, int32_t srcLength,
              uint32_t options,
              UErrorCode *pErrorCode);

/**
 * Converts a sequence of UChars to wchar_t units.
 *
 * @param dest          A buffer for the result string. The result will be zero-terminated if
 *                      the buffer is large enough.
 * @param destCapacity  The size of the buffer (number of UChars). If it is 0, then
 *                      dest may be NULL and the function will only return the length of the 
 *                      result without writing any of the result string (pre-flighting).
 * @param pDestLength   A pointer to receive the number of units written to the destination. If 
 *                      pDestLength!=NULL then *pDestLength is always set to the 
 *                      number of output units corresponding to the transformation of 
 *                      all the input units, even in case of a buffer overflow.
 * @param src           The original source string
 * @param srcLength     The length of the original string. If -1, then src must be zero-terminated.
 * @param pErrorCode    Must be a valid pointer to an error code value,
 *                      which must not indicate a failure before the function call.
 * @return The pointer to destination buffer.
 * @draft ICU 2.0
 */
U_CAPI wchar_t* U_EXPORT2
u_strToWCS(wchar_t *dest, 
           int32_t destCapacity,
           int32_t *pDestLength,
           const UChar *src, 
           int32_t srcLength,
           UErrorCode *pErrorCode);
/**
 * Converts a sequence of wchar_t units to UChars
 *
 * @param dest          A buffer for the result string. The result will be zero-terminated if
 *                      the buffer is large enough.
 * @param destCapacity  The size of the buffer (number of UChars). If it is 0, then
 *                      dest may be NULL and the function will only return the length of the 
 *                      result without writing any of the result string (pre-flighting).
 * @param pDestLength   A pointer to receive the number of units written to the destination. If 
 *                      pDestLength!=NULL then *pDestLength is always set to the 
 *                      number of output units corresponding to the transformation of 
 *                      all the input units, even in case of a buffer overflow.
 * @param src           The original source string
 * @param srcLength     The length of the original string. If -1, then src must be zero-terminated.
 * @param pErrorCode    Must be a valid pointer to an error code value,
 *                      which must not indicate a failure before the function call.
 * @return The pointer to destination buffer.
 * @draft ICU 2.0
 */
U_CAPI UChar* U_EXPORT2
u_strFromWCS(UChar   *dest,
             int32_t destCapacity, 
             int32_t *pDestLength,
             const wchar_t *src,
             int32_t srcLength,
             UErrorCode *pErrorCode);
/**
 * Converts a sequence of UChars (UTF-16) to UTF-8 bytes
 *
 * @param dest          A buffer for the result string. The result will be zero-terminated if
 *                      the buffer is large enough.
 * @param destCapacity  The size of the buffer (number of UChars). If it is 0, then
 *                      dest may be NULL and the function will only return the length of the 
 *                      result without writing any of the result string (pre-flighting).
 * @param pDestLength   A pointer to receive the number of units written to the destination. If 
 *                      pDestLength!=NULL then *pDestLength is always set to the 
 *                      number of output units corresponding to the transformation of 
 *                      all the input units, even in case of a buffer overflow.
 * @param src           The original source string
 * @param srcLength     The length of the original string. If -1, then src must be zero-terminated.
 * @param pErrorCode    Must be a valid pointer to an error code value,
 *                      which must not indicate a failure before the function call.
 * @return The pointer to destination buffer.
 * @draft ICU 2.0
 */
U_CAPI char* U_EXPORT2 
u_strToUTF8(char *dest,           
            int32_t destCapacity,
            int32_t *pDestLength,
            const UChar *src, 
            int32_t srcLength,
            UErrorCode *pErrorCode);

/**
 * Converts a sequence of UTF-8 bytes to UChars (UTF-16).
 *
 * @param dest          A buffer for the result string. The result will be zero-terminated if
 *                      the buffer is large enough.
 * @param destCapacity  The size of the buffer (number of UChars). If it is 0, then
 *                      dest may be NULL and the function will only return the length of the 
 *                      result without writing any of the result string (pre-flighting).
 * @param pDestLength   A pointer to receive the number of units written to the destination. If 
 *                      pDestLength!=NULL then *pDestLength is always set to the 
 *                      number of output units corresponding to the transformation of 
 *                      all the input units, even in case of a buffer overflow.
 * @param src           The original source string
 * @param srcLength     The length of the original string. If -1, then src must be zero-terminated.
 * @param pErrorCode    Must be a valid pointer to an error code value,
 *                      which must not indicate a failure before the function call.
 * @return The pointer to destination buffer.
 * @draft ICU 2.0
 */
U_CAPI UChar* U_EXPORT2
u_strFromUTF8(UChar *dest,             
              int32_t destCapacity,
              int32_t *pDestLength,
              const char *src, 
              int32_t srcLength,
              UErrorCode *pErrorCode);

/**
 * Converts a sequence of UTF32 units to UChars
 *
 * @param dest          A buffer for the result string. The result will be zero-terminated if
 *                      the buffer is large enough.
 * @param destCapacity  The size of the buffer (number of UChars). If it is 0, then
 *                      dest may be NULL and the function will only return the length of the 
 *                      result without writing any of the result string (pre-flighting).
 * @param pDestLength   A pointer to receive the number of units written to the destination. If 
 *                      pDestLength!=NULL then *pDestLength is always set to the 
 *                      number of output units corresponding to the transformation of 
 *                      all the input units, even in case of a buffer overflow.
 * @param src           The original source string
 * @param srcLength     The length of the original string. If -1, then src must be zero-terminated.
 * @param pErrorCode    Must be a valid pointer to an error code value,
 *                      which must not indicate a failure before the function call.
 * @return The pointer to destination buffer.
 * @draft ICU 2.0
 */
U_CAPI UChar32* U_EXPORT2 
u_strToUTF32(UChar32 *dest, 
             int32_t  destCapacity,
             int32_t  *pDestLength,
             const UChar *src, 
             int32_t  srcLength,
             UErrorCode *pErrorCode);

/**
 * Converts a sequence of UChars to UTF32 units.
 *
 * @param dest          A buffer for the result string. The result will be zero-terminated if
 *                      the buffer is large enough.
 * @param destCapacity  The size of the buffer (number of UChars). If it is 0, then
 *                      dest may be NULL and the function will only return the length of the 
 *                      result without writing any of the result string (pre-flighting).
 * @param pDestLength   A pointer to receive the number of units written to the destination. If 
 *                      pDestLength!=NULL then *pDestLength is always set to the 
 *                      number of output units corresponding to the transformation of 
 *                      all the input units, even in case of a buffer overflow.
 * @param src           The original source string
 * @param srcLength     The length of the original string. If -1, then src must be zero-terminated.
 * @param pErrorCode    Must be a valid pointer to an error code value,
 *                      which must not indicate a failure before the function call.
 * @return The pointer to destination buffer.
 * @draft ICU 2.0
 */
U_CAPI UChar* U_EXPORT2 
u_strFromUTF32(UChar   *dest,
               int32_t destCapacity, 
               int32_t *pDestLength,
               const UChar32 *src,
               int32_t srcLength,
               UErrorCode *pErrorCode);

#endif
