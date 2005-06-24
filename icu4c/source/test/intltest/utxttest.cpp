/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 2005, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/************************************************************************
*   Tests for the UText and UTextIterator text abstraction classses
*
************************************************************************/

#include "unicode/utypes.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unicode/utext.h>
#include <unicode/utf8.h>
#include <unicode/ustring.h>
#include "utxttest.h"

UBool  gFailed = FALSE;
#define TEST_ASSERT(x) \
   {if ((x)==FALSE) {errln("Test failure in file %s at line %d\n", __FILE__, __LINE__);\
                     gFailed = TRUE;\
   }}


#define TEST_SUCCESS(status) \
   {if (U_FAILURE(status)) {errln("Test failure in file %s at line %d. Error = \"%s\"\n", \
       __FILE__, __LINE__, u_errorName(status)); \
       gFailed = TRUE;\
   }}

UTextTest::UTextTest() {
}

UTextTest::~UTextTest() {
}

void
UTextTest::runIndexedTest(int32_t index, UBool exec,
                                      const char* &name, char* /*par*/) {
    switch (index) {
        case 0: name = "TextTest";
            if(exec) TextTest();                         break;
        default: name = ""; break;
    }
}

void  UTextTest::TextTest() {
    TestString("abcd\\U00010001xyz");
}

//
//  mapping between native indexes and code points.
//     native indexes could be utf-8, utf-16, utf32, or some code page.
//     The general purpose UText test funciton takes an array of these as
//     expected contents of the text being accessed.
//


void UTextTest::TestString(const UnicodeString &s) {
    int32_t         i;
    int32_t         j;
    UChar32     c;
    int32_t         cpCount = 0;
    UErrorCode  status = U_ZERO_ERROR;

    UnicodeString sa = s.unescape();

    //
    // Build up the mapping between code points and UTF-16 code unit indexes.
    //
    m *cpMap = new m[sa.length() + 1];
    j = 0;
    for (i=0; i<sa.length(); i=sa.moveIndex32(i, 1)) {
        c = sa.char32At(i);
        cpMap[j].nativeIdx = i;
        cpMap[j].cp = c;
        j++;
        cpCount++;
    }
    cpMap[j].nativeIdx = i;   // position following the last char in utf-16 string.    


    // UChar * test, null term

    // UChar * test, with length

    // const UChar * test, null term


    // const UChar * test, length

    // UnicodeString test
    UText *ut;
    ut = utext_openUnicodeString(NULL, &sa, &status);
    TEST_SUCCESS(status);
    TestAccess(sa, ut, cpCount, cpMap);
    utext_close(ut);

    //
    // UTF-8 test
    //

    // Convert the test string from UnicodeString to (char *) in utf-8 format
    int32_t u8Len = sa.extract(0, sa.length(), NULL, 0, "utf-8");
    char *u8String = new char[u8Len + 1];
    sa.extract(0, sa.length(), u8String, u8Len+1, "utf-8");

    // Build up the map of code point indices in the utf-8 string
    m * u8Map = new m[sa.length() + 1];
    i = 0;   // native utf-8 index
    for (j=0; j<cpCount ; j++) {  // code point number
        u8Map[j].nativeIdx = i;
        U8_NEXT(u8String, i, u8Len, c)
        u8Map[j].cp = c;
    }
    u8Map[cpCount].nativeIdx = u8Len;   // position following the last char in utf-8 string.

    // Do the test itself
    status = U_ZERO_ERROR;
    ut = utext_openUTF8(NULL, (uint8_t *)u8String, -1, &status);
    TEST_SUCCESS(status);
    TestAccess(sa, ut, cpCount, u8Map);
    utext_close(ut);

    // UTF-32 test

    // Code Page test

    // Replaceable test

	delete []cpMap;
	delete []u8Map;
	delete []u8String;
}


