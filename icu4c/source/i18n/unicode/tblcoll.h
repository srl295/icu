/*
* Copyright � {1996-1999}, International Business Machines Corporation and others. All Rights Reserved.
*******************************************************************************
*
* File tblcoll.h
*
* Created by: Helena Shih
*
* Modification History:
*
*  Date        Name        Description
*  2/5/97      aliu        Added streamIn and streamOut methods.  Added
*                          constructor which reads RuleBasedCollator object from
*                          a binary file.  Added writeToFile method which streams
*                          RuleBasedCollator out to a binary file.  The streamIn
*                          and streamOut methods use istream and ostream objects
*                          in binary mode.
*  2/12/97     aliu        Modified to use TableCollationData sub-object to
*                          hold invariant data.
*  2/13/97     aliu        Moved several methods into this class from Collation.
*                          Added a private RuleBasedCollator(Locale&) constructor,
*                          to be used by Collator::createDefault().  General
*                          clean up.
*  2/20/97     helena      Added clone, operator==, operator!=, operator=, and copy
*                          constructor and getDynamicClassID.
*  3/5/97      aliu        Modified constructFromFile() to add parameter
*                          specifying whether or not binary loading is to be
*                          attempted.  This is required for dynamic rule loading.
* 05/07/97     helena      Added memory allocation error detection.
*  6/17/97     helena      Added IDENTICAL strength for compare, changed getRules to 
*                          use MergeCollation::getPattern.
*  6/20/97     helena      Java class name change.
*  8/18/97     helena      Added internal API documentation.
* 09/03/97     helena      Added createCollationKeyValues().
* 02/10/98     damiba      Added compare with "length" parameter
* 08/05/98     erm         Synched with 1.2 version of RuleBasedCollator.java
* 04/23/99     stephen     Removed EDecompositionMode, merged with
*                          Normalizer::EMode
* 06/14/99     stephen     Removed kResourceBundleSuffix
* 11/02/99     helena      Collator performance enhancements.  Eliminates the 
*                          UnicodeString construction and special case for NO_OP.
* 11/23/99     srl         More performance enhancements. Updates to NormalizerIterator
*                          internal state management.
* 12/15/99     aliu        Update to support Thai collation.  Move NormalizerIterator
*                          to implementation file.
*******************************************************************************
*/

#ifndef TBLCOLL_H
#define TBLCOLL_H

#include "unicode/utypes.h"
#include "unicode/coll.h"
#include "unicode/chariter.h"
#include "unicode/unistr.h"
#include "unicode/sortkey.h"
#include "unicode/normlzr.h"

class VectorOfPToContractElement;
class VectorOfInt;
class VectorOfPToContractTable;
class VectorOfPToExpandTable;
class MergeCollation;
class CollationElementIterator;
class RuleBasedCollatorStreamer;
class NormalizerIterator; // see tblcoll.cpp
class Collator;
class TableCollationData;

// This is just temporary, for prototyping.
struct collIterate;
typedef void *  UCollator;

enum UColRuleOption {
	UCOL_TAILORING_ONLY,
	UCOL_FULL_RULES
};



