/*
 *******************************************************************************
 * Copyright (C) 1996-2000, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 *
 * $Source: /xsrl/Nsvn/icu/icu4j/src/com/ibm/test/translit/Attic/TransliteratorTest.java,v $ 
 * $Date: 2000/03/10 03:47:47 $ 
 * $Revision: 1.13 $
 *
 *****************************************************************************************
 */
package com.ibm.test.translit;
import com.ibm.text.*;
import com.ibm.test.*;
import com.ibm.util.Utility;
import java.text.*;
import java.util.*;

/**
 * @test
 * @summary General test of Transliterator
 */
public class TransliteratorTest extends TestFmwk {

    public static void main(String[] args) throws Exception {
        new TransliteratorTest().run(args);
    }

    public void TestInstantiation() {
        long ms = System.currentTimeMillis();
        String ID;
        for (Enumeration e = Transliterator.getAvailableIDs(); e.hasMoreElements(); ) {
            ID = (String) e.nextElement();
            try {
                Transliterator t = Transliterator.getInstance(ID);
                // We should get a new instance if we try again
                Transliterator t2 = Transliterator.getInstance(ID);
                if (t != t2) {
                    logln(ID + ":" + t);
                } else {
                    errln("FAIL: " + ID + " returned identical instances");
                }
            } catch (IllegalArgumentException ex) {
                errln("FAIL: " + ID);
                throw ex;
            }
        }

        // Now test the failure path
        try {
            ID = "<Not a valid Transliterator ID>";
            Transliterator t = Transliterator.getInstance(ID);
            errln("FAIL: " + ID + " returned " + t);
        } catch (IllegalArgumentException ex) {
            logln("OK: Bogus ID handled properly");
        }
        
        ms = System.currentTimeMillis() - ms;
        logln("Elapsed time: " + ms + " ms");
    }

    public void TestDisplayName() {
        String ID;
        for (Enumeration e = Transliterator.getAvailableIDs(); e.hasMoreElements(); ) {
            ID = (String) e.nextElement();
            logln(ID + " -> " + Transliterator.getDisplayName(ID));
        }
    }

    public void TestSimpleRules() {
        /* Example: rules 1. ab>x|y
         *                2. yc>z
         *
         * []|eabcd  start - no match, copy e to tranlated buffer
         * [e]|abcd  match rule 1 - copy output & adjust cursor
         * [ex|y]cd  match rule 2 - copy output & adjust cursor
         * [exz]|d   no match, copy d to transliterated buffer
         * [exzd]|   done
         */
        expect("ab>x|y;" +
               "yc>z",
               "eabcd", "exzd");

        /* Another set of rules:
         *    1. ab>x|yzacw
         *    2. za>q
         *    3. qc>r
         *    4. cw>n
         *
         * []|ab       Rule 1
         * [x|yzacw]   No match
         * [xy|zacw]   Rule 2
         * [xyq|cw]    Rule 4
         * [xyqn]|     Done
         */
        expect("ab>x|yzacw;" +
               "za>q;" +
               "qc>r;" +
               "cw>n",
               "ab", "xyqn");

        /* Test categories
         */
        Transliterator t = new RuleBasedTransliterator("<ID>",
                                                       "dummy=\uE100;" +
                                                       "vowel=[aeiouAEIOU];" +
                                                       "lu=[:Lu:];" +
                                                       "{vowel} ({lu}) > !;" +
                                                       "{vowel} > &;" +
                                                       "(!) {lu} > ^;" +
                                                       "{lu} > *;" +
                                                       "a>ERROR");
        expect(t, "abcdefgABCDEFGU", "&bcd&fg!^**!^*&");
    }

    /**
     * Test undefined variable.
     */
    public void TestUndefinedVariable() {
        String rule = "({initial}) a <> \u1161;";
        try {
            Transliterator t = new RuleBasedTransliterator("<ID>", rule);
        } catch (IllegalArgumentException e) {
            logln("OK: Got exception for " + rule + ", as expected: " +
                  e.getMessage());
            return;
        }
        errln("Fail: bogus rule " + rule + " compiled without error");
    }

    /**
     * Test empty context.
     */
    public void TestEmptyContext() {
        expect("() a () > b;", "xay a ", "xby b ");
    }

    /**
     * Test inline set syntax and set variable syntax.
     */
    public void TestInlineSet() {
        expect("[:Ll:] (x) > y; [:Ll:] > z;", "aAbxq", "zAyzz");
        expect("a[0-9]b > qrs", "1a7b9", "1qrs9");

        expect("digit = [0-9];" +
               "alpha = [a-zA-Z];" +
               "alphanumeric = [{digit}{alpha}];" + // ***
               "special = [^{alphanumeric}];" +     // ***
               "{alphanumeric} > -;" +
               "{special} > *;",

               "thx-1138", "---*----");
    }

