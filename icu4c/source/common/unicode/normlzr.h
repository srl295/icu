/*
 ********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1996-1999, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************
 */

/*
* Modification history
* 
* Date      Name      Description
* 02/02/01  synwee    Added converters from EMode to UNormalizationMode, 
*                     getUNormalizationMode and getNormalizerEMode,
*                     useful in tbcoll and unorm.
*                     Added quickcheck method and incorporated it into 
*                     normalize()
*                     Removed hard coded on EMode to UNormalizationMode 
*                     conversion
*/

#ifndef NORMLZR_H
#define NORMLZR_H

#include "unicode/utypes.h"
#include "unicode/unistr.h"
#include "unicode/chariter.h"
#include "unicode/unorm.h"

/* forward declaration */
class ComposedCharIter;

/**
 * <tt>Normalizer</tt> transforms Unicode text into an equivalent composed or
 * decomposed form, allowing for easier sorting and searching of text.
 * <tt>Normalizer</tt> supports the standard normalization forms described in
 * <a href="http://www.unicode.org/unicode/reports/tr15/" target="unicode">
 * Unicode Technical Report #15</a>.
 * <p>
 * Characters with accents or other adornments can be encoded in
 * several different ways in Unicode.  For example, take the character "�"
 * (A-acute).   In Unicode, this can be encoded as a single character (the
 * "composed" form):
 * <pre>
 * \code
 *      00C1    LATIN CAPITAL LETTER A WITH ACUTE</pre>
 * \endcode
 * or as two separate characters (the "decomposed" form):
 * <pre>
 * \code
 *      0041    LATIN CAPITAL LETTER A
 *      0301    COMBINING ACUTE ACCENT</pre>
 * \endcode
 * <p>
 * To a user of your program, however, both of these sequences should be
 * treated as the same "user-level" character "�".  When you are searching or
 * comparing text, you must ensure that these two sequences are treated 
 * equivalently.  In addition, you must handle characters with more than one
 * accent.  Sometimes the order of a character's combining accents is
 * significant, while in other cases accent sequences in different orders are
 * really equivalent.
 * <p>
 * Similarly, the string "ffi" can be encoded as three separate letters:
 * <pre>
 * \code
 *      0066    LATIN SMALL LETTER F
 *      0066    LATIN SMALL LETTER F
 *      0069    LATIN SMALL LETTER I</pre>
 * \endcode
 * or as the single character
 * <pre>
 * \code
 *      FB03    LATIN SMALL LIGATURE FFI</pre>
 * \endcode
 * <p>
 * The ffi ligature is not a distinct semantic character, and strictly speaking
 * it shouldn't be in Unicode at all, but it was included for compatibility
 * with existing character sets that already provided it.  The Unicode standard
 * identifies such characters by giving them "compatibility" decompositions
 * into the corresponding semantic characters.  When sorting and searching, you
 * will often want to use these mappings.
 * <p>
 * <tt>Normalizer</tt> helps solve these problems by transforming text into the
 * canonical composed and decomposed forms as shown in the first example above.  
 * In addition, you can have it perform compatibility decompositions so that 
 * you can treat compatibility characters the same as their equivalents.
 * Finally, <tt>Normalizer</tt> rearranges accents into the proper canonical
 * order, so that you do not have to worry about accent rearrangement on your
 * own.
 * <p>
 * <tt>Normalizer</tt> adds one optional behavior, {@link #IGNORE_HANGUL},
 * that differs from
 * the standard Unicode Normalization Forms.  This option can be passed
 * to the {@link #Normalizer constructors} and to the static
 * {@link #compose compose} and {@link #decompose decompose} methods.  This
 * option, and any that are added in the future, will be turned off by default.
 * <p>
 * There are three common usage models for <tt>Normalizer</tt>.  In the first,
 * the static {@link #normalize normalize()} method is used to process an
 * entire input string at once.  Second, you can create a <tt>Normalizer</tt>
 * object and use it to iterate through the normalized form of a string by
 * calling {@link #first} and {@link #next}.  Finally, you can use the
 * {@link #setIndex setIndex()} and {@link #getIndex} methods to perform
 * random-access iteration, which is very useful for searching.
 * <p>
 * <b>Note:</b> <tt>Normalizer</tt> objects behave like iterators and have
 * methods such as <tt>setIndex</tt>, <tt>next</tt>, <tt>previous</tt>, etc.
 * You should note that while the <tt>setIndex</tt> and <tt>getIndex</tt> refer
 * to indices in the underlying <em>input</em> text being processed, the
 * <tt>next</tt> and <tt>previous</tt> methods it iterate through characters
 * in the normalized <em>output</em>.  This means that there is not
 * necessarily a one-to-one correspondence between characters returned
 * by <tt>next</tt> and <tt>previous</tt> and the indices passed to and
 * returned from <tt>setIndex</tt> and <tt>getIndex</tt>.  It is for this
 * reason that <tt>Normalizer</tt> does not implement the
 * {@link CharacterIterator} interface.
 * <p>
 * <b>Note:</b> <tt>Normalizer</tt> is currently based on version 2.1.8
 * of the <a href="http://www.unicode.org" target="unicode">Unicode Standard</a>.
 * It will be updated as later versions of Unicode are released.  If you are
 * using this class on a JDK that supports an earlier version of Unicode, it
 * is possible that <tt>Normalizer</tt> may generate composed or dedecomposed
 * characters for which your JDK's {@link java.lang.Character} class does not
 * have any data.
 * <p>
 * @author Laura Werner, Mark Davis
 */
