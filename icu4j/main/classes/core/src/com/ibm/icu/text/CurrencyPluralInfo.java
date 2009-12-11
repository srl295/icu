/*
 *******************************************************************************
 * Copyright (C) 2009, International Business Machines Corporation and         *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 */
package com.ibm.icu.text;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Locale;
import java.util.Map;

import com.ibm.icu.impl.CurrencyData;
import com.ibm.icu.util.ULocale;

/**
 * This class represents the information needed by
 * DecimalFormat to format currency plural,
 * such as "3.00 US dollars" or "1.00 US dollar".
 * DecimalFormat creates for itself an instance of
 * CurrencyPluralInfo from its locale data.
 * If you need to change any of these symbols, you can get the
 * CurrencyPluralInfo object from your
 * DecimalFormat and modify it.
 *
 * Following are the information needed for currency plural format and parse:
 * locale information,
 * plural rule of the locale,
 * currency plural pattern of the locale.
 *
 * @draft ICU 4.2
 * @provisional This API might change or be removed in a future release.
 */

public class CurrencyPluralInfo implements Cloneable, Serializable {
    private static final long serialVersionUID = 1;

    /**
     * Create a CurrencyPluralInfo object for the default locale.
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public CurrencyPluralInfo() {
        initialize(ULocale.getDefault());
    }

    /**
     * Create a CurrencyPluralInfo object for the given locale.
     * @param locale the locale
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public CurrencyPluralInfo(Locale locale) {
        initialize(ULocale.forLocale(locale));
    }

    /**
     * Create a CurrencyPluralInfo object for the given locale.
     * @param locale the locale
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public CurrencyPluralInfo(ULocale locale) {
        initialize(locale);
    }

    /**
     * Gets a CurrencyPluralInfo instance for the default locale.
     *
     * @return A CurrencyPluralInfo instance.
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public static CurrencyPluralInfo getInstance() {
        return new CurrencyPluralInfo();
    }

    /**
     * Gets a CurrencyPluralInfo instance for the given locale.
     *
     * @param locale the locale.
     * @return A CurrencyPluralInfo instance.
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public static CurrencyPluralInfo getInstance(Locale locale) {
        return new CurrencyPluralInfo(locale);
    }

    /**
     * Gets a CurrencyPluralInfo instance for the given locale.
     *
     * @param locale the locale.
     * @return A CurrencyPluralInfo instance.
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public static CurrencyPluralInfo getInstance(ULocale locale) {
        return new CurrencyPluralInfo(locale);
    }

    /**
     * Gets plural rules of this locale, used for currency plural format
     *
     * @return plural rule
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public PluralRules getPluralRules() {
        return pluralRules;
    }

    /**
     * Given a plural count, gets currency plural pattern of this locale,
     * used for currency plural format
     *
     * @param  pluralCount currency plural count
     * @return a currency plural pattern based on plural count
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public String getCurrencyPluralPattern(String pluralCount) {
        String currencyPluralPattern = pluralCountToCurrencyUnitPattern.get(pluralCount);
        if (currencyPluralPattern == null) {
            // fall back to "other"
            if (!pluralCount.equals("other")) {
                currencyPluralPattern = pluralCountToCurrencyUnitPattern.get("other");
            }
            if (currencyPluralPattern == null) {
                // no currencyUnitPatterns defined,
                // fallback to predefined default.
                // This should never happen when ICU resource files are
                // available, since currencyUnitPattern of "other" is always
                // defined in root.
                currencyPluralPattern = defaultCurrencyPluralPattern;
            }
        }
        return currencyPluralPattern;
    }

    /**
     * Get locale
     *
     * @return locale
     *
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public ULocale getLocale() {
        return ulocale;
    }

    /**
     * Set plural rules.  These are initially set in the constructor based on the locale, 
     * and usually do not need to be changed.
     *
     * @param ruleDescription new plural rule description
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public void setPluralRules(String ruleDescription) {
        pluralRules = PluralRules.createRules(ruleDescription);
    }

    /**
     * Set currency plural patterns.  These are initially set in the constructor based on the
     * locale, and usually do not need to be changed.
     *
     * @param pluralCount the plural count for which the currency pattern will
     *                    be overridden.
     * @param pattern     the new currency plural pattern
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public void setCurrencyPluralPattern(String pluralCount, String pattern) {
        pluralCountToCurrencyUnitPattern.put(pluralCount, pattern);
    }

    /**
     * Set locale.  This also sets both the plural rules and the currency plural patterns to be
     * the defaults for the locale.
     *
     * @param loc the new locale to set
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public void setLocale(ULocale loc) {
        ulocale = loc;
        initialize(loc);
    }

    /**
     * Standard override
     *
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public Object clone() {
        try {
            CurrencyPluralInfo other = (CurrencyPluralInfo) super.clone();
            // locale is immutable
            other.ulocale = (ULocale)ulocale.clone();
            // plural rule is immutable
            //other.pluralRules = pluralRules;
            // clone content
            //other.pluralCountToCurrencyUnitPattern = pluralCountToCurrencyUnitPattern;
            other.pluralCountToCurrencyUnitPattern = new HashMap<String, String>();
            for (String pluralCount : pluralCountToCurrencyUnitPattern.keySet()) {
                String currencyPattern = pluralCountToCurrencyUnitPattern.get(pluralCount);
                other.pluralCountToCurrencyUnitPattern.put(pluralCount, currencyPattern);
            }
            return other;
        } catch (CloneNotSupportedException e) {
            throw new IllegalStateException();
        }
    }

    /**
     * Override equals
     *
     * @draft ICU 4.2
     * @provisional This API might change or be removed in a future release.
     */
    public boolean equals(Object a) {
        if (a instanceof CurrencyPluralInfo) {
            CurrencyPluralInfo other = (CurrencyPluralInfo)a;
            return pluralRules.equals(other.pluralRules) &&
                   pluralCountToCurrencyUnitPattern.equals(other.pluralCountToCurrencyUnitPattern);
        }
        return false;
    }

