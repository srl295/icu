/*
**********************************************************************
*   Copyright (C) 1998-2002, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*
* File unistr.h
*
* Modification History:
*
*   Date        Name        Description
*   09/25/98    stephen     Creation.
*   11/11/98    stephen     Changed per 11/9 code review.
*   04/20/99    stephen     Overhauled per 4/16 code review.
*   11/18/99    aliu        Made to inherit from Replaceable.  Added method
*                           handleReplaceBetween(); other methods unchanged.
*   06/25/01    grhoten     Remove dependency on iostream.
******************************************************************************
*/

#ifndef UNISTR_H
#define UNISTR_H

#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "unicode/rep.h"

struct UConverter;          // unicode/ucnv.h
class  StringThreadTest;

#ifndef U_COMPARE_CODE_POINT_ORDER
/* see also ustring.h and unorm.h */
/**
 * Option bit for u_strCaseCompare, u_strcasecmp, unorm_compare, etc:
 * Compare strings in code point order instead of code unit order.
 * @draft ICU 2.2
 */
#define U_COMPARE_CODE_POINT_ORDER  0x8000
#endif

#ifndef USTRING_H
/* see ustring.h */
U_CAPI int32_t U_EXPORT2
u_strlen(const UChar *s);
#endif

U_NAMESPACE_BEGIN

class Locale;               // unicode/locid.h
class UCharReference;
class StringCharacterIterator;
class BreakIterator;        // unicode/brkiter.h

/* The <iostream> include has been moved to unicode/ustream.h */

/**
 * Unicode String literals in C++.
 * Dependent on the platform properties, different UnicodeString
 * constructors should be used to create a UnicodeString object from
 * a string literal.
 * The macros are defined for maximum performance.
 * They work only for strings that contain "invariant characters", i.e.,
 * only latin letters, digits, and some punctuation.
 * See utypes.h for details.
 *
 * The string parameter must be a C string literal.
 * The length of the string, not including the terminating
 * <code>NUL</code>, must be specified as a constant.
 * The U_STRING_DECL macro should be invoked exactly once for one
 * such string variable before it is used.
 * @stable ICU 2.0
 */
#if U_SIZEOF_WCHAR_T==U_SIZEOF_UCHAR && U_CHARSET_FAMILY==U_ASCII_FAMILY
#   define UNICODE_STRING(cs, _length) UnicodeString(TRUE, (const UChar *)L ## cs, _length)
#elif U_SIZEOF_UCHAR==1 && U_CHARSET_FAMILY==U_ASCII_FAMILY
#   define UNICODE_STRING(cs, _length) UnicodeString(TRUE, (const UChar *)cs, _length)
#else
#   define UNICODE_STRING(cs, _length) UnicodeString(cs, _length, "")
#endif

/**
 * Unicode String literals in C++.
 * Dependent on the platform properties, different UnicodeString
 * constructors should be used to create a UnicodeString object from
 * a string literal.
 * The macros are defined for improved performance.
 * They work only for strings that contain "invariant characters", i.e.,
 * only latin letters, digits, and some punctuation.
 * See utypes.h for details.
 *
 * The string parameter must be a C string literal.
 * @stable ICU 2.0
 */
#if U_SIZEOF_WCHAR_T==U_SIZEOF_UCHAR && U_CHARSET_FAMILY==U_ASCII_FAMILY
#   define UNICODE_STRING_SIMPLE(cs) UnicodeString(TRUE, (const UChar *)L ## cs, -1)
#elif U_SIZEOF_UCHAR==1 && U_CHARSET_FAMILY==U_ASCII_FAMILY
#   define UNICODE_STRING_SIMPLE(cs) UnicodeString(TRUE, (const UChar *)cs, -1)
#else
#   define UNICODE_STRING_SIMPLE(cs) UnicodeString(cs, "")
#endif

/**
 * UnicodeString is a string class that stores Unicode characters directly and provides
 * similar functionality as the Java String and StringBuffer classes.
 * It is a concrete implementation of the abstract class Replaceable (for transliteration).
 *
 * The UnicodeString class is not suitable for subclassing.
 *
 * <p>For an overview of Unicode strings in C and C++ see the
 * <a href="http://oss.software.ibm.com/icu/userguide/strings.html">User Guide Strings chapter</a>.</p>
 *
 * <p>In ICU, a Unicode string consists of 16-bit Unicode <em>code units</em>.
 * A Unicode character may be stored with either
 * one code unit &mdash; which is the most common case &mdash; or with a matched pair of
 * special code units ("surrogates").
 * The data type for code units is UChar.<br>
 * For single-character handling, a Unicode character code <em>point</em> is a value
 * in the range 0..0x10ffff. ICU uses the UChar32 type for code points.</p>
 *
 * <p>Indexes and offsets into and lengths of strings always count code units, not code points.
 * This is the same as with multi-byte char* strings in traditional string handling.
 * Operations on partial strings typically do not test for code point boundaries.
 * If necessary, the user needs to take care of such boundaries by testing for the code unit
 * values or by using functions like
 * UnicodeString::getChar32Start() and UnicodeString::getChar32Limit()
 * (or, in C, the equivalent macros U16_SET_CP_START() and U16_SET_CP_LIMIT(), see utf.h).</p>
 *
 * UnicodeString methods are more lenient with regard to input parameter values
 * than other ICU APIs. In particular:
 * - If indexes are out of bounds for a UnicodeString object
 *   (<0 or >length()) then they are "pinned" to the nearest boundary.
 * - If primitive string pointer values (e.g., const UChar * or char *)
 *   for input strings are NULL, then those input string parameters are treated
 *   as if they pointed to an empty string.
 * - Most UnicodeString methods do not take a UErrorCode parameter because
 *   there are usually very few opportunities for failure other than a shortage
 *   of memory, error codes in low-level C++ string methods would be inconvenient,
 *   and the error code as the last parameter (ICU convention) would prevent
 *   the use of default parameter values.
 *   Instead, such methods set the UnicodeString into a "bogus" state
 *   (see isBogus()) if an error occurs.
 *
 * In string comparisons, two UnicodeString objects that are both "bogus"
 * compare equal (to be transitive and prevent endless loops in sorting),
 * and a "bogus" string compares less than any non-"bogus" one.
 *
 * <p>UnicodeString uses several storage methods.
 * String contents can be stored inside the UnicodeString object itself,
 * in an allocated and shared buffer, or in an outside buffer that is "aliased".
 * Most of this is done transparently, but careful aliasing in particular provides
 * significant performance improvements.
 * Also, the internal buffer is accessible via special functions.
 * For details see the
 * <a href="http://oss.software.ibm.com/icu/userguide/strings.html">User Guide Strings chapter</a>.</p>
 *
 * @see utf.h
 * @see CharacterIterator
 * @stable ICU 2.0
 */
class U_COMMON_API UnicodeString : public Replaceable
{
public:

  //========================================
  // Read-only operations
  //========================================

  /* Comparison - bitwise only - for international comparison use collation */

  /**
   * Equality operator. Performs only bitwise comparison.
   * @param text The UnicodeString to compare to this one.
   * @return TRUE if <TT>text</TT> contains the same characters as this one,
   * FALSE otherwise.
   * @stable ICU 2.0
   */
  inline UBool operator== (const UnicodeString& text) const;

  /**
   * Inequality operator. Performs only bitwise comparison.
   * @param text The UnicodeString to compare to this one.
   * @return FALSE if <TT>text</TT> contains the same characters as this one,
   * TRUE otherwise.
   * @stable ICU 2.0
   */
  inline UBool operator!= (const UnicodeString& text) const;

  /**
   * Greater than operator. Performs only bitwise comparison.
   * @param text The UnicodeString to compare to this one.
   * @return TRUE if the characters in <TT>text</TT> are bitwise
   * greater than the characters in this, FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool operator> (const UnicodeString& text) const;

  /**
   * Less than operator. Performs only bitwise comparison.
   * @param text The UnicodeString to compare to this one.
   * @return TRUE if the characters in <TT>text</TT> are bitwise
   * less than the characters in this, FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool operator< (const UnicodeString& text) const;

  /**
   * Greater than or equal operator. Performs only bitwise comparison.
   * @param text The UnicodeString to compare to this one.
   * @return TRUE if the characters in <TT>text</TT> are bitwise
   * greater than or equal to the characters in this, FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool operator>= (const UnicodeString& text) const;

  /**
   * Less than or equal operator. Performs only bitwise comparison.
   * @param text The UnicodeString to compare to this one.
   * @return TRUE if the characters in <TT>text</TT> are bitwise
   * less than or equal to the characters in this, FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool operator<= (const UnicodeString& text) const;

  /**
   * Compare the characters bitwise in this UnicodeString to
   * the characters in <TT>text</TT>.
   * @param text The UnicodeString to compare to this one.
   * @return The result of bitwise character comparison: 0 if <TT>text</TT>
   * contains the same characters as this, -1 if the characters in 
   * <TT>text</TT> are bitwise less than the characters in this, +1 if the
   * characters in <TT>text</TT> are bitwise greater than the characters 
   * in this.
   * @stable ICU 2.0
   */
  inline int8_t compare(const UnicodeString& text) const;

  /**
   * Compare the characters bitwise in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) with the characters 
   * in <TT>srcText</TT>
   * @param start the offset at which the compare operation begins
   * @param length the number of characters of text to compare.
   * @param srcText the text to be compared
   * @return The result of bitwise character comparison: 0 if <TT>text</TT>
   * contains the same characters as this, -1 if the characters in 
   * <TT>text</TT> are bitwise less than the characters in this, +1 if the
   * characters in <TT>text</TT> are bitwise greater than the characters 
   * in this.
   * @stable ICU 2.0
   */
  inline int8_t compare(int32_t start,
         int32_t length,
         const UnicodeString& srcText) const;

  /**
   * Compare the characters bitwise in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) with the characters 
   * in <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>).  
   * @param start the offset at which the compare operation begins
   * @param length the number of characters in this to compare.
   * @param srcText the text to be compared
   * @param srcStart the offset into <TT>srcText</TT> to start comparison
   * @param srcLength the number of characters in <TT>src</TT> to compare
   * @return The result of bitwise character comparison: 0 if <TT>text</TT>
   * contains the same characters as this, -1 if the characters in 
   * <TT>text</TT> are bitwise less than the characters in this, +1 if the
   * characters in <TT>text</TT> are bitwise greater than the characters 
   * in this.
   * @stable ICU 2.0
   */
   inline int8_t compare(int32_t start,
         int32_t length,
         const UnicodeString& srcText,
         int32_t srcStart,
         int32_t srcLength) const;

  /**
   * Compare the characters bitwise in this UnicodeString with the first 
   * <TT>srcLength</TT> characters in <TT>srcChars</TT>.
   * @param srcChars The characters to compare to this UnicodeString.
   * @param srcLength the number of characters in <TT>srcChars</TT> to compare
   * @return The result of bitwise character comparison: 0 if <TT>text</TT>
   * contains the same characters as this, -1 if the characters in 
   * <TT>text</TT> are bitwise less than the characters in this, +1 if the
   * characters in <TT>text</TT> are bitwise greater than the characters 
   * in this.
   * @stable ICU 2.0
   */
  inline int8_t compare(const UChar *srcChars,
         int32_t srcLength) const;

  /**
   * Compare the characters bitwise in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) with the first 
   * <TT>length</TT> characters in <TT>srcChars</TT>
   * @param start the offset at which the compare operation begins
   * @param length the number of characters to compare.
   * @param srcChars the characters to be compared
   * @return The result of bitwise character comparison: 0 if <TT>text</TT>
   * contains the same characters as this, -1 if the characters in 
   * <TT>text</TT> are bitwise less than the characters in this, +1 if the
   * characters in <TT>text</TT> are bitwise greater than the characters 
   * in this.
   * @stable ICU 2.0
   */
  inline int8_t compare(int32_t start,
         int32_t length,
         const UChar *srcChars) const;

  /**
   * Compare the characters bitwise in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) with the characters 
   * in <TT>srcChars</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>).  
   * @param start the offset at which the compare operation begins
   * @param length the number of characters in this to compare
   * @param srcChars the characters to be compared
   * @param srcStart the offset into <TT>srcChars</TT> to start comparison
   * @param srcLength the number of characters in <TT>srcChars</TT> to compare
   * @return The result of bitwise character comparison: 0 if <TT>text</TT>
   * contains the same characters as this, -1 if the characters in 
   * <TT>text</TT> are bitwise less than the characters in this, +1 if the
   * characters in <TT>text</TT> are bitwise greater than the characters 
   * in this.
   * @stable ICU 2.0
   */
  inline int8_t compare(int32_t start,
         int32_t length,
         const UChar *srcChars,
         int32_t srcStart,
         int32_t srcLength) const;

  /**
   * Compare the characters bitwise in the range 
   * [<TT>start</TT>, <TT>limit</TT>) with the characters 
   * in <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcLimit</TT>).  
   * @param start the offset at which the compare operation begins
   * @param limit the offset immediately following the compare operation
   * @param srcText the text to be compared
   * @param srcStart the offset into <TT>srcText</TT> to start comparison
   * @param srcLimit the offset into <TT>srcText</TT> to limit comparison
   * @return The result of bitwise character comparison: 0 if <TT>text</TT>
   * contains the same characters as this, -1 if the characters in 
   * <TT>text</TT> are bitwise less than the characters in this, +1 if the
   * characters in <TT>text</TT> are bitwise greater than the characters 
   * in this.
   * @stable ICU 2.0
   */
  inline int8_t compareBetween(int32_t start,
            int32_t limit,
            const UnicodeString& srcText,
            int32_t srcStart,
            int32_t srcLimit) const;

  /**
   * Compare two Unicode strings in code point order.
   * This is different in UTF-16 from how compare(), operator==, startsWith() etc. work
   * if supplementary characters are present:
   *
   * In UTF-16, supplementary characters (with code points U+10000 and above) are
   * stored with pairs of surrogate code units. These have values from 0xd800 to 0xdfff,
   * which means that they compare as less than some other BMP characters like U+feff.
   * This function compares Unicode strings in code point order.
   * If either of the UTF-16 strings is malformed (i.e., it contains unpaired surrogates), then the result is not defined.
   *
   * @param text Another string to compare this one to.
   * @return a negative/zero/positive integer corresponding to whether
   * this string is less than/equal to/greater than the second one
   * in code point order
   * @stable ICU 2.0
   */
  inline int8_t compareCodePointOrder(const UnicodeString& text) const;

  /**
   * Compare two Unicode strings in code point order.
   * This is different in UTF-16 from how compare(), operator==, startsWith() etc. work
   * if supplementary characters are present:
   *
   * In UTF-16, supplementary characters (with code points U+10000 and above) are
   * stored with pairs of surrogate code units. These have values from 0xd800 to 0xdfff,
   * which means that they compare as less than some other BMP characters like U+feff.
   * This function compares Unicode strings in code point order.
   * If either of the UTF-16 strings is malformed (i.e., it contains unpaired surrogates), then the result is not defined.
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param length The number of code units from this string to compare.
   * @param srcText Another string to compare this one to.
   * @return a negative/zero/positive integer corresponding to whether
   * this string is less than/equal to/greater than the second one
   * in code point order
   * @stable ICU 2.0
   */
  inline int8_t compareCodePointOrder(int32_t start,
                                      int32_t length,
                                      const UnicodeString& srcText) const;

  /**
   * Compare two Unicode strings in code point order.
   * This is different in UTF-16 from how compare(), operator==, startsWith() etc. work
   * if supplementary characters are present:
   *
   * In UTF-16, supplementary characters (with code points U+10000 and above) are
   * stored with pairs of surrogate code units. These have values from 0xd800 to 0xdfff,
   * which means that they compare as less than some other BMP characters like U+feff.
   * This function compares Unicode strings in code point order.
   * If either of the UTF-16 strings is malformed (i.e., it contains unpaired surrogates), then the result is not defined.
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param length The number of code units from this string to compare.
   * @param srcText Another string to compare this one to.
   * @param srcStart The start offset in that string at which the compare operation begins.
   * @param srcLength The number of code units from that string to compare.
   * @return a negative/zero/positive integer corresponding to whether
   * this string is less than/equal to/greater than the second one
   * in code point order
   * @stable ICU 2.0
   */
   inline int8_t compareCodePointOrder(int32_t start,
                                       int32_t length,
                                       const UnicodeString& srcText,
                                       int32_t srcStart,
                                       int32_t srcLength) const;

