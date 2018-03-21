// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
package com.ibm.icu.impl.number.parse;

import com.ibm.icu.text.UnicodeSet;
import com.ibm.icu.text.UnicodeSet.EntryRange;

/**
 * A collection of utility functions used by the number parsing package.
 */
public class ParsingUtils {

    public static final int PARSE_FLAG_IGNORE_CASE = 0x0001;
    public static final int PARSE_FLAG_MONETARY_SEPARATORS = 0x0002;
    public static final int PARSE_FLAG_STRICT_SEPARATORS = 0x0004;
    public static final int PARSE_FLAG_STRICT_GROUPING_SIZE = 0x0008;
    public static final int PARSE_FLAG_INTEGER_ONLY = 0x0010;
    public static final int PARSE_FLAG_GROUPING_DISABLED = 0x0020;
    public static final int PARSE_FLAG_FRACTION_GROUPING_DISABLED = 0x0040;
    public static final int PARSE_FLAG_INCLUDE_UNPAIRED_AFFIXES = 0x0080;
    public static final int PARSE_FLAG_USE_FULL_AFFIXES = 0x0100;
    public static final int PARSE_FLAG_EXACT_AFFIX = 0x0200;
    public static final int PARSE_FLAG_PLUS_SIGN_ALLOWED = 0x0400;
    // public static final int PARSE_FLAG_OPTIMIZE = 0x0800; // no longer used

    public static void putLeadCodePoints(UnicodeSet input, UnicodeSet output) {
        for (EntryRange range : input.ranges()) {
            output.add(range.codepoint, range.codepointEnd);
        }
        for (String str : input.strings()) {
            output.add(str.codePointAt(0));
        }
    }

    public static void putLeadCodePoint(String input, UnicodeSet output) {
        if (!input.isEmpty()) {
            output.add(input.codePointAt(0));
        }
    }

}