void UTextTest::TestAccess(const UnicodeString &us, UText *ut, int cpCount, m *cpMap) {
    UErrorCode  status = U_ZERO_ERROR;

    //
    //  Check the length from the UText
    //
    int expectedLen = cpMap[cpCount].nativeIdx;
    int utlen = ut->length(ut);
    TEST_ASSERT(expectedLen == utlen);

    //
    //  Iterate forwards, verify that we get the correct code points
    //   at the correct native offsets.
    //
    int         i = 0;
    int         index;
    int         expectedIndex = 0;
    int         foundIndex = 0;
    UChar32     expectedC;
    UChar32     foundC;
    int32_t     len;

    for (i=0; i<cpCount; i++) {
        expectedIndex = cpMap[i].nativeIdx;
        foundIndex    = utext_getNativeIndex(ut);
        TEST_ASSERT(expectedIndex == foundIndex);
        expectedC     = cpMap[i].cp;
        foundC        = utext_next32(ut);    
        TEST_ASSERT(expectedC == foundC);
        if (gFailed) {
            return;
        }
    }
    foundC = utext_next32(ut);
    TEST_ASSERT(foundC == U_SENTINEL);
    
    // Repeat above, using macros
    utext_setNativeIndex(ut, 0);
    for (i=0; i<cpCount; i++) {
        expectedIndex = cpMap[i].nativeIdx;
        foundIndex    = utext_getNativeIndex(ut);
        TEST_ASSERT(expectedIndex == foundIndex);
        expectedC     = cpMap[i].cp;
        foundC        = UTEXT_NEXT32(ut);    
        TEST_ASSERT(expectedC == foundC);
        if (gFailed) {
            return;
        }
    }
    foundC = utext_next32(ut);
    TEST_ASSERT(foundC == U_SENTINEL);

    //
    //  Forward iteration (above) should have left index at the
    //   end of the input, which should == length().
    //
    len = utext_nativeLength(ut);
    foundIndex  = utext_getNativeIndex(ut);
    TEST_ASSERT(len == foundIndex);

    //
    // Iterate backwards over entire test string
    //
    len = utext_getNativeIndex(ut);
    utext_setNativeIndex(ut, len);
    for (i=cpCount-1; i>=0; i--) {
        expectedC     = cpMap[i].cp;
        expectedIndex = cpMap[i].nativeIdx;
        foundC        = utext_previous32(ut);
        foundIndex    = utext_getNativeIndex(ut);
        TEST_ASSERT(expectedIndex == foundIndex);
        TEST_ASSERT(expectedC == foundC);
        if (gFailed) {
            return;
        }
    }

    //
    //  Backwards iteration, above, should have left our iterator
    //   position at zero, and continued backwards iterationshould fail.
    //
    foundIndex = utext_getNativeIndex(ut);
    TEST_ASSERT(foundIndex == 0);

    foundC = utext_previous32(ut);
    TEST_ASSERT(foundC == U_SENTINEL);
    foundIndex = utext_getNativeIndex(ut);
    TEST_ASSERT(foundIndex == 0);


    // And again, with the macros
    utext_setNativeIndex(ut, len);
    for (i=cpCount-1; i>=0; i--) {
        expectedC     = cpMap[i].cp;
        expectedIndex = cpMap[i].nativeIdx;
        foundC        = UTEXT_PREVIOUS32(ut);
        foundIndex    = utext_getNativeIndex(ut);
        TEST_ASSERT(expectedIndex == foundIndex);
        TEST_ASSERT(expectedC == foundC);
        if (gFailed) {
            return;
        }
    }

    //
    //  Backwards iteration, above, should have left our iterator
    //   position at zero, and continued backwards iterationshould fail.
    //
    foundIndex = utext_getNativeIndex(ut);
    TEST_ASSERT(foundIndex == 0);

    foundC = utext_previous32(ut);
    TEST_ASSERT(foundC == U_SENTINEL);
    foundIndex = utext_getNativeIndex(ut);
    TEST_ASSERT(foundIndex == 0);
    if (gFailed) {
        return;
    }

    //
    //  next32From(), prevous32From(), Iterate in a somewhat random order.
    //
    int  cpIndex = 0;
    for (i=0; i<cpCount; i++) {
        cpIndex = (cpIndex + 9973) % cpCount;
        index         = cpMap[cpIndex].nativeIdx;
        expectedC     = cpMap[cpIndex].cp;
        foundC        = utext_next32From(ut, index);
        TEST_ASSERT(expectedC == foundC);
        TEST_ASSERT(expectedIndex == foundIndex);
        if (gFailed) {
            return;
        }
    }

    cpIndex = 0;
    for (i=0; i<cpCount; i++) {
        cpIndex = (cpIndex + 9973) % cpCount;
        index         = cpMap[cpIndex+1].nativeIdx;
        expectedC     = cpMap[cpIndex].cp;
        foundC        = utext_previous32From(ut, index);
        TEST_ASSERT(expectedC == foundC);
        TEST_ASSERT(expectedIndex == foundIndex);
        if (gFailed) {
            return;
        }
    }


    //
    // moveIndex(int32_t delta);
    //

    // Walk through frontwards, incrementing by one
    utext_setNativeIndex(ut, 0);
    for (i=1; i<=cpCount; i++) {
        utext_moveIndex32(ut, 1);
        index = utext_getNativeIndex(ut);
        expectedIndex = cpMap[i].nativeIdx;
        TEST_ASSERT(expectedIndex == index);
    }

    // Walk through frontwards, incrementing by two
    utext_setNativeIndex(ut, 0);
    for (i=2; i<cpCount; i+=2) {
        utext_moveIndex32(ut, 2);
        index = utext_getNativeIndex(ut);
        expectedIndex = cpMap[i].nativeIdx;
        TEST_ASSERT(expectedIndex == index);
    }

    // walk through the string backwards, decrementing by one.
    i = cpMap[cpCount].nativeIdx;
    utext_setNativeIndex(ut, i);
    for (i=cpCount; i>=0; i--) {
        expectedIndex = cpMap[i].nativeIdx;
        index = utext_getNativeIndex(ut);
        TEST_ASSERT(expectedIndex == index);
        utext_moveIndex32(ut, -1);
    }


    // walk through backwards, decrementing by three
    i = cpMap[cpCount].nativeIdx;
    utext_setNativeIndex(ut, i);
    for (i=cpCount; i>=0; i-=3) {
        expectedIndex = cpMap[i].nativeIdx;
        index = utext_getNativeIndex(ut);
        TEST_ASSERT(expectedIndex == index);
        utext_moveIndex32(ut, -3);
    }


    //
    // Extract
    //
    int bufSize = us.length() + 10;
    UChar *buf = new UChar[bufSize];
    status = U_ZERO_ERROR;
    expectedLen = us.length();
    len = utext_extract(ut, 0, utlen, buf, bufSize, &status);
    TEST_SUCCESS(status);
    TEST_ASSERT(len == expectedLen);
    int compareResult = us.compare(buf, -1);
    TEST_ASSERT(compareResult == 0);

    status = U_ZERO_ERROR;
    len = utext_extract(ut, 0, utlen, NULL, 0, &status);
    TEST_ASSERT(status == U_BUFFER_OVERFLOW_ERROR)
    TEST_ASSERT(len == expectedLen);

    status = U_ZERO_ERROR;
    u_memset(buf, 0x5555, bufSize);
    len = utext_extract(ut, 0, utlen, buf, 1, &status);
    if (us.length() == 0) {
        TEST_SUCCESS(status);
        TEST_ASSERT(buf[0] == 0);
    } else {
        TEST_ASSERT(buf[0] == us.charAt(0));
        TEST_ASSERT(buf[1] == 0x5555);
        if (us.length() == 1) {
            TEST_ASSERT(status == U_STRING_NOT_TERMINATED_WARNING);
        } else {
            TEST_ASSERT(status == U_BUFFER_OVERFLOW_ERROR);
        }
    }

    delete buf;

}