  /**
   * Compare two Unicode strings in code point order.
   * This is different in UTF-16 from how compare(), operator==, startsWith() etc. work
   * if supplementary characters are present:
   *
   * In UTF-16, supplementary characters (with code points U+10000 and above) are
   * stored with pairs of surrogate code units. These have values from 0xd800 to 0xdfff,
   * which means that they compare as less than some other BMP characters like U+feff.
   * This function compares Unicode strings in code point order.
   * If either of the UTF-16 strings is malformed (i.e., it contains unpaired surrogates), then the result is not defined.
   *
   * @param srcChars A pointer to another string to compare this one to.
   * @param srcLength The number of code units from that string to compare.
   * @return a negative/zero/positive integer corresponding to whether
   * this string is less than/equal to/greater than the second one
   * in code point order
   * @stable ICU 2.0
   */
  inline int8_t compareCodePointOrder(const UChar *srcChars,
                                      int32_t srcLength) const;

  /**
   * Compare two Unicode strings in code point order.
   * This is different in UTF-16 from how compare(), operator==, startsWith() etc. work
   * if supplementary characters are present:
   *
   * In UTF-16, supplementary characters (with code points U+10000 and above) are
   * stored with pairs of surrogate code units. These have values from 0xd800 to 0xdfff,
   * which means that they compare as less than some other BMP characters like U+feff.
   * This function compares Unicode strings in code point order.
   * If either of the UTF-16 strings is malformed (i.e., it contains unpaired surrogates), then the result is not defined.
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param length The number of code units from this string to compare.
   * @param srcChars A pointer to another string to compare this one to.
   * @return a negative/zero/positive integer corresponding to whether
   * this string is less than/equal to/greater than the second one
   * in code point order
   * @stable ICU 2.0
   */
  inline int8_t compareCodePointOrder(int32_t start,
                                      int32_t length,
                                      const UChar *srcChars) const;

  /**
   * Compare two Unicode strings in code point order.
   * This is different in UTF-16 from how compare(), operator==, startsWith() etc. work
   * if supplementary characters are present:
   *
   * In UTF-16, supplementary characters (with code points U+10000 and above) are
   * stored with pairs of surrogate code units. These have values from 0xd800 to 0xdfff,
   * which means that they compare as less than some other BMP characters like U+feff.
   * This function compares Unicode strings in code point order.
   * If either of the UTF-16 strings is malformed (i.e., it contains unpaired surrogates), then the result is not defined.
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param length The number of code units from this string to compare.
   * @param srcChars A pointer to another string to compare this one to.
   * @param srcStart The start offset in that string at which the compare operation begins.
   * @param srcLength The number of code units from that string to compare.
   * @return a negative/zero/positive integer corresponding to whether
   * this string is less than/equal to/greater than the second one
   * in code point order
   * @stable ICU 2.0
   */
  inline int8_t compareCodePointOrder(int32_t start,
                                      int32_t length,
                                      const UChar *srcChars,
                                      int32_t srcStart,
                                      int32_t srcLength) const;

  /**
   * Compare two Unicode strings in code point order.
   * This is different in UTF-16 from how compare(), operator==, startsWith() etc. work
   * if supplementary characters are present:
   *
   * In UTF-16, supplementary characters (with code points U+10000 and above) are
   * stored with pairs of surrogate code units. These have values from 0xd800 to 0xdfff,
   * which means that they compare as less than some other BMP characters like U+feff.
   * This function compares Unicode strings in code point order.
   * If either of the UTF-16 strings is malformed (i.e., it contains unpaired surrogates), then the result is not defined.
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param limit The offset after the last code unit from this string to compare.
   * @param srcText Another string to compare this one to.
   * @param srcStart The start offset in that string at which the compare operation begins.
   * @param srcLimit The offset after the last code unit from that string to compare.
   * @return a negative/zero/positive integer corresponding to whether
   * this string is less than/equal to/greater than the second one
   * in code point order
   * @stable ICU 2.0
   */
  inline int8_t compareCodePointOrderBetween(int32_t start,
                                             int32_t limit,
                                             const UnicodeString& srcText,
                                             int32_t srcStart,
                                             int32_t srcLimit) const;

  /**
   * Compare two strings case-insensitively using full case folding.
   * This is equivalent to this->foldCase(options).compare(text.foldCase(options)).
   *
   * @param text Another string to compare this one to.
   * @param options A bit set of options:
   *   - U_FOLD_CASE_DEFAULT or 0 is used for default options:
   *     Comparison in code unit order with default case folding.
   *
   *   - U_COMPARE_CODE_POINT_ORDER
   *     Set to choose code point order instead of code unit order
   *     (see u_strCompare for details).
   *
   *   - U_FOLD_CASE_EXCLUDE_SPECIAL_I
   *
   * @return A negative, zero, or positive integer indicating the comparison result.
   * @stable ICU 2.0
   */
  inline int8_t caseCompare(const UnicodeString& text, uint32_t options) const;

  /**
   * Compare two strings case-insensitively using full case folding.
   * This is equivalent to this->foldCase(options).compare(srcText.foldCase(options)).
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param length The number of code units from this string to compare.
   * @param srcText Another string to compare this one to.
   * @param options A bit set of options:
   *   - U_FOLD_CASE_DEFAULT or 0 is used for default options:
   *     Comparison in code unit order with default case folding.
   *
   *   - U_COMPARE_CODE_POINT_ORDER
   *     Set to choose code point order instead of code unit order
   *     (see u_strCompare for details).
   *
   *   - U_FOLD_CASE_EXCLUDE_SPECIAL_I
   *
   * @return A negative, zero, or positive integer indicating the comparison result.
   * @stable ICU 2.0
   */
  inline int8_t caseCompare(int32_t start,
         int32_t length,
         const UnicodeString& srcText,
         uint32_t options) const;

  /**
   * Compare two strings case-insensitively using full case folding.
   * This is equivalent to this->foldCase(options).compare(srcText.foldCase(options)).
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param length The number of code units from this string to compare.
   * @param srcText Another string to compare this one to.
   * @param srcStart The start offset in that string at which the compare operation begins.
   * @param srcLength The number of code units from that string to compare.
   * @param options A bit set of options:
   *   - U_FOLD_CASE_DEFAULT or 0 is used for default options:
   *     Comparison in code unit order with default case folding.
   *
   *   - U_COMPARE_CODE_POINT_ORDER
   *     Set to choose code point order instead of code unit order
   *     (see u_strCompare for details).
   *
   *   - U_FOLD_CASE_EXCLUDE_SPECIAL_I
   *
   * @return A negative, zero, or positive integer indicating the comparison result.
   * @stable ICU 2.0
   */
  inline int8_t caseCompare(int32_t start,
         int32_t length,
         const UnicodeString& srcText,
         int32_t srcStart,
         int32_t srcLength,
         uint32_t options) const;

  /**
   * Compare two strings case-insensitively using full case folding.
   * This is equivalent to this->foldCase(options).compare(srcChars.foldCase(options)).
   *
   * @param srcChars A pointer to another string to compare this one to.
   * @param srcLength The number of code units from that string to compare.
   * @param options A bit set of options:
   *   - U_FOLD_CASE_DEFAULT or 0 is used for default options:
   *     Comparison in code unit order with default case folding.
   *
   *   - U_COMPARE_CODE_POINT_ORDER
   *     Set to choose code point order instead of code unit order
   *     (see u_strCompare for details).
   *
   *   - U_FOLD_CASE_EXCLUDE_SPECIAL_I
   *
   * @return A negative, zero, or positive integer indicating the comparison result.
   * @stable ICU 2.0
   */
  inline int8_t caseCompare(const UChar *srcChars,
         int32_t srcLength,
         uint32_t options) const;

  /**
   * Compare two strings case-insensitively using full case folding.
   * This is equivalent to this->foldCase(options).compare(srcChars.foldCase(options)).
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param length The number of code units from this string to compare.
   * @param srcChars A pointer to another string to compare this one to.
   * @param options A bit set of options:
   *   - U_FOLD_CASE_DEFAULT or 0 is used for default options:
   *     Comparison in code unit order with default case folding.
   *
   *   - U_COMPARE_CODE_POINT_ORDER
   *     Set to choose code point order instead of code unit order
   *     (see u_strCompare for details).
   *
   *   - U_FOLD_CASE_EXCLUDE_SPECIAL_I
   *
   * @return A negative, zero, or positive integer indicating the comparison result.
   * @stable ICU 2.0
   */
  inline int8_t caseCompare(int32_t start,
         int32_t length,
         const UChar *srcChars,
         uint32_t options) const;

  /**
   * Compare two strings case-insensitively using full case folding.
   * This is equivalent to this->foldCase(options).compare(srcChars.foldCase(options)).
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param length The number of code units from this string to compare.
   * @param srcChars A pointer to another string to compare this one to.
   * @param srcStart The start offset in that string at which the compare operation begins.
   * @param srcLength The number of code units from that string to compare.
   * @param options A bit set of options:
   *   - U_FOLD_CASE_DEFAULT or 0 is used for default options:
   *     Comparison in code unit order with default case folding.
   *
   *   - U_COMPARE_CODE_POINT_ORDER
   *     Set to choose code point order instead of code unit order
   *     (see u_strCompare for details).
   *
   *   - U_FOLD_CASE_EXCLUDE_SPECIAL_I
   *
   * @return A negative, zero, or positive integer indicating the comparison result.
   * @stable ICU 2.0
   */
  inline int8_t caseCompare(int32_t start,
         int32_t length,
         const UChar *srcChars,
         int32_t srcStart,
         int32_t srcLength,
         uint32_t options) const;

  /**
   * Compare two strings case-insensitively using full case folding.
   * This is equivalent to this->foldCase(options).compareBetween(text.foldCase(options)).
   *
   * @param start The start offset in this string at which the compare operation begins.
   * @param limit The offset after the last code unit from this string to compare.
   * @param srcText Another string to compare this one to.
   * @param srcStart The start offset in that string at which the compare operation begins.
   * @param srcLimit The offset after the last code unit from that string to compare.
   * @param options A bit set of options:
   *   - U_FOLD_CASE_DEFAULT or 0 is used for default options:
   *     Comparison in code unit order with default case folding.
   *
   *   - U_COMPARE_CODE_POINT_ORDER
   *     Set to choose code point order instead of code unit order
   *     (see u_strCompare for details).
   *
   *   - U_FOLD_CASE_EXCLUDE_SPECIAL_I
   *
   * @return A negative, zero, or positive integer indicating the comparison result.
   * @stable ICU 2.0
   */
  inline int8_t caseCompareBetween(int32_t start,
            int32_t limit,
            const UnicodeString& srcText,
            int32_t srcStart,
            int32_t srcLimit,
            uint32_t options) const;

  /**
   * Determine if this starts with the characters in <TT>text</TT>
   * @param text The text to match.
   * @return TRUE if this starts with the characters in <TT>text</TT>, 
   * FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool startsWith(const UnicodeString& text) const;

  /**
   * Determine if this starts with the characters in <TT>srcText</TT> 
   * in the range [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>).   
   * @param srcText The text to match.
   * @param srcStart the offset into <TT>srcText</TT> to start matching
   * @param srcLength the number of characters in <TT>srcText</TT> to match
   * @return TRUE if this starts with the characters in <TT>text</TT>, 
   * FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool startsWith(const UnicodeString& srcText,
            int32_t srcStart,
            int32_t srcLength) const;

  /**
   * Determine if this starts with the characters in <TT>srcChars</TT>
   * @param srcChars The characters to match.
   * @param srcLength the number of characters in <TT>srcChars</TT>
   * @return TRUE if this starts with the characters in <TT>srcChars</TT>, 
   * FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool startsWith(const UChar *srcChars,
            int32_t srcLength) const;

  /**
   * Determine if this ends with the characters in <TT>srcChars</TT> 
   * in the range  [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>).   
   * @param srcChars The characters to match.
   * @param srcStart the offset into <TT>srcText</TT> to start matching
   * @param srcLength the number of characters in <TT>srcChars</TT> to match
   * @return TRUE if this ends with the characters in <TT>srcChars</TT>, FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool startsWith(const UChar *srcChars,
            int32_t srcStart,
            int32_t srcLength) const;

  /**
   * Determine if this ends with the characters in <TT>text</TT>
   * @param text The text to match.
   * @return TRUE if this ends with the characters in <TT>text</TT>, 
   * FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool endsWith(const UnicodeString& text) const;

  /**
   * Determine if this ends with the characters in <TT>srcText</TT> 
   * in the range [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>).   
   * @param srcText The text to match.
   * @param srcStart the offset into <TT>srcText</TT> to start matching
   * @param srcLength the number of characters in <TT>srcText</TT> to match
   * @return TRUE if this ends with the characters in <TT>text</TT>, 
   * FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool endsWith(const UnicodeString& srcText,
          int32_t srcStart,
          int32_t srcLength) const;

  /**
   * Determine if this ends with the characters in <TT>srcChars</TT>
   * @param srcChars The characters to match.
   * @param srcLength the number of characters in <TT>srcChars</TT>
   * @return TRUE if this ends with the characters in <TT>srcChars</TT>, 
   * FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool endsWith(const UChar *srcChars,
          int32_t srcLength) const;

  /**
   * Determine if this ends with the characters in <TT>srcChars</TT> 
   * in the range  [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>).   
   * @param srcChars The characters to match.
   * @param srcStart the offset into <TT>srcText</TT> to start matching
   * @param srcLength the number of characters in <TT>srcChars</TT> to match
   * @return TRUE if this ends with the characters in <TT>srcChars</TT>, 
   * FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool endsWith(const UChar *srcChars,
          int32_t srcStart,
          int32_t srcLength) const;


  /* Searching - bitwise only */

