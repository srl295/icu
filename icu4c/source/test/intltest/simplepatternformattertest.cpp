/*
*******************************************************************************
* Copyright (C) 2014, International Business Machines Corporation and         *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* File SIMPLEPATTERNFORMATTERTEST.CPP
*
********************************************************************************
*/
#include "cstring.h"
#include "intltest.h"
#include "simplepatternformatter.h"

class SimplePatternFormatterTest : public IntlTest {
public:
    SimplePatternFormatterTest() {
    }
    void TestNoPlaceholders();
    void TestOnePlaceholder();
    void TestManyPlaceholders();
    void TestOptimization();
    void runIndexedTest(int32_t index, UBool exec, const char *&name, char *par=0);
private:
};

void SimplePatternFormatterTest::runIndexedTest(int32_t index, UBool exec, const char* &name, char* /*par*/) {
  TESTCASE_AUTO_BEGIN;
  TESTCASE_AUTO(TestNoPlaceholders);
  TESTCASE_AUTO(TestOnePlaceholder);
  TESTCASE_AUTO(TestManyPlaceholders);
  TESTCASE_AUTO(TestOptimization);
  TESTCASE_AUTO_END;
}

void SimplePatternFormatterTest::TestNoPlaceholders() {
    UErrorCode status = U_ZERO_ERROR;
    SimplePatternFormatter fmt("This doesn''t have templates '{0}");
    assertEquals("PlaceholderCount", 0, fmt.getPlaceholderCount());
    UnicodeString appendTo;
    assertEquals(
            "format",
            "This doesn't have templates {0}", 
            fmt.format("unused", appendTo, status));
    fmt.compile("This has {} bad {012d placeholders", status);
    assertEquals("PlaceholderCount", 0, fmt.getPlaceholderCount());
    appendTo.remove();
    assertEquals(
            "format",
            "This has {} bad {012d placeholders", 
            fmt.format("unused", appendTo, status));
    assertSuccess("Status", status);
}

void SimplePatternFormatterTest::TestOnePlaceholder() {
    UErrorCode status = U_ZERO_ERROR;
    SimplePatternFormatter fmt;
    fmt.compile("{0} meter", status);
    assertEquals("PlaceholderCount", 1, fmt.getPlaceholderCount());
    UnicodeString appendTo;
    assertEquals(
            "format",
            "1 meter",
            fmt.format("1", appendTo, status));
    assertSuccess("Status", status);

    // assignment
    SimplePatternFormatter s;
    s = fmt;
    appendTo.remove();
    assertEquals(
            "Assignment",
            "1 meter",
            s.format("1", appendTo, status));

    // Copy constructor
    SimplePatternFormatter r(fmt);
    appendTo.remove();
    assertEquals(
            "Copy constructor",
            "1 meter",
            r.format("1", appendTo, status));
    assertSuccess("Status", status);
}