class U_COMMON_API Normalizer
{

 public:
  // This tells us what the bits in the "mode" mean.
  enum {
    COMPAT_BIT         = 1,
    DECOMP_BIT         = 2,
    COMPOSE_BIT     = 4
  };



  /** If DONE is returned, then there are no more normalization results available. */
  enum {
      DONE=0xffff
  };

  /** The mode of a Normalizer object */
  enum EMode {

    /**
     * Null operation for use with the {@link #Normalizer constructors}
     * and the static {@link #normalize normalize} method.  This value tells
     * the <tt>Normalizer</tt> to do nothing but return unprocessed characters
     * from the underlying String or CharacterIterator.  If you have code which
     * requires raw text at some times and normalized text at others, you can
     * use <tt>NO_OP</tt> for the cases where you want raw text, rather
     * than having a separate code path that bypasses <tt>Normalizer</tt>
     * altogether.
     * <p>
     * @see #setMode
     */
    NO_OP         = 0,
    
    /**
     * Canonical decomposition followed by canonical composition.  Used with 
     * the {@link #Normalizer constructors} and the static 
     * {@link #normalize normalize}
     * method to determine the operation to be performed.
     * <p>
     * If all optional features (<i>e.g.</i> {@link #IGNORE_HANGUL}) are turned
     * off, this operation produces output that is in
     * <a href=http://www.unicode.org/unicode/reports/tr15/>Unicode Canonical
     * Form</a>
     * <b>C</b>.
     * <p>
     * @see #setMode
     */
    COMPOSE         = COMPOSE_BIT,

    /**
     * Compatibility decomposition followed by canonical composition.
     * Used with the {@link #Normalizer constructors} and the static
     * {@link #normalize normalize} method to determine the operation to be
     * performed.
     * <p>
     * If all optional features (<i>e.g.</i> {@link #IGNORE_HANGUL}) are turned
     * off, this operation produces output that is in
     * <a href=http://www.unicode.org/unicode/reports/tr15/>Unicode Canonical
     * Form</a>
     * <b>KC</b>.
     * <p>
     * @see #setMode
     */
    COMPOSE_COMPAT     = COMPOSE_BIT | COMPAT_BIT,