    /**
     * Create some inverses and confirm that they work.  We have to be
     * careful how we do this, since the inverses will not be true
     * inverses -- we can't throw any random string at the composition
     * of the transliterators and expect the identity function.  F x
     * F' != I.  However, if we are careful about the input, we will
     * get the expected results.
     */
    public void TestRuleBasedInverse() {
        String RULES =
            "abc>zyx;" +
            "ab>yz;" +
            "bc>zx;" +
            "ca>xy;" +
            "a>x;" +
            "b>y;" +
            "c>z;" +

            "abc<zyx;" +
            "ab<yz;" +
            "bc<zx;" +
            "ca<xy;" +
            "a<x;" +
            "b<y;" +
            "c<z;" +

            "";

        String[] DATA = {
            // Careful here -- random strings will not work.  If we keep
            // the left side to the domain and the right side to the range
            // we will be okay though (left, abc; right xyz).
            "a", "x",
            "abcacab", "zyxxxyy",
            "caccb", "xyzzy",
        };

        Transliterator fwd = new RuleBasedTransliterator("<ID>", RULES);
        Transliterator rev = new RuleBasedTransliterator("<ID>", RULES,
                                     RuleBasedTransliterator.REVERSE, null);
        for (int i=0; i<DATA.length; i+=2) {
            expect(fwd, DATA[i], DATA[i+1]);
            expect(rev, DATA[i+1], DATA[i]);
        }
    }

    /**
     * Basic test of keyboard.
     */
    public void TestKeyboard() {
        Transliterator t = new RuleBasedTransliterator("<ID>", 
                                                       "psch>Y;"
                                                       +"ps>y;"
                                                       +"ch>x;"
                                                       +"a>A;");
        String DATA[] = {
            // insertion, buffer
            "a", "A",
            "p", "Ap",
            "s", "Aps",
            "c", "Apsc",
            "a", "AycA",
            "psch", "AycAY",
            null, "AycAY", // null means finishKeyboardTransliteration
        };

        keyboardAux(t, DATA);
    }

    /**
     * Basic test of keyboard with cursor.
     */
    public void TestKeyboard2() {
        Transliterator t = new RuleBasedTransliterator("<ID>", 
                                                       "ych>Y;"
                                                       +"ps>|y;"
                                                       +"ch>x;"
                                                       +"a>A;");
        String DATA[] = {
            // insertion, buffer
            "a", "A",
            "p", "Ap",
            "s", "Ay",
            "c", "Ayc",
            "a", "AycA",
            "p", "AycAp",
            "s", "AycAy",
            "c", "AycAyc",
            "h", "AycAY",
            null, "AycAY", // null means finishKeyboardTransliteration
        };

        keyboardAux(t, DATA);
    }

    /**
     * Test keyboard transliteration with back-replacement.
     */
    public void TestKeyboard3() {
        // We want th>z but t>y.  Furthermore, during keyboard
        // transliteration we want t>y then yh>z if t, then h are
        // typed.
        String RULES =
            "t>|y;" +
            "yh>z;" +
            "";

        String[] DATA = {
            // Column 1: characters to add to buffer (as if typed)
            // Column 2: expected appearance of buffer after
            //           keyboard xliteration.
            "a", "a",
            "b", "ab",
            "t", "aby",
            "c", "abyc",
            "t", "abycy",
            "h", "abycz",
            null, "abycz", // null means finishKeyboardTransliteration
        };

        Transliterator t = new RuleBasedTransliterator("<ID>", RULES);
        keyboardAux(t, DATA);
    }

    private void keyboardAux(Transliterator t, String[] DATA) {
        Transliterator.Position index = new Transliterator.Position();
        ReplaceableString s = new ReplaceableString();
        for (int i=0; i<DATA.length; i+=2) {
            StringBuffer log;
            if (DATA[i] != null) {
                log = new StringBuffer(s.toString() + " + "
                                       + DATA[i]
                                       + " -> ");
                t.transliterate(s, index, DATA[i]);
            } else {
                log = new StringBuffer(s.toString() + " => ");
                t.finishTransliteration(s, index);
            }
            String str = s.toString();
            // Show the start index '{' and the cursor '|'
            log.append(str.substring(0, index.start)).
                append('{').
                append(str.substring(index.start,
                                     index.cursor)).
                append('|').
                append(str.substring(index.cursor));
            if (str.equals(DATA[i+1])) {
                logln(log.toString());
            } else {
                errln("FAIL: " + log.toString() + ", expected " + DATA[i+1]);
            }
        }
    }

    public void TestArabic() {
        String DATA[] = {
            "Arabic",
                "\u062a\u062a\u0645\u062a\u0639 "+
                "\u0627\u0644\u0644\u063a\u0629 "+
                "\u0627\u0644\u0639\u0631\u0628\u0628\u064a\u0629 "+
                "\u0628\u0628\u0646\u0638\u0645 "+
                "\u0643\u062a\u0627\u0628\u0628\u064a\u0629 "+
                "\u062c\u0645\u064a\u0644\u0629"
        };

        Transliterator t = Transliterator.getInstance("Latin-Arabic");
        for (int i=0; i<DATA.length; i+=2) {
            expect(t, DATA[i], DATA[i+1]);
        }
    }