/**
 * The RuleBasedCollator class provides the simple implementation of Collator,
 * using data-driven tables.  The user can create a customized table-based
 * collation.
 * <P>
 * RuleBasedCollator maps characters to collation keys.
 * <p>
 * Table Collation has the following restrictions for efficiency (other
 * subclasses may be used for more complex languages) :
 *       <p>1. If the French secondary ordering is specified in a collation object, 
 *             it is applied to the whole object.
 *       <p>2. All non-mentioned Unicode characters are at the end of the
 *             collation order.
 *       <p>3. Private use characters are treated as identical.  The private
 *             use area in Unicode is 0xE800-0xF8FF.
 * <p>The collation table is composed of a list of collation rules, where each
 * rule is of three forms:
 * <pre>
 * .    &lt; modifier >
 * .    &lt; relation > &lt; text-argument >
 * .    &lt; reset > &lt; text-argument >
 * </pre>
 * The following demonstrates how to create your own collation rules:
 * <UL Type=round>
 *    <LI><strong>Text Argument</strong>: A text argument is any sequence of
 *        characters, excluding special characters (that is, whitespace
 *        characters and the characters used in modifier, relation and reset).
 *        If those characters are desired, you can put them in single quotes
 *        (e.g. ampersand => '&').<P>
 *    <LI><strong>Modifier</strong>: There is a single modifier,
 *        which is used to specify that all secondary differences are
 *        sorted backwards.
 *        <p>'@' : Indicates that secondary differences, such as accents, are 
 *                 sorted backwards, as in French.<P>
 *    <LI><strong>Relation</strong>: The relations are the following:
 *        <UL Type=square>
 *            <LI>'&lt;' : Greater, as a letter difference (primary)
 *            <LI>';' : Greater, as an accent difference (secondary)
 *            <LI>',' : Greater, as a case difference (tertiary)
 *            <LI>'=' : Equal
 *        </UL><P>
 *    <LI><strong>Reset</strong>: There is a single reset,
 *        which is used primarily for contractions and expansions, but which
 *        can also be used to add a modification at the end of a set of rules.
 *        <p>'&' : Indicates that the next rule follows the position to where
 *            the reset text-argument would be sorted.
 *
 * <p>
 * This sounds more complicated than it is in practice. For example, the
 * following are equivalent ways of expressing the same thing:
 * <pre>
 * .    a &lt; b &lt; c
 * .    a &lt; b & b &lt; c
 * .    a &lt; c & a &lt; b
 * </pre>
 * Notice that the order is important, as the subsequent item goes immediately
 * after the text-argument. The following are not equivalent:
 * <pre>
 * .    a &lt; b & a &lt; c
 * .    a &lt; c & a &lt; b
 * </pre>
 * Either the text-argument must already be present in the sequence, or some
 * initial substring of the text-argument must be present. (e.g. "a &lt; b & ae &lt;
 * e" is valid since "a" is present in the sequence before "ae" is reset). In
 * this latter case, "ae" is not entered and treated as a single character;
 * instead, "e" is sorted as if it were expanded to two characters: "a"
 * followed by an "e". This difference appears in natural languages: in
 * traditional Spanish "ch" is treated as though it contracts to a single
 * character (expressed as "c &lt; ch &lt; d"), while in traditional German "�"
 * (a-umlaut) is treated as though it expands to two characters (expressed as
 * "a & ae ; � &lt; b").
 * <p><strong>Ignorable Characters</strong>
 * <p>For ignorable characters, the first rule must start with a relation (the
 * examples we have used above are really fragments; "a &lt; b" really should be
 * "&lt; a &lt; b"). If, however, the first relation is not "&lt;", then all the 
 * text-arguments up to the first "&lt;" are ignorable. For example, ", - &lt; a &lt; b"
 * makes "-" an ignorable character, as we saw earlier in the word
 * "black-birds". In the samples for different languages, you see that most
 * accents are ignorable.
 * <p><strong>Normalization and Accents</strong>
 * <p>The Collator object automatically normalizes text internally to separate
 * accents from base characters where possible. This is done both when
 * processing the rules, and when comparing two strings. Collator also uses
 * the Unicode canonical mapping to ensure that combining sequences are sorted
 * properly (for more information, see <A HREF="http://www.aw.com/devpress">
 * The Unicode Standard, Version 2.0</A>.)</P>
 * <p><strong>Errors</strong>
 * <p>The following are errors:
 * <UL Type=round>
 *     <LI>A text-argument contains unquoted punctuation symbols
 *        (e.g. "a &lt; b-c &lt; d").
 *     <LI>A relation or reset character not followed by a text-argument
 *        (e.g. "a &lt; , b").
 *     <LI>A reset where the text-argument (or an initial substring of the
 *         text-argument) is not already in the sequence.
 *         (e.g. "a &lt; b & e &lt; f")
 * </UL>
 * <pre>
 * .    Examples:
 * .    Simple:     "&lt; a &lt; b &lt; c &lt; d"
 * .    Norwegian:  "&lt; a,A&lt; b,B&lt; c,C&lt; d,D&lt; e,E&lt; f,F&lt; g,G&lt; h,H&lt; i,I&lt; j,J
 * .                 &lt; k,K&lt; l,L&lt; m,M&lt; n,N&lt; o,O&lt; p,P&lt; q,Q&lt; r,R&lt; s,S&lt; t,T
 * .                 &lt; u,U&lt; v,V&lt; w,W&lt; x,X&lt; y,Y&lt; z,Z
 * .                 &lt; �=a�,�=A�
 * .                 ;aa,AA&lt; �,�&lt; �,�"
 * </pre>
 * <p>To create a table-based collation object, simply supply the collation
 * rules to the RuleBasedCollator contructor.  For example:
 * <pre>
 * .    UErrorCode status = U_ZERO_ERROR;
 * .    RuleBasedCollator *mySimple = new RuleBasedCollator(Simple, status);
 * </pre>
 * <p>Another example:
 * <pre>
 * .    UErrorCode status = U_ZERO_ERROR;
 * .    RuleBasedCollator *myNorwegian = new RuleBasedCollator(Norwegian, status);
 * </pre>
 * To add rules on top of an existing table, simply supply the orginal rules
 * and modifications to RuleBasedCollator constructor.  For example,
 * <pre>
 * .     Traditional Spanish (fragment): ... & C &lt; ch , cH , Ch , CH ...
 * .     German (fragment) : ...&lt; y , Y &lt; z , Z
 * .                         & AE, � & AE, � 
 * .                         & OE , � & OE, � 
 * .                         & UE , � & UE, � 
 * .     Symbols (fragment): ...&lt; y, Y &lt; z , Z
 * .                         & Question-mark ; '?'
 * .                         & Ampersand ; '&'
 * .                         & Dollar-sign ; '$'
 * <p>To create a collation object for traditional Spanish, the user can take
 * the English collation rules and add the additional rules to the table.
 * For example:
 * <pre>
 * .     UErrorCode status = U_ZERO_ERROR;
 * .     UnicodeString rules(DEFAULTRULES);
 * .     rules += "& C &lt; ch, cH, Ch, CH";
 * .     RuleBasedCollator *mySpanish = new RuleBasedCollator(rules, status);
 * </pre>
 * <p>In order to sort symbols in the similiar order of sorting their
 * alphabetic equivalents, you can do the following,
 * <pre>
 * .     UErrorCode status = U_ZERO_ERROR;
 * .     UnicodeString rules(DEFAULTRULES);
 * .     rules += "& Question-mark ; '?' & Ampersand ; '&' & Dollar-sign ; '$' ";
 * .     RuleBasedCollator *myTable = new RuleBasedCollator(rules, status);
 * </pre>
 * <p>Another way of creating the table-based collation object, mySimple,
 * is:
 * <pre>
 * .     UErrorCode status = U_ZERO_ERROR;
 * .     RuleBasedCollator *mySimple = new
 * .           RuleBasedCollator(" &lt; a &lt; b & b &lt; c & c &lt; d", status);
 * </pre>
 * Or,
 * <pre>
 * .     UErrorCode status = U_ZERO_ERROR;
 * .     RuleBasedCollator *mySimple = new
 * .           RuleBasedCollator(" &lt; a &lt; b &lt; d & b &lt; c", status);
 * </pre>
 * Because " &lt; a &lt; b &lt; c &lt; d" is the same as "a &lt; b &lt; d & b &lt; c" or
 * "&lt; a &lt; b & b &lt; c & c &lt; d".
 *
 * <p>To combine collations from two locales, (without error handling for clarity)
 * <pre>
 * .    // Create an en_US Collator object
 * .    Locale locale_en_US("en", "US", "");
 * .    RuleBasedCollator* en_USCollator = (RuleBasedCollator*)
 * .        Collator::createInstance( locale_en_US, success );
 * .
 * .    // Create a da_DK Collator object
 * .    Locale locale_da_DK("da", "DK", "");
 * .    RuleBasedCollator* da_DKCollator = (RuleBasedCollator*)
 * .        Collator::createInstance( locale_da_DK, success );
 * .
 * .    // Combine the two
 * .    // First, get the collation rules from en_USCollator
 * .    UnicodeString rules = en_USCollator->getRules();
 * .    // Second, get the collation rules from da_DKCollator
 * .    rules += da_DKCollator->getRules();
 * .    RuleBasedCollator* newCollator = new RuleBasedCollator( rules, success );
 * .    // newCollator has the combined rules
 * </pre>
 * <p>Another more interesting example would be to make changes on an existing
 * table to create a new collation object.  For example, add
 * "& C &lt; ch, cH, Ch, CH" to the en_USCollation object to create your own
 * English collation object,
 * <pre>
 * .    // Create a new Collator object with additional rules
 * .    rules = en_USCollator->getRules();
 * .    rules += "& C < ch, cH, Ch, CH";
 * .    RuleBasedCollator* myCollator = new RuleBasedCollator( rules, success );
 * .    // myCollator contains the new rules
 * </pre>
 *
 * <p>The following example demonstrates how to change the order of
 * non-spacing accents,
 * <pre>
 * .     UChar contents[] = {
 * .         '=', 0x0301, ';', 0x0300, ';', 0x0302,
 * .         ';', 0x0308, ';', 0x0327, ',', 0x0303,    // main accents
 * .         ';', 0x0304, ';', 0x0305, ';', 0x0306,    // main accents
 * .         ';', 0x0307, ';', 0x0309, ';', 0x030A,    // main accents
 * .         ';', 0x030B, ';', 0x030C, ';', 0x030D,    // main accents
 * .         ';', 0x030E, ';', 0x030F, ';', 0x0310,    // main accents
 * .         ';', 0x0311, ';', 0x0312,                 // main accents
 * .         '&lt;', 'a', ',', 'A', ';', 'a', 'e', ',', 'A', 'E',
 * .         ';', 0x00e6, ',', 0x00c6, '&lt;', 'b', ',', 'B',
 * .         '&lt;', 'c', ',', 'C', '&lt;', 'e', ',', 'E', '&', 
 * .         'C', '&lt;', 'd', ',', 'D', 0 };
 * .     UnicodeString oldRules(contents);
 * .     UErrorCode status = U_ZERO_ERROR;
 * .     // change the order of accent characters
 * .     UChar addOn[] = { '&', ',', 0x0300, ';', 0x0308, ';', 0x0302, 0 };
 * .     oldRules += addOn;
 * .     RuleBasedCollator *myCollation = new RuleBasedCollator(oldRules, status);
 * </pre>
 *
 * <p> The last example shows how to put new primary ordering in before the
 * default setting. For example, in Japanese collation, you can either sort
 * English characters before or after Japanese characters,
 * <pre>
 * .     UErrorCode status = U_ZERO_ERROR;
 * .     // get en_US collation rules
 * .     RuleBasedCollator* en_USCollation = 
 * .         (RuleBasedCollator*) Collator::createInstance(Locale::US, status);
 * .     // Always check the error code after each call.
 * .     if (U_FAILURE(status)) return;
 * .     // add a few Japanese character to sort before English characters
 * .     // suppose the last character before the first base letter 'a' in
 * .     // the English collation rule is 0x2212
 * .     UChar jaString[] = { '&', 0x2212, '&lt;', 0x3041, ',', 0x3042, '&lt;', 0x3043, ',', 0x3044, 0 };
 * .     UnicodeString rules( en_USCollation->getRules() );
 * .     rules += jaString;
 * .     RuleBasedCollator *myJapaneseCollation = new RuleBasedCollator(rules, status);
 * </pre>
 * <p><strong>NOTE</strong>: Typically, a collation object is created with
 * Collator::createInstance().
 * <p>
 * <strong>Note:</strong> <code>RuleBasedCollator</code>s with different Locale,
 * CollationStrength and Decomposition mode settings will return different
 * sort orders for the same set of strings. Locales have specific 
 * collation rules, and the way in which secondary and tertiary differences 
 * are taken into account, for example, will result in a different sorting order
 * for same strings.
 * <p>
 * @see        Collator
 * @version    1.27 4/8/97
 * @author     Helena Shih
 */
