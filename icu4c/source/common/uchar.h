/*
********************************************************************************
*                                                                              *
* COPYRIGHT:                                                                   *
*   (C) Copyright Taligent, Inc.,  1997                                        *
*   (C) Copyright International Business Machines Corporation,  1997-1998      *
*   Licensed Material - Program-Property of IBM - All Rights Reserved.         *
*   US Government Users Restricted Rights - Use, duplication, or disclosure    *
*   restricted by GSA ADP Schedule Contract with IBM Corp.                     *
*                                                                              *
********************************************************************************
*
* File UCHAR.H
*
* Modification History:
*
*   Date        Name        Description
*   04/02/97    aliu        Creation.
*   03/29/99    helena      Updated for C APIs.
*   4/15/99     Madhu       Updated for C Implementation and Javadoc
*   5/20/99     Madhu		Added the function u_getVersion()
********************************************************************************
*/

#ifndef UCHAR_H
#define UCHAR_H

#include "utypes.h"
/*===========================================================================*/
/* Unicode version number                                                    */
/*===========================================================================*/
#define UNICODE_VERSION  "3.0.0.beta"

/**
 * The Unicode C API allows you to query the properties associated with individual 
 * Unicode character values.  
 * <p>
 * The Unicode character information, provided implicitly by the 
 * Unicode character encoding standard, includes information about the script 
 * (for example, symbols or control characters) to which the character belongs,
 * as well as semantic information such as whether a character is a digit or 
 * uppercase, lowercase, or uncased.
 * <P>
 */
    

    struct UCharDigitPair{
        uint16_t fUnicode;
        int8_t     fValue;
    };
    typedef struct UCharDigitPair UCharDigitPair;
    struct BlockScriptMap {
        UChar        fFirstCode;
        UChar        fLastCode;
    };
    typedef struct BlockScriptMap BlockScriptMap;

    
   

    static  bool_t  tablesCreated=FALSE;
    static  bool_t  ulTablesCreated=FALSE;
    static  bool_t  dirTablesCreated=FALSE;
    static  void    createTables(void);
    static  void    createUlTables(void);
    static  void    createDirTables(void);
/**
 * The Unicode C API allows you to query the properties associated with individual 
 * Unicode character values.  
 * <p>
 * The Unicode character information, provided implicitly by the 
 * Unicode character encoding standard, includes information about the script 
 * (for example, symbols or control characters) to which the character belongs,
 * as well as semantic information such as whether a character is a digit or 
 * uppercase, lowercase, or uncased.
 * <P>
 */

/**
 * Constants.
 */

/**
 * The minimum value a UChar can have.  The lowest value a
 * UChar can have is 0x0000.
 */
  static UChar UCHAR_MIN_VALUE;
/**
 * The maximum value a UChar can have.  The greatest value a
 * UChar can have is 0xffff.
 */

 static UChar UCHAR_MAX_VALUE;
/**
 * Data for enumerated Unicode general category types
 */


enum UCharCategory
{
    UNASSIGNED				= 0,
	UPPERCASE_LETTER		= 1,
	LOWERCASE_LETTER		= 2,
	TITLECASE_LETTER		= 3,
	MODIFIER_LETTER			= 4,
	OTHER_LETTER			= 5,
	NON_SPACING_MARK		= 6,
	ENCLOSING_MARK			= 7,
	COMBINING_SPACING_MARK	= 8,
	DECIMAL_DIGIT_NUMBER	= 9,
	LETTER_NUMBER			= 10,
	OTHER_NUMBER			= 11,
	SPACE_SEPARATOR			= 12,
	LINE_SEPARATOR			= 13,
	PARAGRAPH_SEPARATOR		= 14,
	CONTROL					= 15,
	FORMAT					= 16,
	PRIVATE_USE				= 17,
	SURROGATE				= 18,
	DASH_PUNCTUATION		= 19,
	START_PUNCTUATION		= 20,
	END_PUNCTUATION			= 21,
    CONNECTOR_PUNCTUATION	= 22,
	OTHER_PUNCTUATION		= 23,
	MATH_SYMBOL				= 24,
	CURRENCY_SYMBOL			= 25,
	MODIFIER_SYMBOL			= 26,
	OTHER_SYMBOL			= 27,
	INITIAL_PUNCTUATION		= 28,
	FINAL_PUNCTUATION		= 29,
	GENRERAL_OTHER_TYPES    = 30

};

