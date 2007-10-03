/*
*******************************************************************************
* Copyright (C) 2007, International Business Machines Corporation and
* others. All Rights Reserved.
*******************************************************************************
*
*
* File PLURRULE.H
*
* Modification History:*
*   Date        Name        Description
*
********************************************************************************
*/

#ifndef PLURRULE
#define PLURRULE

/**
 * \file
 * \brief C++ API: PluralRules object
 */
 
#if !UCONFIG_NO_FORMATTING

#include "unicode/format.h"
#include "unicode/utypes.h"

U_NAMESPACE_BEGIN

class Hashtable;
class RuleChain;
class RuleParser;

   /**
    * Defines rules for mapping positive long values onto a small set of
    * keywords. Rules are constructed from a text description, consisting
    * of a series of keywords and conditions.  The {@link #select} method
    * examines each condition in order and returns the keyword for the
    * first condition that matches the number.  If none match,
    * default rule(other) is returned.
    * 
    * Examples:<pre>
    *   "one: n is 1; few: n in 2..4"</pre>
    *  This defines two rules, for 'one' and 'few'.  The condition for
    *  'one' is "n is 1" which means that the number must be equal to
    *  1 for this condition to pass.  The condition for 'few' is
    *  "n in 2..4" which means that the number must be between 2 and
    *  4 inclusive for this condition to pass.  All other numbers
    *  are assigned the keyword "other" by the default rule.
    *  </p><pre>
    *    "zero: n is 0; one: n is 1; zero: n mod 100 in 1..19"</pre>
    *  This illustrates that the same keyword can be defined multiple times.
    *  Each rule is examined in order, and the first keyword whose condition
    *  passes is the one returned.  Also notes that a modulus is applied
    *  to n in the last rule.  Thus its condition holds for 119, 219, 319...
    *  </p><pre>
    *    "one: n is 1; few: n mod 10 in 2..4 and n mod 100 not in 12..14"</pre>
    *  This illustrates conjunction and negation.  The condition for 'few'
    *  has two parts, both of which must be met: "n mod 10 in 2..4" and
    *  "n mod 100 not in 12..14".  The first part applies a modulus to n
    *  before the test as in the previous example.  The second part applies
    *  a different modulus and also uses negation, thus it matches all
    *  numbers _not_ in 12, 13, 14, 112, 113, 114, 212, 213, 214...
    */
class U_I18N_API PluralRules : public UObject {
public:
    
    /**
     * Constructor.
     * @param status  Output param set to success/failure code on exit, which
     *                must not indicate a failure before the function call.
     * 
     * @draft ICU 4.0
     */
    PluralRules(UErrorCode& status);
    
    /**
     * Copy constructor.
     * @draft ICU 4.0
     */
    PluralRules(const PluralRules& other);

    /**
     * Destructor.
     * @draft ICU 4.0
     */
    virtual ~PluralRules();
    
    /**
     * Clone
     * @draft ICU 4.0
     */
    PluralRules* clone() const;

    /**
      * Assignment operator.
      * @draft ICU 4.0
      */
    PluralRules& operator=(const PluralRules&);
    
    /**
     * Creates a PluralRules from a description if it is parsable, otherwise 
     * returns null.
     * 
     * @param description rule description
     * @param status      Output param set to success/failure code on exit, which
     *                    must not indicate a failure before the function call.
     * @return            new PluralRules pointer. NULL if there is an error.
     * @draft ICU 4.0
     */
    static PluralRules* U_EXPORT2 createRules(const UnicodeString& description,
                                              UErrorCode& status);
    
    /**
     * The default rules that accept any number.
     * 
     * @param status  Output param set to success/failure code on exit, which
     *                must not indicate a failure before the function call.
     * @return        new PluralRules pointer. NULL if there is an error.
     * @draft ICU 4.0
     */
    static PluralRules* U_EXPORT2 createDefaultRules(UErrorCode& status);
    
    /**
     * Provides access to the predefined <code>PluralRules</code> for a given
     * locale.
     * 
     * @param locale  The locale for which a <code>PluralRules</code> object is
     *                returned.
     * @param status  Output param set to success/failure code on exit, which
     *                must not indicate a failure before the function call.
     * @return        The predefined <code>PluralRules</code> object pointer for
     *                this locale. If there's no predefined rules for this locale,
     *                the rules for the closest parent in the locale hierarchy
     *                that has one will  be returned.  The final fallback always
     *                returns the default 'other' rules.
     * @draft ICU 4.0
     */
    static PluralRules* U_EXPORT2 forLocale(const Locale& locale, UErrorCode& status);
    
    /**
     * Given a number, returns the keyword of the first rule that applies to
     * the number.  This function can be used with isKeyword* functions to
     * determine the keyword for default plural rules.
     * 
     * @param number  The number for which the rule has to be determined.
     * @return        The keyword of the selected rule.
     * @draft ICU 4.0
     */
     UnicodeString select(int32_t number) const;