class U_I18N_API RuleBasedCollator : public Collator 
{
public: 

  // constructor/destructor
  /**
   * RuleBasedCollator constructor.  This takes the table rules and builds
   * a collation table out of them.  Please see RuleBasedCollator class
   * description for more details on the collation rule syntax.
   * @see Locale
   * @param rules the collation rules to build the collation table from.
   * @stable
   */
	RuleBasedCollator(const UnicodeString& rules,
			UErrorCode& status);

  RuleBasedCollator(  const   UnicodeString&  rules,
              ECollationStrength collationStrength,
              UErrorCode&      status);

  RuleBasedCollator(  const   UnicodeString&  rules,
              Normalizer::EMode decompositionMode,
              UErrorCode&      status);

  RuleBasedCollator(  const   UnicodeString&  rules,
              ECollationStrength collationStrength,
              Normalizer::EMode  decompositionMode,
              UErrorCode&      status);

  /** Destructor.
   * @stable
   */
	virtual ~RuleBasedCollator();


  /** Copy constructor.
   * @stable
   */
	RuleBasedCollator(const RuleBasedCollator& other);

  /**
   * Assignment operator.
   * @stable
   */
	RuleBasedCollator& operator=(const RuleBasedCollator& other);
    
  /**
   * Returns true if "other" is the same as "this".
   * @stable
   */
  virtual UBool                  operator==(const Collator& other) const;