    /**
     * Canonical decomposition.  This value is passed to the
     * {@link #Normalizer constructors} and the static 
     * {@link #normalize normalize}
     * method to determine the operation to be performed.
     * <p>
     * If all optional features (<i>e.g.</i> {@link #IGNORE_HANGUL}) are turned
     * off, this operation produces output that is in
     * <a href=http://www.unicode.org/unicode/reports/tr15/>Unicode Canonical 
     * Form</a>
     * <b>D</b>.
     * <p>
     * @see #setMode
     */
    DECOMP         = DECOMP_BIT,

    /**
     * Compatibility decomposition.  This value is passed to the
     * {@link #Normalizer constructors} and the static 
     * {@link #normalize normalize}
     * method to determine the operation to be performed.
     * <p>
     * If all optional features (<i>e.g.</i> {@link #IGNORE_HANGUL}) are turned
     * off, this operation produces output that is in
     * <a href=http://www.unicode.org/unicode/reports/tr15/>Unicode Canonical 
     * Form</a>
     * <b>KD</b>.
     * <p>
     * @see #setMode
     */
    DECOMP_COMPAT     = DECOMP_BIT | COMPAT_BIT
  };

  /** The options for a Normalizer object */
  enum {

    /**
     * Option to disable Hangul/Jamo composition and decomposition.
     * This option applies to Korean text, 
     * which can be represented either in the Jamo alphabet or in Hangul
     * characters, which are really just two or three Jamo combined
     * into one visual glyph.  Since Jamo takes up more storage space than
     * Hangul, applications that process only Hangul text may wish to turn
     * this option on when decomposing text.
     * <p>
     * The Unicode standard treates Hangul to Jamo conversion as a 
     * canonical decomposition, so this option must be turned <b>off</b> if you
     * wish to transform strings into one of the standard
     * <a href="http://www.unicode.org/unicode/reports/tr15/" target="unicode">
     * Unicode Normalization Forms</a>.
     * <p>
     * @see #setOption
     */
    IGNORE_HANGUL     = 0x001
  };

  // Constructors

  /**
   * Creates a new <tt>Normalizer</tt> object for iterating over the
   * normalized form of a given string.
   * <p>
   * @param str   The string to be normalized.  The normalization
   *              will start at the beginning of the string.
   *
   * @param mode  The normalization mode.
   * @stable
   */
  Normalizer(const UnicodeString& str, 
         EMode mode);
    
  /**
   * Creates a new <tt>Normalizer</tt> object for iterating over the
   * normalized form of a given string.
   * <p>
   * The <tt>options</tt> parameter specifies which optional
   * <tt>Normalizer</tt> features are to be enabled for this object.
   * <p>
   * @param str   The string to be normalized.  The normalization
   *              will start at the beginning of the string.
   *
   * @param mode  The normalization mode.
   *
   * @param opt   Any optional features to be enabled.
   *              Currently the only available option is {@link #IGNORE_HANGUL}
   *              If you want the default behavior corresponding to one of the
   *              standard Unicode Normalization Forms, use 0 for this argument
   * @stable
   */
  Normalizer(const UnicodeString& str, 
         EMode mode, 
         int32_t opt);

  /**
   * Creates a new <tt>Normalizer</tt> object for iterating over the
   * normalized form of a given UChar string.
   * <p>
   * @param str   The string to be normalized.  The normalization
   *              will start at the beginning of the string.
   *
   * @param length Lenght of the string
   * @param mode  The normalization mode.
   * @stable
   *
   */
  Normalizer(const UChar* str,
         int32_t length,
         EMode mode);

  /**
   * Creates a new <tt>Normalizer</tt> object for iterating over the
   * normalized form of a given UChar string.
   * <p>
   * @param str   The string to be normalized.  The normalization
   *              will start at the beginning of the string.
   *
   * @param length Lenght of the string
   * @param mode  The normalization mode.
   * @param opt   Any optional features to be enabled.
   *              Currently the only available option is {@link #IGNORE_HANGUL}
   *              If you want the default behavior corresponding to one of the
   *              standard Unicode Normalization Forms, use 0 for this argument
   * @unimplemented 
   *
   */
  Normalizer(const UChar* str,
         int32_t length,
         EMode mode,
         int32_t option);

