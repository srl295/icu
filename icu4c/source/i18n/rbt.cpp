/*
**********************************************************************
*   Copyright (C) 1999, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*   Date        Name        Description
*   11/17/99    aliu        Creation.
**********************************************************************
*/
#include "unicode/rbt.h"
#include "rbt_pars.h"
#include "rbt_data.h"
#include "rbt_rule.h"
#include "unicode/rep.h"

char RuleBasedTransliterator::fgClassID = 0; // Value is irrelevant

void RuleBasedTransliterator::_construct(const UnicodeString& rules,
                                         UTransDirection direction,
                                         UErrorCode& status,
                                         UParseError* parseError) {
    data = 0;
    isDataOwned = TRUE;
    if (U_SUCCESS(status)) {
        data = TransliteratorParser::parse(rules, direction, parseError);
        if (data == 0) {
            status = U_ILLEGAL_ARGUMENT_ERROR;
        } else {
            setMaximumContextLength(data->ruleSet.getMaximumContextLength());
        }
    }
}

RuleBasedTransliterator::RuleBasedTransliterator(const UnicodeString& id,
                                 const TransliterationRuleData* theData,
                                 UnicodeFilter* adoptedFilter) :
    Transliterator(id, adoptedFilter),
    data((TransliterationRuleData*)theData), // cast away const
    isDataOwned(FALSE) {
    setMaximumContextLength(data->ruleSet.getMaximumContextLength());
}

/**
 * Internal constructor.
 */
RuleBasedTransliterator::RuleBasedTransliterator(const UnicodeString& id,
                                                 TransliterationRuleData* theData,
                                                 UBool isDataAdopted) :
    Transliterator(id, 0),
    data(theData),
    isDataOwned(isDataAdopted) {
    setMaximumContextLength(data->ruleSet.getMaximumContextLength());
}

/**
 * Copy constructor.  Since the data object is immutable, we can share
 * it with other objects -- no need to clone it.
 */
RuleBasedTransliterator::RuleBasedTransliterator(
        const RuleBasedTransliterator& other) :
    Transliterator(other), data(other.data),
    isDataOwned(other.isDataOwned) {

    // Only do a deep copy if this is non-owned data, that is,
    // data that will be later deleted.  System transliterators
    // contain owned data.
    if (isDataOwned) {
        data = new TransliterationRuleData(*other.data);
    }
}

/**
 * Destructor.  We do NOT own the data object, so we do not delete it.
 */
RuleBasedTransliterator::~RuleBasedTransliterator() {
    if (isDataOwned) {
        delete data;
    }
}

Transliterator* // Covariant return NOT ALLOWED (for portability)
RuleBasedTransliterator::clone(void) const {
    return new RuleBasedTransliterator(*this);
}

/**
 * Implements {@link Transliterator#handleTransliterate}.
 */
void
RuleBasedTransliterator::handleTransliterate(Replaceable& text, UTransPosition& index,
                                             UBool isIncremental) const {
    /* We keep start and limit fixed the entire time,
     * relative to the text -- limit may move numerically if text is
     * inserted or removed.  The cursor moves from start to limit, with
     * replacements happening under it.
     *
     * Example: rules 1. ab>x|y
     *                2. yc>z
     *
     * |eabcd   start - no match, advance cursor
     * e|abcd   match rule 1 - change text & adjust cursor
     * ex|ycd   match rule 2 - change text & adjust cursor
     * exz|d    no match, advance cursor
     * exzd|    done
     */

    /* A rule like
     *   a>b|a
     * creates an infinite loop. To prevent that, we put an arbitrary
     * limit on the number of iterations that we take, one that is
     * high enough that any reasonable rules are ok, but low enough to
     * prevent a server from hanging.  The limit is 16 times the
     * number of characters n, unless n is so large that 16n exceeds a
     * uint32_t.
     */
    uint32_t loopCount = 0;
    uint32_t loopLimit = index.limit - index.start;
    if (loopLimit >= 0x10000000) {
        loopLimit = 0xFFFFFFFF;
    } else {
        loopLimit <<= 4;
    }

    UBool isPartial = FALSE;

    while (index.start < index.limit && loopCount <= loopLimit) {
        TransliterationRule* r = isIncremental ?
            data->ruleSet.findIncrementalMatch(text, index, *data, isPartial) :
            data->ruleSet.findMatch(text, index, *data);

        /* If we match a rule then apply it by replacing the key
         * with the rule output and repositioning the cursor
         * appropriately.  If we get a partial match, then we
         * can't do anything without more text; return with the
         * cursor at the current position.  If we get null, then
         * there is no match at this position, and we can advance
         * the cursor.
         */
        if (r == 0) {
            if (isPartial) { // always FALSE unless isIncremental
                break;
            } else {
                ++index.start;
            }
        } else {
            // Delegate replacement to TransliterationRule object
            int32_t lenDelta = r->replace(text, index.start, *data);
            index.limit += lenDelta;
            index.contextLimit += lenDelta;
            index.start += r->getCursorPos();
            ++loopCount;
        }
    }
}

UnicodeString& RuleBasedTransliterator::toRules(UnicodeString& rulesSource,
                                                UBool escapeUnprintable) const {
    return data->ruleSet.toRules(rulesSource, *data, escapeUnprintable);
}
