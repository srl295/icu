/*
* Copyright (C) 2001, International Business Machines Corporation and others. All Rights Reserved.
**********************************************************************
*   Date        Name        Description
*   07/26/01    aliu        Creation.
**********************************************************************
*/

#include "quant.h"
#include "unicode/unistr.h"
#include "util.h"

U_NAMESPACE_BEGIN

Quantifier::Quantifier(UnicodeMatcher *adopted,
                       uint32_t _minCount, uint32_t _maxCount) {
    // assert(adopted != 0);
    // assert(minCount <= maxCount);
    matcher = adopted;
    this->minCount = _minCount;
    this->maxCount = _maxCount;
}

Quantifier::Quantifier(const Quantifier& o) :
    UnicodeMatcher(o),
    matcher(o.matcher->clone()),
    minCount(o.minCount),
    maxCount(o.maxCount)
{
}

Quantifier::~Quantifier() {
    delete matcher;
}

/**
 * Implement UnicodeMatcher
 */
UnicodeMatcher* Quantifier::clone() const {
    return new Quantifier(*this);
}

UMatchDegree Quantifier::matches(const Replaceable& text,
                                 int32_t& offset,
                                 int32_t limit,
                                 UBool incremental) {
    int32_t start = offset;
    uint32_t count = 0;
    while (count < maxCount) {
        int32_t pos = offset;
        UMatchDegree m = matcher->matches(text, offset, limit, incremental);
        if (m == U_MATCH) {
            ++count;
            if (pos == offset) {
                // If offset has not moved we have a zero-width match.
                // Don't keep matching it infinitely.
                break;
            }
        } else if (incremental && m == U_PARTIAL_MATCH) {
            return U_PARTIAL_MATCH;
        } else {
            break;
        }
    }
    if (incremental && offset == limit) {
        return U_PARTIAL_MATCH;
    }
    if (count >= minCount) {
        return U_MATCH;
    }
    offset = start;
    return U_MISMATCH;
}

/**
 * Implement UnicodeMatcher
 */
UnicodeString& Quantifier::toPattern(UnicodeString& result,
                                     UBool escapeUnprintable) const {
    matcher->toPattern(result, escapeUnprintable);
    if (minCount == 0) {
        if (maxCount == 1) {
            return result.append((UChar)63); /*?*/
        } else if (maxCount == MAX) {
            return result.append((UChar)42); /***/
        }
        // else fall through
    } else if (minCount == 1 && maxCount == MAX) {
        return result.append((UChar)43); /*+*/
    }
    result.append((UChar)123); /*{*/
    ICU_Utility::appendNumber(result, minCount);
    result.append((UChar)44); /*,*/
    if (maxCount != MAX) {
        ICU_Utility::appendNumber(result, maxCount);
    }
    result.append((UChar)125); /*}*/
    return result;
}

/**
 * Implement UnicodeMatcher
 */
UBool Quantifier::matchesIndexValue(uint8_t v) const {
    return (minCount == 0) || matcher->matchesIndexValue(v);
}

U_NAMESPACE_END

//eof
