// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
package com.ibm.icu.impl.number.parse;

import com.ibm.icu.text.UnicodeSet;

/**
 * A Matcher used only for post-process validation, not for consuming characters at runtime.
 */
public abstract class ValidationMatcher implements NumberParseMatcher {

    @Override
    public boolean match(StringSegment segment, ParsedNumber result) {
        return false;
    }

    @Override
    public boolean matchesEmpty() {
        return false;
    }

    @Override
    public UnicodeSet getLeadCodePoints() {
        return UnicodeSet.EMPTY;
    }

}