  /**
   * Returns true if "other" is not the same as "this".
   * @stable
   */
  virtual UBool                  operator!=(const Collator& other) const;

  /**
   * Makes a deep copy of the object.  The caller owns the returned object.
   * @return the cloned object.
   * @stable
   */
  virtual Collator*               clone(void) const;

  /**
   * Creates a collation element iterator for the source string.  The 
   * caller of this method is responsible for the memory management of 
   * the return pointer.
   * @param source the string over which the CollationElementIterator will iterate.
   * @return the collation element iterator of the source string using this as
   * the based collator.
   * @stable 
   */
	virtual CollationElementIterator* createCollationElementIterator(const UnicodeString& source) const;

  /**
   * Creates a collation element iterator for the source.  The 
   * caller of this method is responsible for the memory management of 
   * the returned pointer.
   * @param source the CharacterIterator which produces the characters over which the
   * CollationElementItgerator will iterate.
   * @return the collation element iterator of the source using this as
   * the based collator.
   * @stable
   */
  virtual CollationElementIterator*       createCollationElementIterator(const CharacterIterator& source) const;

  /**
   * Compares a range of character data stored in two different strings
   * based on the collation rules.  Returns
   * information about whether a string is less than, greater than or
   * equal to another string in a language.
   * This can be overriden in a subclass.
   * @param source the source string.
   * @param target the target string to be compared with the source stirng.
   * @return the comparison result.  GREATER if the source string is greater
   * than the target string, LESS if the source is less than the target.  Otherwise,
   * returns EQUAL.
   * @stable
   */
  virtual     EComparisonResult   compare(    const   UnicodeString&  source, 
                          const   UnicodeString&  target) const;
        
        
  /**
   * Compares a range of character data stored in two different strings
   * based on the collation rules up to the specified length.  Returns
   * information about whether a string is less than, greater than or
   * equal to another string in a language.
   * This can be overriden in a subclass.
   * @param source the source string.
   * @param target the target string to be compared with the source string.
   * @param length compares up to the specified length
   * @return the comparison result.  GREATER if the source string is greater
   * than the target string, LESS if the source is less than the target.  Otherwise,
   * returns EQUAL.
   * @draft
   */ 
  virtual     EComparisonResult   compare(    const   UnicodeString&  source, 
                          const   UnicodeString&  target,
                          int32_t length) const;

