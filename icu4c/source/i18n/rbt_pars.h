/*
* Copyright (C) {1999}, International Business Machines Corporation and others. All Rights Reserved.
**********************************************************************
*   Date        Name        Description
*   11/17/99    aliu        Creation.
**********************************************************************
*/
#ifndef RBT_PARS_H
#define RBT_PARS_H

#include "unicode/rbt.h"
#include "uvector.h"
#include "unicode/parseerr.h"

U_NAMESPACE_BEGIN

class TransliterationRuleData;
class UnicodeMatcher;
class ParseData;
class RuleHalf;
class ParsePosition;

class TransliteratorParser {

    /**
     * This is a reference to external data we don't own.  This works because
     * we only hold this for the duration of the call to parse().
     */
    const UnicodeString& rules;

    UTransDirection direction;

    TransliterationRuleData* data;

    /**
     * We use a single error code during parsing.  Rather than pass it
     * through each API, we keep it here.
     */
    UErrorCode status;

    /**
     * Pointer to user structure in which to return parse error information.
     * May be NULL.
     */
    UParseError& parseError;

    /**
     * Temporary symbol table used during parsing.
     */
    ParseData* parseData;

    /**
     * Temporary vector of matcher variables.  When parsing is complete, this
     * is copied into the array data.variables.  As with data.variables,
     * element 0 corresponds to character data.variablesBase.
     */
    UVector variablesVector;

    /**
     * The next available stand-in for variables.  This starts at some point in
     * the private use area (discovered dynamically) and increments up toward
     * <code>variableLimit</code>.  At any point during parsing, available
     * variables are <code>variableNext..variableLimit-1</code>.
     */
    UChar variableNext;

    /**
     * The last available stand-in for variables.  This is discovered
     * dynamically.  At any point during parsing, available variables are
     * <code>variableNext..variableLimit-1</code>.
     */
    UChar variableLimit;

    /**
     * When we encounter an undefined variable, we do not immediately signal
     * an error, in case we are defining this variable, e.g., "$a = [a-z];".
     * Instead, we save the name of the undefined variable, and substitute
     * in the placeholder char variableLimit - 1, and decrement
     * variableLimit.
     */
    UnicodeString undefinedVariableName;

public:

    static TransliterationRuleData*
        parse(const UnicodeString& rules,
              UTransDirection direction,
              UParseError& parseError,
              UErrorCode& ec);

    /**
     * Parse a given set of rules.  Return up to three pieces of
     * parsed data.  These are the header ::id block, the rule block,
     * and the footer ::id block.  Any or all of these may be empty.
     * If the ::id blocks are empty, their corresponding parameters
     * are returned as the empty string.  If there are no rules, the
     * TransliterationRuleData result is 0.
     * @param ruleDataResult caller owns the pointer stored here.
     * May be NULL.
     * @param headerRule string including semicolons for the header
     * ::id block.  May be empty.
     * @param footerRule string including semicolons for the footer
     * ::id block.  May be empty.
     */
    static void parse(const UnicodeString& rules,
                      UTransDirection direction,
                      TransliterationRuleData*& ruleDataResult,
                      UnicodeString& idBlockResult,
                      int32_t& idSplitPointResult,
                      UParseError& parseError,
                      UErrorCode& ec);

private:

    /**
     * @param rules list of rules, separated by newline characters
     * @exception IllegalArgumentException if there is a syntax error in the
     * rules
     */
    TransliteratorParser(const UnicodeString& rules,
                              UTransDirection direction,
                              UParseError& parseError);

    /**
     * Destructor.
     */
    ~TransliteratorParser();

    /**
     * Parse the given string as a sequence of rules, separated by newline
     * characters ('\n'), and cause this object to implement those rules.  Any
     * previous rules are discarded.  Typically this method is called exactly
     * once, during construction.
     * @exception IllegalArgumentException if there is a syntax error in the
     * rules
     */
    void parseRules(UnicodeString& idBlockResult, int32_t& idSplitPointResult,
                    int32_t& ruleCount);

    /**
     * MAIN PARSER.  Parse the next rule in the given rule string, starting
     * at pos.  Return the index after the last character parsed.  Do not
     * parse characters at or after limit.
     *
     * Important:  The character at pos must be a non-whitespace character
     * that is not the comment character.
     *
     * This method handles quoting, escaping, and whitespace removal.  It
     * parses the end-of-rule character.  It recognizes context and cursor
     * indicators.  Once it does a lexical breakdown of the rule at pos, it
     * creates a rule object and adds it to our rule list.
     */
    int32_t parseRule(int32_t pos, int32_t limit);

    /**
     * Called by main parser upon syntax error.  Search the rule string
     * for the probable end of the rule.  Of course, if the error is that
     * the end of rule marker is missing, then the rule end will not be found.
     * In any case the rule start will be correctly reported.
     * @param msg error description
     * @param rule pattern string
     * @param start position of first character of current rule
     */
    int32_t syntaxError(UErrorCode parseErrorCode, const UnicodeString&, int32_t start);

    /**
     * Parse a UnicodeSet out, store it, and return the stand-in character
     * used to represent it.
     */
    UChar parseSet(const UnicodeString& rule,
                   ParsePosition& pos);

    /**
     * Generate and return a stand-in for a new UnicodeMatcher.  Store
     * the matcher (adopt it).
     */
    UChar generateStandInFor(UnicodeMatcher* adopted);

    /**
     * Append the value of the given variable name to the given
     * UnicodeString.
     */
    void appendVariableDef(const UnicodeString& name,
                           UnicodeString& buf);

    /**
     * Return a stand-in character that refers to the given segments.
     * @param r a reference number >= 1
     * @return a stand-in for the given segment reference
     */
    UChar getSegmentStandin(int32_t r);

    /**
     * Determines what part of the private use region of Unicode we can use for
     * variable stand-ins.  The correct way to do this is as follows: Parse each
     * rule, and for forward and reverse rules, take the FROM expression, and
     * make a hash of all characters used.  The TO expression should be ignored.
     * When done, everything not in the hash is available for use.  In practice,
     * this method may employ some other algorithm for improved speed.
     */
    void determineVariableRange(void);

    /**
     * Returns the index of a character, ignoring quoted text.
     * For example, in the string "abc'hide'h", the 'h' in "hide" will not be
     * found by a search for 'h'.
     * @param text text to be searched
     * @param start the beginning index, inclusive; <code>0 <= start
     * <= limit</code>.
     * @param limit the ending index, exclusive; <code>start <= limit
     * <= text.length()</code>.
     * @param c character to search for
     * @return Offset of the first instance of c, or -1 if not found.
     */
    static int32_t quotedIndexOf(const UnicodeString& text,
                                 int32_t start, int32_t limit,
                                 UChar c);

    friend class RuleHalf;

    // Disallowed methods; no impl.
    TransliteratorParser(const TransliteratorParser&);
    TransliteratorParser& operator=(const TransliteratorParser&);
};

U_NAMESPACE_END

#endif