void SimplePatternFormatterTest::TestManyPlaceholders() {
    UErrorCode status = U_ZERO_ERROR;
    SimplePatternFormatter fmt;
    fmt.compile(
            "Templates {2}{1}{5} and {4} are out of order.", status);
    assertSuccess("Status", status);
    assertFalse("startsWithPlaceholder", fmt.startsWithPlaceholder(2));
    assertEquals("PlaceholderCount", 6, fmt.getPlaceholderCount());
    UnicodeString values[] = {
            "freddy", "tommy", "frog", "billy", "leg", "{0}"};
    UnicodeString *params[] = {
           &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]}; 
    int32_t offsets[6];
    int32_t expectedOffsets[6] = {-1, 22, 18, -1, 35, 27};
    UnicodeString appendTo("Prefix: ");
    assertEquals(
            "format",
            "Prefix: Templates frogtommy{0} and leg are out of order.",
            fmt.format(
                    params,
                    uprv_lengthof(params),
                    appendTo,
                    offsets,
                    uprv_lengthof(offsets),
                    status));
    assertSuccess("Status", status);
    for (int32_t i = 0; i < uprv_lengthof(expectedOffsets); ++i) {
        if (expectedOffsets[i] != offsets[i]) {
            errln("Expected %d, got %d", expectedOffsets[i], offsets[i]);
        }
    }
    appendTo.remove();
    fmt.format(
            params,
            uprv_lengthof(params) - 1,
            appendTo,
            offsets,
            uprv_lengthof(offsets),
            status);
    if (status != U_ILLEGAL_ARGUMENT_ERROR) {
        errln("Expected U_ILLEGAL_ARGUMENT_ERROR");
    }
    status = U_ZERO_ERROR;
    offsets[uprv_lengthof(offsets) - 1] = 289;
    appendTo.remove();
    fmt.format(
            params,
            uprv_lengthof(params),
            appendTo,
            offsets,
            uprv_lengthof(offsets) - 1,
            status);
    assertEquals("Offsets buffer length", 289, offsets[uprv_lengthof(offsets) - 1]);

    // Test assignment
    SimplePatternFormatter s;
    s = fmt;
    appendTo.remove();
    assertEquals(
            "Assignment",
            "Templates frogtommy{0} and leg are out of order.",
            s.format(
                    params,
                    uprv_lengthof(params),
                    appendTo,
                    NULL,
                    0,
                    status));

    // Copy constructor
    SimplePatternFormatter r(fmt);
    appendTo.remove();
    assertEquals(
            "Copy constructor",
            "Templates frogtommy{0} and leg are out of order.",
            r.format(
                    params,
                    uprv_lengthof(params),
                    appendTo,
                    NULL,
                    0,
                    status));
    r.compile("{0} meter", status);
    assertEquals("PlaceholderCount", 1, r.getPlaceholderCount());
    appendTo.remove();
    assertEquals(
            "Replace with new compile",
            "freddy meter",
            r.format("freddy", appendTo, status));
    r.compile("{0}, {1}", status);
    assertEquals("PlaceholderCount", 2, r.getPlaceholderCount());
    appendTo.remove();
    assertEquals(
            "2 arg",
            "foo, bar",
            r.format("foo", "bar", appendTo, status));
    r.compile("{0}, {1} and {2}", status);
    assertEquals("PlaceholderCount", 3, r.getPlaceholderCount());
    appendTo.remove();
    assertEquals(
            "3 arg",
            "foo, bar and baz",
            r.format("foo", "bar", "baz", appendTo, status));
    assertSuccess("Status", status);
}

void SimplePatternFormatterTest::TestOptimization() {
    UErrorCode status = U_ZERO_ERROR;
    SimplePatternFormatter fmt;
    fmt.compile("{2}, {0}, {1} and {3}", status);
    assertSuccess("Status", status);
    assertTrue("startsWithPlaceholder", fmt.startsWithPlaceholder(2));
    assertFalse("startsWithPlaceholder", fmt.startsWithPlaceholder(0));
    UnicodeString values[] = {
            "freddy", "frog", "leg", "by"};
    UnicodeString *params[] = {
           &values[0], &values[1], &values[2], &values[3]}; 
    int32_t offsets[4];
    int32_t expectedOffsets[4] = {5, 13, 0, 22};

    // The pattern starts with {2}, so format should append the result of
    // the rest of the pattern to values[2], the value for {2}.
    assertEquals(
            "format",
            "leg, freddy, frog and by",
            fmt.format(
                    params,
                    uprv_lengthof(params),
                    values[2],
                    offsets,
                    uprv_lengthof(offsets),
                    status));
    assertSuccess("Status", status);
    for (int32_t i = 0; i < uprv_lengthof(expectedOffsets); ++i) {
        if (expectedOffsets[i] != offsets[i]) {
            errln("Expected %d, got %d", expectedOffsets[i], offsets[i]);
        }
    }
}

extern IntlTest *createSimplePatternFormatterTest() {
    return new SimplePatternFormatterTest();
}