  /**
   * The comparison function compares the character data stored in two
   * different string arrays.  Returns information about whether a string
   * array is less than, greater than or equal to another string array.
   * <p>Example of use:
   * <pre>
   * .       UErrorCode status = U_ZERO_ERROR;
   * .       Collator *myCollation = Collator::createInstance(Locale::US, status);
   * .       if (U_FAILURE(status)) return;
   * .       myCollation->setStrength(Collator::PRIMARY);
   * .       // result would be Collator::EQUAL ("abc" == "ABC")
   * .       // (no primary difference between "abc" and "ABC")
   * .       Collator::EComparisonResult result = myCollation->compare(L"abc", 3, L"ABC", 3);
   * .       myCollation->setStrength(Collator::TERTIARY);
   * .       // result would be Collator::LESS (abc" &lt;&lt;&lt; "ABC")
   * .       // (with tertiary difference between "abc" and "ABC")
   * .       Collator::EComparisonResult result = myCollation->compare(L"abc", 3, L"ABC", 3);
   * </pre>
   * @param source the source string array to be compared with.
   * @param sourceLength the length of the source string array.  If this value
   *        is equal to -1, the string array is null-terminated.
   * @param target the string that is to be compared with the source string.
   * @param targetLength the length of the target string array.  If this value
   *        is equal to -1, the string array is null-terminated.
   * @return Returns a byte value. GREATER if source is greater
   * than target; EQUAL if source is equal to target; LESS if source is less
   * than target
   * @stable
   **/
  virtual EComparisonResult   compare(    const   UChar* source, 
                      int32_t sourceLength,
                      const   UChar*  target,
                      int32_t targetLength) const ;

  /** Transforms a specified region of the string into a series of characters
     * that can be compared with CollationKey.compare. Use a CollationKey when
     * you need to do repeated comparisions on the same string. For a single comparison
     * the compare method will be faster.
     * @param source the source string.
     * @param key the transformed key of the source string.
     * @param status the error code status.
     * @return the transformed key.
     * @see CollationKey
     * @draft
     */
  virtual     CollationKey&       getCollationKey(    const   UnicodeString&  source,
                              CollationKey&   key,
                              UErrorCode&  status) const;

  /** Transforms a specified region of the string into a series of characters
     * that can be compared with CollationKey.compare. Use a CollationKey when
     * you need to do repeated comparisions on the same string. For a single comparison
     * the compare method will be faster.
     * @param source the source string.
     * @param key the transformed key of the source string.
     * @param status the error code status.
     * @return the transformed key.
     * @see CollationKey
     * @draft
     */
  virtual CollationKey&       getCollationKey(const UChar *source,
					      int32_t sourceLength,
					      CollationKey&       key,
					      UErrorCode&      status) const;

  /**
   * Generates the hash code for the rule-based collation object.
   * @return the hash code.
   * @stable
   */
  virtual     int32_t             hashCode(void) const;

  /**
   * Gets the table-based rules for the collation object.
   * @return returns the collation rules that the table collation object
   * was created from.
   * @stable
   */
  const       UnicodeString&      getRules(void) const;

  /**
   *  Return the maximum length of any expansion sequences that end
   *  with the specified comparison order.
   * 
   *  @param order a collation order returned by previous or next.
   *  @return the maximum length of any expansion seuences ending
   *          with the specified order.
   * 
   *  @see CollationElementIterator#getMaxExpansion
   * @draft ? pending CollationElementIterator::getMaxExpansion
   */
	int32_t getMaxExpansion(int32_t order) const;

  /**
   * Returns a unique class ID POLYMORPHICALLY.  Pure virtual override.
   * This method is to implement a simple version of RTTI, since not all
   * C++ compilers support genuine RTTI.  Polymorphic operator==() and
   * clone() methods call this method.
   *
   * @return          The class ID for this object. All objects of a
   *                  given class have the same class ID.  Objects of
   *                  other classes have different class IDs.
   */
  virtual UClassID getDynamicClassID(void) const
    { return RuleBasedCollator::getStaticClassID(); }


  /**
   * Returns the class ID for this class.  This is useful only for
   * comparing to a return value from getDynamicClassID().  For example:
   *
   *      Base* polymorphic_pointer = createPolymorphicObject();
   *      if (polymorphic_pointer->getDynamicClassID() ==
   *          Derived::getStaticClassID()) ...
   *
   * @return          The class ID for all objects of this class.
   */
  static UClassID getStaticClassID(void) { return (UClassID)&fgClassID; }