typedef enum UCharCategory UCharCategory;
/**
 * This specifies the language directional property of a character set.
 */

enum UCharDirection   { 
    LEFT_TO_RIGHT               = 0, 
    RIGHT_TO_LEFT               = 1, 
    EUROPEAN_NUMBER             = 2,
    EUROPEAN_NUMBER_SEPARATOR   = 3,
    EUROPEAN_NUMBER_TERMINATOR  = 4,
    ARABIC_NUMBER               = 5,
    COMMON_NUMBER_SEPARATOR     = 6,
    BLOCK_SEPARATOR             = 7,
    SEGMENT_SEPARATOR           = 8,
    WHITE_SPACE_NEUTRAL         = 9, 
    OTHER_NEUTRAL               = 10, 
	LEFT_TO_RIGHT_EMBEDDING     = 11,
    LEFT_TO_RIGHT_OVERRIDE      = 12,
	RIGHT_TO_LEFT_ARABIC        = 13,
	RIGHT_TO_LEFT_EMBEDDING     = 14,
	RIGHT_TO_LEFT_OVERRIDE      = 15,
	POP_DIRECTIONAL_FORMAT      = 16,
	DIR_NON_SPACING_MARK        = 17,
	BOUNDARY_NEUTRAL            = 18
   
	
};

typedef enum UCharDirection UCharDirection;
/**
 * Script range as defined in the Unicode standard.
 */

enum UCharScript {
    BASIC_LATIN,
    LATIN1_SUPPLEMENT,
    LATIN_EXTENDED_A,
    LATIN_EXTENDED_B,
    IPA_EXTENSION,
    SPACING_MODIFIER,
    COMBINING_DIACRITICAL,
    GREEK,
    CYRILLIC,
    ARMENIAN,
    HEBREW,
    ARABIC,
    DEVANAGARI,
    BENGALI,
    GURMUKHI,
    GUJARATI,
    ORIYA,
    TAMIL,
    TELUGU,
    KANNADA,
    MALAYALAM,
    THAI,
    LAO,
    TIBETAN,
    GEORGIAN,
    HANGUL_JAMO,
    LATIN_EXTENDED_ADDITIONAL,
    GREEK_EXTENDED,
    GENERAL_PUNCTUATION,
    SUPER_SUBSCRIPT,
    CURRENCY_SYMBOL_SCRIPT,
    SYMBOL_COMBINING_MARK,
    LETTERLIKE_SYMBOL,
    NUMBER_FORM,
    ARROW,
    MATH_OPERATOR,
    MISC_TECHNICAL,
    CONTROL_PICTURE,
    OPTICAL_CHARACTER,
    ENCLOSED_ALPHANUMERIC,
    BOXDRAWING,
    BLOCK_ELEMENT,
    GEOMETRIC_SHAPE,
    MISC_SYMBOL,
    DINGBAT,
    CJK_SYMBOL_PUNCTUATION,
    HIRAGANA,
    KATAKANA,
    BOPOMOFO,
    HANGUL_COMPATIBILITY_JAMO,
    KANBUN,
    ENCLOSED_CJK_LETTER_MONTH,
    CJK_COMPATIBILITY,
    CJK_UNIFIED_IDEOGRAPH,
    HANGUL_SYLLABLE,
    HIGH_SURROGATE,
    HIGH_PRIVATE_USE_SURROGATE,
    LOW_SURROGATE,
    PRIVATE_USE_CHARACTERS,
    CJK_COMPATIBILITY_IDEOGRAPH,
    ALPHABETIC_PRESENTATION,
    ARABICP_RESENTATION_A,
    COMBINING_HALFMARK,
    CJK_COMPATIBILITY_FORM,
    SMALL_FORM_VARIANT,
    ARABIC_PRESENTATION_B,
    NO_SCRIPT,
    HALFWIDTH_FULLWIDTH_FORM,
    SCRIPT_COUNT
};
typedef enum UCharScript UCharScript;

/**
 * Values returned by the u_getCellWidth() function.
 */
enum UCellWidth
{
    ZERO_WIDTH              = 0,
    HALF_WIDTH              = 1,
    FULL_WIDTH              = 2,
    NEUTRAL                 = 3
};

typedef enum UCellWidth UCellWidth;
/**
 * Functions to classify characters.
 */

