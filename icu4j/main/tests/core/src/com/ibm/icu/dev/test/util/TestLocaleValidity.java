/*
 *******************************************************************************
 * Copyright (C) 2015, International Business Machines Corporation and         *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 */
package com.ibm.icu.dev.test.util;

import java.util.EnumSet;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;

import com.ibm.icu.dev.test.TestFmwk;
import com.ibm.icu.impl.ValidIdentifiers;
import com.ibm.icu.impl.ValidIdentifiers.Datasubtype;
import com.ibm.icu.impl.ValidIdentifiers.Datatype;
import com.ibm.icu.impl.ValidIdentifiers.ValiditySet;
import com.ibm.icu.impl.locale.LocaleValidityChecker;
import com.ibm.icu.impl.locale.LocaleValidityChecker.Where;
import com.ibm.icu.util.ULocale;

/**
 * @author markdavis
 *
 */
public class TestLocaleValidity extends TestFmwk {
    /**
     * Quick check
     */
    public static void main(String[] args) {
        new TestLocaleValidity().run(args);
    }

    public void testBasic() {
        String[][] tests = {
                {"OK", "en-u-kr-latn-digit"},
                {"Incomplete extension 'u' [at index 3]", "en-u"},
                {"Incomplete extension 't' [at index 3]", "en-t"},
                {"OK", "en-u-ca-chinese"},
                {"OK", "en-x-abcdefg"},
                {"OK", "x-abcdefg"},
                {"OK", "en-u-sd-usca"},
                {"OK", "en-US-u-sd-usca"},
                {"OK", "en-AQ-u-sd-usca"},
                {"OK", "en-t-it"},
                {"OK", "und-Cyrl-t-und-latn"},
                {"OK", "root"},
                {"OK", "und"},
                {"OK", "en"},
                {"OK", "en-Hant"},
                {"OK", "zh-Hant-1606nict-1694acad"},
                {"OK", "zh-Hant"},
                {"OK", "zh-Hant-AQ"},
                {"OK", "x-abcdefg-g-foobar"},
                {"Empty subtag [at index 0]", ""},
                {"{u, ca-chinesx}", "en-u-ca-chinesx"},
                {"{illegal, q}", "en-q-abcdefg"},
                {"Incomplete privateuse [at index 0]", "x-abc$defg"},
                {"{script, Latx}", "und-Cyrl-t-und-latx"},
                {"{variant, FOOBAR}", "zh-Hant-1606nict-1694acad-foobar"},
                {"{region, AB}", "zh-Hant-AB"},
                {"{language, ex}", "ex"},
                {"{script, Hanx}", "zh-Hanx"},
                {"{language, qaa}", "qaa"},
                {"Invalid subtag: $ [at index 3]", "EN-$"},
                {"Invalid subtag: $ [at index 0]", "$"},
                // too many items
                {"{u, cu-usd}", "en-u-cu-adp-usd"},

                {"OK", "en-u-ca-buddhist"},
                {"OK", "en-u-cf-account"},
                {"OK", "en-u-co-big5han"},
                {"OK", "en-u-cu-adp"},
                {"OK", "en-u-fw-fri"},
                {"OK", "en-u-hc-h11"},
                {"OK", "en-u-ka-noignore"},
                {"OK", "en-u-kb-false"},
                {"OK", "en-u-kc-false"},
                {"OK", "en-u-kf-false"},
                {"OK", "en-u-kk-false"},
                {"OK", "en-u-kn-false"},
                {"OK", "en-u-kr-latn-digit-symbol"},
                {"OK", "en-u-ks-identic"},
                {"OK", "en-u-kv-currency"},
                {"OK", "en-u-nu-ahom"},
                {"OK", "en-u-sd-usny"},
                {"OK", "en-u-tz-adalv"},
                {"OK", "en-u-va-posix"},
                {"{u, ca-civil}", "en-u-ca-islamicc"}, // deprecated
                {"{u, co-direct}", "en-u-co-direct"}, // deprecated
                {"{u, kh}", "en-u-kh-false"}, // deprecated
                {"{u, tz-aqams}", "en-u-tz-aqams"}, // deprecated
                {"{u, vt}", "en-u-vt-0020-0041"}, // deprecated
        };
        check(tests, Datasubtype.regular, Datasubtype.unknown);
    }

    public void testMissing() {
        String[][] tests = {
                {"OK", "en-u-lb-loose"},
                {"OK", "en-u-lw-breakall"},
                {"OK", "en-u-ms-metric"},
                {"OK", "en-u-ss-none"},
        };
        check(tests, Datasubtype.regular, Datasubtype.unknown);
    }