  /**
   * Returns the binary format of the class's rules.  The format is
   * that of .col files.  
   *
   * @param length Returns the length of the data, in bytes
   * @param status the error code status.
   * @return memory, owned by the caller, of size 'length' bytes.
   * @draft INTERNAL USE ONLY
   */
  uint8_t *cloneRuleData(int32_t &length, UErrorCode &status);

	/**
	 * Returns current rules. Delta defines whether full rules are returned or just the tailoring. 
	 * @param delta one of 	UCOL_TAILORING_ONLY, UCOL_FULL_RULES. 
	 * @return UnicodeString with rules
	 */
	UnicodeString getRules(UColRuleOption delta);

  /* New APIs for 1.7. Not yet implemented */

/**
 * Universal attribute setter
 * @param attr attribute type 
 * @param value attribute value
 * @param status to indicate whether the operation went on smoothly or there were errors
 * @draft API 1.7 freeze
 */
virtual void setAttribute(const UColAttribute attr, const UColAttributeValue value, UErrorCode &status);

/**
 * Universal attribute getter
 * @param attr attribute type
 * @param status to indicate whether the operation went on smoothly or there were errors
 * @return attribute value
 * @draft API 1.7 freeze
 */
virtual UColAttributeValue getAttribute(const UColAttribute attr, UErrorCode &status);

/**
 * Thread safe cloning operation
 * @return pointer to the new clone, user should remove it.
 * @draft API 1.7 freeze
 */
virtual Collator* safeClone(void);


/**
 * String compare that uses user supplied character iteration.
 * The idea is to prevent users from having to convert the whole string into UChar's before comparing
 * since sometimes strings differ on first couple of characters.
 * @param coll collator to be used for comparing
 * @param source pointer to function for iterating over the first string
 * @param target pointer to function for iterating over the second string
 * @return The result of comparing the strings; one of UCOL_EQUAL,
 * UCOL_GREATER, UCOL_LESS
 * @draft API 1.7 freeze
 */
virtual EComparisonResult compare(ForwardCharacterIterator &source,
								 ForwardCharacterIterator &target);

/**
 * Get the sort key as an array of bytes from an UnicodeString.
 * @param source string to be processed.
 * @param result buffer to store result in. If NULL, number of bytes needed will be returned.
 * @param resultLength length of the result buffer. If if not enough the buffer will be filled to capacity. 
 * @return Number of bytes needed for storing the sort key
 * @draft API 1.7 freeze
 */
  virtual int32_t       getSortKey(const   UnicodeString&  source,
						  uint8_t *result,
						  int32_t resultLength) const;

/**
 * Get the sort key as an array of bytes from an UChar buffer.
 * @param source string to be processed.
 * @param sourceLength length of string to be processed. 
 *			If -1, the string is 0 terminated and length will be decided by the function.
 * @param result buffer to store result in. If NULL, number of bytes needed will be returned.
 * @param resultLength length of the result buffer. If if not enough the buffer will be filled to capacity. 
 * @return Number of bytes needed for storing the sort key
 * @draft API 1.7 freeze
 */
  virtual int32_t       getSortKey(const   UChar *source,
						  int32_t sourceLength,
						  uint8_t *result,
						  int32_t resultLength) const;


 /*****************************************************************************
 * PRIVATE
 *****************************************************************************/
private:
  static      char                fgClassID;

        // Streamer used to read/write binary collation data files.
  friend        class                RuleBasedCollatorStreamer;

  // Used to iterate over collation elements in a character source.
  friend      class               CollationElementIterator;
        
  // Collator ONLY needs access to RuleBasedCollator(const Locale&, UErrorCode&)
  friend class Collator;
        
// This is just temporary, for prototyping.
  friend int32_t ucol_getNextCE(const UCollator *coll, collIterate *source, UErrorCode *status);
  friend int32_t ucol_getSortKey(const    UCollator    *coll, const    UChar        *source,
        int32_t        sourceLength, uint8_t        *result, int32_t        resultLength);
  friend int32_t ucol_getSortKeyEx(const    UCollator    *coll, const    UChar        *source,
        int32_t        sourceLength, uint8_t        *result, int32_t        resultLength);


  /** Default constructor
   */
  RuleBasedCollator();

  /**
   * Create a new entry in the expansion table that contains the orderings
   * for the given characers.  If anOrder is valid, it is added to the
   * beginning of the expanded list of orders.
   */
  int32_t                addExpansion(int32_t anOrder,
                             const UnicodeString &expandChars);
  /**
   * Create a table-based collation object with the given rules.
   * @see RuleBasedCollator#RuleBasedCollator
   * @exception FormatException If the rules format is incorrect.
   */
  void                build(  const   UnicodeString&  rules,
                  UErrorCode&      success);