  /**
   * Creates a new <tt>Normalizer</tt> object for iterating over the
   * normalized form of the given text.
   * <p>
   * @param iter  The input text to be normalized.  The normalization
   *              will start at the beginning of the string.
   *
   * @param mode  The normalization mode.
   * @stable
   *
   */
  Normalizer(const CharacterIterator& iter, 
         EMode mode);

  /**
   * Creates a new <tt>Normalizer</tt> object for iterating over the
   * normalized form of the given text.
   * <p>
   * @param iter  The input text to be normalized.  The normalization
   *              will start at the beginning of the string.
   *
   * @param mode  The normalization mode.
   *
   * @param opt   Any optional features to be enabled.
   *              Currently the only available option is {@link #IGNORE_HANGUL}
   *              If you want the default behavior corresponding to one of the
   *              standard Unicode Normalization Forms, use 0 for this argument
   * @stable
   */
  Normalizer(const CharacterIterator& iter, 
         EMode mode, 
         int32_t opt);

  /**
   * Copy constructor.
   * @stable
   */
  Normalizer(const Normalizer& copy);

  /**
   * Destructor
   * @stable
   */
  ~Normalizer();


  //-------------------------------------------------------------------------
  // Static utility methods
  //-------------------------------------------------------------------------

  /**
   * Normalizes a <tt>String</tt> using the given normalization operation.
   * <p>
   * The <tt>options</tt> parameter specifies which optional
   * <tt>Normalizer</tt> features are to be enabled for this operation.
   * Currently the only available option is {@link #IGNORE_HANGUL}.
   * If you want the default behavior corresponding to one of the standard
   * Unicode Normalization Forms, use 0 for this argument.
   * <p>
   * @param source    the input string to be normalized.
   *
   * @param aMode     the normalization mode
   *
   * @param options   the optional features to be enabled.
   *
   * @param result    The normalized string (on output).
   *
   * @param status    The error code.
   * @stable
   */
  static void normalize(const UnicodeString& source, 
            EMode mode, 
            int32_t options,
            UnicodeString& result, 
            UErrorCode &status);

  /**
   * Compose a <tt>String</tt>.
   * <p>
   * The <tt>options</tt> parameter specifies which optional
   * <tt>Normalizer</tt> features are to be enabled for this operation.
   * Currently the only available option is {@link #IGNORE_HANGUL}.
   * If you want the default behavior corresponding
   * to Unicode Normalization Form <b>C</b> or <b>KC</b>,
   * use 0 for this argument.
   * <p>
   * @param source    the string to be composed.
   *
   * @param compat    Perform compatibility decomposition before composition.
   *                  If this argument is <tt>false</tt>, only canonical
   *                  decomposition will be performed.
   *
   * @param options   the optional features to be enabled.
   *
   * @param result    The composed string (on output).
   *
   * @param status    The error code.
   * @stable
   */
  static void compose(const UnicodeString& source, 
              UBool compat,
              int32_t options,
              UnicodeString& result, 
              UErrorCode &status);

  /**
   * Static method to decompose a <tt>String</tt>.
   * <p>
   * The <tt>options</tt> parameter specifies which optional
   * <tt>Normalizer</tt> features are to be enabled for this operation.
   * Currently the only available option is {@link #IGNORE_HANGUL}.
   * The desired options should be OR'ed together to determine the value
   * of this argument.  If you want the default behavior corresponding
   * to Unicode Normalization Form <b>D</b> or <b>KD</b>,
   * use 0 for this argument.
   * <p>
   * @param str   the string to be decomposed.
   *
   * @param compat    Perform compatibility decomposition.
   *                  If this argument is <tt>false</tt>, only canonical
   *                  decomposition will be performed.
   *
   * @param options   the optional features to be enabled.
   *
   * @param result    The composed string (on output).
   *
   * @param status    The error code.
   *
   * @return      the decomposed string.
   * @stable
   */
  static void decompose(const UnicodeString& source, 
            UBool compat,
            int32_t options,
            UnicodeString& result, 
            UErrorCode &status);