  /**
   * Locate in this the first occurrence of the characters in <TT>text</TT>,
   * using bitwise comparison.
   * @param text The text to search for.
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(const UnicodeString& text) const;

  /**
   * Locate in this the first occurrence of the characters in <TT>text</TT>
   * starting at offset <TT>start</TT>, using bitwise comparison.
   * @param text The text to search for.
   * @param start The offset at which searching will start.
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(const UnicodeString& text,
              int32_t start) const;

  /**
   * Locate in this the first occurrence in the range
   * [<TT>start</TT>, <TT>start + length</TT>) of the characters 
   * in <TT>text</TT>, using bitwise comparison.
   * @param text The text to search for.
   * @param start The offset at which searching will start.
   * @param length The number of characters to search
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(const UnicodeString& text,
              int32_t start,
              int32_t length) const;

  /**
   * Locate in this the first occurrence in the range
   * [<TT>start</TT>, <TT>start + length</TT>) of the characters
   *  in <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>), 
   * using bitwise comparison.   
   * @param srcText The text to search for.
   * @param srcStart the offset into <TT>srcText</TT> at which
   * to start matching
   * @param srcLength the number of characters in <TT>srcText</TT> to match
   * @param start the offset into this at which to start matching
   * @param length the number of characters in this to search
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(const UnicodeString& srcText,
              int32_t srcStart,
              int32_t srcLength,
              int32_t start,
              int32_t length) const;

  /**
   * Locate in this the first occurrence of the characters in
   * <TT>srcChars</TT> 
   * starting at offset <TT>start</TT>, using bitwise comparison. 
   * @param srcChars The text to search for.
   * @param srcLength the number of characters in <TT>srcChars</TT> to match
   * @param start the offset into this at which to start matching
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(const UChar *srcChars,
              int32_t srcLength,
              int32_t start) const;

  /**
   * Locate in this the first occurrence in the range
   * [<TT>start</TT>, <TT>start + length</TT>) of the characters 
   * in <TT>srcChars</TT>, using bitwise comparison.
   * @param srcChars The text to search for.
   * @param srcLength the number of characters in <TT>srcChars</TT>
   * @param start The offset at which searching will start.
   * @param length The number of characters to search
   * @return The offset into this of the start of <TT>srcChars</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(const UChar *srcChars,
              int32_t srcLength,
              int32_t start,
              int32_t length) const;
 
  /**
   * Locate in this the first occurrence in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) of the characters 
   * in <TT>srcChars</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>), 
   * using bitwise comparison.   
   * @param srcChars The text to search for.
   * @param srcStart the offset into <TT>srcChars</TT> at which 
   * to start matching
   * @param srcLength the number of characters in <TT>srcChars</TT> to match
   * @param start the offset into this at which to start matching
   * @param length the number of characters in this to search
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  int32_t indexOf(const UChar *srcChars,
              int32_t srcStart,
              int32_t srcLength,
              int32_t start,
              int32_t length) const;

  /**
   * Locate in this the first occurrence of the BMP code point <code>c</code>,
   * using bitwise comparison.
   * @param c The code unit to search for.
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(UChar c) const;

  /**
   * Locate in this the first occurrence of the code point <TT>c</TT>, 
   * using bitwise comparison.
   *
   * @param c The code point to search for.
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(UChar32 c) const;

  /**
   * Locate in this the first occurrence of the BMP code point <code>c</code>,
   * starting at offset <TT>start</TT>, using bitwise comparison.
   * @param c The code unit to search for.
   * @param start The offset at which searching will start.
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(UChar c,
              int32_t start) const;

  /**
   * Locate in this the first occurrence of the code point <TT>c</TT>
   * starting at offset <TT>start</TT>, using bitwise comparison.
   *
   * @param c The code point to search for.
   * @param start The offset at which searching will start.
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(UChar32 c,
              int32_t start) const;

  /**
   * Locate in this the first occurrence of the BMP code point <code>c</code>
   * in the range [<TT>start</TT>, <TT>start + length</TT>), 
   * using bitwise comparison.   
   * @param c The code unit to search for.
   * @param start the offset into this at which to start matching
   * @param length the number of characters in this to search
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(UChar c,
              int32_t start,
              int32_t length) const;

  /**
   * Locate in this the first occurrence of the code point <TT>c</TT> 
   * in the range [<TT>start</TT>, <TT>start + length</TT>), 
   * using bitwise comparison.   
   *
   * @param c The code point to search for.
   * @param start the offset into this at which to start matching
   * @param length the number of characters in this to search
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t indexOf(UChar32 c,
              int32_t start,
              int32_t length) const;

  /**
   * Locate in this the last occurrence of the characters in <TT>text</TT>, 
   * using bitwise comparison.
   * @param text The text to search for.
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(const UnicodeString& text) const;

  /**
   * Locate in this the last occurrence of the characters in <TT>text</TT>
   * starting at offset <TT>start</TT>, using bitwise comparison.
   * @param text The text to search for.
   * @param start The offset at which searching will start.
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(const UnicodeString& text,
              int32_t start) const;

  /**
   * Locate in this the last occurrence in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) of the characters
   * in <TT>text</TT>, using bitwise comparison.
   * @param text The text to search for.
   * @param start The offset at which searching will start.
   * @param length The number of characters to search
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(const UnicodeString& text,
              int32_t start,
              int32_t length) const;

  /**
   * Locate in this the last occurrence in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) of the characters 
   * in <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>), 
   * using bitwise comparison.   
   * @param srcText The text to search for.
   * @param srcStart the offset into <TT>srcText</TT> at which 
   * to start matching
   * @param srcLength the number of characters in <TT>srcText</TT> to match
   * @param start the offset into this at which to start matching
   * @param length the number of characters in this to search
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(const UnicodeString& srcText,
              int32_t srcStart,
              int32_t srcLength,
              int32_t start,
              int32_t length) const;

  /**
   * Locate in this the last occurrence of the characters in <TT>srcChars</TT> 
   * starting at offset <TT>start</TT>, using bitwise comparison. 
   * @param srcChars The text to search for.
   * @param srcLength the number of characters in <TT>srcChars</TT> to match
   * @param start the offset into this at which to start matching
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(const UChar *srcChars,
              int32_t srcLength,
              int32_t start) const;

  /**
   * Locate in this the last occurrence in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) of the characters 
   * in <TT>srcChars</TT>, using bitwise comparison.
   * @param srcChars The text to search for.
   * @param srcLength the number of characters in <TT>srcChars</TT>
   * @param start The offset at which searching will start.
   * @param length The number of characters to search
   * @return The offset into this of the start of <TT>srcChars</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(const UChar *srcChars,
              int32_t srcLength,
              int32_t start,
              int32_t length) const;
 
  /**
   * Locate in this the last occurrence in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) of the characters 
   * in <TT>srcChars</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>), 
   * using bitwise comparison.   
   * @param srcChars The text to search for.
   * @param srcStart the offset into <TT>srcChars</TT> at which
   * to start matching
   * @param srcLength the number of characters in <TT>srcChars</TT> to match
   * @param start the offset into this at which to start matching
   * @param length the number of characters in this to search
   * @return The offset into this of the start of <TT>text</TT>, 
   * or -1 if not found.
   * @stable ICU 2.0
   */
  int32_t lastIndexOf(const UChar *srcChars,
              int32_t srcStart,
              int32_t srcLength,
              int32_t start,
              int32_t length) const;

  /**
   * Locate in this the last occurrence of the BMP code point <code>c</code>,
   * using bitwise comparison.
   * @param c The code unit to search for.
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(UChar c) const;

  /**
   * Locate in this the last occurrence of the code point <TT>c</TT>, 
   * using bitwise comparison.
   *
   * @param c The code point to search for.
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(UChar32 c) const;

  /**
   * Locate in this the last occurrence of the BMP code point <code>c</code>
   * starting at offset <TT>start</TT>, using bitwise comparison.
   * @param c The code unit to search for.
   * @param start The offset at which searching will start.
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(UChar c,
              int32_t start) const;

  /**
   * Locate in this the last occurrence of the code point <TT>c</TT>
   * starting at offset <TT>start</TT>, using bitwise comparison.
   *
   * @param c The code point to search for.
   * @param start The offset at which searching will start.
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(UChar32 c,
              int32_t start) const;

  /**
   * Locate in this the last occurrence of the BMP code point <code>c</code>
   * in the range [<TT>start</TT>, <TT>start + length</TT>), 
   * using bitwise comparison.   
   * @param c The code unit to search for.
   * @param start the offset into this at which to start matching
   * @param length the number of characters in this to search
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(UChar c,
              int32_t start,
              int32_t length) const;

  /**
   * Locate in this the last occurrence of the code point <TT>c</TT> 
   * in the range [<TT>start</TT>, <TT>start + length</TT>), 
   * using bitwise comparison.   
   *
   * @param c The code point to search for.
   * @param start the offset into this at which to start matching
   * @param length the number of characters in this to search
   * @return The offset into this of <TT>c</TT>, or -1 if not found.
   * @stable ICU 2.0
   */
  inline int32_t lastIndexOf(UChar32 c,
              int32_t start,
              int32_t length) const;


  /* Character access */

  /**
   * Return the code unit at offset <tt>offset</tt>.
   * If the offset is not valid (0..length()-1) then U+ffff is returned.
   * @param offset a valid offset into the text
   * @return the code unit at offset <tt>offset</tt>
   *         or 0xffff if the offset is not valid for this string
   * @stable ICU 2.0
   */
  inline UChar charAt(int32_t offset) const;

  /**
   * Return the code unit at offset <tt>offset</tt>.
   * If the offset is not valid (0..length()-1) then U+ffff is returned.
   * @param offset a valid offset into the text
   * @return the code unit at offset <tt>offset</tt>
   * @stable ICU 2.0
   */
  inline UChar operator[] (int32_t offset) const;

  /**
   * Return the code point that contains the code unit
   * at offset <tt>offset</tt>.
   * If the offset is not valid (0..length()-1) then U+ffff is returned.
   * @param offset a valid offset into the text
   * that indicates the text offset of any of the code units
   * that will be assembled into a code point (21-bit value) and returned
   * @return the code point of text at <tt>offset</tt>
   *         or 0xffff if the offset is not valid for this string
   * @stable ICU 2.0
   */
  inline UChar32 char32At(int32_t offset) const;

  /**
   * Adjust a random-access offset so that
   * it points to the beginning of a Unicode character.
   * The offset that is passed in points to
   * any code unit of a code point,
   * while the returned offset will point to the first code unit
   * of the same code point.
   * In UTF-16, if the input offset points to a second surrogate
   * of a surrogate pair, then the returned offset will point
   * to the first surrogate.
   * @param offset a valid offset into one code point of the text
   * @return offset of the first code unit of the same code point
   * @see U16_SET_CP_START
   * @stable ICU 2.0
   */
  inline int32_t getChar32Start(int32_t offset) const;

  /**
   * Adjust a random-access offset so that
   * it points behind a Unicode character.
   * The offset that is passed in points behind
   * any code unit of a code point,
   * while the returned offset will point behind the last code unit
   * of the same code point.
   * In UTF-16, if the input offset points behind the first surrogate
   * (i.e., to the second surrogate)
   * of a surrogate pair, then the returned offset will point
   * behind the second surrogate (i.e., to the first surrogate).
   * @param offset a valid offset after any code unit of a code point of the text
   * @return offset of the first code unit after the same code point
   * @see U16_SET_CP_LIMIT
   * @stable ICU 2.0
   */
  inline int32_t getChar32Limit(int32_t offset) const;

  /**
   * Move the code unit index along the string by delta code points.
   * Interpret the input index as a code unit-based offset into the string,
   * move the index forward or backward by delta code points, and
   * return the resulting index.
   * The input index should point to the first code unit of a code point,
   * if there is more than one.
   *
   * Both input and output indexes are code unit-based as for all
   * string indexes/offsets in ICU (and other libraries, like MBCS char*).
   * If delta<0 then the index is moved backward (toward the start of the string).
   * If delta>0 then the index is moved forward (toward the end of the string).
   *
   * This behaves like CharacterIterator::move32(delta, kCurrent).
   *
   * Behavior for out-of-bounds indexes:
   * <code>moveIndex32</code> pins the input index to 0..length(), i.e.,
   * if the input index<0 then it is pinned to 0;
   * if it is index>length() then it is pinned to length().
   * Afterwards, the index is moved by <code>delta</code> code points
   * forward or backward,
   * but no further backward than to 0 and no further forward than to length().
   * The resulting index return value will be in between 0 and length(), inclusively.
   *
   * Examples:
   * <pre>
   * // s has code points 'a' U+10000 'b' U+10ffff U+2029
   * UnicodeString s=UNICODE_STRING("a\\U00010000b\\U0010ffff\\u2029", 31).unescape();
   *
   * // initial index: position of U+10000
   * int32_t index=1;
   *
   * // the following examples will all result in index==4, position of U+10ffff
   *
   * // skip 2 code points from some position in the string
   * index=s.moveIndex32(index, 2); // skips U+10000 and 'b'
   *
   * // go to the 3rd code point from the start of s (0-based)
   * index=s.moveIndex32(0, 3); // skips 'a', U+10000, and 'b'
   *
   * // go to the next-to-last code point of s
   * index=s.moveIndex32(s.length(), -2); // backward-skips U+2029 and U+10ffff
   * </pre>
   *
   * @param index input code unit index
   * @param delta (signed) code point count to move the index forward or backward
   *        in the string
   * @return the resulting code unit index
   * @stable ICU 2.0
   */
  int32_t moveIndex32(int32_t index, int32_t delta) const;

  /* Substring extraction */

  /**
   * Copy the characters in the range 
   * [<tt>start</tt>, <tt>start + length</tt>) into the array <tt>dst</tt>,
   * beginning at <tt>dstStart</tt>.
   * If the string aliases to <code>dst</code> itself as an external buffer,
   * then extract() will not copy the contents.
   *
   * @param start offset of first character which will be copied into the array
   * @param length the number of characters to extract
   * @param dst array in which to copy characters.  The length of <tt>dst</tt>
   * must be at least (<tt>dstStart + length</tt>).
   * @param dstStart the offset in <TT>dst</TT> where the first character
   * will be extracted
   * @stable ICU 2.0
   */
  inline void extract(int32_t start, 
           int32_t length, 
           UChar *dst, 
           int32_t dstStart = 0) const;

  /**
   * Copy the contents of the string into dest.
   * This is a convenience function that
   * checks if there is enough space in dest,
   * extracts the entire string if possible,
   * and NUL-terminates dest if possible.
   *
   * If the string fits into dest but cannot be NUL-terminated
   * (length()==destCapacity) then the error code is set to U_STRING_NOT_TERMINATED_WARNING.
   * If the string itself does not fit into dest
   * (length()>destCapacity) then the error code is set to U_BUFFER_OVERFLOW_ERROR.
   *
   * If the string aliases to <code>dest</code> itself as an external buffer,
   * then extract() will not copy the contents.
   *
   * @param dest Destination string buffer.
   * @param destCapacity Number of UChars available at dest.
   * @param errorCode ICU error code.
   * @return length()
   * @stable ICU 2.0
   */
  int32_t
  extract(UChar *dest, int32_t destCapacity,
          UErrorCode &errorCode) const;

  /**
   * Copy the characters in the range 
   * [<tt>start</tt>, <tt>start + length</tt>) into the  UnicodeString
   * <tt>target</tt>.
   * @param start offset of first character which will be copied
   * @param length the number of characters to extract
   * @param target UnicodeString into which to copy characters.
   * @return A reference to <TT>target</TT>
   * @stable ICU 2.0
   */
  inline void extract(int32_t start,
           int32_t length,
           UnicodeString& target) const;

  /**
   * Copy the characters in the range [<tt>start</tt>, <tt>limit</tt>) 
   * into the array <tt>dst</tt>, beginning at <tt>dstStart</tt>.
   * @param start offset of first character which will be copied into the array
   * @param limit offset immediately following the last character to be copied
   * @param dst array in which to copy characters.  The length of <tt>dst</tt> 
   * must be at least (<tt>dstStart + (limit - start)</tt>).
   * @param dstStart the offset in <TT>dst</TT> where the first character
   * will be extracted
   * @stable ICU 2.0
   */
  inline void extractBetween(int32_t start, 
              int32_t limit, 
              UChar *dst, 
              int32_t dstStart = 0) const;

  /**
   * Copy the characters in the range [<tt>start</tt>, <tt>limit</tt>) 
   * into the UnicodeString <tt>target</tt>.  Replaceable API.
   * @param start offset of first character which will be copied
   * @param limit offset immediately following the last character to be copied
   * @param target UnicodeString into which to copy characters.
   * @return A reference to <TT>target</TT>
   * @stable ICU 2.0
   */
  virtual void extractBetween(int32_t start,
              int32_t limit,
              UnicodeString& target) const;

  /**
   * Copy the characters in the range 
   * [<tt>start</TT>, <tt>start + length</TT>) into an array of characters
   * in a specified codepage.
   * The output string is NUL-terminated.
   *
   * @param start offset of first character which will be copied
   * @param startLength the number of characters to extract
   * @param target the target buffer for extraction
   * @param codepage the desired codepage for the characters.  0 has 
   * the special meaning of the default codepage
   * If <code>codepage</code> is an empty string (<code>""</code>),
   * then a simple conversion is performed on the codepage-invariant
   * subset ("invariant characters") of the platform encoding. See utypes.h.
   * If <TT>target</TT> is NULL, then the number of bytes required for
   * <TT>target</TT> is returned. It is assumed that the target is big enough
   * to fit all of the characters.
   * @return the output string length, not including the terminating NUL
   * @stable ICU 2.0
   */
  inline int32_t extract(int32_t start,
                 int32_t startLength,
                 char *target,
                 const char *codepage = 0) const;

  /**
   * Copy the characters in the range 
   * [<tt>start</TT>, <tt>start + length</TT>) into an array of characters
   * in a specified codepage.
   * This function does not write any more than <code>targetLength</code>
   * characters but returns the length of the entire output string
   * so that one can allocate a larger buffer and call the function again
   * if necessary.
   * The output string is NUL-terminated if possible.
   *
   * @param start offset of first character which will be copied
   * @param startLength the number of characters to extract
   * @param target the target buffer for extraction
   * @param targetLength the length of the target buffer
   * @param codepage the desired codepage for the characters.  0 has 
   * the special meaning of the default codepage
   * If <code>codepage</code> is an empty string (<code>""</code>),
   * then a simple conversion is performed on the codepage-invariant
   * subset ("invariant characters") of the platform encoding. See utypes.h.
   * If <TT>target</TT> is NULL, then the number of bytes required for
   * <TT>target</TT> is returned.
   * @return the output string length, not including the terminating NUL
   * @stable ICU 2.0
   */
  int32_t extract(int32_t start,
           int32_t startLength,
           char *target,
           uint32_t targetLength,
           const char *codepage = 0) const;