  /** Add expanding entries for pre-composed unicode characters so that this
   * collator can be used reasonably well with decomposition turned off.
   */
  void                addComposedChars(void);

  /**
   * Look up for unmapped values in the expanded character table.
   */
  void                commit(void);
  /**
   * Increment of the last order based on the collation strength.
   * @param s the collation strength.
   * @param lastOrder the last collation order.
   * @return the new collation order.
   */
  int32_t             increment(  Collator::ECollationStrength    s, 
                  int32_t                         lastOrder);
  /**
   * Adds a character and its designated order into the collation table.
   * @param ch the Unicode character,
   * @param anOrder the order.
   * @param status the error code status.
   */
  void                addOrder(   UChar        ch, 
                  int32_t        anOrder, 
                  UErrorCode&  status);
  /**
   * Adds the expanding string into the collation table, for example, a-umlaut in German.
   * @param groupChars the contracting characters.
   * @param expChars the expanding characters.
   * @param anOrder the order.
   * @param status the error code status.
   */
  void                addExpandOrder(const    UnicodeString&          groupChars, 
                     const    UnicodeString&          expChars, 
                     int32_t                            anOrder,
                     UErrorCode&                       status);
  /**
   * Adds the contracting string into the collation table, for example, ch in Spanish.
   * @param groupChars the contracting characters.
   * @param anOrder the order.
   * @param status the error code status.
   */
  void                addContractOrder(const  UnicodeString&          groupChars, 
                       int32_t                        anOrder,
                       UErrorCode&                     status);
  /**
   * Adds the contracting string into the collation table, for example, ch in Spanish.
   * @param groupChars the contracting characters.
   * @param anOrder the order.
   * @param fwd TRUE if this is for the forward direction
   * @param status the error code status.
   */
  void                addContractOrder(const  UnicodeString&          groupChars, 
                       int32_t                        anOrder,
                       UBool                            fwd,
                       UErrorCode&                     status);
  /**
   * If the given string has been specified as a contracting string
   * in this collation table, return its ordering, otherwise return UNMAPPED.
   * @param groupChars the string
   * @return the order of the contracted character, or UNMAPPED if
   * there isn't one.
   */
  int32_t                getContractOrder(const    UnicodeString            &groupChars) const;
  /**
   *  Gets the entry of list of the contracting string in the collation
   *  table.
   *  @param ch the starting character of the contracting string
   *  @return the entry of contracting element which starts with the specified
   *  character in the list of contracting elements.
   */
  VectorOfPToContractElement* 
  getContractValues(UChar     ch) const;
  /**
   *  Ges the entry of list of the contracting string in the collation
   *  table.
   *  @param index the index of the contract character list
   *  @return the entry of the contracting element of the specified index in the
   *  list.
   */
  VectorOfPToContractElement* 
  getContractValues(int32_t     index) const;
  /**
   *  Gets the entry of value list of the expanding string in the collation
   *  table at the specified index.
   *  @param order the order of the expanding string value list
   *  @return the entry of the expanding-char element of the specified index in 
   *  the list.
   */
  VectorOfInt*        getExpandValueList(int32_t     order) const;

  /**
   *  Gets the comarison order of a character from the collation table.
   *  @param ch the Unicode character
   *  @return the comparison order of a character.
   */
  int32_t                getCharOrder(UChar ch) const;

  /**
   *  Gets the comarison order of a character from the collation table.
   *  @param list the contracting element table.
   *  @param name the contracting char string.
   *  @return the comparison order of the contracting character.
   */
  static        int32_t             getEntry(   VectorOfPToContractElement*     list, 
                        const   UnicodeString&          name,
                        UBool                    fwd);

  /**
   * Flattens the given object persistently to a file.  The file name
   * argument should be a path name that can be passed directly to the
   * underlying OS.  Once a RuleBasedCollator has been written to a file,
   * it can be resurrected by calling the RuleBasedCollator(const char*)
   * constructor, which operates very quickly.
   * @param fileName the output file name.
   * @return TRUE if writing to the file was successful, FALSE otherwise.
   */
  UBool              writeToFile(const char* fileName) const; // True on success
  /*  UBool              prepareForBundle() const;*/

  /**
   * Add this table collation to the cache.  This involves adding the
   * enclosed TableCollationData to the cache, and then marking our
   * pointer as "not owned" by setting dataIsOwned to false.
   * @param key the unique that represents this collation data object.
   */
  void                addToCache(         const UnicodeString& key);