  /**
  * Converts C's Normalizer::EMode to UNormalizationMode
  * @param mode member of the enum Normalizer::EMode
  * @param status error codes status
  * @return UNormalizationMode equivalent of Normalizer::EMode
  */
  inline static UNormalizationMode getUNormalizationMode(EMode mode, 
                                                  UErrorCode& status);

  /**
  * Converts C++'s UNormalizationMode to Normalizer::EMode
  * @param mode member of the enum UNormalizationMode
  * @param status error codes status
  * @return Normalizer::EMode equivalent of UNormalizationMode
  */
  inline static EMode getNormalizerEMode(UNormalizationMode mode, 
                                         UErrorCode& status);

  /**
  * Performing quick check on a string, to quickly determine if the string is 
  * in a particular normalization format.
  * Three types of result can be returned UQUICK_CHECK_YES, UQUICK_CHECK_NO or
  * UQUICK_CHECK_MAYBE. Result UQUICK_CHECK_YES indicates that the argument
  * string is in the desired normalized format, UQUICK_CHECK_NO determines that
  * argument string is not in the desired normalized format. A 
  * UQUICK_CHECK_MAYBE result indicates that a more thorough check is required, 
  * the user may have to put the string in its normalized form and compare the 
  * results.
  * @param source       string for determining if it is in a normalized format
  * @paran mode         normalization format
  * @param status A pointer to an UErrorCode to receive any errors
  * @return UQUICK_CHECK_YES, UQUICK_CHECK_NO or UQUICK_CHECK_MAYBE
  */
  static UQUICK_CHECK_VALUES
  quickCheck(const UnicodeString& source,
             EMode                mode, 
             UErrorCode&          status);

  /**
  * Performing quick check on a string, to quickly determine if the string is 
  * in a particular normalization format.
  * Three types of result can be returned UQUICK_CHECK_YES, UQUICK_CHECK_NO or
  * UQUICK_CHECK_MAYBE. Result UQUICK_CHECK_YES indicates that the argument
  * string is in the desired normalized format, UQUICK_CHECK_NO determines that
  * argument string is not in the desired normalized format. A 
  * UQUICK_CHECK_MAYBE result indicates that a more thorough check is required, 
  * the user may have to put the string in its normalized form and compare the 
  * results.
  * @param source string for determining if it is in a normalized format
  * @paran mode normalization format
  * @param options the optional features to be enabled.
  * @param status A pointer to an UErrorCode to receive any errors
  * @return UQUICK_CHECK_YES, UQUICK_CHECK_NO or UQUICK_CHECK_MAYBE
  */
  static UQUICK_CHECK_VALUES
  quickCheckWithOption(const UnicodeString& source,
                       EMode                mode, 
                       int32_t              options,
                       UErrorCode&          status);

  //-------------------------------------------------------------------------
  // CharacterIterator overrides
  //-------------------------------------------------------------------------
  
  /**
   * Return the current character in the normalized text.
   * @draft
   */
  UChar32              current(void) const;

  /**
   * Return the first character in the normalized text.  This resets
   * the <tt>Normalizer's</tt> position to the beginning of the text.
   * @draft
   */
  UChar32              first(void);

  /**
   * Return the last character in the normalized text.  This resets
   * the <tt>Normalizer's</tt> position to be just before the
   * the input text corresponding to that normalized character.
   * @draft
   */
  UChar32              last(void);

