// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
/*
**********************************************************************
* Copyright (c) 2004-2014, International Business Machines
* Corporation and others.  All Rights Reserved.
**********************************************************************
* Author: Alan Liu
* Created: April 20, 2004
* Since: ICU 3.0
**********************************************************************
*/
package com.ibm.icu.text;

import java.io.ObjectStreamException;
import java.text.FieldPosition;
import java.text.ParsePosition;

import com.ibm.icu.util.CurrencyAmount;
import com.ibm.icu.util.ULocale;

/**
 * Temporary internal concrete subclass of MeasureFormat implementing parsing and formatting of
 * CurrencyAmount objects. This class is likely to be redesigned and rewritten in the near future.
 *
 * <p>
 * This class currently delegates to DecimalFormat for parsing and formatting.
 *
 * @see com.ibm.icu.text.UFormat
 * @see com.ibm.icu.text.DecimalFormat
 * @author Alan Liu
 */
class CurrencyFormat extends MeasureFormat {
    // Generated by serialver from JDK 1.4.1_01
    static final long serialVersionUID = -931679363692504634L;

    public CurrencyFormat(ULocale locale) {
        super(locale, FormatWidth.DEFAULT_CURRENCY);
    }

    /**
     * Override Format.format().
     *
     * @see java.text.Format#format(java.lang.Object, java.lang.StringBuffer, java.text.FieldPosition)
     */
    @Override
    public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos) {
        if (!(obj instanceof CurrencyAmount)) {
            throw new IllegalArgumentException("Invalid type: " + obj.getClass().getName());
        }
        return super.format(obj, toAppendTo, pos);
    }

    /**
     * Override Format.parseObject().
     *
     * @see java.text.Format#parseObject(java.lang.String, java.text.ParsePosition)
     */
    @Override
    public CurrencyAmount parseObject(String source, ParsePosition pos) {
        return getNumberFormatInternal().parseCurrency(source, pos);
    }

    // Serialization

    private Object writeReplace() throws ObjectStreamException {
        return toCurrencyProxy();
    }

    // Preserve backward serialize backward compatibility.
    private Object readResolve() throws ObjectStreamException {
        return new CurrencyFormat(getLocale(ULocale.ACTUAL_LOCALE));
    }
}