  /**
   * RuleBasedCollator constructor.  This constructor takes a locale.  The only
   * caller of this class should be Collator::createInstance().  If createInstance()
   * happens to know that the requested locale's collation is implemented as
   * a RuleBasedCollator, it can then call this constructor.  OTHERWISE IT SHOULDN'T,
   * since this constructor ALWAYS RETURNS A VALID COLLATION TABLE.  It does this
   * by falling back to defaults.
   */
  RuleBasedCollator(      const Locale& desiredLocale,
              UErrorCode& status);
  /**
   * Internal constructFromXyx() methods.  These methods do object construction
   * from various sources.  They act like assignment operators; whatever used
   * to be in this object is discarded.  <P>FROM RULES.  This constructor turns
   * around and calls build().  <P>FROM CACHE.  This constructor tries to get the
   * requested cached TableCollationData object, and wrap us around it.  <P>FROM FILE.
   * There are two constructors named constructFromFile().  One takes a const char*:
   * this is a path name to be passed directly to the host OS, where a flattened
   * table collation (produced by writeToFile()) resides.  The other method takes
   * a Locale, and a UnicodeString locale file name.  The distinction is this:
   * the Locale is the locale we are seeking.  The file name is the name of the
   * data file (either binary, as produced by writeToFile(), or ASCII, as read
   * by ResourceBundle).  Within the file, if it is found, the method will look
   * for the given Locale.
   */
  void                constructFromRules( const UnicodeString& rules,
                      UErrorCode& status);
  void                constructFromFile(  const Locale&           locale,
                      const UnicodeString&    localeFileName,
                      UBool                  tryBinaryFile,
                      UErrorCode&              status);
  void                constructFromFile(  const char* fileName,
                      UErrorCode& status);
  void                constructFromCache( const UnicodeString& key,
                      UErrorCode& status);
  const char*         constructFromBundle(const Locale& fileName,
					  UErrorCode& status);


  //--------------------------------------------------------------------------
  // Internal Static Utility Methods
  /**
   * Creates the path name with given information.
   * @param prefix the prefix of the file name.
   * @param name the actual file name.
   * @param suffix the suffix of the file name.
   * @return the generated file name.
   */
  static  char*               createPathName( const UnicodeString&    prefix,
                          const UnicodeString&    name,
                          const UnicodeString&    suffix);

  int32_t getStrengthOrder(NormalizerIterator* cursor, 
                                    UErrorCode status) const;
  VectorOfInt* makeReorderedBuffer(NormalizerIterator* cursor,
                                   UChar colFirst,
                                   int32_t lastValue,
                                   VectorOfInt* lastExpansion) const;
  int32_t strengthOrder(int32_t value) const ;
  int32_t nextContractChar(NormalizerIterator *cursor, 
                           UChar ch,
                           UErrorCode& status) const;
  /**
     * Chops off the last portion of the locale name.  For example, from "en_US_CA"
     * to "en_US" and "en_US" to "en".
     * @param localeName the locale name.
     */
  static  void                chopLocale(UnicodeString&   localeName);

  //--------------------------------------------------------------------------
  // Constants

  static  const   int32_t             UNMAPPED;
  static  const   int32_t             CHARINDEX;  // need look up in .commit()
  static  const   int32_t             EXPANDCHARINDEX; // Expand index follows
  static  const   int32_t             CONTRACTCHARINDEX;  // contract indexes follow

  static  const   int32_t             PRIMARYORDERINCREMENT;
  static  const   int32_t             MAXIGNORABLE;
  static  const   int32_t             SECONDARYORDERINCREMENT;
  static  const   int32_t             TERTIARYORDERINCREMENT;
  static  const   int32_t             PRIMARYORDERMASK;
  static  const   int32_t             SECONDARYORDERMASK;
  static  const   int32_t             TERTIARYORDERMASK;
  static  const   int32_t             SECONDARYRESETMASK;
  static  const   int32_t             IGNORABLEMASK;
  static  const   int32_t             PRIMARYDIFFERENCEONLY;
  static  const   int32_t             SECONDARYDIFFERENCEONLY;
  static  const   int32_t             PRIMARYORDERSHIFT;
  static  const   int32_t             SECONDARYORDERSHIFT;
  static  const   int32_t             SORTKEYOFFSET;
  static  const   int32_t             CONTRACTCHAROVERFLOW;

  static const int16_t                FILEID;

  static       UnicodeString      DEFAULTRULES;

  static  const char*             kFilenameSuffix;

        //--------------------------------------------------------------------------
        // Data Members

  UBool              isOverIgnore;
  UChar             lastChar;
  MergeCollation*     mPattern;
  UnicodeString       sbuffer;
  UnicodeString       tbuffer;
  UnicodeString       key;
  NormalizerIterator  *cursor1;
  NormalizerIterator  *cursor2;
  UBool              dataIsOwned;
  TableCollationData* data;
};

inline UBool
RuleBasedCollator::operator!=(const Collator& other) const
{
  return !(*this == other);
}

inline void
RuleBasedCollator::addContractOrder(const UnicodeString &groupChars,
                    int32_t                anOrder,
                    UErrorCode            &status)
{
  addContractOrder(groupChars, anOrder, TRUE, status);
}




#endif
