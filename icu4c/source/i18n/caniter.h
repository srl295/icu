/*
 *******************************************************************************
 * Copyright (C) 1996-2000, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 *
 * $Source: /xsrl/Nsvn/icu/icu/source/i18n/Attic/caniter.h,v $ 
 * $Date: 2002/03/13 18:29:24 $ 
 * $Revision: 1.2 $
 *
 *****************************************************************************************
 */

#ifndef CANITER_H
#define CANITER_H

#include "unicode/unistr.h"
#include "unicode/uniset.h"
#include "unicode/normlzr.h"
#include "unicode/unicode.h"

U_NAMESPACE_BEGIN

class Hashtable;

/**
 * This class allows one to iterate through all the strings that are canonically equivalent to a given
 * string. For example, here are some sample results:
Results for: {LATIN CAPITAL LETTER A WITH RING ABOVE}{LATIN SMALL LETTER D}{COMBINING DOT ABOVE}{COMBINING CEDILLA}
1: \u0041\u030A\u0064\u0307\u0327
 = {LATIN CAPITAL LETTER A}{COMBINING RING ABOVE}{LATIN SMALL LETTER D}{COMBINING DOT ABOVE}{COMBINING CEDILLA}
2: \u0041\u030A\u0064\u0327\u0307
 = {LATIN CAPITAL LETTER A}{COMBINING RING ABOVE}{LATIN SMALL LETTER D}{COMBINING CEDILLA}{COMBINING DOT ABOVE}
3: \u0041\u030A\u1E0B\u0327
 = {LATIN CAPITAL LETTER A}{COMBINING RING ABOVE}{LATIN SMALL LETTER D WITH DOT ABOVE}{COMBINING CEDILLA}
4: \u0041\u030A\u1E11\u0307
 = {LATIN CAPITAL LETTER A}{COMBINING RING ABOVE}{LATIN SMALL LETTER D WITH CEDILLA}{COMBINING DOT ABOVE}
5: \u00C5\u0064\u0307\u0327
 = {LATIN CAPITAL LETTER A WITH RING ABOVE}{LATIN SMALL LETTER D}{COMBINING DOT ABOVE}{COMBINING CEDILLA}
6: \u00C5\u0064\u0327\u0307
 = {LATIN CAPITAL LETTER A WITH RING ABOVE}{LATIN SMALL LETTER D}{COMBINING CEDILLA}{COMBINING DOT ABOVE}
7: \u00C5\u1E0B\u0327
 = {LATIN CAPITAL LETTER A WITH RING ABOVE}{LATIN SMALL LETTER D WITH DOT ABOVE}{COMBINING CEDILLA}
8: \u00C5\u1E11\u0307
 = {LATIN CAPITAL LETTER A WITH RING ABOVE}{LATIN SMALL LETTER D WITH CEDILLA}{COMBINING DOT ABOVE}
9: \u212B\u0064\u0307\u0327
 = {ANGSTROM SIGN}{LATIN SMALL LETTER D}{COMBINING DOT ABOVE}{COMBINING CEDILLA}
10: \u212B\u0064\u0327\u0307
 = {ANGSTROM SIGN}{LATIN SMALL LETTER D}{COMBINING CEDILLA}{COMBINING DOT ABOVE}
11: \u212B\u1E0B\u0327
 = {ANGSTROM SIGN}{LATIN SMALL LETTER D WITH DOT ABOVE}{COMBINING CEDILLA}
12: \u212B\u1E11\u0307
 = {ANGSTROM SIGN}{LATIN SMALL LETTER D WITH CEDILLA}{COMBINING DOT ABOVE}
 *<br>Note: the code is intended for use with small strings, and is not suitable for larger ones,
 * since it has not been optimized for that situation.
 *@author M. Davis
 *@draft
 */
class U_I18N_API CanonicalIterator {
public:
    /**
     *@param source string to get results for
     */
    CanonicalIterator(UnicodeString source, UErrorCode status);    

    /** Destructor
     *  Cleans pieces
     */
    ~CanonicalIterator();

    /**
     *@return gets the source: NOTE: it is the NFD form of source
     */
    UnicodeString getSource();    

    /**
     * Resets the iterator so that one can start again from the beginning.
     */
    void reset();    

    /**
     *@return the next string that is canonically equivalent. The value null is returned when
     * the iteration is done.
     */
    UnicodeString next();    

    /**
     *@param set the source string to iterate against. This allows the same iterator to be used
     * while changing the source string, saving object creation.
     */
    void setSource(UnicodeString newSource, UErrorCode status);    

    /**
     * Dumb recursive implementation of permutation. 
     * TODO: optimize
     * @param source the string to find permutations for
     * @return the results in a set.
     */
    static Hashtable *permute(UnicodeString &source, UErrorCode status);     
    
private:
    // ===================== PRIVATES ==============================
    
    // fields
    UnicodeString source;
    UBool done;

    // 2 dimensional array holds the pieces of the string with
    // their different canonically equivalent representations
    UnicodeString **pieces;
    int32_t pieces_length;
    int32_t *pieces_lengths;

    // current is used in iterating to combine pieces
    int32_t *current;
    int32_t current_length;
    
    // transient fields
    UnicodeString buffer;
    
    // we have a segment, in NFD. Find all the strings that are canonically equivalent to it.
    UnicodeString *getEquivalents(UnicodeString segment, int32_t &result_len, UErrorCode status); //private String[] getEquivalents(String segment)
    
    //Set getEquivalents2(String segment);
    Hashtable *getEquivalents2(UnicodeString segment, UErrorCode status);
    
    /**
     * See if the decomposition of cp2 is at segment starting at segmentPos 
     * (with canonical rearrangment!)
     * If so, take the remainder, and return the equivalents 
     */
    //Set extract(int comp, String segment, int segmentPos, StringBuffer buffer);
    Hashtable *extract(UChar32 comp, UnicodeString segment, int32_t segmentPos, UnicodeString buffer, UErrorCode status);

    void cleanPieces();
};
U_NAMESPACE_END
#endif
