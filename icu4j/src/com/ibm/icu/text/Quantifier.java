/*
 *******************************************************************************
 * Copyright (C) 2001, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 *
 * $Source: /xsrl/Nsvn/icu/icu4j/src/com/ibm/icu/text/Quantifier.java,v $ 
 * $Date: 2001/10/04 18:24:15 $ 
 * $Revision: 1.1 $
 *
 *****************************************************************************************
 */
package com.ibm.text;

class Quantifier implements UnicodeMatcher {

    private UnicodeMatcher matcher;

    private int minCount;

    private int maxCount;

    /**
     * Maximum count a quantifier can have.
     */
    public static final int MAX = Integer.MAX_VALUE;

    public Quantifier(UnicodeMatcher theMatcher,
                      int theMinCount, int theMaxCount) {
        if (theMatcher == null || minCount < 0 || maxCount < 0 || minCount > maxCount) {
            throw new IllegalArgumentException();
        }
        matcher = theMatcher;
        minCount = theMinCount;
        maxCount = theMaxCount;
    }

    /**
     * Implement UnicodeMatcher API.
     */
    public int matches(Replaceable text,
                       int[] offset,
                       int limit,
                       boolean incremental) {
        int start = offset[0];
        int count = 0;
        while (count < maxCount) {
            int m = matcher.matches(text, offset, limit, incremental);
            if (m == U_MATCH) {
                ++count;
            } else if (incremental && m == U_PARTIAL_MATCH) {
                return U_PARTIAL_MATCH;
            } else {
                break;
            }
        }
        if (incremental && offset[0] == limit) {
            return U_PARTIAL_MATCH;
        }
        if (count >= minCount) {
            return U_MATCH;
        }
        offset[0] = start;
        return U_MISMATCH;
    }

    static private final int[] POW10 = {1, 10, 100, 1000, 10000, 100000, 1000000,
                                        10000000, 100000000, 1000000000};

    static private void appendNumber(StringBuffer result, int n) {
        // assert(n >= 0);
        // assert(n < 1e10);
        boolean show = false; // true if we should display digits
        for (int p=9; p>=0; --p) {
            int d = n / POW10[p];
            n -= d * POW10[p];
            if (d != 0 || p == 0) {
                show = true;
            }
            if (show) {
                result.append((char)('0'+d));
            }
        }
    }

    /**
     * Implement UnicodeMatcher API
     */
    public String toPattern(boolean escapeUnprintable) {
        StringBuffer result = new StringBuffer();
        result.append(matcher.toPattern(escapeUnprintable));
        if (minCount == 0) {
            if (maxCount == 1) {
                return result.append('?').toString();
            } else if (maxCount == MAX) {
                return result.append('*').toString();
            }
            // else fall through
        } else if (minCount == 1 && maxCount == MAX) {
            return result.append('+').toString();
        }
        result.append('{');
        appendNumber(result, minCount);
        result.append(',');
        if (maxCount != MAX) {
            appendNumber(result, maxCount);
        }
        result.append('}');
        return result.toString();
    }

    /**
     * Implement UnicodeMatcher API
     */
    public boolean matchesIndexValue(byte v) {
        return (minCount == 0) || matcher.matchesIndexValue(v);
    }
}

//eof