  /**
   * Return the next character in the normalized text and advance
   * the iteration position by one.  If the end
   * of the text has already been reached, {@link #DONE} is returned.
   * @draft
   */
  UChar32              next(void);

  /**
   * Return the previous character in the normalized text and decrement
   * the iteration position by one.  If the beginning
   * of the text has already been reached, {@link #DONE} is returned.
   * @draft
   */
  UChar32              previous(void);

  /**
   * Set the iteration position in the input text that is being normalized
   * and return the first normalized character at that position.
   * <p>
   * <b>Note:</b> This method sets the position in the <em>input</em> text,
   * while {@link #next} and {@link #previous} iterate through characters
   * in the normalized <em>output</em>.  This means that there is not
   * necessarily a one-to-one correspondence between characters returned
   * by <tt>next</tt> and <tt>previous</tt> and the indices passed to and
   * returned from <tt>setIndex</tt> and {@link #getIndex}.
   * <p>
   * @param index the desired index in the input text.
   *
   * @return      the first normalized character that is the result of iterating
   *              forward starting at the given index.
   * @draft
   */
  UChar32              setIndex(UTextOffset index);

  /**
   * Reset the iterator so that it is in the same state that it was just after
   * it was constructed.  A subsequent call to <tt>next</tt> will return the first
   * character in the normalized text.  In contrast, calling <tt>setIndex(0)</tt> followed
   * by <tt>next</tt> will return the <em>second</em> character in the normalized text,
   * because <tt>setIndex</tt> itself returns the first character
   * @stable
   */
  void                reset(void);

  /**
   * Retrieve the current iteration position in the input text that is
   * being normalized.  This method is useful in applications such as
   * searching, where you need to be able to determine the position in
   * the input text that corresponds to a given normalized output character.
   * <p>
   * <b>Note:</b> This method sets the position in the <em>input</em>, while
   * {@link #next} and {@link #previous} iterate through characters in the
   * <em>output</em>.  This means that there is not necessarily a one-to-one
   * correspondence between characters returned by <tt>next</tt> and
   * <tt>previous</tt> and the indices passed to and returned from
   * <tt>setIndex</tt> and {@link #getIndex}.
   * @stable
   */
  UTextOffset            getIndex(void) const;

  /**
   * Retrieve the index of the start of the input text.  This is the begin index
   * of the <tt>CharacterIterator</tt> or the start (i.e. 0) of the <tt>String</tt>
   * over which this <tt>Normalizer</tt> is iterating
   * @stable
   */
  UTextOffset            startIndex(void) const;

  /**
   * Retrieve the index of the end of the input text.  This is the end index
   * of the <tt>CharacterIterator</tt> or the length of the <tt>String</tt>
   * over which this <tt>Normalizer</tt> is iterating
   * @stable
   */
  UTextOffset            endIndex(void) const;


  /**
   * Returns true when both iterators refer to the same character in the same
   * character-storage object.
   * @stable
   */
  //  virtual UBool    operator==(const CharacterIterator& that) const;
  UBool        operator==(const Normalizer& that) const;
  inline UBool        operator!=(const Normalizer& that) const;

  /**
   * Returns a pointer to a new Normalizer that is a clone of this one.
   * The caller is responsible for deleting the new clone.
   * @stable
   */
  Normalizer*        clone(void) const;

  /**
   * Generates a hash code for this iterator.
   * @stable
   */
  int32_t                hashCode(void) const;

  //-------------------------------------------------------------------------
  // Property access methods
  //-------------------------------------------------------------------------

