/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2002, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#include "ustrtest.h"
#include "unicode/unistr.h"
#include "unicode/uchar.h"
#include "unicode/ustring.h"
#include "unicode/locid.h"
#include "unicode/ucnv.h"
#include "cmemory.h"

#if 0
#include "unicode/ustream.h"

#if U_IOSTREAM_SOURCE >= 199711
#include <iostream>
using namespace std;
#elif U_IOSTREAM_SOURCE >= 198506
#include <iostream.h>
#endif

#endif

#define LENGTHOF(array) (sizeof(array)/sizeof((array)[0]))

void UnicodeStringTest::runIndexedTest( int32_t index, UBool exec, const char* &name, char *par)
{
    if (exec) logln("TestSuite UnicodeStringTest: ");
    switch (index) {
        case 0: name = "TestBasicManipulation"; if (exec) TestBasicManipulation(); break;
        case 1: name = "TestCompare"; if (exec) TestCompare(); break;
        case 2: name = "TestExtract"; if (exec) TestExtract(); break;
        case 3: name = "TestRemoveReplace"; if (exec) TestRemoveReplace(); break;
        case 4:
            name = "StringCaseTest";
            if (exec) {
                logln("StringCaseTest---"); logln("");
                StringCaseTest test;
                callTest(test, par);
            }
            break;
        case 5: name = "TestSearching"; if (exec) TestSearching(); break;
        case 6: name = "TestSpacePadding"; if (exec) TestSpacePadding(); break;
        case 7: name = "TestPrefixAndSuffix"; if (exec) TestPrefixAndSuffix(); break;
        case 8: name = "TestFindAndReplace"; if (exec) TestFindAndReplace(); break;
        case 9: name = "TestCellWidth"; if (exec) TestCellWidth(); break;
        case 10: name = "TestReverse"; if (exec) TestReverse(); break;
        case 11: name = "TestMiscellaneous"; if (exec) TestMiscellaneous(); break;
        case 12: name = "TestStackAllocation"; if (exec) TestStackAllocation(); break;
        case 13: name = "TestUnescape"; if (exec) TestUnescape(); break;
        case 14: name = "TestCountChar32"; if (exec) TestCountChar32(); break;
        case 15: name = "TestBogus"; if (exec) TestBogus(); break;

        default: name = ""; break; //needed to end loop
    }
}

void
UnicodeStringTest::TestBasicManipulation()
{
    UnicodeString   test1("Now is the time for all men to come swiftly to the aid of the party.\n");
    UnicodeString   expectedValue;

    test1.insert(24, "good ");
    expectedValue = "Now is the time for all good men to come swiftly to the aid of the party.\n";
    if (test1 != expectedValue)
        errln("insert() failed:  expected \"" + expectedValue + "\"\n,got \"" + test1 + "\"");

    test1.remove(41, 8);
    expectedValue = "Now is the time for all good men to come to the aid of the party.\n";
    if (test1 != expectedValue)
        errln("remove() failed:  expected \"" + expectedValue + "\"\n,got \"" + test1 + "\"");
    
    test1.replace(58, 6, "ir country");
    expectedValue = "Now is the time for all good men to come to the aid of their country.\n";
    if (test1 != expectedValue)
        errln("replace() failed:  expected \"" + expectedValue + "\"\n,got \"" + test1 + "\"");
    
    UChar     temp[80];
    test1.extract(0, 15, temp);
    
    UnicodeString       test2(temp, 15);
    
    expectedValue = "Now is the time";
    if (test2 != expectedValue)
        errln("extract() failed:  expected \"" + expectedValue + "\"\n,got \"" + test2 + "\"");
    
    test2 += " for me to go!\n";
    expectedValue = "Now is the time for me to go!\n";
    if (test2 != expectedValue)
        errln("operator+=() failed:  expected \"" + expectedValue + "\"\n,got \"" + test2 + "\"");
    
    if (test1.length() != 70)
        errln("length() failed: expected 70, got " + test1.length());
    if (test2.length() != 30)
        errln("length() failed: expected 30, got " + test2.length());

    UnicodeString test3;
    test3.append((UChar32)0x20402);
    if(test3 != CharsToUnicodeString("\\uD841\\uDC02")){
        errln((UnicodeString)"append failed for UChar32, expected \"\\\\ud841\\\\udc02\", got " + prettify(test3));
    }
    if(test3.length() != 2){
        errln("append or length failed for UChar32, expected 2, got " + test3.length());
    }
    test3.append((UChar32)0x0074);
    if(test3 != CharsToUnicodeString("\\uD841\\uDC02t")){
        errln((UnicodeString)"append failed for UChar32, expected \"\\\\uD841\\\\uDC02t\", got " + prettify(test3));
    }
    if(test3.length() != 3){
        errln((UnicodeString)"append or length failed for UChar32, expected 2, got " + test3.length());
    }

    // test some UChar32 overloads
    if( test3.setTo((UChar32)0x10330).length() != 2 ||
        test3.insert(0, (UChar32)0x20100).length() != 4 ||
        test3.replace(2, 2, (UChar32)0xe0061).length() != 4 ||
        (test3 = (UChar32)0x14001).length() != 2
    ) {
        errln((UnicodeString)"simple UChar32 overloads for replace, insert, setTo or = failed");
    }

    {
        // test moveIndex32()
        UnicodeString s=UNICODE_STRING("\\U0002f999\\U0001d15f\\u00c4\\u1ed0", 32).unescape();

        if(
            s.moveIndex32(2, -1)!=0 ||
            s.moveIndex32(2, 1)!=4 ||
            s.moveIndex32(2, 2)!=5 ||
            s.moveIndex32(5, -2)!=2 ||
            s.moveIndex32(0, -1)!=0 ||
            s.moveIndex32(6, 1)!=6
        ) {
            errln("UnicodeString::moveIndex32() failed");
        }
    }

	{
		// test new 2.2 constructors and setTo function that parallel Java's substring function.
		UnicodeString src("Hello folks how are you?");
		UnicodeString target1("how are you?");
		if (target1 != UnicodeString(src, 12)) {
			errln("UnicodeString(const UnicodeString&, int32_t) failed");
		}
		UnicodeString target2("folks");
		if (target2 != UnicodeString(src, 6, 5)) {
			errln("UnicodeString(const UnicodeString&, int32_t, int32_t) failed");
		}
		if (target1 != target2.setTo(src, 12)) {
			errln("UnicodeString::setTo(const UnicodeString&, int32_t) failed");
		}
	}
}

