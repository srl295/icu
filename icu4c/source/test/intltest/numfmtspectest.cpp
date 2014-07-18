/*
*******************************************************************************
* Copyright (C) 2014, International Business Machines Corporation and         *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* File NUMFMTSPECTEST.CPP
*
*******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>

#include "intltest.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/localpointer.h"
#include "unicode/decimfmt.h"
#include "unicode/dtfmtsym.h"
#include "uassert.h"

#define LENGTHOF(array) (int32_t)(sizeof(array) / sizeof((array)[0]))

static const UChar kJPY[] = {0x4A, 0x50, 0x59};

static void fixNonBreakingSpace(UnicodeString &str) {
    for (int32_t i = 0; i < str.length(); ++i) {
        if (str[i] == 0xa0) {
            str.setCharAt(i, 0x20);
        }
    }    
}

static NumberFormat *nfWithPattern(const char *pattern) {
    UnicodeString upattern(pattern, -1, US_INV);
    upattern = upattern.unescape();
    UErrorCode status = U_ZERO_ERROR;
    DecimalFormat *result = new DecimalFormat(
            upattern, new DecimalFormatSymbols("fr", status), status);
    U_ASSERT(status == U_ZERO_ERROR);
    return result;
}

static UnicodeString format(double d, const NumberFormat &fmt) {
    UnicodeString result;
    fmt.format(d, result);
    fixNonBreakingSpace(result);
    return result;
}

class NumberFormatSpecificationTest : public IntlTest {
public:
    NumberFormatSpecificationTest() {
    }
    void TestBasicPatterns();
    void TestNfSetters();
    void TestRounding();
    void TestSignificantDigits();
    void TestScientificNotation();
    void TestPercent();
    void TestPerMilli();
    void TestPadding();
    void runIndexedTest(int32_t index, UBool exec, const char *&name, char *par=0);
private:
    void assertPatternFr(
            const char *expected, double x, const char *pattern);
    
};

void NumberFormatSpecificationTest::runIndexedTest(
        int32_t index, UBool exec, const char *&name, char *) {
    if (exec) {
        logln("TestSuite NumberFormatSpecificationTest: ");
    }
    TESTCASE_AUTO_BEGIN;
    TESTCASE_AUTO(TestBasicPatterns);
    TESTCASE_AUTO(TestNfSetters);
    TESTCASE_AUTO(TestRounding);
    TESTCASE_AUTO(TestSignificantDigits);
    TESTCASE_AUTO(TestScientificNotation);
    TESTCASE_AUTO(TestPercent);
    TESTCASE_AUTO(TestPerMilli);
    TESTCASE_AUTO(TestPadding);
    TESTCASE_AUTO_END;
}

void NumberFormatSpecificationTest::TestBasicPatterns() {
    assertPatternFr("1 234,57", 1234.567, "#,##0.##");
    assertPatternFr("1234,57", 1234.567, "0.##");
    assertPatternFr("1235", 1234.567, "0");
    assertPatternFr("1 234,567", 1234.567, "#,##0.###");
    assertPatternFr("1234,567", 1234.567, "###0.#####");
    assertPatternFr("1234,5670", 1234.567, "###0.0000#");
    assertPatternFr("01234,5670", 1234.567, "00000.0000");
    // Broken ticket 11026
    // assertPatternFr("1 234,57 \\u20ac", 1234.567, "#,##0.00 \\u00a4");
}

void NumberFormatSpecificationTest::TestNfSetters() {
    LocalPointer<NumberFormat> nf(nfWithPattern("#,##0.##"));
    nf->setMaximumIntegerDigits(5);
    nf->setMinimumIntegerDigits(4);
    assertEquals("", "34 567,89", format(1234567.89, *nf));
    assertEquals("", "0 034,56", format(34.56, *nf));
}

void NumberFormatSpecificationTest::TestRounding() {
    assertPatternFr("1,0", 1.25, "0.5");
    assertPatternFr("2,0", 1.75, "0.5");
    assertPatternFr("-1,0", -1.25, "0.5");
    assertPatternFr("-02,0", -1.75, "00.5");
    assertPatternFr("0", 2.0, "4");
    assertPatternFr("8", 6.0, "4");
    assertPatternFr("8", 10.0, "4");
    assertPatternFr("99,90", 99.0, "2.70");
    assertPatternFr("273,00", 272.0, "2.73");
    assertPatternFr("1 03,60", 104.0, "#,#3.70");
}

void NumberFormatSpecificationTest::TestSignificantDigits() {
    assertPatternFr("1230", 1234.0, "@@@");
    assertPatternFr("1 234", 1234.0, "@,@@@");
    assertPatternFr("1 235 000", 1234567.0, "@,@@@");
    assertPatternFr("1 234 567", 1234567.0, "@@@@,@@@");
    assertPatternFr("12 34 567,00", 1234567.0, "@@@@,@@,@@@");
    assertPatternFr("12 34 567,0", 1234567.0, "@@@@,@@,@@#");
    assertPatternFr("12 34 567", 1234567.0, "@@@@,@@,@##");
    assertPatternFr("12 34 567", 1234567.001, "@@@@,@@,@##");
    assertPatternFr("12 34 567", 1234567.001, "@@@@,@@,###");
    assertPatternFr("1 200", 1234.0, "#,#@@");
}

void NumberFormatSpecificationTest::TestScientificNotation() {
    assertPatternFr("1,23E4", 12345.0, "0.00E0");
    assertPatternFr("123,00E2", 12300.0, "000.00E0");
    assertPatternFr("123,0E2", 12300.0, "000.0#E0");
    assertPatternFr("123,0E2", 12300.1, "000.0#E0");
    assertPatternFr("123,01E2", 12301.0, "000.0#E0");
    assertPatternFr("123,01E+02", 12301.0, "000.0#E+00");
    assertPatternFr("12,3E3", 12345.0, "##0.00E0");
    assertPatternFr("12,300E3", 12300.1, "##0.0000E0");
    assertPatternFr("12,30E3", 12300.1, "##0.000#E0");
    assertPatternFr("12,301E3", 12301.0, "##0.000#E0");
    // broken ticket 11020
    // assertPatternFr("1,25E4", 12301.2, "0.05E0");
    assertPatternFr("170,0E-3", 0.17, "##0.000#E0");

}

void NumberFormatSpecificationTest::TestPercent() {
    assertPatternFr("57,3%", 0.573, "0.0%");
    assertPatternFr("%57,3", 0.573, "%0.0");
    assertPatternFr("p%p57,3", 0.573, "p%p0.0");
    assertPatternFr("p%p0,6", 0.573, "p'%'p0.0");
    assertPatternFr("%3,260", 0.0326, "%@@@@");
    assertPatternFr("%1 540", 15.43, "%#,@@@");
    assertPatternFr("%1 656,4", 16.55, "%#,##4.1");
    assertPatternFr("%16,3E3", 162.55, "%##0.00E0");  
}

void NumberFormatSpecificationTest::TestPerMilli() {
    assertPatternFr("573,0\\u2030", 0.573, "0.0\\u2030");
    assertPatternFr("\\u2030573,0", 0.573, "\\u20300.0");
    assertPatternFr("p\\u2030p573,0", 0.573, "p\\u2030p0.0");
    assertPatternFr("p\\u2030p0,6", 0.573, "p'\\u2030'p0.0");
    assertPatternFr("\\u203032,60", 0.0326, "\\u2030@@@@");
    assertPatternFr("\\u203015 400", 15.43, "\\u2030#,@@@");
    assertPatternFr("\\u203016 551,7", 16.55, "\\u2030#,##4.1");
    assertPatternFr("\\u2030163E3", 162.55, "\\u2030##0.00E0");
}

void NumberFormatSpecificationTest::TestPadding() {
    assertPatternFr("$***1 234", 1234, "$**####,##0");
    assertPatternFr("xxx$1 234", 1234, "*x$####,##0");
    assertPatternFr("1 234xxx$", 1234, "####,##0*x$");
    assertPatternFr("1 234$xxx", 1234, "####,##0$*x");
    assertPatternFr("ne1 234nx", -1234, "####,##0$*x;ne#n");
    assertPatternFr("n1 234*xx", -1234, "####,##0$*x;n#'*'");
    assertPatternFr("yyyy%432,6", 4.33, "*y%4.2######");
    // Broken ticket 11026
    // assertPatternFr("EUR *433,00", 433.0, "\\u00a4\\u00a4 **####0.00");
    // Broken ticket 11026
    // assertPatternFr("EUR *433,00", 433.0, "\\u00a4\\u00a4 **#######0");
    {
        UnicodeString upattern("\\u00a4\\u00a4 **#######0", -1, US_INV);
        upattern = upattern.unescape();
        UErrorCode status = U_ZERO_ERROR;
        UnicodeString result;
        DecimalFormat fmt(
                upattern, new DecimalFormatSymbols("fr", status), status);
        fmt.setCurrency(kJPY);
        fmt.format(433.22, result);
        assertSuccess("", status);
        assertEquals("", "JPY ****433", result);
    }
    {
        UnicodeString upattern(
            "\\u00a4\\u00a4 **#######0;\\u00a4\\u00a4 (#)", -1, US_INV);
        upattern = upattern.unescape();
        UErrorCode status = U_ZERO_ERROR;
        UnicodeString result;
        DecimalFormat fmt(
                upattern,
                new DecimalFormatSymbols("en_US", status),
                status);
        fmt.format(-433.22, result);
        assertSuccess("", status);
        assertEquals("", "USD (433.22)", result);
    }
    const char *paddedSciPattern = "QU**00.#####E0";
    assertPatternFr("QU***43,3E-1", 4.33, paddedSciPattern);
    {
        UErrorCode status = U_ZERO_ERROR;
        DecimalFormatSymbols *sym = new DecimalFormatSymbols("fr", status);
        sym->setSymbol(DecimalFormatSymbols::kExponentialSymbol, "EE");
        DecimalFormat fmt(
                paddedSciPattern,
                sym,
                status);
        UnicodeString result;
        fmt.format(4.33, result);
        assertSuccess("", status);
        assertEquals("", "QU**43,3EE-1", result);
    }
    // padding cannot work as intended with scientific notation.
    assertPatternFr("QU**43,32E-1", 4.332, paddedSciPattern);
}

void NumberFormatSpecificationTest::assertPatternFr(
        const char *expected,
        double x,
        const char *pattern) {
    UnicodeString upattern(pattern, -1, US_INV);
    UnicodeString uexpected(expected, -1, US_INV);
    upattern = upattern.unescape();
    uexpected = uexpected.unescape();
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString result;
    DecimalFormat fmt(
            upattern, new DecimalFormatSymbols("fr", status), status);
    fmt.format(x, result);
    fixNonBreakingSpace(result);
    assertSuccess("", status);
    assertEquals("", uexpected, result);
}

extern IntlTest *createNumberFormatSpecificationTest() {
    return new NumberFormatSpecificationTest();
}

#endif