    /**
     * Compose the Kana transliterator forward and reverse and try
     * some strings that should come out unchanged.
     */
    public void TestCompoundKana() {
        Transliterator t = new CompoundTransliterator("Latin-Kana;Kana-Latin");
        expect(t, "aaaaa", "aaaaa");
    }

    /**
     * Compose the hex transliterators forward and reverse.
     */
    public void TestCompoundHex() {
        Transliterator a = Transliterator.getInstance("Unicode-Hex");
        Transliterator b = Transliterator.getInstance("Hex-Unicode");
        Transliterator[] trans = { a, b };
        Transliterator ab = new CompoundTransliterator(trans);

        // Do some basic tests of b
        expect(b, "\\u0030\\u0031", "01");

        String s = "abcde";
        expect(ab, s, s);

        trans = new Transliterator[] { b, a };
        Transliterator ba = new CompoundTransliterator(trans);
        ReplaceableString str = new ReplaceableString(s);
        a.transliterate(str);
        expect(ba, str.toString(), str.toString());
    }

    /**
     * Do some basic tests of filtering.
     */
    public void TestFiltering() {
        Transliterator hex = Transliterator.getInstance("Unicode-Hex");
        hex.setFilter(new UnicodeFilter() {
            public boolean contains(char c) {
                return c != 'c';
            }
        });
        String s = "abcde";
        String out = hex.transliterate(s);
        String exp = "\\u0061\\u0062c\\u0064\\u0065";
        if (out.equals(exp)) {
            logln("Ok:   \"" + exp + "\"");
        } else {
            logln("FAIL: \"" + out + "\", wanted \"" + exp + "\"");
        }
    }

    /**
     * Test pattern quoting and escape mechanisms.
     */
    public void TestPatternQuoting() {
        // Array of 3n items
        // Each item is <rules>, <input>, <expected output>
        String[] DATA = {
            "\u4E01>'[male adult]'", "\u4E01", "[male adult]",
        };

        for (int i=0; i<DATA.length; i+=3) {
            logln("Pattern: " + Utility.escape(DATA[i]));
            Transliterator t = new RuleBasedTransliterator("<ID>", DATA[i]);
            expect(t, DATA[i+1], DATA[i+2]);
        }
    }

    //======================================================================
    // Support methods
    //======================================================================

    void expect(String rules, String source, String expectedResult) {
        expect(new RuleBasedTransliterator("<ID>", rules), source, expectedResult);
    }

    void expect(Transliterator t, String source, String expectedResult,
                Transliterator reverseTransliterator) {
        expect(t, source, expectedResult);
        if (reverseTransliterator != null) {
            expect(reverseTransliterator, expectedResult, source);
        }
    }

    void expect(Transliterator t, String source, String expectedResult) {
        String result = t.transliterate(source);
        expectAux(t.getID() + ":String", source, result, expectedResult);

        ReplaceableString rsource = new ReplaceableString(source);
        t.transliterate(rsource);
        result = rsource.toString();
        expectAux(t.getID() + ":Replaceable", source, result, expectedResult);

        // Test keyboard (incremental) transliteration -- this result
        // must be the same after we finalize (see below).
        rsource.getStringBuffer().setLength(0);
        Transliterator.Position index = new Transliterator.Position();
        StringBuffer log = new StringBuffer();

        for (int i=0; i<source.length(); ++i) {
            if (i != 0) {
                log.append(" + ");
            }
            log.append(source.charAt(i)).append(" -> ");
            t.transliterate(rsource, index,
                            String.valueOf(source.charAt(i)));
            // Append the string buffer with a vertical bar '|' where
            // the committed index is.
            String s = rsource.toString();
            log.append(s.substring(0, index.cursor)).
                append('|').
                append(s.substring(index.cursor));
        }
        
        // As a final step in keyboard transliteration, we must call
        // transliterate to finish off any pending partial matches that
        // were waiting for more input.
        t.finishTransliteration(rsource, index);
        result = rsource.toString();
        log.append(" => ").append(rsource.toString());

        expectAux(t.getID() + ":Keyboard", log.toString(),
                  result.equals(expectedResult),
                  expectedResult);
    }

    void expectAux(String tag, String source,
                   String result, String expectedResult) {
        expectAux(tag, source + " -> " + result,
                  result.equals(expectedResult),
                  expectedResult);
    }
    
    void expectAux(String tag, String summary, boolean pass,
                   String expectedResult) {
        if (pass) {
            logln("("+tag+") " + Utility.escape(summary));
        } else {
            errln("FAIL: ("+tag+") "
                  + Utility.escape(summary)
                  + ", expected " + Utility.escape(expectedResult));
        }
    }
}