void
UnicodeStringTest::TestCompare()
{
    UnicodeString   test1("this is a test");
    UnicodeString   test2("this is a test");
    UnicodeString   test3("this is a test of the emergency broadcast system");
    UnicodeString   test4("never say, \"this is a test\"!!");

    UnicodeString   test5((UChar)0x5000);
    UnicodeString   test6((UChar)0x5100);

    UChar         uniChars[] = { 0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 
                 0x20, 0x61, 0x20, 0x74, 0x65, 0x73, 0x74, 0 };
    char            chars[] = "this is a test";

    // test operator== and operator!=
    if (test1 != test2 || test1 == test3 || test1 == test4)
        errln("operator== or operator!= failed");

    // test operator> and operator<
    if (test1 > test2 || test1 < test2 || !(test1 < test3) || !(test1 > test4) ||
        !(test5 < test6)
    ) {
        errln("operator> or operator< failed");
    }

    // test operator>= and operator<=
    if (!(test1 >= test2) || !(test1 <= test2) || !(test1 <= test3) || !(test1 >= test4))
        errln("operator>= or operator<= failed");

    // test compare(UnicodeString)
    if (test1.compare(test2) != 0 || test1.compare(test3) >= 0 || test1.compare(test4) <= 0)
        errln("compare(UnicodeString) failed");

    //test compare(offset, length, UnicodeString)
    if(test1.compare(0, 14, test2) != 0 ||
        test3.compare(0, 14, test2) != 0 ||
        test4.compare(12, 14, test2) != 0 ||
        test3.compare(0, 18, test1) <=0  )
        errln("compare(offset, length, UnicodeString) failes");

    // test compare(UChar*)
    if (test2.compare(uniChars) != 0 || test3.compare(uniChars) <= 0 || test4.compare(uniChars) >= 0)
        errln("compare(UChar*) failed");

    // test compare(char*)
    if (test2.compare(chars) != 0 || test3.compare(chars) <= 0 || test4.compare(chars) >= 0)
        errln("compare(char*) failed");

    // test compare(UChar*, length)
    if (test1.compare(uniChars, 4) <= 0 || test1.compare(uniChars, 4) <= 0)
        errln("compare(UChar*, length) failed");

    // test compare(thisOffset, thisLength, that, thatOffset, thatLength)
    if (test1.compare(0, 14, test2, 0, 14) != 0 
    || test1.compare(0, 14, test3, 0, 14) != 0
    || test1.compare(0, 14, test4, 12, 14) != 0)
        errln("1. compare(thisOffset, thisLength, that, thatOffset, thatLength) failed");

    if (test1.compare(10, 4, test2, 0, 4) >= 0 
    || test1.compare(10, 4, test3, 22, 9) <= 0
    || test1.compare(10, 4, test4, 22, 4) != 0)
        errln("2. compare(thisOffset, thisLength, that, thatOffset, thatLength) failed");

    // test compareBetween
    if (test1.compareBetween(0, 14, test2, 0, 14) != 0 || test1.compareBetween(0, 14, test3, 0, 14) != 0
                    || test1.compareBetween(0, 14, test4, 12, 26) != 0)
        errln("compareBetween failed");

    if (test1.compareBetween(10, 14, test2, 0, 4) >= 0 || test1.compareBetween(10, 14, test3, 22, 31) <= 0
                    || test1.compareBetween(10, 14, test4, 22, 26) != 0)
        errln("compareBetween failed");

    // test compare() etc. with strings that share a buffer but are not equal
    test2=test1; // share the buffer, length() too large for the stackBuffer
    test2.truncate(1); // change only the length, not the buffer
    if( test1==test2 || test1<=test2 ||
        test1.compare(test2)<=0 ||
        test1.compareCodePointOrder(test2)<=0 ||
        test1.caseCompare(test2, U_FOLD_CASE_DEFAULT)<=0
    ) {
        errln("UnicodeStrings that share a buffer but have different lengths compare as equal");
    }

    /* test compareCodePointOrder() */
    {
        /* these strings are in ascending order */
        static const UChar strings[][4]={
            { 0x61, 0 },                    /* U+0061 */
            { 0x20ac, 0xd801, 0 },          /* U+20ac U+d801 */
            { 0x20ac, 0xd800, 0xdc00, 0 },  /* U+20ac U+10000 */
            { 0xd800, 0 },                  /* U+d800 */
            { 0xd800, 0xff61, 0 },          /* U+d800 U+ff61 */
            { 0xdfff, 0 },                  /* U+dfff */
            { 0xff61, 0xdfff, 0 },          /* U+ff61 U+dfff */
            { 0xff61, 0xd800, 0xdc02, 0 },  /* U+ff61 U+10002 */
            { 0xd800, 0xdc02, 0 },          /* U+10002 */
            { 0xd84d, 0xdc56, 0 }           /* U+23456 */
        };
        UnicodeString u[20]; // must be at least as long as strings[]
        int32_t i;

        for(i=0; i<(int32_t)(sizeof(strings)/sizeof(strings[0])); ++i) {
            u[i]=UnicodeString(TRUE, strings[i], -1);
        }

        for(i=0; i<(int32_t)(sizeof(strings)/sizeof(strings[0])-1); ++i) {
            if(u[i].compareCodePointOrder(u[i+1])>=0) {
                errln("error: UnicodeString::compareCodePointOrder() fails for string %d and the following one\n", i);
            }
        }
    }

    /* test caseCompare() */
    {
        static const UChar
        _mixed[]=               { 0x61, 0x42, 0x131, 0x3a3, 0xdf,       0x130,       0x49,  0xfb03,           0xd93f, 0xdfff, 0 },
        _otherDefault[]=        { 0x41, 0x62, 0x131, 0x3c3, 0x73, 0x53, 0x69, 0x307, 0x69,  0x46, 0x66, 0x49, 0xd93f, 0xdfff, 0 },
        _otherExcludeSpecialI[]={ 0x41, 0x62, 0x131, 0x3c3, 0x53, 0x73, 0x69,        0x131, 0x66, 0x46, 0x69, 0xd93f, 0xdfff, 0 },
        _different[]=           { 0x41, 0x62, 0x131, 0x3c3, 0x73, 0x53, 0x130,       0x49,  0x46, 0x66, 0x49, 0xd93f, 0xdffd, 0 };

        UnicodeString
            mixed(TRUE, _mixed, -1),
            otherDefault(TRUE, _otherDefault, -1),
            otherExcludeSpecialI(TRUE, _otherExcludeSpecialI, -1),
            different(TRUE, _different, -1);

        int8_t result;

        /* test caseCompare() */
        result=mixed.caseCompare(otherDefault, U_FOLD_CASE_DEFAULT);
        if(result!=0) {
            errln("error: mixed.caseCompare(other, default)=%ld instead of 0\n", result);
        }
        result=mixed.caseCompare(otherExcludeSpecialI, U_FOLD_CASE_EXCLUDE_SPECIAL_I);
        if(result!=0) {
            errln("error: mixed.caseCompare(otherExcludeSpecialI, U_FOLD_CASE_EXCLUDE_SPECIAL_I)=%ld instead of 0\n", result);
        }
        result=mixed.caseCompare(otherDefault, U_FOLD_CASE_EXCLUDE_SPECIAL_I);
        if(result==0) {
            errln("error: mixed.caseCompare(other, U_FOLD_CASE_EXCLUDE_SPECIAL_I)=0 instead of !=0\n");
        }

        /* test caseCompare() */
        result=mixed.caseCompare(different, U_FOLD_CASE_DEFAULT);
        if(result<=0) {
            errln("error: mixed.caseCompare(different, default)=%ld instead of positive\n", result);
        }

        /* test caseCompare() - include the folded sharp s (U+00df) with different lengths */
        result=mixed.caseCompare(1, 4, different, 1, 5, U_FOLD_CASE_DEFAULT);
        if(result!=0) {
            errln("error: mixed.caseCompare(mixed, 1, 4, different, 1, 5, default)=%ld instead of 0\n", result);
        }

        /* test caseCompare() - stop in the middle of the sharp s (U+00df) */
        result=mixed.caseCompare(1, 4, different, 1, 4, U_FOLD_CASE_DEFAULT);
        if(result<=0) {
            errln("error: mixed.caseCompare(1, 4, different, 1, 4, default)=%ld instead of positive\n", result);
        }
    }

    // test that srcLength=-1 is handled in functions that
    // take input const UChar */int32_t srcLength (j785)
    {
        static const UChar u[]={ 0x61, 0x308, 0x62, 0 };
        UnicodeString s=UNICODE_STRING("a\\u0308b", 8).unescape();

        if(s.compare(u, -1)!=0 || s.compare(0, 999, u, 0, -1)!=0) {
            errln("error UnicodeString::compare(..., const UChar *, srcLength=-1) does not work");
        }

        if(s.compareCodePointOrder(u, -1)!=0 || s.compareCodePointOrder(0, 999, u, 0, -1)!=0) {
            errln("error UnicodeString::compareCodePointOrder(..., const UChar *, srcLength=-1, ...) does not work");
        }

        if(s.caseCompare(u, -1, U_FOLD_CASE_DEFAULT)!=0 || s.caseCompare(0, 999, u, 0, -1, U_FOLD_CASE_DEFAULT)!=0) {
            errln("error UnicodeString::caseCompare(..., const UChar *, srcLength=-1, ...) does not work");
        }

        if(s.indexOf(u, 1, -1, 0, 999)!=1 || s.indexOf(u+1, -1, 0, 999)!=1 || s.indexOf(u+1, -1, 0)!=1) {
            errln("error UnicodeString::indexOf(const UChar *, srcLength=-1, ...) does not work");
        }

        if(s.lastIndexOf(u, 1, -1, 0, 999)!=1 || s.lastIndexOf(u+1, -1, 0, 999)!=1 || s.lastIndexOf(u+1, -1, 0)!=1) {
            errln("error UnicodeString::lastIndexOf(const UChar *, srcLength=-1, ...) does not work");
        }

        UnicodeString s2, s3;
        s2.replace(0, 0, u+1, -1);
        s3.replace(0, 0, u, 1, -1);
        if(s.compare(1, 999, s2)!=0 || s2!=s3) {
            errln("error UnicodeString::replace(..., const UChar *, srcLength=-1, ...) does not work");
        }
    }
}

