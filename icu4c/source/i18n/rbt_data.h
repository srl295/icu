/*
* Copyright (C) {1999}, International Business Machines Corporation and others. All Rights Reserved.
**********************************************************************
*   Date        Name        Description
*   11/17/99    aliu        Creation.
**********************************************************************
*/
#ifndef RBT_DATA_H
#define RBT_DATA_H

#include "rbt_set.h"

U_NAMESPACE_BEGIN

class UnicodeFunctor;
class UnicodeString;
class UnicodeMatcher;
class UnicodeReplacer;
class Hashtable;

/**
 * The rule data for a RuleBasedTransliterators.  RBT objects hold
 * a const pointer to a TRD object that they do not own.  TRD objects
 * are essentially the parsed rules in compact, usable form.  The
 * TRD objects themselves are held for the life of the process in
 * a static cache owned by Transliterator.
 *
 * This class' API is a little asymmetric.  There is a method to
 * define a variable, but no way to define a set.  This is because the
 * sets are defined by the parser in a UVector, and the vector is
 * copied into a fixed-size array here.  Once this is done, no new
 * sets may be defined.  In practice, there is no need to do so, since
 * generating the data and using it are discrete phases.  When there
 * is a need to access the set data during the parse phase, another
 * data structure handles this.  See the parsing code for more
 * details.
 */
class TransliterationRuleData {

public:

    // PUBLIC DATA MEMBERS

    /**
     * Rule table.  May be empty.
     */
    TransliterationRuleSet ruleSet;

    /**
     * Map variable name (String) to variable (UnicodeString).  A variable name
     * corresponds to zero or more characters, stored in a UnicodeString in
     * this hash.  One or more of these chars may also correspond to a
     * UnicodeMatcher, in which case the character in the UnicodeString in this hash is
     * a stand-in: it is an index for a secondary lookup in
     * data.variables.  The stand-in also represents the UnicodeMatcher in
     * the stored rules.
     */
    Hashtable* variableNames;

    /**
     * Map category variable (UChar) to set (UnicodeFunctor).
     * Variables that correspond to a set of characters are mapped
     * from variable name to a stand-in character in data.variableNames.
     * The stand-in then serves as a key in this hash to lookup the
     * actual UnicodeFunctor object.  In addition, the stand-in is
     * stored in the rule text to represent the set of characters.
     * variables[i] represents character (variablesBase + i).
     */
    UnicodeFunctor** variables;

    /**
     * The character that represents variables[0].  Characters
     * variablesBase through variablesBase +
     * variablesLength - 1 represent UnicodeFunctor objects.
     */
    UChar variablesBase;

    /**
     * The length of variables.
     */
    int32_t variablesLength;

public:

    TransliterationRuleData(UErrorCode& status);

    TransliterationRuleData(const TransliterationRuleData&);

    ~TransliterationRuleData();

    /**
     * Given a stand-in character, return the UnicodeMatcher that it
     * represents, or NULL.
     */
    UnicodeMatcher* lookupMatcher(UChar32 standIn) const;

    /**
     * Given a stand-in character, return the UnicodeReplacer that it
     * represents, or NULL.
     */
    UnicodeReplacer* lookupReplacer(UChar32 standIn) const;
};

U_NAMESPACE_END

#endif