     /**
      * Returns a list of all rule keywords used in this <code>PluralRules</code>
      * object.  The rule 'other' is always present by default.
      * 
      * @param status Output param set to success/failure code on exit, which
      *               must not indicate a failure before the function call.
      * @return       StringEnumeration with the keywords.
      *               The caller must delete the object.
      * @draft ICU 4.0
      */
      StringEnumeration* getKeywords(UErrorCode& status) const;
  
      /**
       * Returns TRUE if the given keyword is defined in this 
       * <code>PluralRules</code> object.
       * 
       * @param keyword  the input keyword.
       * @return         TRUE if the input keyword is defined.
       *                 Otherwise, return FALSE.
       * @draft ICU 4.0
       */    
      UBool isKeyword(const UnicodeString& keyword) const;
      
      /**
       * Returns TRUE if the given keyword is common name for 'zero' plural form.
       * 
       * @param keyword  the input keyword.
       * @return         TRUE if the input keyword is common name for 'zero'
       *                 plural form.
       *                 Otherwise, return FALSE.
       * @draft ICU 4.0
       */    
      UBool isKeywordZero(const UnicodeString& keyword) const;
        
      /**
       * Returns TRUE if the given keyword is common name for 'singular' plural form.
       * 
       * @param keyword  the input keyword.
       * @return         TRUE if the input keyword is for 'singular' plural form.
       *                 Otherwise, return FALSE.
       * @draft ICU 4.0
       */    
      UBool isKeywordOne(const UnicodeString& keyword) const;
      
      /**
       * Returns TRUE if the given keyword is common name for 'dual' plural form.
       * 
       * @param keyword  the input keyword.
       * @return         TRUE if the input keyword is for 'dual' plural form.
       *                 Otherwise, return FALSE.
       * @draft ICU 4.0
       */    
      UBool isKeywordTwo(const UnicodeString& keyword) const;
      
      /**
       * Returns TRUE if the given keyword is common name for 'paucal' or other
       * special plural form.
       * 
       * @param keyword  the input keyword.
       * @return         TRUE if the input keyword is for 'paucal' or other
       *                 special plural form.
       *                 Otherwise, return FALSE.
       * @draft ICU 4.0
       */    
      UBool isKeywordFew(const UnicodeString& keyword) const;
      
      
      /**
       * Returns TRUE if the given keyword is common name for arabic (11 to 99)
       * plural form.
       * 
       * @param keyword  the input keyword.
       * @return         TRUE if the input keyword is for arabic (11 to 99)
       *                 plural form.
       *                 Otherwise, return FALSE.
       * @draft ICU 4.0
       */    
      UBool isKeywordMany(const UnicodeString& keyword) const;
      
      
      /**
       * Returns TRUE if the given keyword is default plural form.  The default
       * rule is applied if there is no other form in the rule applies.  It 
       * can additionally be assigned rules of its own.
       * 
       * @param keyword  the input keyword.
       * @return         TRUE if the input keyword is for default plural form.
       *                 Otherwise, return FALSE.
       * @draft ICU 4.0
       */    
      UBool isKeywordOther(const UnicodeString& keyword) const;
      
      /**
       * Returns keyword for default plural form.
       * 
       * @return         keyword for default plural form.
       * @draft ICU 4.0
       */    
      UnicodeString getKeywordOther() const;
      
      /**
       * Compares the equality of two PluralRules objects.
       *
       * @param other The other PluralRules object to be compared with.
       * @return      True if the given PluralRules is the same as this 
       *              PluralRules; false otherwise.
       * @draft ICU 4.0
       */     
      virtual UBool operator==(const PluralRules& other) const;
      
      /**
       * Compares the inequality of two PluralRules objects.
       *
       * @param other The PluralRules object to be compared with.
       * @return      True if the given PluralRules is not the same as this 
       *              PluralRules; false otherwise.
       * @draft ICU 4.0
       */
      UBool operator!=(const PluralRules& other) const  {return !operator==(other);}

      
      /**
       * ICU "poor man's RTTI", returns a UClassID for this class.
       *
       * @draft ICU 4.0
       *
      */
      static UClassID U_EXPORT2 getStaticClassID(void);

      /**
       * ICU "poor man's RTTI", returns a UClassID for the actual class.
       *
       * @draft ICU 4.0
       */
      virtual UClassID getDynamicClassID() const;
      

private: 
    Hashtable       *fLocaleStringsHash;
    UnicodeString   localeName;
    RuleChain       *rules;
    RuleParser      *parser;
    
    PluralRules();   // default constructor not implemented
    void getRuleData(UErrorCode& status);   
    int32_t getRepeatLimit() const; 
    UErrorCode parseDescription(UnicodeString& ruleData, RuleChain& rules);
    void getNextLocale(const UnicodeString& localeData, int32_t* curIndex, UnicodeString& localeName);
    void addRules(RuleChain& rules, UErrorCode& err);
    void addRules(const UnicodeString& localeName, RuleChain& rules, UBool addToHash, UErrorCode& err);
    void initHashtable(UErrorCode& err);
    int32_t getNumberValue(const UnicodeString& token) const;
   
};

U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_FORMATTING */

#endif // _PLURRULE
//eof