  /**
   * Convert the UnicodeString into a codepage string using an existing UConverter.
   * The output string is NUL-terminated if possible.
   *
   * This function avoids the overhead of opening and closing a converter if
   * multiple strings are extracted.
   *
   * @param dest destination string buffer, can be NULL if destCapacity==0
   * @param destCapacity the number of chars available at dest
   * @param cnv the converter object to be used (ucnv_resetFromUnicode() will be called),
   *        or NULL for the default converter
   * @param errorCode normal ICU error code
   * @return the length of the output string, not counting the terminating NUL;
   *         if the length is greater than destCapacity, then the string will not fit
   *         and a buffer of the indicated length would need to be passed in
   * @stable ICU 2.0
   */
  int32_t extract(char *dest, int32_t destCapacity,
                  UConverter *cnv,
                  UErrorCode &errorCode) const;

  /* Length operations */

  /**
   * Return the length of the UnicodeString object.  
   * The length is the number of characters in the text.
   * @return the length of the UnicodeString object
   * @stable ICU 2.0
   */
  inline int32_t length(void) const;

  /**
   * Count Unicode code points in the length UChar code units of the string.
   * A code point may occupy either one or two UChar code units.
   * Counting code points involves reading all code units.
   *
   * This functions is basically the inverse of moveIndex32().
   *
   * @param start the index of the first code unit to check
   * @param length the number of UChar code units to check
   * @return the number of code points in the specified code units
   * @stable ICU 2.0
   */
  int32_t
  countChar32(int32_t start=0, int32_t length=INT32_MAX) const;

  /**
   * Check if the length UChar code units of the string
   * contain more Unicode code points than a certain number.
   * This is more efficient than counting all code points in this part of the string
   * and comparing that number with a threshold.
   * This function may not need to scan the string at all if the length
   * falls within a certain range, and
   * never needs to count more than 'number+1' code points.
   * Logically equivalent to (countChar32(start, length)>number).
   * A Unicode code point may occupy either one or two UChar code units.
   *
   * @param start the index of the first code unit to check (0 for the entire string)
   * @param length the number of UChar code units to check
   *               (use INT32_MAX for the entire string; remember that start/length
   *                values are pinned)
   * @param number The number of code points in the (sub)string is compared against
   *               the 'number' parameter.
   * @return Boolean value for whether the string contains more Unicode code points
   *         than 'number'. Same as (u_countChar32(s, length)>number).
   * @see countChar32
   * @see u_strHasMoreChar32Than
   * @draft ICU 2.4
   */
  UBool
  hasMoreChar32Than(int32_t start, int32_t length, int32_t number) const;

  /**
   * Determine if this string is empty.
   * @return TRUE if this string contains 0 characters, FALSE otherwise.
   * @stable ICU 2.0
   */
  inline UBool isEmpty(void) const;

  /**
   * Return the capacity of the internal buffer of the UnicodeString object.
   * This is useful together with the getBuffer functions.
   * See there for details.
   *
   * @return the number of UChars available in the internal buffer
   * @see getBuffer
   * @stable ICU 2.0
   */
  inline int32_t getCapacity(void) const;

  /* Other operations */

  /**
   * Generate a hash code for this object.
   * @return The hash code of this UnicodeString.
   * @stable ICU 2.0
   */
  inline int32_t hashCode(void) const;

  /**
   * Determine if this object contains a valid string.
   * A bogus string has no value. It is different from an empty string.
   * It can be used to indicate that no string value is available.
   * getBuffer() and getTerminatedBuffer() return NULL, and
   * length() returns 0.
   *
   * @return TRUE if the string is valid, FALSE otherwise
   * @see setToBogus()
   * @stable ICU 2.0
   */
  inline UBool isBogus(void) const;


  //========================================
  // Write operations
  //========================================

  /* Assignment operations */

  /**
   * Assignment operator.  Replace the characters in this UnicodeString
   * with the characters from <TT>srcText</TT>.
   * @param srcText The text containing the characters to replace
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString &operator=(const UnicodeString &srcText);

  /**
   * Almost the same as the assignment operator.
   * Replace the characters in this UnicodeString
   * with the characters from <code>srcText</code>.
   *
   * This function works the same for all strings except for ones that
   * are readonly aliases.
   * Starting with ICU 2.4, the assignment operator and the copy constructor
   * allocate a new buffer and copy the buffer contents even for readonly aliases.
   * This function implements the old, more efficient but less safe behavior
   * of making this string also a readonly alias to the same buffer.
   * The fastCopyFrom function must be used only if it is known that the lifetime of
   * this UnicodeString is at least as long as the lifetime of the aliased buffer
   * including its contents, for example for strings from resource bundles
   * or aliases to string contents.
   *
   * @param src The text containing the characters to replace.
   * @return a reference to this
   * @draft ICU 2.4
   */
  UnicodeString &fastCopyFrom(const UnicodeString &src);

  /**
   * Assignment operator.  Replace the characters in this UnicodeString
   * with the code unit <TT>ch</TT>.
   * @param ch the code unit to replace
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& operator= (UChar ch);

  /**
   * Assignment operator.  Replace the characters in this UnicodeString
   * with the code point <TT>ch</TT>.
   * @param ch the code point to replace
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& operator= (UChar32 ch);

  /**
   * Set the text in the UnicodeString object to the characters
   * in <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcText.length()</TT>).
   * <TT>srcText</TT> is not modified.
   * @param srcText the source for the new characters
   * @param srcStart the offset into <TT>srcText</TT> where new characters
   * will be obtained
   * @return a reference to this
   * @draft ICU2.2
   */
  inline UnicodeString& setTo(const UnicodeString& srcText, 
               int32_t srcStart);

  /**
   * Set the text in the UnicodeString object to the characters
   * in <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>).
   * <TT>srcText</TT> is not modified.
   * @param srcText the source for the new characters
   * @param srcStart the offset into <TT>srcText</TT> where new characters
   * will be obtained
   * @param srcLength the number of characters in <TT>srcText</TT> in the
   * replace string.
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& setTo(const UnicodeString& srcText, 
               int32_t srcStart, 
               int32_t srcLength);

  /**
   * Set the text in the UnicodeString object to the characters in 
   * <TT>srcText</TT>.  
   * <TT>srcText</TT> is not modified.
   * @param srcText the source for the new characters
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& setTo(const UnicodeString& srcText);

  /**
   * Set the characters in the UnicodeString object to the characters
   * in <TT>srcChars</TT>. <TT>srcChars</TT> is not modified.
   * @param srcChars the source for the new characters
   * @param srcLength the number of Unicode characters in srcChars.
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& setTo(const UChar *srcChars,
               int32_t srcLength);

  /**
   * Set the characters in the UnicodeString object to the code unit
   * <TT>srcChar</TT>.
   * @param srcChar the code unit which becomes the UnicodeString's character 
   * content
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString& setTo(UChar srcChar);

  /**
   * Set the characters in the UnicodeString object to the code point
   * <TT>srcChar</TT>.
   * @param srcChar the code point which becomes the UnicodeString's character 
   * content
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString& setTo(UChar32 srcChar);

  /**
   * Aliasing setTo() function, analogous to the readonly-aliasing UChar* constructor.
   * The text will be used for the UnicodeString object, but
   * it will not be released when the UnicodeString is destroyed.
   * This has copy-on-write semantics:
   * When the string is modified, then the buffer is first copied into
   * newly allocated memory.
   * The aliased buffer is never modified.
   * In an assignment to another UnicodeString, the text will be aliased again,
   * so that both strings then alias the same readonly-text.
   *
   * @param isTerminated specifies if <code>text</code> is <code>NUL</code>-terminated.
   *                     This must be true if <code>textLength==-1</code>.
   * @param text The characters to alias for the UnicodeString.
   * @param textLength The number of Unicode characters in <code>text</code> to alias.
   *                   If -1, then this constructor will determine the length
   *                   by calling <code>u_strlen()</code>.
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString &setTo(UBool isTerminated,
                       const UChar *text,
                       int32_t textLength);

  /**
   * Aliasing setTo() function, analogous to the writable-aliasing UChar* constructor.
   * The text will be used for the UnicodeString object, but
   * it will not be released when the UnicodeString is destroyed.
   * This has write-through semantics:
   * For as long as the capacity of the buffer is sufficient, write operations
   * will directly affect the buffer. When more capacity is necessary, then
   * a new buffer will be allocated and the contents copied as with regularly
   * constructed strings.
   * In an assignment to another UnicodeString, the buffer will be copied.
   * The extract(UChar *dst) function detects whether the dst pointer is the same
   * as the string buffer itself and will in this case not copy the contents.
   *
   * @param buffer The characters to alias for the UnicodeString.
   * @param buffLength The number of Unicode characters in <code>buffer</code> to alias.
   * @param buffCapacity The size of <code>buffer</code> in UChars.
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString &setTo(UChar *buffer,
                       int32_t buffLength,
                       int32_t buffCapacity);

  /**
   * Make this UnicodeString object invalid.
   * The string will test TRUE with isBogus().
   *
   * A bogus string has no value. It is different from an empty string.
   * It can be used to indicate that no string value is available.
   * getBuffer() and getTerminatedBuffer() return NULL, and
   * length() returns 0.
   *
   * This utility function is used throughout the UnicodeString
   * implementation to indicate that a UnicodeString operation failed,
   * and may be used in other functions,
   * especially but not exclusively when such functions do not
   * take a UErrorCode for simplicity.
   *
   * The following methods, and no others, will clear a string object's bogus flag:
   * - remove()
   * - remove(0, INT32_MAX)
   * - truncate(0)
   * - operator=() (assignment operator)
   * - setTo(...)
   *
   * The simplest ways to turn a bogus string into an empty one
   * is to use the remove() function.
   * Examples for other functions that are equivalent to "set to empty string":
   * \code
   * if(s.isBogus()) {
   *   s.remove();           // set to an empty string (remove all), or
   *   s.remove(0, INT32_MAX); // set to an empty string (remove all), or
   *   s.truncate(0);        // set to an empty string (complete truncation), or
   *   s=UnicodeString();    // assign an empty string, or
   *   s.setTo((UChar32)-1); // set to a pseudo code point that is out of range, or
   *   static const UChar nul=0;
   *   s.setTo(&nul, 0);     // set to an empty C Unicode string
   * }
   * \endcode
   *
   * @see isBogus()
   * @stable ICU 2.0
   */
  void setToBogus();

  /**
   * Set the character at the specified offset to the specified character.
   * @param offset A valid offset into the text of the character to set
   * @param ch The new character
   * @return A reference to this
   * @stable ICU 2.0
   */
  UnicodeString& setCharAt(int32_t offset, 
               UChar ch);


  /* Append operations */

  /**
   * Append operator. Append the code unit <TT>ch</TT> to the UnicodeString
   * object.
   * @param ch the code unit to be appended
   * @return a reference to this
   * @stable ICU 2.0
   */
 inline  UnicodeString& operator+= (UChar ch);

  /**
   * Append operator. Append the code point <TT>ch</TT> to the UnicodeString
   * object.
   * @param ch the code point to be appended
   * @return a reference to this
   * @stable ICU 2.0
   */
 inline  UnicodeString& operator+= (UChar32 ch);

  /**
   * Append operator. Append the characters in <TT>srcText</TT> to the
   * UnicodeString object at offset <TT>start</TT>. <TT>srcText</TT> is
   * not modified.
   * @param srcText the source for the new characters
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& operator+= (const UnicodeString& srcText);

  /**
   * Append the characters
   * in <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>) to the 
   * UnicodeString object at offset <TT>start</TT>. <TT>srcText</TT> 
   * is not modified.
   * @param srcText the source for the new characters
   * @param srcStart the offset into <TT>srcText</TT> where new characters 
   * will be obtained
   * @param srcLength the number of characters in <TT>srcText</TT> in 
   * the append string
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& append(const UnicodeString& srcText, 
            int32_t srcStart, 
            int32_t srcLength);

  /**
   * Append the characters in <TT>srcText</TT> to the UnicodeString object at 
   * offset <TT>start</TT>. <TT>srcText</TT> is not modified.
   * @param srcText the source for the new characters
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& append(const UnicodeString& srcText);

  /**
   * Append the characters in <TT>srcChars</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>) to the UnicodeString 
   * object at offset 
   * <TT>start</TT>. <TT>srcChars</TT> is not modified.
   * @param srcChars the source for the new characters
   * @param srcStart the offset into <TT>srcChars</TT> where new characters 
   * will be obtained
   * @param srcLength the number of characters in <TT>srcChars</TT> in 
   * the append string
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& append(const UChar *srcChars, 
            int32_t srcStart, 
            int32_t srcLength);

  /**
   * Append the characters in <TT>srcChars</TT> to the UnicodeString object 
   * at offset <TT>start</TT>. <TT>srcChars</TT> is not modified.
   * @param srcChars the source for the new characters
   * @param srcLength the number of Unicode characters in <TT>srcChars</TT>
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& append(const UChar *srcChars,
            int32_t srcLength);

  /**
   * Append the code unit <TT>srcChar</TT> to the UnicodeString object.
   * @param srcChar the code unit to append
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& append(UChar srcChar);

  /**
   * Append the code point <TT>srcChar</TT> to the UnicodeString object.
   * @param srcChar the code point to append
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& append(UChar32 srcChar);


  /* Insert operations */

  /**
   * Insert the characters in <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>) into the UnicodeString 
   * object at offset <TT>start</TT>. <TT>srcText</TT> is not modified.
   * @param start the offset where the insertion begins
   * @param srcText the source for the new characters 
   * @param srcStart the offset into <TT>srcText</TT> where new characters 
   * will be obtained
   * @param srcLength the number of characters in <TT>srcText</TT> in 
   * the insert string
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& insert(int32_t start, 
            const UnicodeString& srcText, 
            int32_t srcStart, 
            int32_t srcLength);

  /**
   * Insert the characters in <TT>srcText</TT> into the UnicodeString object
   * at offset <TT>start</TT>. <TT>srcText</TT> is not modified.
   * @param start the offset where the insertion begins
   * @param srcText the source for the new characters 
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& insert(int32_t start, 
            const UnicodeString& srcText);

  /**
   * Insert the characters in <TT>srcChars</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>) into the UnicodeString
   *  object at offset <TT>start</TT>. <TT>srcChars</TT> is not modified.
   * @param start the offset at which the insertion begins
   * @param srcChars the source for the new characters
   * @param srcStart the offset into <TT>srcChars</TT> where new characters 
   * will be obtained
   * @param srcLength the number of characters in <TT>srcChars</TT> 
   * in the insert string
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& insert(int32_t start, 
            const UChar *srcChars, 
            int32_t srcStart, 
            int32_t srcLength);

  /**
   * Insert the characters in <TT>srcChars</TT> into the UnicodeString object 
   * at offset <TT>start</TT>. <TT>srcChars</TT> is not modified.
   * @param start the offset where the insertion begins
   * @param srcChars the source for the new characters
   * @param srcLength the number of Unicode characters in srcChars.
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& insert(int32_t start, 
            const UChar *srcChars,
            int32_t srcLength);

  /**
   * Insert the code unit <TT>srcChar</TT> into the UnicodeString object at 
   * offset <TT>start</TT>.
   * @param start the offset at which the insertion occurs
   * @param srcChar the code unit to insert
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& insert(int32_t start, 
            UChar srcChar);

  /**
   * Insert the code point <TT>srcChar</TT> into the UnicodeString object at 
   * offset <TT>start</TT>.
   * @param start the offset at which the insertion occurs
   * @param srcChar the code point to insert
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& insert(int32_t start, 
            UChar32 srcChar);


  /* Replace operations */

