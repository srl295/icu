/*
 *******************************************************************************
 * Copyright (C) 2008-2014, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 */
package com.ibm.icu.dev.test.format;

import com.ibm.icu.dev.test.TestFmwk;
import com.ibm.icu.text.MeasureFormat;
import com.ibm.icu.text.MeasureFormat.FormatWidth;
import com.ibm.icu.text.PluralRanges;
import com.ibm.icu.text.PluralRules;
import com.ibm.icu.text.PluralRules.Factory;
import com.ibm.icu.text.PluralRules.StandardPluralCategories;
import com.ibm.icu.util.Currency;
import com.ibm.icu.util.Measure;
import com.ibm.icu.util.MeasureUnit;
import com.ibm.icu.util.ULocale;

/**
 * @author markdavis
 * 
 */
public class PluralRangesTest extends TestFmwk {
    public static void main(String[] args) {
        new PluralRangesTest().run(args);
    }

    public void TestLocaleData() {
        String[][] tests = {
                {"de", "other", "one", "one"},
                {"xxx", "few", "few", "few" },
                {"de", "one", "other", "other"},
                {"de", "other", "one", "one"},
                {"de", "other", "other", "other"},
                {"ro", "one", "few", "few"},
                {"ro", "one", "other", "other"},
                {"ro", "few", "one", "few"},
        };
        for (String[] test : tests) {
            final ULocale locale = new ULocale(test[0]);
            final StandardPluralCategories start = StandardPluralCategories.valueOf(test[1]);
            final StandardPluralCategories end = StandardPluralCategories.valueOf(test[2]);
            final StandardPluralCategories expected = StandardPluralCategories.valueOf(test[3]);
            final PluralRanges pluralRanges = Factory.getDefaultFactory().getPluralRanges(locale);

            StandardPluralCategories actual = pluralRanges.get(start, end);
            assertEquals("Deriving range category", expected, actual);
        }
    }

    public void TestFormatting() {
        Object[][] tests = {
                {0.0, 1.0, ULocale.FRANCE, FormatWidth.WIDE, MeasureUnit.FAHRENHEIT, "de 0 à 1 degré Fahrenheit"},
                {1.0, 2.0, ULocale.FRANCE, FormatWidth.WIDE, MeasureUnit.FAHRENHEIT, "de 1 à 2 degrés Fahrenheit"},
                {3.1, 4.25, ULocale.FRANCE, FormatWidth.SHORT, MeasureUnit.FAHRENHEIT, "3,1–4,25 °F"},
                {3.1, 4.25, ULocale.ENGLISH, FormatWidth.SHORT, MeasureUnit.FAHRENHEIT, "3.1–4.25°F"},
                {3.1, 4.25, ULocale.CHINESE, FormatWidth.WIDE, MeasureUnit.INCH, "3.1-4.25英寸"},
                {0.0, 1.0, ULocale.ENGLISH, FormatWidth.WIDE, MeasureUnit.INCH, "0–1 inches"},
                
                {0.0, 1.0, ULocale.ENGLISH, FormatWidth.WIDE, Currency.getInstance("EUR"), 
                    IllegalArgumentException.class},
        };
        for (Object[] test : tests) {
            double low = (Double) test[0];
            double high = (Double) test[1];
            final ULocale locale = (ULocale) test[2];
            final FormatWidth width = (FormatWidth) test[3];
            final MeasureUnit unit = (MeasureUnit) test[4];
            final Object expected = test[5];

            MeasureFormat mf = MeasureFormat.getInstance(locale, width);
            Object actual;
            try {
                actual = mf.formatMeasureRange(new Measure(low, unit), new Measure(high, unit));
            } catch (Exception e) {
                actual = e.getClass();
            }
            assertEquals("Formatting unit", expected, actual);
        }
    }

    public void TestBasic() {
        PluralRanges a = new PluralRanges();
        a.add(StandardPluralCategories.one, StandardPluralCategories.other, StandardPluralCategories.one);
        StandardPluralCategories actual = a.get(StandardPluralCategories.one, StandardPluralCategories.other);
        assertEquals("range", StandardPluralCategories.one, actual);
        a.freeze();
        try {
            a.add(StandardPluralCategories.one, StandardPluralCategories.one, StandardPluralCategories.one);
            errln("Failed to cause exception on frozen instance");
        } catch (UnsupportedOperationException e) {
        }
    }
}
