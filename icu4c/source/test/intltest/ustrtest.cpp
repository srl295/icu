/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-1999, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#include "ustrtest.h"
#include "unicode/unistr.h"
#include "unicode/unicode.h"
#include "unicode/locid.h"
#include <stdio.h>

#if U_IOSTREAM_SOURCE >= 199711
#include <iostream>
using namespace std;
#elif U_IOSTREAM_SOURCE >= 198506
#include <iostream.h>
#endif

UnicodeStringTest::UnicodeStringTest()
{
}

UnicodeStringTest::~UnicodeStringTest()
{
}

void UnicodeStringTest::runIndexedTest( int32_t index, UBool exec, const char* &name, char* /*par*/ )
{
    if (exec) logln("TestSuite LocaleTest: ");
    switch (index) {
        case 0: name = "TestBasicManipulation"; if (exec) TestBasicManipulation(); break;
        case 1: name = "TestCompare"; if (exec) TestCompare(); break;
        case 2: name = "TestExtract"; if (exec) TestExtract(); break;
        case 3: name = "TestRemoveReplace"; if (exec) TestRemoveReplace(); break;
        case 4: name = "TestCaseConversion"; if (exec) TestCaseConversion(); break;
        case 5: name = "TestNothing"; break;
        case 6: name = "TestSearching"; if (exec) TestSearching(); break;
        case 7: name = "TestSpacePadding"; if (exec) TestSpacePadding(); break;
        case 8: name = "TestPrefixAndSuffix"; if (exec) TestPrefixAndSuffix(); break;
        case 9: name = "TestFindAndReplace"; if (exec) TestFindAndReplace(); break;
        case 10: name = "TestCellWidth"; if (exec) TestCellWidth(); break;
        case 11: name = "TestReverse"; if (exec) TestReverse(); break;
        case 12: name = "TestMiscellaneous"; if (exec) TestMiscellaneous(); break;
        case 13: name = "TestStackAllocation"; if (exec) TestStackAllocation(); break;
        case 14: name = "TestUnescape"; if (exec) TestUnescape(); break;

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

    /* test compareCodePointOrder() */
    {
        /* these strings are in ascending order */
        static const UChar strings[5][3]={
            { 0x61, 0 },            /* U+0061 */
            { 0x20ac, 0 },          /* U+20ac */
            { 0xff61, 0 },          /* U+ff61 */
            { 0xd800, 0xdc02, 0 },  /* U+10002 */
            { 0xd84d, 0xdc56, 0 }   /* U+23456 */
        };
        UnicodeString u[5];
        int i;

        for(i=0; i<5; ++i) {
            u[i]=UnicodeString(TRUE, strings[i], -1);
        }

        for(i=0; i<4; ++i) {
            if(u[i].compareCodePointOrder(u[i+1])>=0) {
                errln("error: UnicodeString::compareCodePointOrder() fails for string %d and the following one\n", i);
            }
        }
    }
}

void
UnicodeStringTest::TestExtract()
{
    UnicodeString  test1("Now is the time for all good men to come to the aid of their country.");
    UnicodeString  test2;
    UChar          test3[13] = {1, 2, 3, 4, 5, 6, 7, 8, 8, 10, 11, 12, 13};
    char           test4[13] = {1, 2, 3, 4, 5, 6, 7, 8, 8, 10, 11, 12, 13};
    UnicodeString  test5;
    char           test6[13] = {1, 2, 3, 4, 5, 6, 7, 8, 8, 10, 11, 12, 13};

    test1.extract(11, 12, test2);
    test1.extract(11, 12, test3);
    if (test1.extract(11, 12, test4) != 12 || test4[12] != 13) {
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

    for (UTextOffset i = 0; i < 12; i++) {
        if (test1[(UTextOffset)(11 + i)] != test2[i]) {
            errln(UnicodeString("extracting into a UnicodeString failed at position ") + i);
            break;
        }
        if (test1[(UTextOffset)(11 + i)] != test3[i]) {
            errln(UnicodeString("extracting into an array of UChar failed at position ") + i);
            break;
        }
        if (((char)test1[(UTextOffset)(11 + i)]) != test4b[i]) {
            errln(UnicodeString("extracting into an array of char failed at position ") + i);
            break;
        }
        if (test1[(UTextOffset)(11 + i)] != test5[i]) {
            errln(UnicodeString("extracting with extractBetween failed at position ") + i);
            break;
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

    for (UTextOffset i = 0; i < test1.length(); i++)
        if (test5[i] != 0x53 && test5[i] != 0x50 && test5[i] != 0x41 && test5[i] != 0x4d && test5[i] != 0x20)
            test1[i] = 0x78;

    if (test1 != "xxx SPAM xx SPAM SPAM SPAM xx xxx SPAM")
        errln("One of the remove methods failed:\n"
              "  expected \"xxx SPAM xx SPAM SPAM SPAM xx xxx SPAM\",\n"
              "  got \"" + test1 + "\"");

    test1.remove();
    if (test1.length() != 0)
        errln("Remove() failed: expected empty string, got \"" + test1 + "\"");
}

void
UnicodeStringTest::TestCaseConversion()
{
    UChar uppercaseGreek[] =
        { 0x399, 0x395, 0x3a3, 0x3a5, 0x3a3, 0x20, 0x03a7, 0x3a1, 0x399, 0x3a3, 0x3a4,
        0x39f, 0x3a3, 0 };
        // "IESUS CHRISTOS"

    UChar lowercaseGreek[] = 
        { 0x3b9, 0x3b5, 0x3c3, 0x3c5, 0x3c2, 0x20, 0x03c7, 0x3c1, 0x3b9, 0x3c3, 0x3c4,
        0x3bf, 0x3c2, 0 };
        // "iesus christos"

    UChar lowercaseTurkish[] = 
        { 0x69, 0x73, 0x74, 0x61, 0x6e, 0x62, 0x75, 0x6c, 0x2c, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x63, 0x6f, 
        0x6e, 0x73, 0x74, 0x61, 0x6e, 0x74, 0x0131, 0x6e, 0x6f, 0x70, 0x6c, 0x65, 0x21, 0 };

    UChar uppercaseTurkish[] = 
        { 0x54, 0x4f, 0x50, 0x4b, 0x41, 0x50, 0x49, 0x20, 0x50, 0x41, 0x4c, 0x41, 0x43, 0x45, 0x2c, 0x20,
        0x0130, 0x53, 0x54, 0x41, 0x4e, 0x42, 0x55, 0x4c, 0 };
    
    UnicodeString expectedResult;
    UnicodeString   test3;

    test3 += (UChar32)0x0130;
    test3 += "STANBUL, NOT CONSTANTINOPLE!";

    UnicodeString   test4(test3);
    test4.toLower();
    expectedResult = "istanbul, not constantinople!";
    if (test4 != expectedResult)
        errln("1. toLower failed: expected \"" + expectedResult + "\", got \"" + test4 + "\".");

    test4 = test3;
    test4.toLower(Locale("tr", "TR"));
    expectedResult = lowercaseTurkish;
    if (test4 != expectedResult)
        errln("2. toLower failed: expected \"" + expectedResult + "\", got \"" + test4 + "\".");

    test3 = "topkap";
    test3 += (UChar32)0x0131;
    test3 += " palace, istanbul";
    test4 = test3;

    test4.toUpper();
    expectedResult = "TOPKAPI PALACE, ISTANBUL";
    if (test4 != expectedResult)
        errln("toUpper failed: expected \"" + expectedResult + "\", got \"" + test4 + "\".");

    test4 = test3;
    test4.toUpper(Locale("tr", "TR"));
    expectedResult = uppercaseTurkish;
    if (test4 != expectedResult)
        errln("toUpper failed: expected \"" + expectedResult + "\", got \"" + test4 + "\".");

    test3 = CharsToUnicodeString("S\\u00FC\\u00DFmayrstra\\u00DFe");

    test3.toUpper(Locale("de", "DE"));
    expectedResult = CharsToUnicodeString("S\\u00DCSSMAYRSTRASSE");
    if (test3 != expectedResult)
        errln("toUpper failed: expected \"" + expectedResult + "\", got \"" + test3 + "\".");
    
    test4.replace(0, test4.length(), uppercaseGreek);

    test4.toLower(Locale("el", "GR"));
    expectedResult = lowercaseGreek;
    if (test4 != expectedResult)
        errln("toLower failed: expected \"" + expectedResult + "\", got \"" + test4 + "\".");
    
    test4.replace(0, test4.length(), lowercaseGreek);

    test4.toUpper();
    expectedResult = uppercaseGreek;
    if (test4 != expectedResult)
        errln("toUpper failed: expected \"" + expectedResult + "\", got \"" + test4 + "\".");

    // more string case mapping tests with the new implementation
    {
        static const UChar

        beforeLower[]= { 0x61, 0x42, 0x49,  0x3a3, 0xdf, 0x3a3, 0x2f, 0xd93f, 0xdfff },
        lowerRoot[]=   { 0x61, 0x62, 0x69,  0x3c3, 0xdf, 0x3c2, 0x2f, 0xd93f, 0xdfff },
        lowerTurkish[]={ 0x61, 0x62, 0x131, 0x3c3, 0xdf, 0x3c2, 0x2f, 0xd93f, 0xdfff },

        beforeUpper[]= { 0x61, 0x42, 0x69,  0x3c2, 0xdf,       0x3c3, 0x2f, 0xfb03,           0xfb03,           0xfb03,           0xd93f, 0xdfff },
        upperRoot[]=   { 0x41, 0x42, 0x49,  0x3a3, 0x53, 0x53, 0x3a3, 0x2f, 0x46, 0x46, 0x49, 0x46, 0x46, 0x49, 0x46, 0x46, 0x49, 0xd93f, 0xdfff },
        upperTurkish[]={ 0x41, 0x42, 0x130, 0x3a3, 0x53, 0x53, 0x3a3, 0x2f, 0x46, 0x46, 0x49, 0x46, 0x46, 0x49, 0x46, 0x46, 0x49, 0xd93f, 0xdfff },

        beforeMiniUpper[]=  { 0xdf, 0x61 },
        miniUpper[]=        { 0x53, 0x53, 0x41 };

        UnicodeString s;

        /* lowercase with root locale */
        s=UnicodeString(FALSE, beforeLower, sizeof(beforeLower)/U_SIZEOF_UCHAR).toLower();
        if( s.length()!=(sizeof(lowerRoot)/U_SIZEOF_UCHAR) ||
            s!=UnicodeString(FALSE, lowerRoot, s.length())
        ) {
            errln("error in toLower(root locale)=\"" + s + "\" expected \"" + UnicodeString(FALSE, lowerRoot, sizeof(lowerRoot)/U_SIZEOF_UCHAR) + "\"");
        }

        /* lowercase with turkish locale */
        s=UnicodeString(FALSE, beforeLower, sizeof(beforeLower)/U_SIZEOF_UCHAR).setCharAt(0, beforeLower[0]).toLower(Locale("tr"));
        if( s.length()!=(sizeof(lowerTurkish)/U_SIZEOF_UCHAR) ||
            s!=UnicodeString(FALSE, lowerTurkish, s.length())
        ) {
            errln("error in toLower(turkish locale)=\"" + s + "\" expected \"" + UnicodeString(FALSE, lowerTurkish, sizeof(lowerTurkish)/U_SIZEOF_UCHAR) + "\"");
        }

        /* uppercase with root locale */
        s=UnicodeString(FALSE, beforeUpper, sizeof(beforeUpper)/U_SIZEOF_UCHAR).setCharAt(0, beforeUpper[0]).toUpper();
        if( s.length()!=(sizeof(upperRoot)/U_SIZEOF_UCHAR) ||
            s!=UnicodeString(FALSE, upperRoot, s.length())
        ) {
            errln("error in toUpper(root locale)=\"" + s + "\" expected \"" + UnicodeString(FALSE, upperRoot, sizeof(upperRoot)/U_SIZEOF_UCHAR) + "\"");
        }

        /* uppercase with turkish locale */
        s=UnicodeString(FALSE, beforeUpper, sizeof(beforeUpper)/U_SIZEOF_UCHAR).toUpper(Locale("tr"));
        if( s.length()!=(sizeof(upperTurkish)/U_SIZEOF_UCHAR) ||
            s!=UnicodeString(FALSE, upperTurkish, s.length())
        ) {
            errln("error in toUpper(turkish locale)=\"" + s + "\" expected \"" + UnicodeString(FALSE, upperTurkish, sizeof(upperTurkish)/U_SIZEOF_UCHAR) + "\"");
        }

        /* uppercase a short string with root locale */
        s=UnicodeString(FALSE, beforeMiniUpper, sizeof(beforeMiniUpper)/U_SIZEOF_UCHAR).setCharAt(0, beforeMiniUpper[0]).toUpper();
        if( s.length()!=(sizeof(miniUpper)/U_SIZEOF_UCHAR) ||
            s!=UnicodeString(FALSE, miniUpper, s.length())
        ) {
            errln("error in toUpper(root locale)=\"" + s + "\" expected \"" + UnicodeString(FALSE, miniUpper, sizeof(miniUpper)/U_SIZEOF_UCHAR) + "\"");
        }
    }
}

void
UnicodeStringTest::TestSearching()
{
    UnicodeString test1("test test ttest tetest testesteststt");
    UnicodeString test2("test");
    UChar testChar = 0x74;
    
    UChar32 testChar32 = 0x20402;
    UChar testData[]={0xd841, 0xdc02, 0x71, 0xdc02, 0xd841, 0x71, 0xd841, 0xdc02, 0x71, 0x72, 0xd841, 0xdc02, 0x71, 0xd841, 0xdc02, 0x71, 0xdc02, 0xd841, 0x73, 0x0000};
    UnicodeString test3(testData);
    UnicodeString test4(testChar32);



    uint16_t occurrences = 0;
    UTextOffset startPos = 0;
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

    UTextOffset endPos = 28;
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
    test1.findAndReplace(test3, test2, 2, 32);
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

    if (test1.numDisplayCells() != 44)
        errln("numDisplayCells() failed: expected 44, got " + test1.numDisplayCells());
    if (test2.numDisplayCells() != 11)
        errln("numDisplayCells() failed: expected 11, got " + test2.numDisplayCells());
    if (test3.numDisplayCells() != 20)
        errln("numDisplayCells() failed: expected 20, got " + test3.numDisplayCells());
    if (test4.numDisplayCells() != 11)
        errln("numDisplayCells() failed: expected 11, got " + test4.numDisplayCells());
    if (test5.numDisplayCells() != 11)
        errln("numDisplayCells() failed: expected 11, got " + test5.numDisplayCells());
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
}

void
UnicodeStringTest::TestMiscellaneous()
{
    UnicodeString   test1("This is a test");
    UnicodeString   test2("This is a test");
    UnicodeString   test3("Me too!");
    const UChar*  test4;

    if (test1.isBogus() || test2.isBogus() || test3.isBogus())
        errln("A string returned true for isBogus()!");

    if (test1.hashCode() != test2.hashCode() || test1.hashCode() == test3.hashCode())
        errln("hashCode() failed");

    test4 = test1.getUChars();

    if (test1 != test2)
        errln("getUChars() affected the string!");


    UTextOffset i;
    for (i = 0; i < test2.length(); i++){
        if (test2[i] != test4[i])
            errln(UnicodeString("getUChars() failed: strings differ at position ") + i);
    }

#if U_IOSTREAM_SOURCE
    logln("Testing the operator \"<<\" \n");
    cout<<"Testing the \"<<\" operator---test1="<<test1<<". "<<test3<<"\n";
#endif
}

void
UnicodeStringTest::TestStackAllocation()
{
     UChar            testString[] ={ 
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
    if( c32Test.length() != Unicode::charLength(0x10ff2a) ||
        c32Test.char32At(c32Test.length() - 1) != 0x10ff2a
    ) {
        errln("The UnicodeString(UChar32) constructor does not work with a 0x10ff2a filler");
    }

    // test the (new) capacity constructor
    UnicodeString capTest(5, 0x2a, 5);
    if( capTest.length() != 5 * Unicode::charLength(0x2a) ||
        capTest.char32At(0) != 0x2a ||
        capTest.char32At(4) != 0x2a
    ) {
        errln("The UnicodeString capacity constructor does not work with an ASCII filler");
    }

    capTest = UnicodeString(5, 0x10ff2a, 5);
    if( capTest.length() != 5 * Unicode::charLength(0x10ff2a) ||
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
    if (!UNICODE_STRING("wrong \\u sequence", 17).unescape().empty()) {
        errln("FAIL: unescaping of a string with an illegal escape sequence did not return an empty string");
    }
}