void
UnicodeStringTest::TestExtract()
{
    UnicodeString  test1("Now is the time for all good men to come to the aid of their country.", "");
    UnicodeString  test2;
    UChar          test3[13] = {1, 2, 3, 4, 5, 6, 7, 8, 8, 10, 11, 12, 13};
    char           test4[13] = {1, 2, 3, 4, 5, 6, 7, 8, 8, 10, 11, 12, 13};
    UnicodeString  test5;
    char           test6[13] = {1, 2, 3, 4, 5, 6, 7, 8, 8, 10, 11, 12, 13};

    test1.extract(11, 12, test2);
    test1.extract(11, 12, test3);
    if (test1.extract(11, 12, test4) != 12 || test4[12] != 0) {
        errln("UnicodeString.extract(char *) failed to return the correct size of destination buffer.");
    }
    test1.extractBetween(11, 23, test5);
    if (test1.extract(60, 71, test6) != 9) {
        errln("UnicodeString.extract() failed to return the correct size of destination buffer for end of buffer.");
    }
    if (test1.extract(11, 12, test6) != 12) {
        errln("UnicodeString.extract() failed to return the correct size of destination buffer.");
    }

    // convert test4 back to Unicode for comparison
    UnicodeString test4b(test4, 12);

    if (test1.extract(11, 12, (char *)NULL) != 12) {
        errln("UnicodeString.extract(NULL) failed to return the correct size of destination buffer.");
    }
    if (test1.extract(11, -1, test6) != 0) {
        errln("UnicodeString.extract(-1) failed to stop reading the string.");
    }

    for (int32_t i = 0; i < 12; i++) {
        if (test1.charAt((int32_t)(11 + i)) != test2.charAt(i)) {
            errln(UnicodeString("extracting into a UnicodeString failed at position ") + i);
            break;
        }
        if (test1.charAt((int32_t)(11 + i)) != test3[i]) {
            errln(UnicodeString("extracting into an array of UChar failed at position ") + i);
            break;
        }
        if (((char)test1.charAt((int32_t)(11 + i))) != test4b.charAt(i)) {
            errln(UnicodeString("extracting into an array of char failed at position ") + i);
            break;
        }
        if (test1.charAt((int32_t)(11 + i)) != test5.charAt(i)) {
            errln(UnicodeString("extracting with extractBetween failed at position ") + i);
            break;
        }
    }

    // test preflighting and overflows with invariant conversion
    if (test1.extract(0, 10, (char *)NULL, "") != 10) {
        errln("UnicodeString.extract(0, 10, (char *)NULL, \"\") != 10");
    }

    test4[2] = (char)0xff;
    if (test1.extract(0, 10, test4, 2, "") != 10) {
        errln("UnicodeString.extract(0, 10, test4, 2, \"\") != 10");
    }
    if (test4[2] != (char)0xff) {
        errln("UnicodeString.extract(0, 10, test4, 2, \"\") overwrote test4[2]");
    }

    {
        // test new, NUL-terminating extract() function
        UnicodeString s("terminate", "");
        UChar dest[20]={
            0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
            0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5
        };
        UErrorCode errorCode;
        int32_t length;

        errorCode=U_ZERO_ERROR;
        length=s.extract((UChar *)NULL, 0, errorCode);
        if(errorCode!=U_BUFFER_OVERFLOW_ERROR || length!=s.length()) {
            errln("UnicodeString.extract(NULL, 0)==%d (%s) expected %d (U_BUFFER_OVERFLOW_ERROR)", length, s.length(), u_errorName(errorCode));
        }

        errorCode=U_ZERO_ERROR;
        length=s.extract(dest, s.length()-1, errorCode);
        if(errorCode!=U_BUFFER_OVERFLOW_ERROR || length!=s.length()) {
            errln("UnicodeString.extract(dest too short)==%d (%s) expected %d (U_BUFFER_OVERFLOW_ERROR)",
                length, u_errorName(errorCode), s.length());
        }

        errorCode=U_ZERO_ERROR;
        length=s.extract(dest, s.length(), errorCode);
        if(errorCode!=U_STRING_NOT_TERMINATED_WARNING || length!=s.length()) {
            errln("UnicodeString.extract(dest just right without NUL)==%d (%s) expected %d (U_STRING_NOT_TERMINATED_WARNING)",
                length, u_errorName(errorCode), s.length());
        }
        if(dest[length-1]!=s[length-1] || dest[length]!=0xa5) {
            errln("UnicodeString.extract(dest just right without NUL) did not extract the string correctly");
        }

        errorCode=U_ZERO_ERROR;
        length=s.extract(dest, s.length()+1, errorCode);
        if(errorCode!=U_ZERO_ERROR || length!=s.length()) {
            errln("UnicodeString.extract(dest large enough)==%d (%s) expected %d (U_ZERO_ERROR)",
                length, u_errorName(errorCode), s.length());
        }
        if(dest[length-1]!=s[length-1] || dest[length]!=0 || dest[length+1]!=0xa5) {
            errln("UnicodeString.extract(dest large enough) did not extract the string correctly");
        }
    }

    {
        // test new UConverter extract() and constructor
        UnicodeString s=UNICODE_STRING("\\U0002f999\\U0001d15f\\u00c4\\u1ed0", 32).unescape();
        char buffer[32];
        static const char expect[]={
            (char)0xf0, (char)0xaf, (char)0xa6, (char)0x99,
            (char)0xf0, (char)0x9d, (char)0x85, (char)0x9f,
            (char)0xc3, (char)0x84,
            (char)0xe1, (char)0xbb, (char)0x90
        };
        UErrorCode errorCode=U_ZERO_ERROR;
        UConverter *cnv=ucnv_open("UTF-8", &errorCode);
        int32_t length;

        if(U_SUCCESS(errorCode)) {
            // test preflighting
            if( (length=s.extract(NULL, 0, cnv, errorCode))!=13 ||
                errorCode!=U_BUFFER_OVERFLOW_ERROR
            ) {
                errln("UnicodeString::extract(NULL, UConverter) preflighting failed (length=%ld, %s)",
                      length, u_errorName(errorCode));
            }
            errorCode=U_ZERO_ERROR;
            if( (length=s.extract(buffer, 2, cnv, errorCode))!=13 ||
                errorCode!=U_BUFFER_OVERFLOW_ERROR
            ) {
                errln("UnicodeString::extract(too small, UConverter) preflighting failed (length=%ld, %s)",
                      length, u_errorName(errorCode));
            }

            // try error cases
            errorCode=U_ZERO_ERROR;
            if( s.extract(NULL, 2, cnv, errorCode)==13 || U_SUCCESS(errorCode)) {
                errln("UnicodeString::extract(UConverter) succeeded with an illegal destination");
            }
            errorCode=U_ILLEGAL_ARGUMENT_ERROR;
            if( s.extract(NULL, 0, cnv, errorCode)==13 || U_SUCCESS(errorCode)) {
                errln("UnicodeString::extract(UConverter) succeeded with a previous error code");
            }
            errorCode=U_ZERO_ERROR;

            // extract for real
            if( (length=s.extract(buffer, sizeof(buffer), cnv, errorCode))!=13 ||
                uprv_memcmp(buffer, expect, 13)!=0 ||
                buffer[13]!=0 ||
                U_FAILURE(errorCode)
            ) {
                errln("UnicodeString::extract(UConverter) conversion failed (length=%ld, %s)",
                      length, u_errorName(errorCode));
            }

            // try the constructor
            UnicodeString t(expect, sizeof(expect), cnv, errorCode);
            if(U_FAILURE(errorCode) || s!=t) {
                errln("UnicodeString(UConverter) conversion failed (%s)",
                      u_errorName(errorCode));
            }

            ucnv_close(cnv);
        }
    }
}

