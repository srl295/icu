/*
* Copyright (C) 1999, International Business Machines Corporation and others. All Rights Reserved.
**********************************************************************
*   Date        Name        Description
*   11/17/99    aliu        Creation.
**********************************************************************
*/
#ifndef UNIFILT_H
#define UNIFILT_H

#include "unicode/unimatch.h"

/**
 * <code>UnicodeFilter</code> defines a protocol for selecting a
 * subset of the full range (U+0000 to U+FFFF) of Unicode characters.
 * Currently, filters are used in conjunction with classes like {@link
 * Transliterator} to only process selected characters through a
 * transformation.
 *
 * @see UnicodeFilterLogic
 * @draft
 */
class U_I18N_API UnicodeFilter : public UnicodeMatcher {

public:
    /**
     * Destructor
     * @draft */
    virtual ~UnicodeFilter();

    /**
     * Returns <tt>true</tt> for characters that are in the selected
     * subset.  In other words, if a character is <b>to be
     * filtered</b>, then <tt>contains()</tt> returns
     * <b><tt>false</tt></b>.
     * @draft
     */
    virtual UBool contains(UChar32 c) const = 0;

    /**
     * Returns a copy of this object.  All UnicodeFilter objects have
     * to support cloning in order to allow classes using
     * UnicodeFilters, such as Transliterator, to implement cloning.
     * @draft
     */
    virtual UnicodeFilter* clone() const = 0;

    /**
     * Implement UnicodeMatcher API.
     */
    virtual UMatchDegree matches(const Replaceable& text,
                                 int32_t& offset,
                                 int32_t limit,
                                 UBool incremental) const;

protected:

    UnicodeFilter();
};

inline UnicodeFilter::UnicodeFilter() {}
inline UnicodeFilter::~UnicodeFilter() {}

#endif