  /**
   * Set the normalization mode for this object.
   * <p>
   * <b>Note:</b>If the normalization mode is changed while iterating
   * over a string, calls to {@link #next} and {@link #previous} may
   * return previously buffers characters in the old normalization mode
   * until the iteration is able to re-sync at the next base character.
   * It is safest to call {@link #setText setText()}, {@link #first},
   * {@link #last}, etc. after calling <tt>setMode</tt>.
   * <p>
   * @param newMode the new mode for this <tt>Normalizer</tt>.
   * The supported modes are:
   * <ul>
   *  <li>{@link #COMPOSE}        - Unicode canonical decompositiion
   *                                  followed by canonical composition.
   *  <li>{@link #COMPOSE_COMPAT} - Unicode compatibility decompositiion
   *                                  follwed by canonical composition.
   *  <li>{@link #DECOMP}         - Unicode canonical decomposition
   *  <li>{@link #DECOMP_COMPAT}  - Unicode compatibility decomposition.
   *  <li>{@link #NO_OP}          - Do nothing but return characters
   *                                  from the underlying input text.
   * </ul>
   *
   * @see #getMode
   * @stable
   */
  void setMode(EMode newMode);

  /**
   * Return the basic operation performed by this <tt>Normalizer</tt>
   *
   * @see #setMode
   * @stable
   */
  EMode getMode(void) const;

  /**
   * Set options that affect this <tt>Normalizer</tt>'s operation.
   * Options do not change the basic composition or decomposition operation
   * that is being performed , but they control whether
   * certain optional portions of the operation are done.
   * Currently the only available option is:
   * <p>
   * <ul>
   *   <li>{@link #IGNORE_HANGUL} - Do not decompose Hangul syllables into the
   *       Jamo alphabet and vice-versa.  This option is off by default 
   *       (<i>i.e.</i> Hangul processing is enabled) since the Unicode 
   *       standard specifies that Hangul to Jamo is a canonical decomposition.
   *       For any of the standard Unicode Normalization
   *       Forms, you should leave this option off.
   * </ul>
   * <p>
   * @param   option  the option whose value is to be set.
   * @param   value   the new setting for the option.  Use <tt>true</tt> to
   *                  turn the option on and <tt>false</tt> to turn it off.
   *
   * @see #getOption
   * @stable
   */
  void setOption(int32_t option, 
         UBool value);

  /**
   * Determine whether an option is turned on or off.
   * <p>
   * @see #setOption
   * @stable
   */
  UBool getOption(int32_t option) const;

  /**
   * Set the input text over which this <tt>Normalizer</tt> will iterate.
   * The iteration position is set to the beginning.
   * @stable
   */
  void setText(const UnicodeString& newText, 
           UErrorCode &status);

  /**
   * Set the input text over which this <tt>Normalizer</tt> will iterate.
   * The iteration position is set to the beginning.
   * @stable
   */
  void setText(const CharacterIterator& newText, 
           UErrorCode &status);

  /**
   * Set the input text over which this <tt>Normalizer</tt> will iterate.
   * The iteration position is set to the beginning.
   * @stable
   */
  void setText(const UChar* newText,
                    int32_t length,
            UErrorCode &status);
  /**
   * Copies the text under iteration into the UnicodeString referred to by 
   * "result".
   * @param result Receives a copy of the text under iteration.
   * @draft should also return the result UnicodeString &
   */
  void            getText(UnicodeString&  result);

  /**
   * Returns the text under iteration into the UChar* buffer pointer.
   * @param result Receives a copy of the text under iteration.
   * @unimplemented
   */
  const UChar*     getText(int32_t&  count);

private:
  // Private utility methods for iteration
  // For documentation, see the source code
  UChar nextCompose(void);
  UChar prevCompose(void);
  UChar nextDecomp(void);
  UChar prevDecomp(void);

  UChar curForward(void);
  UChar curBackward(void);

  void    init(CharacterIterator* iter, 
         EMode mode, 
         int32_t option);
  void    initBuffer(void);
  void    clearBuffer(void);

  // Utilities used by Compose
  static void        bubbleAppend(UnicodeString& target, 
                     UChar ch, 
                     uint32_t cclass);
  static uint32_t     getComposeClass(UChar ch);
  static uint16_t    composeLookup(UChar ch);
  static uint16_t    composeAction(uint16_t baseIndex, 
                      uint16_t comIndex);
  static void        explode(UnicodeString& target, 
                uint16_t index);
  static UChar    pairExplode(UnicodeString& target, 
                    uint16_t action);