/**
 * Determines whether the specified UChar is a lowercase character
 * according to Unicode 2.1.2.
 *
 * @param ch    the character to be tested
 * @return  true if the character is lowercase; false otherwise.
 * @see UNICODE_VERSION
 * @see uisupper()
 * @see uistitle()
 * @see uislower()
 */
CAPI bool_t U_EXPORT2
u_islower(UChar c);

/**
 * Determines whether the specified character is an uppercase character
 * according to Unicode 2.1.2.
 *
 * @param ch    the character to be tested
 * @return  true if the character is uppercase; false otherwise.
 * @see uislower()
 * @see uistitle
 * @see utolower()
 */
CAPI bool_t U_EXPORT2
u_isupper(UChar c);

/**
 * Determines whether the specified character is a titlecase character
 * according to Unicode 2.1.2.
 *
 * @param ch    the character to be tested
 * @return  true if the character is titlecase; false otherwise.
 * @see uisupper()
 * @see uislower()
 * @see utotitle()
 */
CAPI bool_t U_EXPORT2
u_istitle(UChar c);

/**
 * Determines whether the specified character is a digit according to Unicode
 * 2.1.2.
 *
 * @param ch    the character to be tested
 * @return  true if the character is a digit; false otherwise.
 */
CAPI bool_t U_EXPORT2
u_isdigit(UChar c);

/**
 * Determines whether the specified numeric value is actually a defined character
 * according to Unicode 2.1.2.
 *
 * @param ch    the character to be tested
 * @return  true if the character has a defined Unicode meaning; false otherwise.
 *
 * @see uisdigit()
 * @see uisalpha()
 * @see uisalnum()
 * @see uisupper()
 * @see uislower()
 * @see uistitle()
 */
CAPI bool_t U_EXPORT2
u_isdefined(UChar c);

/**
 * Determines whether the specified character is a letter
 * according to Unicode 2.1.2.
 *
 * @param ch    the character to be tested
 * @return  true if the character is a letter; false otherwise.
 *
 * @see uisdigit()
 * @see uisalnum()
 */
CAPI bool_t U_EXPORT2
u_isalpha(UChar c);

/**
 * Determines if the specified character is a space character or not.
 *
 * @param ch    the character to be tested
 * @return  true if the character is a space character; false otherwise.
 */
CAPI bool_t U_EXPORT2
u_isspace(UChar c);

/**
 * Determines whether the specified character is a control character or not.
 *
 * @param ch    the character to be tested
 * @return  true if the Unicode character is a control character; false otherwise.
 *
 * @see uisprint()
 */
CAPI bool_t U_EXPORT2
u_iscntrl(UChar c);


/**
 * Determines whether the specified character is a printable character according 
 * to Unicode 2.1.2.
 *
 * @param ch    the character to be tested
 * @return  true if the Unicode character is a printable character; false otherwise.
 *
 * @see uiscntrl()
 */
CAPI bool_t U_EXPORT2
u_isprint(UChar c);

/**
 * Determines whether the specified character is of the base form according 
 * to Unicode 2.1.2.
 *
 * @param ch    the character to be tested
 * @return  true if the Unicode character is of the base form; false otherwise.
 *
 * @see uisalpha()
 * @see uisdigit()
 */
CAPI bool_t U_EXPORT2
u_isbase(UChar c);
/**
  * Returns the linguistic direction property of a character.
  * <P>
  * Returns the linguistic direction property of a character.
  * For example, 0x0041 (letter A) has the LEFT_TO_RIGHT directional 
  * property.
  * @see UCharDirection
  */
CAPI UCharDirection U_EXPORT2
u_charDirection(UChar c);