  /**
   * Replace the characters in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) with the characters in 
   * <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>). 
   * <TT>srcText</TT> is not modified.
   * @param start the offset at which the replace operation begins
   * @param length the number of characters to replace. The character at 
   * <TT>start + length</TT> is not modified.
   * @param srcText the source for the new characters
   * @param srcStart the offset into <TT>srcText</TT> where new characters 
   * will be obtained
   * @param srcLength the number of characters in <TT>srcText</TT> in 
   * the replace string
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString& replace(int32_t start, 
             int32_t length, 
             const UnicodeString& srcText, 
             int32_t srcStart, 
             int32_t srcLength);

  /**
   * Replace the characters in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) 
   * with the characters in <TT>srcText</TT>.  <TT>srcText</TT> is
   *  not modified.
   * @param start the offset at which the replace operation begins
   * @param length the number of characters to replace. The character at
   * <TT>start + length</TT> is not modified.
   * @param srcText the source for the new characters
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString& replace(int32_t start, 
             int32_t length, 
             const UnicodeString& srcText);

  /**
   * Replace the characters in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) with the characters in 
   * <TT>srcChars</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcStart + srcLength</TT>). <TT>srcChars</TT> 
   * is not modified.
   * @param start the offset at which the replace operation begins
   * @param length the number of characters to replace.  The character at
   * <TT>start + length</TT> is not modified.
   * @param srcChars the source for the new characters
   * @param srcStart the offset into <TT>srcChars</TT> where new characters
   * will be obtained
   * @param srcLength the number of characters in <TT>srcChars</TT> 
   * in the replace string
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString& replace(int32_t start, 
             int32_t length, 
             const UChar *srcChars, 
             int32_t srcStart, 
             int32_t srcLength);

  /**
   * Replace the characters in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) with the characters in
   * <TT>srcChars</TT>.  <TT>srcChars</TT> is not modified.
   * @param start the offset at which the replace operation begins
   * @param length number of characters to replace.  The character at
   * <TT>start + length</TT> is not modified.
   * @param srcChars the source for the new characters
   * @param srcLength the number of Unicode characters in srcChars
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& replace(int32_t start, 
             int32_t length, 
             const UChar *srcChars,
             int32_t srcLength);

  /**
   * Replace the characters in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) with the code unit
   * <TT>srcChar</TT>.
   * @param start the offset at which the replace operation begins
   * @param length the number of characters to replace.  The character at
   * <TT>start + length</TT> is not modified.
   * @param srcChar the new code unit
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& replace(int32_t start, 
             int32_t length, 
             UChar srcChar);

  /**
   * Replace the characters in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) with the code point
   * <TT>srcChar</TT>.
   * @param start the offset at which the replace operation begins
   * @param length the number of characters to replace.  The character at
   * <TT>start + length</TT> is not modified.
   * @param srcChar the new code point
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& replace(int32_t start, 
             int32_t length, 
             UChar32 srcChar);

  /**
   * Replace the characters in the range [<TT>start</TT>, <TT>limit</TT>) 
   * with the characters in <TT>srcText</TT>. <TT>srcText</TT> is not modified.
   * @param start the offset at which the replace operation begins
   * @param limit the offset immediately following the replace range
   * @param srcText the source for the new characters
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& replaceBetween(int32_t start, 
                int32_t limit, 
                const UnicodeString& srcText);

  /**
   * Replace the characters in the range [<TT>start</TT>, <TT>limit</TT>) 
   * with the characters in <TT>srcText</TT> in the range 
   * [<TT>srcStart</TT>, <TT>srcLimit</TT>). <TT>srcText</TT> is not modified.
   * @param start the offset at which the replace operation begins
   * @param limit the offset immediately following the replace range
   * @param srcText the source for the new characters
   * @param srcStart the offset into <TT>srcChars</TT> where new characters 
   * will be obtained
   * @param srcLimit the offset immediately following the range to copy 
   * in <TT>srcText</TT>
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& replaceBetween(int32_t start, 
                int32_t limit, 
                const UnicodeString& srcText, 
                int32_t srcStart, 
                int32_t srcLimit);

  /**
   * Replace a substring of this object with the given text.
   * @param start the beginning index, inclusive; <code>0 <= start
   * <= limit</code>.
   * @param limit the ending index, exclusive; <code>start <= limit
   * <= length()</code>.
   * @param text the text to replace characters <code>start</code>
   * to <code>limit - 1</code>
   * @stable ICU 2.0
   */
  virtual void handleReplaceBetween(int32_t start,
                                    int32_t limit,
                                    const UnicodeString& text);

  /**
   * Replaceable API
   * @return TRUE if it has MetaData
   * @draft ICU 2.4
   */
  virtual UBool hasMetaData() const;
  
  /**
   * Copy a substring of this object, retaining attribute (out-of-band)
   * information.  This method is used to duplicate or reorder substrings.
   * The destination index must not overlap the source range.
   * 
   * @param start the beginning index, inclusive; <code>0 <= start <=
   * limit</code>.
   * @param limit the ending index, exclusive; <code>start <= limit <=
   * length()</code>.
   * @param dest the destination index.  The characters from
   * <code>start..limit-1</code> will be copied to <code>dest</code>.
   * Implementations of this method may assume that <code>dest <= start ||
   * dest >= limit</code>.
   * @stable ICU 2.0
   */
  virtual void copy(int32_t start, int32_t limit, int32_t dest);

  /* Search and replace operations */

  /**
   * Replace all occurrences of characters in oldText with the characters 
   * in newText
   * @param oldText the text containing the search text
   * @param newText the text containing the replacement text
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& findAndReplace(const UnicodeString& oldText,
                const UnicodeString& newText);

  /**
   * Replace all occurrences of characters in oldText with characters 
   * in newText
   * in the range [<TT>start</TT>, <TT>start + length</TT>).
   * @param start the start of the range in which replace will performed
   * @param length the length of the range in which replace will be performed
   * @param oldText the text containing the search text
   * @param newText the text containing the replacement text
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& findAndReplace(int32_t start,
                int32_t length,
                const UnicodeString& oldText,
                const UnicodeString& newText);

  /**
   * Replace all occurrences of characters in oldText in the range 
   * [<TT>oldStart</TT>, <TT>oldStart + oldLength</TT>) with the characters 
   * in newText in the range 
   * [<TT>newStart</TT>, <TT>newStart + newLength</TT>) 
   * in the range [<TT>start</TT>, <TT>start + length</TT>).
   * @param start the start of the range in which replace will performed
   * @param length the length of the range in which replace will be performed
   * @param oldText the text containing the search text
   * @param oldStart the start of the search range in <TT>oldText</TT>
   * @param oldLength the length of the search range in <TT>oldText</TT>
   * @param newText the text containing the replacement text
   * @param newStart the start of the replacement range in <TT>newText</TT>
   * @param newLength the length of the replacement range in <TT>newText</TT>
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString& findAndReplace(int32_t start,
                int32_t length,
                const UnicodeString& oldText,
                int32_t oldStart,
                int32_t oldLength,
                const UnicodeString& newText,
                int32_t newStart,
                int32_t newLength);


  /* Remove operations */

  /**
   * Remove all characters from the UnicodeString object.
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& remove(void);

  /**
   * Remove the characters in the range 
   * [<TT>start</TT>, <TT>start + length</TT>) from the UnicodeString object.
   * @param start the offset of the first character to remove
   * @param length the number of characters to remove
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& remove(int32_t start, 
                               int32_t length = (int32_t)INT32_MAX);

  /**
   * Remove the characters in the range 
   * [<TT>start</TT>, <TT>limit</TT>) from the UnicodeString object.
   * @param start the offset of the first character to remove
   * @param limit the offset immediately following the range to remove
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& removeBetween(int32_t start,
                                      int32_t limit = (int32_t)INT32_MAX);


  /* Length operations */

  /**
   * Pad the start of this UnicodeString with the character <TT>padChar</TT>.  
   * If the length of this UnicodeString is less than targetLength, 
   * length() - targetLength copies of padChar will be added to the
   * beginning of this UnicodeString.
   * @param targetLength the desired length of the string
   * @param padChar the character to use for padding. Defaults to 
   * space (U+0020)
   * @return TRUE if the text was padded, FALSE otherwise.
   * @stable ICU 2.0
   */
  UBool padLeading(int32_t targetLength,
                    UChar padChar = 0x0020);

  /**
   * Pad the end of this UnicodeString with the character <TT>padChar</TT>.  
   * If the length of this UnicodeString is less than targetLength, 
   * length() - targetLength copies of padChar will be added to the
   * end of this UnicodeString.
   * @param targetLength the desired length of the string
   * @param padChar the character to use for padding. Defaults to 
   * space (U+0020)
   * @return TRUE if the text was padded, FALSE otherwise.
   * @stable ICU 2.0
   */
  UBool padTrailing(int32_t targetLength,
                     UChar padChar = 0x0020);

  /**
   * Truncate this UnicodeString to the <TT>targetLength</TT>.
   * @param targetLength the desired length of this UnicodeString.
   * @return TRUE if the text was truncated, FALSE otherwise
   * @stable ICU 2.0
   */
  inline UBool truncate(int32_t targetLength);

  /**
   * Trims leading and trailing whitespace from this UnicodeString.
   * @return a reference to this
   * @stable ICU 2.0
   */
  UnicodeString& trim(void);


  /* Miscellaneous operations */

  /**
   * Reverse this UnicodeString in place.
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& reverse(void);

  /**
   * Reverse the range [<TT>start</TT>, <TT>start + length</TT>) in
   * this UnicodeString.
   * @param start the start of the range to reverse
   * @param length the number of characters to to reverse
   * @return a reference to this
   * @stable ICU 2.0
   */
  inline UnicodeString& reverse(int32_t start,
             int32_t length);

  /**
   * Convert the characters in this to UPPER CASE following the conventions of
   * the default locale.
   * @return A reference to this.
   * @stable ICU 2.0
   */
  UnicodeString& toUpper(void);

  /**
   * Convert the characters in this to UPPER CASE following the conventions of
   * a specific locale.
   * @param locale The locale containing the conventions to use.
   * @return A reference to this.
   * @stable ICU 2.0
   */
  UnicodeString& toUpper(const Locale& locale);

  /**
   * Convert the characters in this to lower case following the conventions of
   * the default locale.
   * @return A reference to this.
   * @stable ICU 2.0
   */
  UnicodeString& toLower(void);

  /**
   * Convert the characters in this to lower case following the conventions of
   * a specific locale.
   * @param locale The locale containing the conventions to use.
   * @return A reference to this.
   * @stable ICU 2.0
   */
  UnicodeString& toLower(const Locale& locale);

#if !UCONFIG_NO_BREAK_ITERATION

  /**
   * Titlecase this string, convenience function using the default locale.
   *
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
   * @param titleIter A break iterator to find the first characters of words
   *                  that are to be titlecased.
   *                  If none is provided (0), then a standard titlecase
   *                  break iterator is opened.
   * @return A reference to this.
   * @draft ICU 2.1
   */
  UnicodeString &toTitle(BreakIterator *titleIter);

  /**
   * Titlecase this string.
   *
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
   * @param titleIter A break iterator to find the first characters of words
   *                  that are to be titlecased.
   *                  If none is provided (0), then a standard titlecase
   *                  break iterator is opened.
   * @param locale    The locale to consider.
   * @return A reference to this.
   * @draft ICU 2.1
   */
  UnicodeString &toTitle(BreakIterator *titleIter, const Locale &locale);

#endif

  /**
   * Case-fold the characters in this string.
   * Case-folding is locale-independent and not context-sensitive,
   * but there is an option for whether to include or exclude mappings for dotted I
   * and dotless i that are marked with 'I' in CaseFolding.txt.
   * The result may be longer or shorter than the original.
   *
   * @param options Either U_FOLD_CASE_DEFAULT or U_FOLD_CASE_EXCLUDE_SPECIAL_I
   * @return A reference to this.
   * @stable ICU 2.0
   */
  UnicodeString &foldCase(uint32_t options=0 /*U_FOLD_CASE_DEFAULT*/);

  //========================================
  // Access to the internal buffer
  //========================================

  /**
   * Get a read/write pointer to the internal buffer.
   * The buffer is guaranteed to be large enough for at least minCapacity UChars,
   * writable, and is still owned by the UnicodeString object.
   * Calls to getBuffer(minCapacity) must not be nested, and
   * must be matched with calls to releaseBuffer(newLength).
   * If the string buffer was read-only or shared,
   * then it will be reallocated and copied.
   *
   * An attempted nested call will return 0, and will not further modify the
   * state of the UnicodeString object.
   * It also returns 0 if the string is bogus.
   *
   * The actual capacity of the string buffer may be larger than minCapacity.
   * getCapacity() returns the actual capacity.
   * For many operations, the full capacity should be used to avoid reallocations.
   *
   * While the buffer is "open" between getBuffer(minCapacity)
   * and releaseBuffer(newLength), the following applies:
   * - The string length is set to 0.
   * - Any read API call on the UnicodeString object will behave like on a 0-length string.
   * - Any write API call on the UnicodeString object is disallowed and will have no effect.
   * - You can read from and write to the returned buffer.
   * - The previous string contents will still be in the buffer;
   *   if you want to use it, then you need to call length() before getBuffer(minCapacity).
   *   If the length() was greater than minCapacity, then any contents after minCapacity
   *   may be lost.
   *   The buffer contents is not NUL-terminated by getBuffer().
   *   If length()<getCapacity() then you can terminate it by writing a NUL
   *   at index length().
   * - You must call releaseBuffer(newLength) before and in order to
   *   return to normal UnicodeString operation.
   *
   * @param minCapacity the minimum number of UChars that are to be available
   *        in the buffer, starting at the returned pointer;
   *        default to the current string capacity if minCapacity==-1
   * @return a writable pointer to the internal string buffer,
   *         or 0 if an error occurs (nested calls, out of memory)
   *
   * @see releaseBuffer
   * @see getTerminatedBuffer()
   * @stable ICU 2.0
   */
  UChar *getBuffer(int32_t minCapacity);

  /**
   * Release a read/write buffer on a UnicodeString object with an
   * "open" getBuffer(minCapacity).
   * This function must be called in a matched pair with getBuffer(minCapacity).
   * releaseBuffer(newLength) must be called if and only if a getBuffer(minCapacity) is "open".
   *
   * It will set the string length to newLength, at most to the current capacity.
   * If newLength==-1 then it will set the length according to the
   * first NUL in the buffer, or to the capacity if there is no NUL.
   *
   * After calling releaseBuffer(newLength) the UnicodeString is back to normal operation.
   *
   * @param newLength the new length of the UnicodeString object;
   *        defaults to the current capacity if newLength is greater than that;
   *        if newLength==-1, it defaults to u_strlen(buffer) but not more than
   *        the current capacity of the string
   *
   * @see getBuffer(int32_t minCapacity)
   * @stable ICU 2.0
   */
  void releaseBuffer(int32_t newLength=-1);

  /**
   * Get a read-only pointer to the internal buffer.
   * This can be called at any time on a valid UnicodeString.
   *
   * It returns 0 if the string is bogus, or
   * during an "open" getBuffer(minCapacity).
   *
   * It can be called as many times as desired.
   * The pointer that it returns will remain valid until the UnicodeString object is modified,
   * at which time the pointer is semantically invalidated and must not be used any more.
   *
   * The capacity of the buffer can be determined with getCapacity().
   * The part after length() may or may not be initialized and valid,
   * depending on the history of the UnicodeString object.
   *
   * The buffer contents is (probably) not NUL-terminated.
   * You can check if it is with
   * <code>(s.length()<s.getCapacity() && buffer[s.length()]==0)</code>.
   * (See getTerminatedBuffer().)
   *
   * The buffer may reside in read-only memory. Its contents must not
   * be modified.
   *
   * @return a read-only pointer to the internal string buffer,
   *         or 0 if the string is empty or bogus
   *
   * @see getBuffer(int32_t minCapacity)
   * @see getTerminatedBuffer()
   * @stable ICU 2.0
   */
  inline const UChar *getBuffer() const;

