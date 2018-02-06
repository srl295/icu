// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT

#include "numparse_types.h"
#include <cmath>

using namespace icu;
using namespace icu::numparse;
using namespace icu::numparse::impl;


ParsedNumber::ParsedNumber() {
    clear();
}

void ParsedNumber::clear() {
    quantity.bogus = true;
    charEnd = 0;
    flags = 0;
    prefix.setToBogus();
    suffix.setToBogus();
    currencyCode.setToBogus();
}

void ParsedNumber::setCharsConsumed(const StringSegment& segment) {
    charEnd = segment.getOffset();
}

bool ParsedNumber::success() const {
    return charEnd > 0 && 0 == (flags & FLAG_FAIL);
}

bool ParsedNumber::seenNumber() const {
    return !quantity.bogus || 0 != (flags & FLAG_NAN) || 0 != (flags & FLAG_INFINITY);
}

double ParsedNumber::getDouble() const {
    bool sawNegative = 0 != (flags & FLAG_NEGATIVE);
    bool sawNaN = 0 != (flags & FLAG_NAN);
    bool sawInfinity = 0 != (flags & FLAG_INFINITY);

    // Check for NaN, infinity, and -0.0
    if (sawNaN) {
        return NAN;
    }
    if (sawInfinity) {
        if (sawNegative) {
            return -INFINITY;
        } else {
            return INFINITY;
        }
    }
    if (quantity.isZero() && sawNegative) {
        return -0.0;
    }

    if (quantity.fitsInLong()) {
        long l = quantity.toLong();
        if (0 != (flags & FLAG_NEGATIVE)) {
            l *= -1;
        }
        return l;
    }

    // TODO: MIN_LONG
    return quantity.toDouble();
}



#endif /* #if !UCONFIG_NO_FORMATTING */