void
UnicodeStringTest::TestRemoveReplace()
{
    UnicodeString   test1("The rain in Spain stays mainly on the plain");
    UnicodeString   test2("eat SPAMburgers!");
    UChar         test3[] = { 0x53, 0x50, 0x41, 0x4d, 0x4d, 0 };
    char            test4[] = "SPAM";
    UnicodeString&  test5 = test1;

    test1.replace(4, 4, test2, 4, 4);
    test1.replace(12, 5, test3, 4);
    test3[4] = 0;
    test1.replace(17, 4, test3);
    test1.replace(23, 4, test4);
    test1.replaceBetween(37, 42, test2, 4, 8);

    if (test1 != "The SPAM in SPAM SPAMs SPAMly on the SPAM")
        errln("One of the replace methods failed:\n"
              "  expected \"The SPAM in SPAM SPAMs SPAMly on the SPAM\",\n"
              "  got \"" + test1 + "\"");

    test1.remove(21, 1);
    test1.removeBetween(26, 28);

    if (test1 != "The SPAM in SPAM SPAM SPAM on the SPAM")
        errln("One of the remove methods failed:\n"
              "  expected \"The SPAM in SPAM SPAM SPAM on the SPAM\",\n"
              "  got \"" + test1 + "\"");

    for (int32_t i = 0; i < test1.length(); i++) {
        if (test5[i] != 0x53 && test5[i] != 0x50 && test5[i] != 0x41 && test5[i] != 0x4d && test5[i] != 0x20) {
#ifdef U_USE_DEPRECATED_UCHAR_REFERENCE
            test1[i] = 0x78;
#else
            test1.setCharAt(i, 0x78);
#endif
        }
    }

    if (test1 != "xxx SPAM xx SPAM SPAM SPAM xx xxx SPAM")
        errln("One of the remove methods failed:\n"
              "  expected \"xxx SPAM xx SPAM SPAM SPAM xx xxx SPAM\",\n"
              "  got \"" + test1 + "\"");

    test1.remove();
    if (test1.length() != 0)
        errln("Remove() failed: expected empty string, got \"" + test1 + "\"");
}