  /**
   * Get a read-only pointer to the internal buffer,
   * making sure that it is NUL-terminated.
   * This can be called at any time on a valid UnicodeString.
   *
   * It returns 0 if the string is bogus, or
   * during an "open" getBuffer(minCapacity), or if the buffer cannot
   * be NUL-terminated (because memory allocation failed).
   *
   * It can be called as many times as desired.
   * The pointer that it returns will remain valid until the UnicodeString object is modified,
   * at which time the pointer is semantically invalidated and must not be used any more.
   *
   * The capacity of the buffer can be determined with getCapacity().
   * The part after length()+1 may or may not be initialized and valid,
   * depending on the history of the UnicodeString object.
   *
   * The buffer contents is guaranteed to be NUL-terminated.
   * getTerminatedBuffer() may reallocate the buffer if a terminating NUL
   * is written.
   * For this reason, this function is not const, unlike getBuffer().
   * Note that a UnicodeString may also contain NUL characters as part of its contents.
   *
   * The buffer may reside in read-only memory. Its contents must not
   * be modified.
   *
   * @return a read-only pointer to the internal string buffer,
   *         or 0 if the string is empty or bogus
   *
   * @see getBuffer(int32_t minCapacity)
   * @see getBuffer()
   * @draft ICU 2.2
   */
  inline const UChar *getTerminatedBuffer();

  //========================================
  // Constructors
  //========================================

  /** Construct an empty UnicodeString.  
   * @stable ICU 2.0
   */
  UnicodeString();

  /**
   * Construct a UnicodeString with capacity to hold <TT>capacity</TT> UChars
   * @param capacity the number of UChars this UnicodeString should hold
   * before a resize is necessary; if count is greater than 0 and count
   * code points c take up more space than capacity, then capacity is adjusted
   * accordingly.
   * @param c is used to initially fill the string
   * @param count specifies how many code points c are to be written in the
   *              string
   * @stable ICU 2.0
   */
  UnicodeString(int32_t capacity, UChar32 c, int32_t count);

  /**
   * Single UChar (code unit) constructor.
   * @param ch the character to place in the UnicodeString
   * @stable ICU 2.0
   */
  UnicodeString(UChar ch);

  /**
   * Single UChar32 (code point) constructor.
   * @param ch the character to place in the UnicodeString
   * @stable ICU 2.0
   */
  UnicodeString(UChar32 ch);

  /**
   * UChar* constructor.
   * @param text The characters to place in the UnicodeString.  <TT>text</TT>
   * must be NULL (U+0000) terminated.
   * @stable ICU 2.0
   */
  UnicodeString(const UChar *text);

  /**
   * UChar* constructor.
   * @param text The characters to place in the UnicodeString.
   * @param textLength The number of Unicode characters in <TT>text</TT>
   * to copy.
   * @stable ICU 2.0
   */
  UnicodeString(const UChar *text,
        int32_t textLength);

  /**
   * Readonly-aliasing UChar* constructor.
   * The text will be used for the UnicodeString object, but
   * it will not be released when the UnicodeString is destroyed.
   * This has copy-on-write semantics:
   * When the string is modified, then the buffer is first copied into
   * newly allocated memory.
   * The aliased buffer is never modified.
   * In an assignment to another UnicodeString, the text will be aliased again,
   * so that both strings then alias the same readonly-text.
   *
   * @param isTerminated specifies if <code>text</code> is <code>NUL</code>-terminated.
   *                     This must be true if <code>textLength==-1</code>.
   * @param text The characters to alias for the UnicodeString.
   * @param textLength The number of Unicode characters in <code>text</code> to alias.
   *                   If -1, then this constructor will determine the length
   *                   by calling <code>u_strlen()</code>.
   * @stable ICU 2.0
   */
  UnicodeString(UBool isTerminated,
                const UChar *text,
                int32_t textLength);

  /**
   * Writable-aliasing UChar* constructor.
   * The text will be used for the UnicodeString object, but
   * it will not be released when the UnicodeString is destroyed.
   * This has write-through semantics:
   * For as long as the capacity of the buffer is sufficient, write operations
   * will directly affect the buffer. When more capacity is necessary, then
   * a new buffer will be allocated and the contents copied as with regularly
   * constructed strings.
   * In an assignment to another UnicodeString, the buffer will be copied.
   * The extract(UChar *dst) function detects whether the dst pointer is the same
   * as the string buffer itself and will in this case not copy the contents.
   *
   * @param buffer The characters to alias for the UnicodeString.
   * @param buffLength The number of Unicode characters in <code>buffer</code> to alias.
   * @param buffCapacity The size of <code>buffer</code> in UChars.
   * @stable ICU 2.0
   */
  UnicodeString(UChar *buffer, int32_t buffLength, int32_t buffCapacity);

  /**
   * char* constructor.
   * @param codepageData an array of bytes, null-terminated
   * @param codepage the encoding of <TT>codepageData</TT>.  The special
   * value 0 for <TT>codepage</TT> indicates that the text is in the 
   * platform's default codepage.
   * If <code>codepage</code> is an empty string (<code>""</code>),
   * then a simple conversion is performed on the codepage-invariant
   * subset ("invariant characters") of the platform encoding. See utypes.h.
   * @stable ICU 2.0
   */
  UnicodeString(const char *codepageData,
        const char *codepage = 0);

  /**
   * char* constructor.
   * @param codepageData an array of bytes.
   * @param dataLength The number of bytes in <TT>codepageData</TT>.
   * @param codepage the encoding of <TT>codepageData</TT>.  The special
   * value 0 for <TT>codepage</TT> indicates that the text is in the 
   * platform's default codepage.
   * If <code>codepage</code> is an empty string (<code>""</code>),
   * then a simple conversion is performed on the codepage-invariant
   * subset ("invariant characters") of the platform encoding. See utypes.h.
   * @stable ICU 2.0
   */
  UnicodeString(const char *codepageData,
        int32_t dataLength,
        const char *codepage = 0);

  /**
   * char * / UConverter constructor.
   * This constructor uses an existing UConverter object to
   * convert the codepage string to Unicode and construct a UnicodeString
   * from that.
   *
   * The converter is reset at first.
   * If the error code indicates a failure before this constructor is called,
   * or if an error occurs during conversion or construction,
   * then the string will be bogus.
   *
   * This function avoids the overhead of opening and closing a converter if
   * multiple strings are constructed.
   *
   * @param src input codepage string
   * @param srcLength length of the input string, can be -1 for NUL-terminated strings
   * @param cnv converter object (ucnv_resetToUnicode() will be called),
   *        can be NULL for the default converter
   * @param errorCode normal ICU error code
   * @stable ICU 2.0
   */
  UnicodeString(
        const char *src, int32_t srcLength,
        UConverter *cnv,
        UErrorCode &errorCode);


  /**
   * Copy constructor.
   * @param that The UnicodeString object to copy.
   * @stable ICU 2.0
   */
  UnicodeString(const UnicodeString& that);

  /**
   * 'Substring' constructor from tail of source string.
   * @param src The UnicodeString object to copy.
   * @param srcStart The offset into <tt>src</tt> at which to start copying.
   * @draft ICU2.2
   */
  UnicodeString(const UnicodeString& src, int32_t srcStart);

  /**
   * 'Substring' constructor from subrange of source string.
   * @param src The UnicodeString object to copy.
   * @param srcStart The offset into <tt>src</tt> at which to start copying.
   * @param srcLength The number of characters from <tt>src</tt> to copy.
   * @draft ICU2.2
   */
  UnicodeString(const UnicodeString& src, int32_t srcStart, int32_t srcLength);

  /** Destructor. 
   * @stable ICU 2.0
  */
  ~UnicodeString();


  /* Miscellaneous operations */

  /**
   * Returns the number of display cells occupied by the range
   * [<TT>start</TT>, <TT>length</TT>).
   * This function is designed for Asian text and properly takes into account
   * halfwidth and fullwidth variants of various CJK characters and the 
   * combining behavior of the Hangul Jamo characters (with some limitations;
   * see documentation for Unicode::getCellWidth()).<BR>
   * In order to avoid dealing with fractions, this function can either be
   * construed to return twice the actual number of display cells or to 
   * treat a "cell" as the width of a halfwidth character rather than the
   * width of a fullwidth character.
   * @param start the start of the range
   * @param length the number of characters to measure
   * @param asian The <TT>asian</TT> parameter controls whether characters
   * considered NEUTRAL by the Unicode class are treated as halfwidth or 
   * fullwidth here.  If you set <TT>asian</TT> to FALSE, neutrals are 
   * treated as halfwidth, and this function returns a close approximation
   * of how many Latin display cells the text will take up in a monospaced
   * font.
   * @return the number of display cells occupied by the specified substring.
   * @obsolete ICU 2.6. Use UCHAR_EAST_ASIAN_WIDTH instead since this API will be removed in that release.
   */
  int32_t numDisplayCells(int32_t start = 0,
              int32_t length = INT32_MAX,
              UBool asian = TRUE) const;

#ifdef U_USE_DEPRECATED_UCHAR_REFERENCE
  /**
   * Return a modifiable reference to a code unit of the string.
   * This is unsafe because copying the UCharReference can leave it with
   * a dangling pointer to this UnicodeString object.
   * It also causes inefficient code because in most cases, the r-value
   * operator[] const is intended to be used instead of creating
   * a UCharReference object.
   *
   * @param pos The index of the code unit to refer to.
   * @return A modifiable UCharReference to that code unit.
   * @obsolete ICU 2.4. Use charAt(), setCharAt(), and operator[] const instead since this API will be removed in that release.
   */
  UCharReference operator[] (int32_t pos);
#endif

  /**
   * Unescape a string of characters and return a string containing
   * the result.  The following escape sequences are recognized:
   *
   * \uhhhh       4 hex digits; h in [0-9A-Fa-f]
   * \Uhhhhhhhh   8 hex digits
   * \xhh         1-2 hex digits
   * \ooo         1-3 octal digits; o in [0-7]
   * \cX          control-X; X is masked with 0x1F
   *
   * as well as the standard ANSI C escapes:
   *
   * \a => U+0007, \b => U+0008, \t => U+0009, \n => U+000A,
   * \v => U+000B, \f => U+000C, \r => U+000D, \e => U+001B,
   * \" => U+0022, \' => U+0027, \? => U+003F, \\ => U+005C
   *
   * Anything else following a backslash is generically escaped.  For
   * example, "[a\-z]" returns "[a-z]".
   *
   * If an escape sequence is ill-formed, this method returns an empty
   * string.  An example of an ill-formed sequence is "\u" followed by
   * fewer than 4 hex digits.
   *
   * This function is similar to u_unescape() but not identical to it.
   * The latter takes a source char*, so it does escape recognition
   * and also invariant conversion.
   *
   * @return a string with backslash escapes interpreted, or an
   * empty string on error.
   * @see UnicodeString#unescapeAt()
   * @see u_unescape()
   * @see u_unescapeAt()
   * @stable ICU 2.0
   */
  UnicodeString unescape() const;

  /**
   * Unescape a single escape sequence and return the represented
   * character.  See unescape() for a listing of the recognized escape
   * sequences.  The character at offset-1 is assumed (without
   * checking) to be a backslash.  If the escape sequence is
   * ill-formed, or the offset is out of range, (UChar32)0xFFFFFFFF is
   * returned.
   *
   * @param offset an input output parameter.  On input, it is the
   * offset into this string where the escape sequence is located,
   * after the initial backslash.  On output, it is advanced after the
   * last character parsed.  On error, it is not advanced at all.
   * @return the character represented by the escape sequence at
   * offset, or (UChar32)0xFFFFFFFF on error.
   * @see UnicodeString#unescape()
   * @see u_unescape()
   * @see u_unescapeAt()
   * @stable ICU 2.0
   */
  UChar32 unescapeAt(int32_t &offset) const;

  /**
   * ICU "poor man's RTTI", returns a UClassID for the actual class.
   *
   * @draft ICU 2.2
   */
  virtual inline UClassID getDynamicClassID() const;

  /**
   * ICU "poor man's RTTI", returns a UClassID for this class.
   *
   * @draft ICU 2.2
   */
  static inline UClassID getStaticClassID();

  //========================================
  // Implementation methods
  //========================================

protected:
  /**
   * Implement Replaceable::getLength() (see jitterbug 1027).
   * @draft ICU 2.4
   */
  virtual int32_t getLength() const;

  /**
   * The change in Replaceable to use virtual getCharAt() allows
   * UnicodeString::charAt() to be inline again (see jitterbug 709).
   * @draft ICU 2.4
   */
  virtual UChar getCharAt(int32_t offset) const;

  /**
   * The change in Replaceable to use virtual getChar32At() allows
   * UnicodeString::char32At() to be inline again (see jitterbug 709).
   * @draft ICU 2.4
   */
  virtual UChar32 getChar32At(int32_t offset) const;

private:

  inline int8_t
  doCompare(int32_t start,
           int32_t length,
           const UnicodeString& srcText,
           int32_t srcStart,
           int32_t srcLength) const;

  int8_t doCompare(int32_t start,
           int32_t length,
           const UChar *srcChars,
           int32_t srcStart,
           int32_t srcLength) const;

  inline int8_t
  doCompareCodePointOrder(int32_t start,
                          int32_t length,
                          const UnicodeString& srcText,
                          int32_t srcStart,
                          int32_t srcLength) const;

  int8_t doCompareCodePointOrder(int32_t start,
                                 int32_t length,
                                 const UChar *srcChars,
                                 int32_t srcStart,
                                 int32_t srcLength) const;

  inline int8_t
  doCaseCompare(int32_t start,
                int32_t length,
                const UnicodeString &srcText,
                int32_t srcStart,
                int32_t srcLength,
                uint32_t options) const;

  int8_t
  doCaseCompare(int32_t start,
                int32_t length,
                const UChar *srcChars,
                int32_t srcStart,
                int32_t srcLength,
                uint32_t options) const;

  int32_t doIndexOf(UChar c,
            int32_t start,
            int32_t length) const;

  int32_t doIndexOf(UChar32 c,
                        int32_t start,
                        int32_t length) const;

  int32_t doLastIndexOf(UChar c,
                int32_t start,
                int32_t length) const;

  int32_t doLastIndexOf(UChar32 c,
                            int32_t start,
                            int32_t length) const;

  void doExtract(int32_t start, 
         int32_t length, 
         UChar *dst, 
         int32_t dstStart) const;

  inline void doExtract(int32_t start,
         int32_t length,
         UnicodeString& target) const;

  inline UChar doCharAt(int32_t offset)  const;

  UnicodeString& doReplace(int32_t start, 
               int32_t length, 
               const UnicodeString& srcText, 
               int32_t srcStart, 
               int32_t srcLength);

  UnicodeString& doReplace(int32_t start, 
               int32_t length, 
               const UChar *srcChars, 
               int32_t srcStart, 
               int32_t srcLength);

  UnicodeString& doReverse(int32_t start,
               int32_t length);

  // calculate hash code
  int32_t doHashCode(void) const;

  // get pointer to start of array
  inline UChar* getArrayStart(void);
  inline const UChar* getArrayStart(void) const;

  // allocate the array; result may be fStackBuffer
  // sets refCount to 1 if appropriate
  // sets fArray, fCapacity, and fFlags
  // returns boolean for success or failure
  UBool allocate(int32_t capacity);

  // release the array if owned
  void releaseArray(void);

  // turn a bogus string into an empty one
  void unBogus();

  // implements assigment operator, copy constructor, and fastCopyFrom()
  UnicodeString &copyFrom(const UnicodeString &src, UBool fastCopy=FALSE);

  // Pin start and limit to acceptable values.
  inline void pinIndex(int32_t& start) const;
  inline void pinIndices(int32_t& start,
                         int32_t& length) const;

  /* Internal extract() using UConverter. */
  int32_t doExtract(int32_t start, int32_t length,
                    char *dest, int32_t destCapacity,
                    UConverter *cnv,
                    UErrorCode &errorCode) const;

  /*
   * Real constructor for converting from codepage data.
   * It assumes that it is called with !fRefCounted.
   *
   * If <code>codepage==0</code>, then the default converter
   * is used for the platform encoding.
   * If <code>codepage</code> is an empty string (<code>""</code>),
   * then a simple conversion is performed on the codepage-invariant
   * subset ("invariant characters") of the platform encoding. See utypes.h.
   */
  void doCodepageCreate(const char *codepageData,
                        int32_t dataLength,
                        const char *codepage);