/**
 * Returns a value indicating the display-cell width of the character
 * when used in Asian text, according to the Unicode standard (see p. 6-130
 * of The Unicode Standard, Version 2.0).  The results for various characters
 * are as follows:
 * <P>
 *      ZERO_WIDTH: Characters which are considered to take up no display-cell space:
 *          control characters
 *          format characters
 *          line and paragraph separators
 *          non-spacing marks
 *          combining Hangul jungseong
 *          combining Hangul jongseong
 *          unassigned Unicode values
 * <P>
 *      HALF_WIDTH: Characters which take up half a cell in standard Asian text:
 *          all characters in the General Scripts Area except combining Hangul choseong
 *              and the characters called out specifically above as ZERO_WIDTH
 *          alphabetic and Arabic presentation forms
 *          halfwidth CJK punctuation
 *          halfwidth Katakana
 *          halfwidth Hangul Jamo
 *          halfwidth forms, arrows, and shapes
 * <P>
 *      FULL_WIDTH:  Characters which take up a full cell in standard Asian text:
 *          combining Hangul choseong
 *          all characters in the CJK Phonetics and Symbols Area
 *          all characters in the CJK Ideographs Area
 *          all characters in the Hangul Syllables Area
 *          CJK compatibility ideographs
 *          CJK compatibility forms
 *          small form variants
 *          fullwidth ASCII
 *          fullwidth punctuation and currency signs
 * <P>
 *      NEUTRAL:  Characters whose cell width is context-dependent:
 *          all characters in the Symbols Area, except those specifically called out above
 *          all characters in the Surrogates Area
 *          all charcaters in the Private Use Area
 * <P>
 * For Korean text, this algorithm should work properly with properly normalized Korean
 * text.  Precomposed Hangul syllables and non-combining jamo are all considered full-
 * width characters.  For combining jamo, we treat we treat choseong (initial consonants)
 * as double-width characters and junseong (vowels) and jongseong (final consonants)
 * as non-spacing marks.  This will work right in text that uses the precomposed
 * choseong characters instead of teo choseong characters in a row, and which uses the
 * choseong filler character at the beginning of syllables that don't have an initial
 * consonant.  The results may be slightly off with Korean text following different
 * conventions.
 */
CAPI uint16_t U_EXPORT2
u_charCellWidth(UChar c);

/**
 * Returns a value indicating a character category according to Unicode
 * 2.1.2.
 * @param c            the character to be tested
 * @return a value of type int, the character category.
 * @see UCharCategory
 */
CAPI int8_t U_EXPORT2
u_charType(UChar c);

/**
 * Retrives the decimal numeric value of a digit character.
 * @param c the digit character for which to get the numeric value
 * @return the numeric value of ch in decimal radix.  This method returns
 * -1 if ch is not a valid digit character.
 */
CAPI int32_t U_EXPORT2
u_charDigitValue(UChar c);

/**
 *
 * Returns the script associated with a character.
 * @see #UCharScript
 */
CAPI UCharScript     U_EXPORT2
u_charScript(UChar    ch);

/** 
 * The following functions are java specific.
 */
/**
  * A convenience method for determining if a Unicode character 
  * is allowed to start in a Unicode identifier.
  * A character may start a Unicode identifier if and only if
  * it is a letter.
  *
  * @param   c  the Unicode character.
  * @return  TRUE if the character may start a Unicode identifier;
  *          FALSE otherwise.
  * @see     u_isalpha
  * @see     u_isIDPart
  */
CAPI bool_t U_EXPORT2
u_isIDStart(UChar c);
/**
  * A convenience method for determining if a Unicode character
  * may be part of a Unicode identifier other than the starting
  * character.
  * <P>
  * A character may be part of a Unicode identifier if and only if
  * it is one of the following:
  * <ul>
  * <li>  a letter
  * <li>  a connecting punctuation character (such as "_").
  * <li>  a digit
  * <li>  a numeric letter (such as a Roman numeral character)
  * <li>  a combining mark
  * <li>  a non-spacing mark
  * <li>  an ignorable control character
  * </ul>
  * 
  * @param   c  the Unicode character.
  * @return  TRUE if the character may be part of a Unicode identifier;
  *          FALSE otherwise.
  * @see     u_isIDIgnorable
  * @see     u_isIDStart
     */
CAPI bool_t U_EXPORT2
u_isIDPart(UChar c);
/**
  * A convenience method for determining if a Unicode character 
  * should be regarded as an ignorable character 
  * in a Unicode identifier.
  * <P>
  * The following Unicode characters are ignorable in a 
  * Unicode identifier:
  * <table>
  * <tr><td>0x0000 through 0x0008,</td>
  *                                 <td>ISO control characters that</td></tr>
  * <tr><td>0x000E through 0x001B,</td> <td>are not whitespace</td></tr>
  * <tr><td>and 0x007F through 0x009F</td></tr>
  * <tr><td>0x200C through 0x200F</td>  <td>join controls</td></tr>
  * <tr><td>0x200A through 0x200E</td>  <td>bidirectional controls</td></tr>
  * <tr><td>0x206A through 0x206F</td>  <td>format controls</td></tr>
  * <tr><td>0xFEFF</td>               <td>zero-width no-break space</td></tr>
  * </table>
  * 
  * @param   c  the Unicode character.
  * @return  TRUE if the character may be part of a Unicode identifier;
  *          FALSE otherwise.
  * @see     u_isIDPart
  */