void
UnicodeStringTest::TestSearching()
{
    UnicodeString test1("test test ttest tetest testesteststt");
    UnicodeString test2("test");
    UChar testChar = 0x74;
    
    UChar32 testChar32 = 0x20402;
    UChar testData[]={
        //   0       1       2       3       4       5       6       7
        0xd841, 0xdc02, 0x0071, 0xdc02, 0xd841, 0x0071, 0xd841, 0xdc02,

        //   8       9      10      11      12      13      14      15
        0x0071, 0x0072, 0xd841, 0xdc02, 0x0071, 0xd841, 0xdc02, 0x0071,

        //  16      17      18      19
        0xdc02, 0xd841, 0x0073, 0x0000
    };
    UnicodeString test3(testData);
    UnicodeString test4(testChar32);

    uint16_t occurrences = 0;
    int32_t startPos = 0;
    for ( ;
          startPos != -1 && startPos < test1.length();
          (startPos = test1.indexOf(test2, startPos)) != -1 ? (++occurrences, startPos += 4) : 0)
        ;
    if (occurrences != 6)
        errln("indexOf failed: expected to find 6 occurrences, found " + occurrences);
    
    for ( occurrences = 0, startPos = 10;
          startPos != -1 && startPos < test1.length();
          (startPos = test1.indexOf(test2, startPos)) != -1 ? (++occurrences, startPos += 4) : 0)
        ;
    if (occurrences != 4)
        errln("indexOf with starting offset failed: expected to find 4 occurrences, found " + occurrences);

    int32_t endPos = 28;
    for ( occurrences = 0, startPos = 5;
          startPos != -1 && startPos < test1.length();
          (startPos = test1.indexOf(test2, startPos, endPos - startPos)) != -1 ? (++occurrences, startPos += 4) : 0)
        ;
    if (occurrences != 4)
        errln("indexOf with starting and ending offsets failed: expected to find 4 occurrences, found " + occurrences);

    //using UChar32 string
    for ( startPos=0, occurrences=0;
          startPos != -1 && startPos < test3.length();
          (startPos = test3.indexOf(test4, startPos)) != -1 ? (++occurrences, startPos += 2) : 0)
        ;
    if (occurrences != 4)
        errln((UnicodeString)"indexOf failed: expected to find 4 occurrences, found " + occurrences);

    for ( startPos=10, occurrences=0;
          startPos != -1 && startPos < test3.length();
          (startPos = test3.indexOf(test4, startPos)) != -1 ? (++occurrences, startPos += 2) : 0)
        ;
    if (occurrences != 2)
        errln("indexOf failed: expected to find 2 occurrences, found " + occurrences);
    //---

    for ( occurrences = 0, startPos = 0;
          startPos != -1 && startPos < test1.length();
          (startPos = test1.indexOf(testChar, startPos)) != -1 ? (++occurrences, startPos += 1) : 0)
        ;
    if (occurrences != 16)
        errln("indexOf with character failed: expected to find 16 occurrences, found " + occurrences);

    for ( occurrences = 0, startPos = 10;
          startPos != -1 && startPos < test1.length();
          (startPos = test1.indexOf(testChar, startPos)) != -1 ? (++occurrences, startPos += 1) : 0)
        ;
    if (occurrences != 12)
        errln("indexOf with character & start offset failed: expected to find 12 occurrences, found " + occurrences);

    for ( occurrences = 0, startPos = 5, endPos = 28;
          startPos != -1 && startPos < test1.length();
          (startPos = test1.indexOf(testChar, startPos, endPos - startPos)) != -1 ? (++occurrences, startPos += 1) : 0)
        ;
    if (occurrences != 10)
        errln("indexOf with character & start & end offsets failed: expected to find 10 occurrences, found " + occurrences);

    //testing for UChar32
    UnicodeString subString;
    for( occurrences =0, startPos=0; startPos < test3.length(); startPos +=1){
        subString.append(test3, startPos, test3.length());
        if(subString.indexOf(testChar32) != -1 ){
             ++occurrences;
        }
        subString.remove();
    }
    if (occurrences != 14)
        errln((UnicodeString)"indexOf failed: expected to find 14 occurrences, found " + occurrences);

    for ( occurrences = 0, startPos = 0;
          startPos != -1 && startPos < test3.length();
          (startPos = test3.indexOf(testChar32, startPos)) != -1 ? (++occurrences, startPos += 1) : 0)
        ;
    if (occurrences != 4)
        errln((UnicodeString)"indexOf failed: expected to find 4 occurrences, found " + occurrences);
     
    endPos=test3.length();
    for ( occurrences = 0, startPos = 5;
          startPos != -1 && startPos < test3.length();
          (startPos = test3.indexOf(testChar32, startPos, endPos - startPos)) != -1 ? (++occurrences, startPos += 1) : 0)
        ;
    if (occurrences != 3)
        errln((UnicodeString)"indexOf with character & start & end offsets failed: expected to find 2 occurrences, found " + occurrences);
    //---

    for ( occurrences = 0, startPos = 32;
          startPos != -1;
          (startPos = test1.lastIndexOf(test2, 5, startPos - 5)) != -1 ? ++occurrences : 0)
        ;
    if (occurrences != 4)
        errln("lastIndexOf with starting and ending offsets failed: expected to find 4 occurrences, found " + occurrences);

    for ( occurrences = 0, startPos = 32;
          startPos != -1;
          (startPos = test1.lastIndexOf(testChar, 5, startPos - 5)) != -1 ? ++occurrences : 0)
        ;
    if (occurrences != 11)
        errln("lastIndexOf with character & start & end offsets failed: expected to find 11 occurrences, found " + occurrences);

    //testing UChar32
    startPos=test3.length();
    for ( occurrences = 0;
          startPos != -1;
          (startPos = test3.lastIndexOf(testChar32, 5, startPos - 5)) != -1 ? ++occurrences : 0)
        ;
    if (occurrences != 3)
        errln((UnicodeString)"lastIndexOf with character & start & end offsets failed: expected to find 3 occurrences, found " + occurrences);


    for ( occurrences = 0, endPos = test3.length();  endPos > 0; endPos -= 1){
        subString.remove();
        subString.append(test3, 0, endPos);
        if(subString.lastIndexOf(testChar32) != -1 ){
            ++occurrences;
        }
    }
    if (occurrences != 18)
        errln((UnicodeString)"indexOf failed: expected to find 18 occurrences, found " + occurrences);
    //---

    // test that indexOf(UChar32) and lastIndexOf(UChar32)
    // do not find surrogate code points when they are part of matched pairs
    // (= part of supplementary code points)
    // Jitterbug 1542
    if(test3.indexOf((UChar32)0xd841) != 4 || test3.indexOf((UChar32)0xdc02) != 3) {
        errln("error: UnicodeString::indexOf(UChar32 surrogate) finds a partial supplementary code point");
    }
    if(test3.lastIndexOf((UChar32)0xd841, 0, 17) != 4 || test3.lastIndexOf((UChar32)0xdc02, 0, 17) != 16) {
        errln("error: UnicodeString::lastIndexOf(UChar32 surrogate) finds a partial supplementary code point");
    }
}

void
UnicodeStringTest::TestSpacePadding()
{
    UnicodeString test1("hello");
    UnicodeString test2("   there");
    UnicodeString test3("Hi!  How ya doin'?  Beautiful day, isn't it?");
    UnicodeString test4;
    UBool returnVal;
    UnicodeString expectedValue;

    returnVal = test1.padLeading(15);
    expectedValue = "          hello";
    if (returnVal == FALSE || test1 != expectedValue)
        errln("padLeading() failed: expected \"" + expectedValue + "\", got \"" + test1 + "\".");

    returnVal = test2.padTrailing(15);
    expectedValue = "   there       ";
    if (returnVal == FALSE || test2 != expectedValue)
        errln("padTrailing() failed: expected \"" + expectedValue + "\", got \"" + test2 + "\".");

    expectedValue = test3;
    returnVal = test3.padTrailing(15);
    if (returnVal == TRUE || test3 != expectedValue)
        errln("padTrailing() failed: expected \"" + expectedValue + "\", got \"" + test3 + "\".");

    expectedValue = "hello";
    test4.setTo(test1).trim();

    if (test4 != expectedValue || test1 == expectedValue || test4 != expectedValue)
        errln("trim(UnicodeString&) failed");
    
    test1.trim();
    if (test1 != expectedValue)
        errln("trim() failed: expected \"" + expectedValue + "\", got \"" + test1 + "\".");

    test2.trim();
    expectedValue = "there";
    if (test2 != expectedValue)
        errln("trim() failed: expected \"" + expectedValue + "\", got \"" + test2 + "\".");

    test3.trim();
    expectedValue = "Hi!  How ya doin'?  Beautiful day, isn't it?";
    if (test3 != expectedValue)
        errln("trim() failed: expected \"" + expectedValue + "\", got \"" + test3 + "\".");

    returnVal = test1.truncate(15);
    expectedValue = "hello";
    if (returnVal == TRUE || test1 != expectedValue)
        errln("truncate() failed: expected \"" + expectedValue + "\", got \"" + test1 + "\".");

    returnVal = test2.truncate(15);
    expectedValue = "there";
    if (returnVal == TRUE || test2 != expectedValue)
        errln("truncate() failed: expected \"" + expectedValue + "\", got \"" + test2 + "\".");

    returnVal = test3.truncate(15);
    expectedValue = "Hi!  How ya doi";
    if (returnVal == FALSE || test3 != expectedValue)
        errln("truncate() failed: expected \"" + expectedValue + "\", got \"" + test3 + "\".");
}

void
UnicodeStringTest::TestPrefixAndSuffix()
{
    UnicodeString test1("Now is the time for all good men to come to the aid of their country.");
    UnicodeString test2("Now");
    UnicodeString test3("country.");
    UnicodeString test4("count");

    if (!test1.startsWith(test2))
        errln("startsWith() failed: \"" + test2 + "\" should be a prefix of \"" + test1 + "\".");

    if (test1.startsWith(test3))
        errln("startsWith() failed: \"" + test3 + "\" shouldn't be a prefix of \"" + test1 + "\".");

    if (test1.endsWith(test2))
        errln("endsWith() failed: \"" + test2 + "\" shouldn't be a suffix of \"" + test1 + "\".");

    if (!test1.endsWith(test3))
        errln("endsWith() failed: \"" + test3 + "\" should be a suffix of \"" + test1 + "\".");

    if (!test3.startsWith(test4))
        errln("startsWith() failed: \"" + test4 + "\" should be a prefix of \"" + test3 + "\".");

    if (test4.startsWith(test3))
        errln("startsWith() failed: \"" + test3 + "\" shouldn't be a prefix of \"" + test4 + "\".");
}