  /*
   * Worker function for creating a UnicodeString from
   * a codepage string using a UConverter.
   */
  void
  doCodepageCreate(const char *codepageData,
                   int32_t dataLength,
                   UConverter *converter,
                   UErrorCode &status);
  /*
   * This function is called when write access to the array
   * is necessary.
   *
   * We need to make a copy of the array if
   * the buffer is read-only, or
   * the buffer is refCounted (shared), and refCount>1, or
   * the buffer is too small.
   *
   * Return FALSE if memory could not be allocated.
   */
  UBool cloneArrayIfNeeded(int32_t newCapacity = -1,
                            int32_t growCapacity = -1,
                            UBool doCopyArray = TRUE,
                            int32_t **pBufferToDelete = 0,
                            UBool forceClone = FALSE);

  // common function for case mappings
  UnicodeString &
  caseMap(BreakIterator *titleIter,
          const Locale& locale,
          uint32_t options,
          int32_t toWhichCase);

  // ref counting
  void addRef(void);
  int32_t removeRef(void);
  int32_t refCount(void) const;

  // constants
  enum {
    US_STACKBUF_SIZE=7, // Size of stack buffer for small strings
    kInvalidUChar=0xffff, // invalid UChar index
    kGrowSize=128, // grow size for this buffer
    kInvalidHashCode=0, // invalid hash code
    kEmptyHashCode=1, // hash code for empty string

    // bit flag values for fFlags
    kIsBogus=1,         // this string is bogus, i.e., not valid or NULL
    kUsingStackBuffer=2,// fArray==fStackBuffer
    kRefCounted=4,      // there is a refCount field before the characters in fArray
    kBufferIsReadonly=8,// do not write to this buffer
    kOpenGetBuffer=16,  // getBuffer(minCapacity) was called (is "open"),
                        // and releaseBuffer(newLength) must be called

    // combined values for convenience
    kShortString=kUsingStackBuffer,
    kLongString=kRefCounted,
    kReadonlyAlias=kBufferIsReadonly,
    kWritableAlias=0
  };

  friend class StringCharacterIterator;
  friend class StringThreadTest;

  /*
   * The following are all the class fields that are stored
   * in each UnicodeString object.
   * Note that UnicodeString has virtual functions,
   * therefore there is an implicit vtable pointer
   * as the first real field.
   * The fields should be aligned such that no padding is
   * necessary, mostly by having larger types first.
   * On 32-bit machines, the size should be 32 bytes,
   * on 64-bit machines (8-byte pointers), it should be 40 bytes.
   */
  // (implicit) *vtable;
  int32_t   fLength;        // number of characters in fArray
  int32_t   fCapacity;      // sizeof fArray
  UChar     *fArray;        // the Unicode data
  uint16_t  fFlags;         // bit flags: see constants above
  UChar     fStackBuffer [ US_STACKBUF_SIZE ]; // buffer for small strings

  /**
   * The address of this static class variable serves as this class's ID
   * for ICU "poor man's RTTI".
   */
  static const char fgClassID;
};

U_NAMESPACE_END

//========================================
// Array copying
//========================================
/**
 * Copy an array of UnicodeString OBJECTS (not pointers).
 * @internal
 */
inline void 
uprv_arrayCopy(const U_NAMESPACE_QUALIFIER UnicodeString *src, U_NAMESPACE_QUALIFIER UnicodeString *dst, int32_t count)
{ while(count-- > 0) *dst++ = *src++; }

/**
 * Copy an array of UnicodeString OBJECTS (not pointers).
 * @internal
 */
inline void 
uprv_arrayCopy(const U_NAMESPACE_QUALIFIER UnicodeString *src, int32_t srcStart, 
        U_NAMESPACE_QUALIFIER UnicodeString *dst, int32_t dstStart, int32_t count)
{ uprv_arrayCopy(src+srcStart, dst+dstStart, count); }

U_NAMESPACE_BEGIN

//========================================
// Inline members
//========================================

//========================================
// Privates
//========================================

inline void
UnicodeString::pinIndex(int32_t& start) const
{
  // pin index
  if(start < 0) {
    start = 0;
  } else if(start > fLength) {
    start = fLength;
  }
}

inline void
UnicodeString::pinIndices(int32_t& start,
                          int32_t& _length) const
{
  // pin indices
  if(start < 0) {
    start = 0;
  } else if(start > fLength) {
    start = fLength;
  }
  if(_length < 0) {
    _length = 0;
  } else if(_length > (fLength - start)) {
    _length = (fLength - start);
  }
}

inline UChar* 
UnicodeString::getArrayStart()
{ return fArray; }

inline const UChar* 
UnicodeString::getArrayStart() const
{ return fArray; }

//========================================
// Read-only implementation methods
//========================================
inline UClassID
UnicodeString::getStaticClassID()
{ return (UClassID)&fgClassID; }

inline UClassID
UnicodeString::getDynamicClassID() const
{ return UnicodeString::getStaticClassID(); }

inline int32_t  
UnicodeString::length() const
{ return fLength; }

inline int32_t 
UnicodeString::getCapacity() const
{ return fCapacity; }

inline int32_t 
UnicodeString::hashCode() const
{ return doHashCode(); }

inline UBool 
UnicodeString::isBogus() const
{ return (UBool)(fFlags & kIsBogus); }

inline const UChar *
UnicodeString::getBuffer() const {
  if(!(fFlags&(kIsBogus|kOpenGetBuffer))) {
    return fArray;
  } else {
    return 0;
  }
}

//========================================
// Read-only alias methods
//========================================
inline int8_t
UnicodeString::doCompare(int32_t start,
              int32_t length,
              const UnicodeString& srcText,
              int32_t srcStart,
              int32_t srcLength) const
{
  if(srcText.isBogus()) {
    return (int8_t)!isBogus(); // 0 if both are bogus, 1 otherwise
  } else {
    srcText.pinIndices(srcStart, srcLength);
    return doCompare(start, length, srcText.fArray, srcStart, srcLength);
  }
}

inline UBool
UnicodeString::operator== (const UnicodeString& text) const
{
  if(isBogus()) {
    return text.isBogus();
  } else {
    return
      !text.isBogus() &&
      fLength == text.fLength &&
      doCompare(0, fLength, text, 0, text.fLength) == 0;
  }
}

inline UBool
UnicodeString::operator!= (const UnicodeString& text) const
{ return (! operator==(text)); }

inline UBool
UnicodeString::operator> (const UnicodeString& text) const
{ return doCompare(0, fLength, text, 0, text.fLength) == 1; }

inline UBool
UnicodeString::operator< (const UnicodeString& text) const
{ return doCompare(0, fLength, text, 0, text.fLength) == -1; }

inline UBool
UnicodeString::operator>= (const UnicodeString& text) const
{ return doCompare(0, fLength, text, 0, text.fLength) != -1; }

inline UBool
UnicodeString::operator<= (const UnicodeString& text) const
{ return doCompare(0, fLength, text, 0, text.fLength) != 1; }

inline int8_t 
UnicodeString::compare(const UnicodeString& text) const
{ return doCompare(0, fLength, text, 0, text.fLength); }

inline int8_t 
UnicodeString::compare(int32_t start,
               int32_t _length,
               const UnicodeString& srcText) const
{ return doCompare(start, _length, srcText, 0, srcText.fLength); }

inline int8_t 
UnicodeString::compare(const UChar *srcChars,
               int32_t srcLength) const
{ return doCompare(0, fLength, srcChars, 0, srcLength); }

inline int8_t 
UnicodeString::compare(int32_t start,
               int32_t _length,
               const UnicodeString& srcText,
               int32_t srcStart,
               int32_t srcLength) const
{ return doCompare(start, _length, srcText, srcStart, srcLength); }

inline int8_t
UnicodeString::compare(int32_t start,
               int32_t _length,
               const UChar *srcChars) const
{ return doCompare(start, _length, srcChars, 0, _length); }

inline int8_t 
UnicodeString::compare(int32_t start,
               int32_t _length,
               const UChar *srcChars,
               int32_t srcStart,
               int32_t srcLength) const
{ return doCompare(start, _length, srcChars, srcStart, srcLength); }

inline int8_t
UnicodeString::compareBetween(int32_t start,
                  int32_t limit,
                  const UnicodeString& srcText,
                  int32_t srcStart,
                  int32_t srcLimit) const
{ return doCompare(start, limit - start, 
           srcText, srcStart, srcLimit - srcStart); }

inline int8_t
UnicodeString::doCompareCodePointOrder(int32_t start,
                                       int32_t length,
                                       const UnicodeString& srcText,
                                       int32_t srcStart,
                                       int32_t srcLength) const
{
  if(srcText.isBogus()) {
    return (int8_t)!isBogus(); // 0 if both are bogus, 1 otherwise
  } else {
    srcText.pinIndices(srcStart, srcLength);
    return doCompareCodePointOrder(start, length, srcText.fArray, srcStart, srcLength);
  }
}

inline int8_t 
UnicodeString::compareCodePointOrder(const UnicodeString& text) const
{ return doCompareCodePointOrder(0, fLength, text, 0, text.fLength); }

inline int8_t 
UnicodeString::compareCodePointOrder(int32_t start,
                                     int32_t _length,
                                     const UnicodeString& srcText) const
{ return doCompareCodePointOrder(start, _length, srcText, 0, srcText.fLength); }

inline int8_t 
UnicodeString::compareCodePointOrder(const UChar *srcChars,
                                     int32_t srcLength) const
{ return doCompareCodePointOrder(0, fLength, srcChars, 0, srcLength); }

inline int8_t 
UnicodeString::compareCodePointOrder(int32_t start,
                                     int32_t _length,
                                     const UnicodeString& srcText,
                                     int32_t srcStart,
                                     int32_t srcLength) const
{ return doCompareCodePointOrder(start, _length, srcText, srcStart, srcLength); }

inline int8_t
UnicodeString::compareCodePointOrder(int32_t start,
                                     int32_t _length,
                                     const UChar *srcChars) const
{ return doCompareCodePointOrder(start, _length, srcChars, 0, _length); }

inline int8_t 
UnicodeString::compareCodePointOrder(int32_t start,
                                     int32_t _length,
                                     const UChar *srcChars,
                                     int32_t srcStart,
                                     int32_t srcLength) const
{ return doCompareCodePointOrder(start, _length, srcChars, srcStart, srcLength); }

inline int8_t
UnicodeString::compareCodePointOrderBetween(int32_t start,
                                            int32_t limit,
                                            const UnicodeString& srcText,
                                            int32_t srcStart,
                                            int32_t srcLimit) const
{ return doCompareCodePointOrder(start, limit - start, 
           srcText, srcStart, srcLimit - srcStart); }

inline int8_t
UnicodeString::doCaseCompare(int32_t start,
                             int32_t length,
                             const UnicodeString &srcText,
                             int32_t srcStart,
                             int32_t srcLength,
                             uint32_t options) const
{
  if(srcText.isBogus()) {
    return (int8_t)!isBogus(); // 0 if both are bogus, 1 otherwise
  } else {
    srcText.pinIndices(srcStart, srcLength);
    return doCaseCompare(start, length, srcText.fArray, srcStart, srcLength, options);
  }
}

inline int8_t 
UnicodeString::caseCompare(const UnicodeString &text, uint32_t options) const {
  return doCaseCompare(0, fLength, text, 0, text.fLength, options);
}

inline int8_t 
UnicodeString::caseCompare(int32_t start,
                           int32_t _length,
                           const UnicodeString &srcText,
                           uint32_t options) const {
  return doCaseCompare(start, _length, srcText, 0, srcText.fLength, options);
}

inline int8_t 
UnicodeString::caseCompare(const UChar *srcChars,
                           int32_t srcLength,
                           uint32_t options) const {
  return doCaseCompare(0, fLength, srcChars, 0, srcLength, options);
}

inline int8_t 
UnicodeString::caseCompare(int32_t start,
                           int32_t _length,
                           const UnicodeString &srcText,
                           int32_t srcStart,
                           int32_t srcLength,
                           uint32_t options) const {
  return doCaseCompare(start, _length, srcText, srcStart, srcLength, options);
}

inline int8_t
UnicodeString::caseCompare(int32_t start,
                           int32_t _length,
                           const UChar *srcChars,
                           uint32_t options) const {
  return doCaseCompare(start, _length, srcChars, 0, _length, options);
}

inline int8_t 
UnicodeString::caseCompare(int32_t start,
                           int32_t _length,
                           const UChar *srcChars,
                           int32_t srcStart,
                           int32_t srcLength,
                           uint32_t options) const {
  return doCaseCompare(start, _length, srcChars, srcStart, srcLength, options);
}

inline int8_t
UnicodeString::caseCompareBetween(int32_t start,
                                  int32_t limit,
                                  const UnicodeString &srcText,
                                  int32_t srcStart,
                                  int32_t srcLimit,
                                  uint32_t options) const {
  return doCaseCompare(start, limit - start, srcText, srcStart, srcLimit - srcStart, options);
}

inline int32_t 
UnicodeString::indexOf(const UnicodeString& srcText,
               int32_t srcStart,
               int32_t srcLength,
               int32_t start,
               int32_t _length) const
{
  if(!srcText.isBogus()) {
    srcText.pinIndices(srcStart, srcLength);
    if(srcLength > 0) {
      return indexOf(srcText.getArrayStart(), srcStart, srcLength, start, _length);
    }
  }
  return -1;
}

inline int32_t 
UnicodeString::indexOf(const UnicodeString& text) const
{ return indexOf(text, 0, text.fLength, 0, fLength); }

inline int32_t 
UnicodeString::indexOf(const UnicodeString& text,
               int32_t start) const {
  pinIndex(start);
  return indexOf(text, 0, text.fLength, start, fLength - start);
}

inline int32_t 
UnicodeString::indexOf(const UnicodeString& text,
               int32_t start,
               int32_t _length) const
{ return indexOf(text, 0, text.fLength, start, _length); }

inline int32_t 
UnicodeString::indexOf(const UChar *srcChars,
               int32_t srcLength,
               int32_t start) const {
  pinIndex(start);
  return indexOf(srcChars, 0, srcLength, start, fLength - start);
}

inline int32_t 
UnicodeString::indexOf(const UChar *srcChars,
               int32_t srcLength,
               int32_t start,
               int32_t _length) const
{ return indexOf(srcChars, 0, srcLength, start, _length); }

inline int32_t 
UnicodeString::indexOf(UChar c,
               int32_t start,
               int32_t _length) const
{ return doIndexOf(c, start, _length); }

inline int32_t 
UnicodeString::indexOf(UChar32 c,
               int32_t start,
               int32_t _length) const
{ return doIndexOf(c, start, _length); }

inline int32_t 
UnicodeString::indexOf(UChar c) const
{ return doIndexOf(c, 0, fLength); }

inline int32_t 
UnicodeString::indexOf(UChar32 c) const
{ return indexOf(c, 0, fLength); }

inline int32_t 
UnicodeString::indexOf(UChar c,
               int32_t start) const {
  pinIndex(start);
  return doIndexOf(c, start, fLength - start);
}

inline int32_t 
UnicodeString::indexOf(UChar32 c,
               int32_t start) const {
  pinIndex(start);
  return indexOf(c, start, fLength - start);
}

inline int32_t 
UnicodeString::lastIndexOf(const UChar *srcChars,
               int32_t srcLength,
               int32_t start,
               int32_t _length) const
{ return lastIndexOf(srcChars, 0, srcLength, start, _length); }

inline int32_t 
UnicodeString::lastIndexOf(const UChar *srcChars,
               int32_t srcLength,
               int32_t start) const {
  pinIndex(start);
  return lastIndexOf(srcChars, 0, srcLength, start, fLength - start);
}

inline int32_t 
UnicodeString::lastIndexOf(const UnicodeString& srcText,
               int32_t srcStart,
               int32_t srcLength,
               int32_t start,
               int32_t _length) const
{
  if(!srcText.isBogus()) {
    srcText.pinIndices(srcStart, srcLength);
    if(srcLength > 0) {
      return lastIndexOf(srcText.getArrayStart(), srcStart, srcLength, start, _length);
    }
  }
  return -1;
}

inline int32_t 
UnicodeString::lastIndexOf(const UnicodeString& text,
               int32_t start,
               int32_t _length) const
{ return lastIndexOf(text, 0, text.fLength, start, _length); }

inline int32_t 
UnicodeString::lastIndexOf(const UnicodeString& text,
               int32_t start) const {
  pinIndex(start);
  return lastIndexOf(text, 0, text.fLength, start, fLength - start);
}

