/*
*******************************************************************************
* Copyright (C) 1997-2001, International Business Machines Corporation and others. All Rights Reserved.
*******************************************************************************
*/

#ifndef NFRULE_H
#define NFRULE_H

#include "unicode/rbnf.h"

#if U_HAVE_RBNF

#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "unicode/unistr.h"

U_NAMESPACE_BEGIN

class FieldPosition;
class Formattable;
class NFRuleList;
class NFRuleSet;
class NFSubstitution;
class ParsePosition;
class RuleBasedNumberFormat;
class UnicodeString;

class NFRule : public UObject {
public:

    enum ERuleType {
        kNoBase = 0,
        kNegativeNumberRule = -1,
        kImproperFractionRule = -2,
        kProperFractionRule = -3,
        kMasterRule = -4,
        kOtherRule = -5
    };

    static void makeRules(UnicodeString& definition,
                          const NFRuleSet* ruleSet, 
                          const NFRule* predecessor, 
                          const RuleBasedNumberFormat* rbnf, 
                          NFRuleList& ruleList,
                          UErrorCode& status);

    NFRule(const RuleBasedNumberFormat* rbnf);
    ~NFRule();

    UBool operator==(const NFRule& rhs) const;
    UBool operator!=(const NFRule& rhs) const { return !operator==(rhs); }

    ERuleType getType() const { return (ERuleType)(baseValue <= kNoBase ? (ERuleType)baseValue : kOtherRule); }
    void setType(ERuleType ruleType) { baseValue = (int32_t)ruleType; }

    int64_t getBaseValue() const { return baseValue; }
    void setBaseValue(int64_t value);

    double getDivisor() const { return uprv_pow(radix, exponent); }

    void doFormat(int64_t number, UnicodeString& toAppendTo, int32_t pos) const;
    void doFormat(double  number, UnicodeString& toAppendTo, int32_t pos) const;

    UBool doParse(const UnicodeString& text, 
                  ParsePosition& pos, 
                  UBool isFractional, 
                  double upperBound,
                  Formattable& result) const;

    UBool shouldRollBack(double number) const;

    void appendRuleText(UnicodeString& result) const;

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
    void parseRuleDescriptor(UnicodeString& descriptor, UErrorCode& status);
    void extractSubstitutions(const NFRuleSet* ruleSet, const NFRule* predecessor, const RuleBasedNumberFormat* rbnf, UErrorCode& status);
    NFSubstitution* extractSubstitution(const NFRuleSet* ruleSet, const NFRule* predecessor, const RuleBasedNumberFormat* rbnf, UErrorCode& status);
    
    int16_t expectedExponent() const;
    int32_t indexOfAny(const UChar* const strings[]) const;
    double matchToDelimiter(const UnicodeString& text, int32_t startPos, double baseValue,
                            const UnicodeString& delimiter, ParsePosition& pp, const NFSubstitution* sub, 
                            double upperBound) const;
    void stripPrefix(UnicodeString& text, const UnicodeString& prefix, ParsePosition& pp) const;

    int32_t prefixLength(const UnicodeString& str, const UnicodeString& prefix) const;
    UBool allIgnorable(const UnicodeString& str) const;
    int32_t findText(const UnicodeString& str, const UnicodeString& key, 
                     int32_t startingAt, int32_t* resultCount) const;

private:
    int64_t baseValue;
    int16_t radix;
    int16_t exponent;
    UnicodeString ruleText;
    NFSubstitution* sub1;
    NFSubstitution* sub2;
    const RuleBasedNumberFormat* formatter;

    /**
     * The address of this static class variable serves as this class's ID
     * for ICU "poor man's RTTI".
     */
    static const char fgClassID;
};

U_NAMESPACE_END

/* U_HAVE_RBNF */
#endif

// NFRULE_H
#endif