void
UnicodeStringTest::TestFindAndReplace()
{
    UnicodeString test1("One potato, two potato, three potato, four\n");
    UnicodeString test2("potato");
    UnicodeString test3("MISSISSIPPI");

    UnicodeString expectedValue;

    test1.findAndReplace(test2, test3);
    expectedValue = "One MISSISSIPPI, two MISSISSIPPI, three MISSISSIPPI, four\n";
    if (test1 != expectedValue)
        errln("findAndReplace failed: expected \"" + expectedValue + "\", got \"" + test1 + "\".");
    test1.findAndReplace(2, 32, test3, test2);
    expectedValue = "One potato, two potato, three MISSISSIPPI, four\n";
    if (test1 != expectedValue)
        errln("findAndReplace failed: expected \"" + expectedValue + "\", got \"" + test1 + "\".");
}

void
UnicodeStringTest::TestCellWidth()
{
    UChar     testData2[] = { 0x4d, 0x6f, 0x308, 0x74, 0x6c, 0x65, 0x79, 0x20, 0x43, 0x72, 0x75, 0x308, 0x65, 0x0000 };
    UChar     testData3[] = { 0x31, 0x39, 0x39, 0x37, 0x5e74, 0x20, 0x516d, 0x6708, 0x20, 0x30, 0x33, 0x65e5, 0x5e73, 0x6210, 0x0000 };
    UChar     testData4[] = { 0x39, 0x37, 0xb144, 0x36, 0xc6d4, 0x30, 0x33, 0xc77c, 0x0000 };
    UChar     testData5[] = { 0x39, 0x37, 0x1103, 0x1167, 0x11ab, 0x36, 0x110b, 0x117b, 0x11af, 0x30, 0x33, 0x110b, 0x1175, 0x11af, 0x0000 };

    UnicodeString   test1("The rain in Spain stays mainly on the plain.");
    UnicodeString   test2(testData2);
    UnicodeString   test3(testData3);
    UnicodeString   test4(testData4);
    UnicodeString   test5(testData5);
    int32_t testVal = test1.numDisplayCells();

    if (testVal != 44)
        errln("test1.numDisplayCells() failed: expected 44, got %d", testVal);
    testVal = test2.numDisplayCells();
    if (testVal != 11)
        errln("test2.numDisplayCells() failed: expected 11, got %d", testVal);
    testVal = test3.numDisplayCells();
    if (testVal != 20)
        errln("test3.numDisplayCells() failed: expected 20, got %d", testVal);
    testVal = test4.numDisplayCells();
    if (testVal != 11)
        errln("test4.numDisplayCells() failed: expected 11, got %d", testVal);
    testVal = test5.numDisplayCells();
    if (testVal != 11)
        errln("test5.numDisplayCells() failed: expected 11, got %d", testVal);
}

void
UnicodeStringTest::TestReverse()
{
    UnicodeString test("backwards words say to used I");

    test.reverse();
    test.reverse(2, 4);
    test.reverse(7, 2);
    test.reverse(10, 3);
    test.reverse(14, 5);
    test.reverse(20, 9);

    if (test != "I used to say words backwards")
        errln("reverse() failed:  Expected \"I used to say words backwards\",\n got \""
            + test + "\"");

    test=UNICODE_STRING("\\U0002f999\\U0001d15f\\u00c4\\u1ed0", 32).unescape();
    test.reverse();
    if(test.char32At(0)!=0x1ed0 || test.char32At(1)!=0xc4 || test.char32At(2)!=0x1d15f || test.char32At(4)!=0x2f999) {
        errln("reverse() failed with supplementary characters");
    }
}

void
UnicodeStringTest::TestMiscellaneous()
{
    UnicodeString   test1("This is a test");
    UnicodeString   test2("This is a test");
    UnicodeString   test3("Me too!");

    // test getBuffer(minCapacity) and releaseBuffer()
    test1=UnicodeString(); // make sure that it starts with its stackBuffer
    UChar *p=test1.getBuffer(20);
    if(test1.getCapacity()<20) {
        errln("UnicodeString::getBuffer(20).getCapacity()<20");
    }

    test1.append((UChar)7); // must not be able to modify the string here
    test1.setCharAt(3, 7);
    test1.reverse();
    if( test1.length()!=0 ||
        test1.charAt(0)!=0xffff || test1.charAt(3)!=0xffff ||
        test1.getBuffer(10)!=0 || test1.getBuffer()!=0
    ) {
        errln("UnicodeString::getBuffer(minCapacity) allows read or write access to the UnicodeString");
    }

    p[0]=1;
    p[1]=2;
    p[2]=3;
    test1.releaseBuffer(3);
    test1.append((UChar)4);

    if(test1.length()!=4 || test1.charAt(0)!=1 || test1.charAt(1)!=2 || test1.charAt(2)!=3 || test1.charAt(3)!=4) {
        errln("UnicodeString::releaseBuffer(newLength) does not properly reallow access to the UnicodeString");
    }

    // test releaseBuffer() without getBuffer(minCapacity) - must not have any effect
    test1.releaseBuffer(1);
    if(test1.length()!=4 || test1.charAt(0)!=1 || test1.charAt(1)!=2 || test1.charAt(2)!=3 || test1.charAt(3)!=4) {
        errln("UnicodeString::releaseBuffer(newLength) without getBuffer(minCapacity) changed the UnicodeString");
    }

    // test getBuffer(const)
    const UChar *q=test1.getBuffer(), *r=test1.getBuffer();
    if( test1.length()!=4 ||
        q[0]!=1 || q[1]!=2 || q[2]!=3 || q[3]!=4 ||
        r[0]!=1 || r[1]!=2 || r[2]!=3 || r[3]!=4
    ) {
        errln("UnicodeString::getBuffer(const) does not return a usable buffer pointer");
    }

    // test releaseBuffer() with a NUL-terminated buffer
    test1.getBuffer(20)[2]=0;
    test1.releaseBuffer(); // implicit -1
    if(test1.length()!=2 || test1.charAt(0)!=1 || test1.charAt(1) !=2) {
        errln("UnicodeString::releaseBuffer(-1) does not properly set the length of the UnicodeString");
    }

    // test releaseBuffer() with a non-NUL-terminated buffer
    p=test1.getBuffer(256);
    for(int32_t i=0; i<test1.getCapacity(); ++i) {
        p[i]=(UChar)1;      // fill the buffer with all non-NUL code units
    }
    test1.releaseBuffer();  // implicit -1
    if(test1.length()!=test1.getCapacity() || test1.charAt(1)!=1 || test1.charAt(100)!=1 || test1.charAt(test1.getCapacity()-1)!=1) {
        errln("UnicodeString::releaseBuffer(-1 but no NUL) does not properly set the length of the UnicodeString");
    }

    // test getTerminatedBuffer()
    test1=UnicodeString("This is another test.", "");
    test2=UnicodeString("This is another test.", "");
    q=test1.getTerminatedBuffer();
    if(q[test1.length()]!=0 || test1!=test2 || test2.compare(q, -1)!=0) {
        errln("getTerminatedBuffer()[length]!=0");
    }

    const UChar u[]={ 5, 6, 7, 8, 0 };
    test1.setTo(FALSE, u, 3);
    q=test1.getTerminatedBuffer();
    if(q==u || q[0]!=5 || q[1]!=6 || q[2]!=7 || q[3]!=0) {
        errln("UnicodeString(u[3]).getTerminatedBuffer() returns a bad buffer");
    }

    test1.setTo(TRUE, u, -1);
    q=test1.getTerminatedBuffer();
    if(q!=u || test1.length()!=4 || q[3]!=8 || q[4]!=0) {
        errln("UnicodeString(u[-1]).getTerminatedBuffer() returns a bad buffer");
    }

/*
#if U_IOSTREAM_SOURCE
    logln("Testing the operator \"<<\" \n");
    cout<<"Testing the \"<<\" operator---test1="<<test1<<". "<<test3<<endl;
#endif
*/
}