  // Utilities used by Decompose
  static void        fixCanonical(UnicodeString& result);    // Reorders combining marks
  static uint8_t    getClass(UChar ch);                    // Gets char's combining class

  // Other static utility methods
  static void doAppend(const UChar source[], 
               uint16_t offset, 
               UnicodeString& dest);
  static void doInsert(const UChar source[], 
               uint16_t offset, 
               UnicodeString& dest, 
               UTextOffset pos);
  static uint16_t doReplace(const UChar source[], 
               uint16_t offset, 
               UnicodeString& dest, 
               UTextOffset pos);

  static void hangulToJamo(UChar ch, 
               UnicodeString& result, 
               uint16_t decompLimit);
  static void jamoAppend(UChar ch, 
             uint16_t decompLimit, 
             UnicodeString& dest);
  static void jamoToHangul(UnicodeString& buffer, 
               UTextOffset start);

  //-------------------------------------------------------------------------
  // Private data
  //-------------------------------------------------------------------------

  EMode         fMode;
  int32_t       fOptions;
  int16_t    minDecomp;

  // The input text and our position in it
  CharacterIterator*  text;

  // A buffer for holding intermediate results
  UnicodeString       buffer;
  UTextOffset          bufferPos;
  UTextOffset          bufferLimit;
  UChar             currentChar;

  // Another buffer for use during iterative composition
  UnicodeString       explodeBuf;

  enum {
    EMPTY = -1,
    STR_INDEX_SHIFT = 2, //Must agree with the constants used in NormalizerBuilder
    STR_LENGTH_MASK = 0x0003
  };

  enum {
    HANGUL_BASE = 0xac00,
    HANGUL_LIMIT = 0xd7a4,
    JAMO_LBASE = 0x1100,
    JAMO_VBASE = 0x1161,
    JAMO_TBASE = 0x11a7,
    JAMO_LCOUNT = 19,
    JAMO_VCOUNT = 21,
    JAMO_TCOUNT = 28,
    JAMO_NCOUNT = JAMO_VCOUNT * JAMO_TCOUNT
  };

  friend class ComposedCharIter;
};

inline UBool
Normalizer::operator!= (const Normalizer& other) const
{ return ! operator==(other); }

inline UNormalizationMode Normalizer::getUNormalizationMode(
                                   Normalizer::EMode  mode, UErrorCode &status)
{
  if (U_SUCCESS(status))
  { 
    switch (mode)
    {
    case Normalizer::NO_OP : 
      return UNORM_NONE;
    case Normalizer::COMPOSE :
      return UNORM_NFC;
    case Normalizer::COMPOSE_COMPAT :
      return UNORM_NFKC;
    case Normalizer::DECOMP :
      return UNORM_NFD;
    case Normalizer::DECOMP_COMPAT :
      return UNORM_NFKD;
    default : 
      status = U_ILLEGAL_ARGUMENT_ERROR; 
    }
  }
  return UNORM_DEFAULT;
}

inline Normalizer::EMode Normalizer::getNormalizerEMode(
                                  UNormalizationMode mode, UErrorCode &status)
{
  if (U_SUCCESS(status))
  {
    switch (mode)
    {
    case UNORM_NONE :
      return Normalizer::NO_OP;
    case UNORM_NFD :
      return Normalizer::DECOMP;
    case UNORM_NFKD :
      return Normalizer::DECOMP_COMPAT;
    case UNORM_NFC :
      return Normalizer::COMPOSE;
    case UNORM_NFKC :
      return Normalizer::COMPOSE_COMPAT;
    default : 
      status = U_ILLEGAL_ARGUMENT_ERROR; 
    }
  }
  return Normalizer::DECOMP_COMPAT;
}

#endif // _NORMLZR