    /**
     * Given a number, returns the keyword of the first rule that applies
     * to the number.
     */
    String select(double number) {
        return pluralRules.select(number);
    }

    /**
     * Currency plural pattern iterator.
     *
     * @return a iterator on the currency plural pattern key set.
     */
    Iterator<String> pluralPatternIterator() {
        return pluralCountToCurrencyUnitPattern.keySet().iterator();
    }

    private void initialize(ULocale uloc) {
        ulocale = uloc;
        pluralRules = PluralRules.forLocale(uloc);
        setupCurrencyPluralPattern(uloc);
    }

    private void setupCurrencyPluralPattern(ULocale uloc) {
        pluralCountToCurrencyUnitPattern = new HashMap<String, String>();
        
        String numberStylePattern = NumberFormat.getPattern(uloc, NumberFormat.NUMBERSTYLE);
        // Split the number style pattern into pos and neg if applicable
        int separatorIndex = numberStylePattern.indexOf(";");
        String negNumberPattern = null;
        if (separatorIndex != -1) {
            negNumberPattern = numberStylePattern.substring(separatorIndex + 1);
            numberStylePattern = numberStylePattern.substring(0, separatorIndex);
        }
        Map<String, String> map = CurrencyData.provider.getInstance(uloc, true).getUnitPatterns();
        for (Map.Entry<String, String> e : map.entrySet()) {
            String pluralCount = e.getKey();
            String pattern = e.getValue();
            
            // replace {0} with numberStylePattern
            // and {1} with triple currency sign
            String patternWithNumber = pattern.replace("{0}", numberStylePattern);
            String patternWithCurrencySign = patternWithNumber.replace("{1}", tripleCurrencyStr);
            if (separatorIndex != -1) {
                String negPattern = pattern;
                String negWithNumber = negPattern.replace("{0}", negNumberPattern);
                String negWithCurrSign = negWithNumber.replace("{1}", tripleCurrencyStr);
                StringBuffer posNegPatterns = new StringBuffer(patternWithCurrencySign);
                posNegPatterns.append(";");
                posNegPatterns.append(negWithCurrSign);
                patternWithCurrencySign = posNegPatterns.toString();
            }
            pluralCountToCurrencyUnitPattern.put(pluralCount, patternWithCurrencySign);
        }
    }


    //-------------------- private data member ---------------------
    //
    // triple currency sign char array
    private static final char[] tripleCurrencySign = {0xA4, 0xA4, 0xA4};
    // triple currency sign string
    private static final String tripleCurrencyStr = new String(tripleCurrencySign);

    // default currency plural pattern char array
    private static final char[] defaultCurrencyPluralPatternChar = {0, '.', '#', '#', ' ', 0xA4, 0xA4, 0xA4};
    // default currency plural pattern string
    private static final String defaultCurrencyPluralPattern = new String(defaultCurrencyPluralPatternChar);

    // map from plural count to currency plural pattern, for example
    // one (plural count) --> {0} {1} (currency plural pattern,
    // in which {0} is the amount number, and {1} is the currency plural name).
    private Map<String, String> pluralCountToCurrencyUnitPattern = null;

    /*
     * The plural rule is used to format currency plural name,
     * for example: "3.00 US Dollars".
     * If there are 3 currency signs in the currency pattern,
     * the 3 currency signs will be replaced by the currency plural name.
     */
    private PluralRules pluralRules = null;

    // locale
    private ULocale ulocale = null;
}