void
UnicodeStringTest::TestStackAllocation()
{
    UChar           testString[] ={ 
        0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x63, 0x72, 0x61, 0x7a, 0x79, 0x20, 0x74, 0x65, 0x73, 0x74, 0x2e, 0 };
    UChar           guardWord = 0x4DED;
    UnicodeString*  test = 0;

    test = new  UnicodeString(testString);
    if (*test != "This is a crazy test.")
        errln("Test string failed to initialize properly.");
    if (guardWord != 0x04DED)
        errln("Test string initialization overwrote guard word!");

    test->insert(8, "only ");
    test->remove(15, 6);
    if (*test != "This is only a test.")
        errln("Manipulation of test string failed to work right.");
    if (guardWord != 0x4DED)
        errln("Manipulation of test string overwrote guard word!");

    // we have to deinitialize and release the backing store by calling the destructor
    // explicitly, since we can't overload operator delete
    delete test;

    UChar workingBuffer[] = {
        0x4e, 0x6f, 0x77, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74, 0x69, 0x6d, 0x65, 0x20,
        0x66, 0x6f, 0x72, 0x20, 0x61, 0x6c, 0x6c, 0x20, 0x6d, 0x65, 0x6e, 0x20, 0x74, 0x6f, 0x20,
        0x63, 0x6f, 0x6d, 0x65, 0xffff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    UChar guardWord2 = 0x4DED;

    test = new UnicodeString(workingBuffer, 35, 100);
    if (*test != "Now is the time for all men to come")
        errln("Stack-allocated backing store failed to initialize correctly.");
    if (guardWord2 != 0x4DED)
        errln("Stack-allocated backing store overwrote guard word!");

    test->insert(24, "good ");
    if (*test != "Now is the time for all good men to come")
        errln("insert() on stack-allocated UnicodeString didn't work right");
    if (guardWord2 != 0x4DED)
        errln("insert() on stack-allocated UnicodeString overwrote guard word!");

    if (workingBuffer[24] != 0x67)
        errln("insert() on stack-allocated UnicodeString didn't affect backing store");

    *test += " to the aid of their country.";
    if (*test != "Now is the time for all good men to come to the aid of their country.")
        errln("Stack-allocated UnicodeString overflow didn't work");
    if (guardWord2 != 0x4DED)
        errln("Stack-allocated UnicodeString overflow overwrote guard word!");

    *test = "ha!";
    if (*test != "ha!")
        errln("Assignment to stack-allocated UnicodeString didn't work");
    if (workingBuffer[0] != 0x4e)
        errln("Change to UnicodeString after overflow are still affecting original buffer");
    if (guardWord2 != 0x4DED)
        errln("Change to UnicodeString after overflow overwrote guard word!");

    // test read-only aliasing with setTo()
    workingBuffer[0] = 0x20ac;
    workingBuffer[1] = 0x125;
    workingBuffer[2] = 0;
    test->setTo(TRUE, workingBuffer, 2);
    if(test->length() != 2 || test->charAt(0) != 0x20ac || test->charAt(1) != 0x125) {
        errln("UnicodeString.setTo(readonly alias) does not alias correctly");
    }
    workingBuffer[1] = 0x109;
    if(test->charAt(1) != 0x109) {
        errln("UnicodeString.setTo(readonly alias) made a copy: did not see change in buffer");
    }

    test->setTo(TRUE, workingBuffer, -1);
    if(test->length() != 2 || test->charAt(0) != 0x20ac || test->charAt(1) != 0x109) {
        errln("UnicodeString.setTo(readonly alias, length -1) does not alias correctly");
    }

    test->setTo(FALSE, workingBuffer, -1);
    if(!test->isBogus()) {
        errln("UnicodeString.setTo(unterminated readonly alias, length -1) does not result in isBogus()");
    }
    
    delete test;
     
    test=new UnicodeString();
    UChar buffer[]={0x0061, 0x0062, 0x20ac, 0x0043, 0x0042, 0x0000};
    test->setTo(buffer, 4, 10);
    if(test->length() !=4 || test->charAt(0) != 0x0061 || test->charAt(1) != 0x0062 ||
        test->charAt(2) != 0x20ac || test->charAt(3) != 0x0043){
        errln((UnicodeString)"UnicodeString.setTo(UChar*, length, capacity) does not work correctly\n" + prettify(*test));
    }
    delete test;


    // test the UChar32 constructor
    UnicodeString c32Test((UChar32)0x10ff2a);
    if( c32Test.length() != UTF_CHAR_LENGTH(0x10ff2a) ||
        c32Test.char32At(c32Test.length() - 1) != 0x10ff2a
    ) {
        errln("The UnicodeString(UChar32) constructor does not work with a 0x10ff2a filler");
    }

    // test the (new) capacity constructor
    UnicodeString capTest(5, (UChar32)0x2a, 5);
    if( capTest.length() != 5 * UTF_CHAR_LENGTH(0x2a) ||
        capTest.char32At(0) != 0x2a ||
        capTest.char32At(4) != 0x2a
    ) {
        errln("The UnicodeString capacity constructor does not work with an ASCII filler");
    }

    capTest = UnicodeString(5, (UChar32)0x10ff2a, 5);
    if( capTest.length() != 5 * UTF_CHAR_LENGTH(0x10ff2a) ||
        capTest.char32At(0) != 0x10ff2a ||
        capTest.char32At(4) != 0x10ff2a
    ) {
        errln("The UnicodeString capacity constructor does not work with a 0x10ff2a filler");
    }

    capTest = UnicodeString(5, (UChar32)0, 0);
    if(capTest.length() != 0) {
        errln("The UnicodeString capacity constructor does not work with a 0x10ff2a filler");
    }
}

/**
 * Test the unescape() function.
 */
void UnicodeStringTest::TestUnescape(void) {
    UnicodeString IN("abc\\u4567 \\n\\r \\U00101234xyz");
    UnicodeString OUT("abc");
    OUT.append((UChar)0x4567);
    OUT.append(" ");
    OUT.append((UChar)0xA);
    OUT.append((UChar)0xD);
    OUT.append(" ");
    OUT.append((UChar32)0x00101234);
    OUT.append("xyz");
    UnicodeString result = IN.unescape();
    if (result != OUT) {
        errln("FAIL: " + prettify(IN) + ".unescape() -> " +
              prettify(result) + ", expected " +
              prettify(OUT));
    }

    // test that an empty string is returned in case of an error
    if (!UNICODE_STRING("wrong \\u sequence", 17).unescape().isEmpty()) {
        errln("FAIL: unescaping of a string with an illegal escape sequence did not return an empty string");
    }
}

/* test code point counting functions --------------------------------------- */

/* reference implementation of UnicodeString::hasMoreChar32Than() */
static int32_t
_refUnicodeStringHasMoreChar32Than(const UnicodeString &s, int32_t start, int32_t length, int32_t number) {
    int32_t count=s.countChar32(start, length);
    return count>number;
}

/* compare the real function against the reference */
void
UnicodeStringTest::_testUnicodeStringHasMoreChar32Than(const UnicodeString &s, int32_t start, int32_t length, int32_t number) {
    if(s.hasMoreChar32Than(start, length, number)!=_refUnicodeStringHasMoreChar32Than(s, start, length, number)) {
        errln("hasMoreChar32Than(%d, %d, %d)=%hd is wrong\n",
                start, length, number, s.hasMoreChar32Than(start, length, number));
    }
}

void
UnicodeStringTest::TestCountChar32(void) {
    {
        UnicodeString s=UNICODE_STRING("\\U0002f999\\U0001d15f\\u00c4\\u1ed0", 32).unescape();

        // test countChar32()
        // note that this also calls and tests u_countChar32(length>=0)
        if(
            s.countChar32()!=4 ||
            s.countChar32(1)!=4 ||
            s.countChar32(2)!=3 ||
            s.countChar32(2, 3)!=2 ||
            s.countChar32(2, 0)!=0
        ) {
            errln("UnicodeString::countChar32() failed");
        }

        // NUL-terminate the string buffer and test u_countChar32(length=-1)
        const UChar *buffer=s.getTerminatedBuffer();
        if(
            u_countChar32(buffer, -1)!=4 ||
            u_countChar32(buffer+1, -1)!=4 ||
            u_countChar32(buffer+2, -1)!=3 ||
            u_countChar32(buffer+3, -1)!=3 ||
            u_countChar32(buffer+4, -1)!=2 ||
            u_countChar32(buffer+5, -1)!=1 ||
            u_countChar32(buffer+6, -1)!=0
        ) {
            errln("u_countChar32(length=-1) failed");
        }

        // test u_countChar32() with bad input
        if(u_countChar32(NULL, 5)!=0 || u_countChar32(buffer, -2)!=0) {
            errln("u_countChar32(bad input) failed (returned non-zero counts)");
        }
    }

    /* test data and variables for hasMoreChar32Than() */
    static const UChar str[]={
        0x61, 0x62, 0xd800, 0xdc00,
        0xd801, 0xdc01, 0x63, 0xd802,
        0x64, 0xdc03, 0x65, 0x66,
        0xd804, 0xdc04, 0xd805, 0xdc05,
        0x67
    };
    UnicodeString string(str, LENGTHOF(str));
    int32_t start, length, number;

    /* test hasMoreChar32Than() */
    for(length=string.length(); length>=0; --length) {
        for(start=0; start<=length; ++start) {
            for(number=-1; number<=((length-start)+2); ++number) {
                _testUnicodeStringHasMoreChar32Than(string, start, length-start, number);
            }
        }
    }

    /* test hasMoreChar32Than() with pinning */
    for(start=-1; start<=string.length()+1; ++start) {
        for(number=-1; number<=((string.length()-start)+2); ++number) {
            _testUnicodeStringHasMoreChar32Than(string, start, 0x7fffffff, number);
        }
    }

    /* test hasMoreChar32Than() with a bogus string */
    string.setToBogus();
    for(length=-1; length<=1; ++length) {
        for(start=-1; start<=length; ++start) {
            for(number=-1; number<=((length-start)+2); ++number) {
                _testUnicodeStringHasMoreChar32Than(string, start, length-start, number);
            }
        }
    }
}

void
UnicodeStringTest::TestBogus() {
    UnicodeString   test1("This is a test");
    UnicodeString   test2("This is a test");
    UnicodeString   test3("Me too!");

    // test isBogus() and setToBogus()
    if (test1.isBogus() || test2.isBogus() || test3.isBogus()) {
        errln("A string returned TRUE for isBogus()!");
    }

    test3.setTo(FALSE, (const UChar *)0, -1);
    if(!test3.isBogus()) {
        errln("A bogus string returned FALSE for isBogus()!");
    }
    if (test1.hashCode() != test2.hashCode() || test1.hashCode() == test3.hashCode()) {
        errln("hashCode() failed");
    }
    if(test3.getBuffer()!=0 || test3.getBuffer(20)!=0 || test3.getTerminatedBuffer()!=0) {
        errln("bogus.getBuffer()!=0");
    }

    // verify that non-assignment modifications fail and do not revive a bogus string
    test3.append((UChar)0x61);
    if(!test3.isBogus() || test3.getBuffer()!=0) {
        errln("bogus.append('a') worked but must not");
    }

    test3.findAndReplace(UnicodeString((UChar)0x61), test2);
    if(!test3.isBogus() || test3.getBuffer()!=0) {
        errln("bogus.findAndReplace() worked but must not");
    }

    test3.trim();
    if(!test3.isBogus() || test3.getBuffer()!=0) {
        errln("bogus.trim() revived bogus but must not");
    }

    test3.remove();
    if(!test3.isBogus() || test3.getBuffer()!=0) {
        errln("bogus.remove() revived bogus but must not");
    }

    if(!test3.setCharAt(0, 0x62).isBogus() || !test3.isEmpty()) {
        errln("bogus.setCharAt(0, 'b') worked but must not");
    }

    if(test3.truncate(0) || !test3.isBogus() || !test3.isEmpty()) {
        errln("bogus.truncate(0) revived bogus but must not");
    }

    // verify that assignments revive a bogus string
    test3.setToBogus();
    if(!test3.isBogus() || (test3=test1).isBogus() || test3!=test1) {
        errln("bogus.operator=() failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.fastCopyFrom(test1).isBogus() || test3!=test1) {
        errln("bogus.fastCopyFrom() failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo(test1).isBogus() || test3!=test1) {
        errln("bogus.setTo(UniStr) failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo(test1, 0).isBogus() || test3!=test1) {
        errln("bogus.setTo(UniStr, 0) failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo(test1, 0, 0x7fffffff).isBogus() || test3!=test1) {
        errln("bogus.setTo(UniStr, 0, len) failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo(test1.getBuffer(), test1.length()).isBogus() || test3!=test1) {
        errln("bogus.setTo(const UChar *, len) failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo((UChar)0x2028).isBogus() || test3!=UnicodeString((UChar)0x2028)) {
        errln("bogus.setTo(UChar) failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo((UChar32)0x1d157).isBogus() || test3!=UnicodeString((UChar32)0x1d157)) {
        errln("bogus.setTo(UChar32) failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo(FALSE, test1.getBuffer(), test1.length()).isBogus() || test3!=test1) {
        errln("bogus.setTo(readonly alias) failed");
    }

    // writable alias to another string's buffer: very bad idea, just convenient for this test
    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo((UChar *)test1.getBuffer(), test1.length(), test1.getCapacity()).isBogus() || test3!=test1) {
        errln("bogus.setTo(writable alias) failed");
    }

    // same with simple, documented ways to turn a bogus string into an empty one
    test3.setToBogus();
    if(!test3.isBogus() || (test3=UnicodeString()).isBogus() || !test3.isEmpty()) {
        errln("bogus.operator=(UnicodeString()) failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo(UnicodeString()).isBogus() || !test3.isEmpty()) {
        errln("bogus.setTo(UnicodeString()) failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo((UChar32)-1).isBogus() || !test3.isEmpty()) {
        errln("bogus.setTo((UChar32)-1) failed");
    }

    static const UChar nul=0;

    test3.setToBogus();
    if(!test3.isBogus() || test3.setTo(&nul, 0).isBogus() || !test3.isEmpty()) {
        errln("bogus.setTo(&nul, 0) failed");
    }

    test3.setToBogus();
    if(!test3.isBogus() || test3.getBuffer()!=0) {
        errln("setToBogus() failed to make a string bogus");
    }

    if(test1.isBogus() || !(test1=test3).isBogus()) {
        errln("normal=bogus failed to make the left string bogus");
    }
}