CAPI bool_t U_EXPORT2
u_isIDIgnorable(UChar c);
    /**
     * A convenience method for determining if a Unicode character
     * is allowed as the first character in a Java identifier.
     * <P>
     * A character may start a Java identifier if and only if
     * it is one of the following:
     * <ul>
     * <li>  a letter
     * <li>  a currency symbol (such as "$")
     * <li>  a connecting punctuation symbol (such as "_").
     * </ul>
     *
     * @param   c  the Unicode character.
     * @return  TRUE if the character may start a Java identifier;
     *          FALSE otherwise.
     * @see     u_isJavaIDPart
     * @see     u_isalpha
     * @see     u_isIDStart
     */
CAPI bool_t U_EXPORT2
u_isJavaIDStart(UChar c);
    /**
     * A convenience method for determining if a Unicode character 
     * may be part of a Java identifier other than the starting
     * character.
     * <P>
     * A character may be part of a Java identifier if and only if
     * it is one of the following:
     * <ul>
     * <li>  a letter
     * <li>  a currency symbol (such as "$")
     * <li>  a connecting punctuation character (such as "_").
     * <li>  a digit
     * <li>  a numeric letter (such as a Roman numeral character)
     * <li>  a combining mark
     * <li>  a non-spacing mark
     * <li>  an ignorable control character
     * </ul>
     * 
     * @param   c the Unicode character.
     * @return  TRUE if the character may be part of a Unicode identifier; 
     *          FALSE otherwise.
     * @see     u_isIDIgnorable
     * @see     u_isJavaIDStart
     * @see     u_isalpha
     * @see     u_isdigit
     * @see     u_isIDPart
     */

CAPI bool_t U_EXPORT2
u_isJavaIDPart(UChar c);

/**
 * Functions to change character case.
 */

/**
 * The given character is mapped to its lowercase equivalent according to
 * Unicode 2.1.2; if the character has no lowercase equivalent, the character 
 * itself is returned.
 * <P>
 * A character has a lowercase equivalent if and only if a lowercase mapping
 * is specified for the character in the Unicode 2.1.2 attribute table.
 * <P>
 * utolower() only deals with the general letter case conversion.
 * For language specific case conversion behavior, use ustrToUpper().
 * For example, the case conversion for dot-less i and dotted I in Turkish,
 * or for final sigma in Greek.
 *
 * @param ch    the character to be converted
 * @return  the lowercase equivalent of the character, if any;
 *      otherwise the character itself.
 */
CAPI UChar U_EXPORT2
u_tolower(UChar c);

/**
 * The given character is mapped to its uppercase equivalent according to Unicode
 * 2.1.2; if the character has no uppercase equivalent, the character itself is 
 * returned.
 * <P>
 * utoupper() only deals with the general letter case conversion.
 * For language specific case conversion behavior, use ustrToUpper().
 * For example, the case conversion for dot-less i and dotted I in Turkish,
 * or ess-zed (i.e., "sharp S") in German.
 *
 * @param ch    the character to be converted
 * @return  the uppercase equivalent of the character, if any;
 *      otherwise the character itself.
 */
CAPI UChar U_EXPORT2
u_toupper(UChar c);
/**
 * The given character is mapped to its titlecase equivalent according to Unicode
 * 2.1.2.  There are only four Unicode characters that are truly titlecase forms
 * that are distinct from uppercase forms.  As a rule, if a character has no
 * true titlecase equivalent, its uppercase equivalent is returned.
 * <P>
 * A character has a titlecase equivalent if and only if a titlecase mapping
 * is specified for the character in the Unicode 2.1.2 data.
 *
 * @param ch    the character to be converted
 * @return  the titlecase equivalent of the character, if any;
 *      otherwise the character itself.
 */
CAPI UChar U_EXPORT2
u_totitle(UChar c);

/**
 *
 *The function is used to get the Unicode standard Version that is used
 *@return the Unicode stabdard Version number
 */
CAPI const char* U_EXPORT2
u_getVersion(void);

#endif /*_UCHAR*/
/*eof*/