inline int32_t 
UnicodeString::lastIndexOf(const UnicodeString& text) const
{ return lastIndexOf(text, 0, text.fLength, 0, fLength); }

inline int32_t 
UnicodeString::lastIndexOf(UChar c,
               int32_t start,
               int32_t _length) const
{ return doLastIndexOf(c, start, _length); }

inline int32_t 
UnicodeString::lastIndexOf(UChar32 c,
               int32_t start,
               int32_t _length) const {
  return doLastIndexOf(c, start, _length);
}

inline int32_t 
UnicodeString::lastIndexOf(UChar c) const
{ return doLastIndexOf(c, 0, fLength); }

inline int32_t 
UnicodeString::lastIndexOf(UChar32 c) const {
  return lastIndexOf(c, 0, fLength);
}

inline int32_t 
UnicodeString::lastIndexOf(UChar c,
               int32_t start) const {
  pinIndex(start);
  return doLastIndexOf(c, start, fLength - start);
}

inline int32_t 
UnicodeString::lastIndexOf(UChar32 c,
               int32_t start) const {
  pinIndex(start);
  return lastIndexOf(c, start, fLength - start);
}

inline UBool 
UnicodeString::startsWith(const UnicodeString& text) const
{ return compare(0, text.fLength, text, 0, text.fLength) == 0; }

inline UBool 
UnicodeString::startsWith(const UnicodeString& srcText,
              int32_t srcStart,
              int32_t srcLength) const
{ return doCompare(0, srcLength, srcText, srcStart, srcLength) == 0; }

inline UBool 
UnicodeString::startsWith(const UChar *srcChars,
              int32_t srcLength) const
{ return doCompare(0, srcLength, srcChars, 0, srcLength) == 0; }

inline UBool 
UnicodeString::startsWith(const UChar *srcChars,
              int32_t srcStart,
              int32_t srcLength) const
{ return doCompare(0, srcLength, srcChars, srcStart, srcLength) == 0;}

inline UBool 
UnicodeString::endsWith(const UnicodeString& text) const
{ return doCompare(fLength - text.fLength, text.fLength, 
           text, 0, text.fLength) == 0; }

inline UBool 
UnicodeString::endsWith(const UnicodeString& srcText,
            int32_t srcStart,
            int32_t srcLength) const {
  srcText.pinIndices(srcStart, srcLength);
  return doCompare(fLength - srcLength, srcLength, 
                   srcText, srcStart, srcLength) == 0;
}

inline UBool 
UnicodeString::endsWith(const UChar *srcChars,
            int32_t srcLength) const {
  if(srcLength < 0) {
    srcLength = u_strlen(srcChars);
  }
  return doCompare(fLength - srcLength, srcLength, 
                   srcChars, 0, srcLength) == 0;
}

inline UBool 
UnicodeString::endsWith(const UChar *srcChars,
            int32_t srcStart,
            int32_t srcLength) const {
  if(srcLength < 0) {
    srcLength = u_strlen(srcChars + srcStart);
  }
  return doCompare(fLength - srcLength, srcLength, 
                   srcChars, srcStart, srcLength) == 0;
}

//========================================
// replace
//========================================
inline UnicodeString& 
UnicodeString::replace(int32_t start, 
               int32_t _length, 
               const UnicodeString& srcText) 
{ return doReplace(start, _length, srcText, 0, srcText.fLength); }

inline UnicodeString& 
UnicodeString::replace(int32_t start, 
               int32_t _length, 
               const UnicodeString& srcText, 
               int32_t srcStart, 
               int32_t srcLength)
{ return doReplace(start, _length, srcText, srcStart, srcLength); }

inline UnicodeString& 
UnicodeString::replace(int32_t start, 
               int32_t _length, 
               const UChar *srcChars,
               int32_t srcLength)
{ return doReplace(start, _length, srcChars, 0, srcLength); }

inline UnicodeString& 
UnicodeString::replace(int32_t start, 
               int32_t _length, 
               const UChar *srcChars, 
               int32_t srcStart, 
               int32_t srcLength)
{ return doReplace(start, _length, srcChars, srcStart, srcLength); }

inline UnicodeString& 
UnicodeString::replace(int32_t start, 
               int32_t _length, 
               UChar srcChar)
{ return doReplace(start, _length, &srcChar, 0, 1); }

inline UnicodeString&
UnicodeString::replace(int32_t start, 
               int32_t _length, 
               UChar32 srcChar) {
  UChar buffer[U16_MAX_LENGTH];
  int32_t count = 0;
  UBool isError = FALSE;
  U16_APPEND(buffer, count, U16_MAX_LENGTH, srcChar, isError);
  return doReplace(start, _length, buffer, 0, count);
}

inline UnicodeString& 
UnicodeString::replaceBetween(int32_t start, 
                  int32_t limit, 
                  const UnicodeString& srcText)
{ return doReplace(start, limit - start, srcText, 0, srcText.fLength); }

inline UnicodeString&
UnicodeString::replaceBetween(int32_t start, 
                  int32_t limit, 
                  const UnicodeString& srcText, 
                  int32_t srcStart, 
                  int32_t srcLimit)
{ return doReplace(start, limit - start, srcText, srcStart, srcLimit - srcStart); }

inline UnicodeString& 
UnicodeString::findAndReplace(const UnicodeString& oldText,
                  const UnicodeString& newText)
{ return findAndReplace(0, fLength, oldText, 0, oldText.fLength, 
            newText, 0, newText.fLength); }

inline UnicodeString& 
UnicodeString::findAndReplace(int32_t start,
                  int32_t _length,
                  const UnicodeString& oldText,
                  const UnicodeString& newText)
{ return findAndReplace(start, _length, oldText, 0, oldText.fLength, 
            newText, 0, newText.fLength); }

// ============================
// extract
// ============================
inline void
UnicodeString::doExtract(int32_t start,
             int32_t _length,
             UnicodeString& target) const
{ target.replace(0, target.fLength, *this, start, _length); }

inline void  
UnicodeString::extract(int32_t start, 
               int32_t _length, 
               UChar *target, 
               int32_t targetStart) const
{ doExtract(start, _length, target, targetStart); }

inline void 
UnicodeString::extract(int32_t start,
               int32_t _length,
               UnicodeString& target) const
{ doExtract(start, _length, target); }

inline int32_t
UnicodeString::extract(int32_t start,
               int32_t _length,
               char *dst,
               const char *codepage) const

{
  // This dstSize value will be checked explicitly
  return extract(start, _length, dst, dst!=0 ? 0xffffffff : 0, codepage);
}

inline void  
UnicodeString::extractBetween(int32_t start, 
                  int32_t limit, 
                  UChar *dst, 
                  int32_t dstStart) const
{ doExtract(start, limit - start, dst, dstStart); }

inline UChar
UnicodeString::doCharAt(int32_t offset) const
{
  if((uint32_t)offset < (uint32_t)fLength) {
    return fArray[offset];
  } else {
    return kInvalidUChar;
  }
}

inline UChar
UnicodeString::charAt(int32_t offset) const
{ return doCharAt(offset); }

inline UChar
UnicodeString::operator[] (int32_t offset) const
{ return doCharAt(offset); }

inline UChar32
UnicodeString::char32At(int32_t offset) const
{
  if((uint32_t)offset < (uint32_t)fLength) {
    UChar32 c;
    U16_GET(fArray, 0, offset, fLength, c);
    return c;
  } else {
    return kInvalidUChar;
  }
}

inline int32_t
UnicodeString::getChar32Start(int32_t offset) const {
  if((uint32_t)offset < (uint32_t)fLength) {
    U16_SET_CP_START(fArray, 0, offset);
    return offset;
  } else {
    return 0;
  }
}

inline int32_t
UnicodeString::getChar32Limit(int32_t offset) const {
  if((uint32_t)offset < (uint32_t)fLength) {
    U16_SET_CP_LIMIT(fArray, 0, offset, fLength);
    return offset;
  } else {
    return fLength;
  }
}

inline UBool
UnicodeString::isEmpty() const {
  return fLength == 0;
}

//========================================
// Write implementation methods
//========================================
inline const UChar *
UnicodeString::getTerminatedBuffer() {
  if(fFlags&(kIsBogus|kOpenGetBuffer)) {
    return 0;
  } else if(fLength<fCapacity && fArray[fLength]==0) {
    return fArray;
  } else if(cloneArrayIfNeeded(fLength+1)) {
    fArray[fLength]=0;
    return fArray;
  } else {
    return 0;
  }
}

inline UnicodeString& 
UnicodeString::operator= (UChar ch) 
{ return doReplace(0, fLength, &ch, 0, 1); }

inline UnicodeString& 
UnicodeString::operator= (UChar32 ch) 
{ return replace(0, fLength, ch); }

inline UnicodeString& 
UnicodeString::setTo(const UnicodeString& srcText, 
             int32_t srcStart, 
             int32_t srcLength)
{
  unBogus();
  return doReplace(0, fLength, srcText, srcStart, srcLength);
}

inline UnicodeString& 
UnicodeString::setTo(const UnicodeString& srcText, 
             int32_t srcStart)
{
  unBogus();
  srcText.pinIndex(srcStart);
  return doReplace(0, fLength, srcText, srcStart, srcText.fLength - srcStart);
}

inline UnicodeString& 
UnicodeString::setTo(const UnicodeString& srcText)
{
  unBogus();
  return doReplace(0, fLength, srcText, 0, srcText.fLength);
}

inline UnicodeString& 
UnicodeString::setTo(const UChar *srcChars,
             int32_t srcLength)
{
  unBogus();
  return doReplace(0, fLength, srcChars, 0, srcLength);
}

inline UnicodeString& 
UnicodeString::setTo(UChar srcChar)
{
  unBogus();
  return doReplace(0, fLength, &srcChar, 0, 1);
}

inline UnicodeString& 
UnicodeString::setTo(UChar32 srcChar)
{
  unBogus();
  return replace(0, fLength, srcChar);
}

inline UnicodeString& 
UnicodeString::operator+= (UChar ch)
{ return doReplace(fLength, 0, &ch, 0, 1); }

inline UnicodeString& 
UnicodeString::operator+= (UChar32 ch) {
  UChar buffer[U16_MAX_LENGTH];
  int32_t _length = 0;
  UBool isError = FALSE;
  U16_APPEND(buffer, _length, U16_MAX_LENGTH, ch, isError);
  return doReplace(fLength, 0, buffer, 0, _length);
}

inline UnicodeString& 
UnicodeString::operator+= (const UnicodeString& srcText)
{ return doReplace(fLength, 0, srcText, 0, srcText.fLength); }

inline UnicodeString& 
UnicodeString::append(const UnicodeString& srcText, 
              int32_t srcStart, 
              int32_t srcLength)
{ return doReplace(fLength, 0, srcText, srcStart, srcLength); }

inline UnicodeString& 
UnicodeString::append(const UnicodeString& srcText)
{ return doReplace(fLength, 0, srcText, 0, srcText.fLength); }

inline UnicodeString& 
UnicodeString::append(const UChar *srcChars, 
              int32_t srcStart, 
              int32_t srcLength)
{ return doReplace(fLength, 0, srcChars, srcStart, srcLength); }

inline UnicodeString& 
UnicodeString::append(const UChar *srcChars,
              int32_t srcLength)
{ return doReplace(fLength, 0, srcChars, 0, srcLength); }

inline UnicodeString& 
UnicodeString::append(UChar srcChar)
{ return doReplace(fLength, 0, &srcChar, 0, 1); }

inline UnicodeString& 
UnicodeString::append(UChar32 srcChar) {
  UChar buffer[U16_MAX_LENGTH];
  int32_t _length = 0;
  UBool isError = FALSE;
  U16_APPEND(buffer, _length, U16_MAX_LENGTH, srcChar, isError);
  return doReplace(fLength, 0, buffer, 0, _length);
}

inline UnicodeString& 
UnicodeString::insert(int32_t start, 
              const UnicodeString& srcText, 
              int32_t srcStart, 
              int32_t srcLength)
{ return doReplace(start, 0, srcText, srcStart, srcLength); }

inline UnicodeString& 
UnicodeString::insert(int32_t start, 
              const UnicodeString& srcText)
{ return doReplace(start, 0, srcText, 0, srcText.fLength); }

inline UnicodeString& 
UnicodeString::insert(int32_t start, 
              const UChar *srcChars, 
              int32_t srcStart, 
              int32_t srcLength)
{ return doReplace(start, 0, srcChars, srcStart, srcLength); }

inline UnicodeString& 
UnicodeString::insert(int32_t start, 
              const UChar *srcChars,
              int32_t srcLength)
{ return doReplace(start, 0, srcChars, 0, srcLength); }

inline UnicodeString& 
UnicodeString::insert(int32_t start, 
              UChar srcChar)
{ return doReplace(start, 0, &srcChar, 0, 1); }

inline UnicodeString& 
UnicodeString::insert(int32_t start, 
              UChar32 srcChar)
{ return replace(start, 0, srcChar); }


inline UnicodeString& 
UnicodeString::remove(int32_t start, 
             int32_t _length)
{
  if(start <= 0 && _length == INT32_MAX) {
    // remove(guaranteed everything) of a bogus string makes the string empty and non-bogus
    return remove();
  } else {
    return doReplace(start, _length, NULL, 0, 0);
  }
}

inline UnicodeString& 
UnicodeString::remove()
{
  // remove() of a bogus string makes the string empty and non-bogus
  if(isBogus()) {
    unBogus();
  } else {
    fLength = 0;
  }
  return *this;
}

inline UnicodeString& 
UnicodeString::removeBetween(int32_t start,
                int32_t limit)
{ return doReplace(start, limit - start, NULL, 0, 0); }

inline UBool 
UnicodeString::truncate(int32_t targetLength)
{
  if(isBogus() && targetLength == 0) {
    // truncate(0) of a bogus string makes the string empty and non-bogus
    unBogus();
    return FALSE;
  } else if((uint32_t)targetLength < (uint32_t)fLength) {
    fLength = targetLength;
    return TRUE;
  } else {
    return FALSE;
  }
}

inline UnicodeString& 
UnicodeString::reverse()
{ return doReverse(0, fLength); }

inline UnicodeString& 
UnicodeString::reverse(int32_t start,
               int32_t _length)
{ return doReverse(start, _length); }


#ifdef U_USE_DEPRECATED_UCHAR_REFERENCE

//========================================
// class UCharReference
//========================================

/**
 * A proxy class to allow the UnicodeString::operator[] function to
 * work as a readable and a writable operator.
 * @obsolete ICU 2.4. Use charAt(), setCharAt(), and operator[] const instead since this API will be removed in that release.
 */
class U_COMMON_API UCharReference : public UObject {
public:
  UCharReference();
  inline UCharReference(UnicodeString *string,
         int32_t pos);
  inline UCharReference(const UCharReference& that);
  ~UCharReference();

  inline UCharReference& operator= (const UCharReference& that);
  inline UCharReference& operator= (UChar c);

  inline operator UChar();

  /**
   * ICU "poor man's RTTI", returns a UClassID for the actual class.
   *
   * @draft ICU 2.2
   */
  virtual inline UClassID getDynamicClassID() const { return getStaticClassID(); }

  /**
   * ICU "poor man's RTTI", returns a UClassID for this class.
   *
   * @draft ICU 2.2
   */
  static inline UClassID getStaticClassID() { return (UClassID)&fgClassID; }

private:
  UnicodeString *fString;
  int32_t fPos;

  /**
   * The address of this static class variable serves as this class's ID
   * for ICU "poor man's RTTI".
   */
  static const char fgClassID;
};


//========================================
// Inline members
//========================================
inline
UCharReference::UCharReference(UnicodeString *string, 
                   int32_t pos)
  : UObject(), fString(string), fPos(pos)
{}

inline
UCharReference::UCharReference(const UCharReference& that)
: UObject(that)
{ this->operator=(that); }

inline
UCharReference::~UCharReference()
{}

inline UCharReference&
UCharReference::operator= (const UCharReference& that)
{ fString->setCharAt(fPos, that.fString->charAt(that.fPos)); return *this; }

inline UCharReference& 
UCharReference::operator= (UChar c)
{ fString->setCharAt(fPos, c); return *this; }

inline
UCharReference::operator UChar()
{ return fString->charAt(fPos); }

#endif

U_NAMESPACE_END

#endif