    public void testTSubtags() {
        String[][] tests = {
                //                {"OK", "und-Cyrl-t-und-latn-m0-ungegn-2007"},
                //                {"{t, ungegg}", "und-Cyrl-t-und-latn-m0-ungegg-2007"},
                //                {"OK", "en-t-i0-handwrit"},
                //                {"OK", "en-t-k0-101key"},
                //                {"OK", "en-t-m0-alaloc"},
                //                {"OK", "en-t-t0-und"},
                //                {"OK", "en-t-x0-anythin"},
        };
        check(tests, Datasubtype.regular, Datasubtype.unknown);
    }

    public void testDeprecated() {
        LocaleValidityChecker regularAndDeprecated = new LocaleValidityChecker(EnumSet.of(Datasubtype.regular, Datasubtype.deprecated));
        String[][] tests = {
                {"OK", "en-u-ca-islamicc"}, // deprecated
                {"OK", "en-u-co-direct"}, // deprecated
                {"OK", "en-u-kh-false"}, // deprecated
                {"OK", "en-u-tz-aqams"}, // deprecated
                {"OK", "en-u-vt-0020"}, // deprecated
        };
        check(tests, Datasubtype.regular, Datasubtype.unknown, Datasubtype.deprecated);
    }

    private void check(String[][] tests, Datasubtype... datasubtypes) {
        int count = 0;
        LocaleValidityChecker regularAndUnknown = new LocaleValidityChecker(datasubtypes);
        for (String[] test : tests) {
            check(++count, regularAndUnknown, test[0], test[1]);
        }
    }

    private void check(int count, LocaleValidityChecker all, String expected, String locale) {
        ULocale ulocale;
        try {
            ulocale = new ULocale.Builder().setLanguageTag(locale).build();
        } catch (Exception e) {
            assertEquals(count + ". " + locale, expected, e.getMessage());
            return;
        }
        Where where = new Where();
        all.isValid(ulocale, where);
        assertEquals(count + ". " + locale, expected, where.toString());

        //        ULocale ulocale2 = ULocale.forLanguageTag(locale);
        //        final String languageTag2 = ulocale2.toLanguageTag();
        //
        //        if (languageTag.equals(languageTag2)) {
        //            return;
        //        }
        //        all.isValid(ulocale2, where);
        //        assertEquals(ulocale2 + ", " + ulocale2.toLanguageTag(), expected, where.toString());

        // problem: ULocale("$").toLanguageTag() becomes valid
    }


    // Quick testing for now

    public void testValidIdentifierData() {
        showValid(Datasubtype.unknown, Datatype.script, EnumSet.of(Datasubtype.regular, Datasubtype.unknown), "Zzzz");
        showValid(null, Datatype.script, EnumSet.of(Datasubtype.regular), "Zzzz");
        showValid(Datasubtype.regular, Datatype.subdivision, EnumSet.of(Datasubtype.regular), "US-CA");
        showValid(Datasubtype.regular, Datatype.subdivision, EnumSet.of(Datasubtype.regular), "US", "CA");
        showValid(null, Datatype.subdivision, EnumSet.of(Datasubtype.regular), "US-?");
        showValid(null, Datatype.subdivision, EnumSet.of(Datasubtype.regular), "US", "?");
        if (isVerbose()) {
            showAll();
        }
    }

    private static void showAll() {
        Map<Datatype, Map<Datasubtype, ValiditySet>> data = ValidIdentifiers.getData();
        for (Entry<Datatype, Map<Datasubtype, ValiditySet>> e1 : data.entrySet()) {
            System.out.println(e1.getKey());
            for (Entry<Datasubtype, ValiditySet> e2 : e1.getValue().entrySet()) {
                System.out.println("\t" + e2.getKey());
                System.out.println("\t\t" + e2.getValue());
            }
        }
    }

    /**
     * @param expected TODO
     * @param script
     * @param of
     * @param string
     */
    private void showValid(Datasubtype expected, Datatype datatype, Set<Datasubtype> datasubtypes, String code) {
        Datasubtype value = ValidIdentifiers.isValid(datatype, datasubtypes, code);   
        assertEquals(datatype + ", " + datasubtypes + ", " + code, expected, value);
    }
    private void showValid(Datasubtype expected, Datatype datatype, Set<Datasubtype> datasubtypes, String code, String code2) {
        Datasubtype value = ValidIdentifiers.isValid(datatype, datasubtypes, code, code2);   
        assertEquals(datatype + ", " + datasubtypes + ", " + code + ", " + code2, expected, value);
    }


}
