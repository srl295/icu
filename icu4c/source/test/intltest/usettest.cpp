/*
**********************************************************************
*   Copyright (C) 2001 Alan Liu ,International Business Machines Corporation and
*   others. All Rights Reserved.
**********************************************************************
*   Date        Name        Description
*   10/20/99    alan        Creation.
*   03/22/2000  Madhu       Added additional tests
**********************************************************************
*/

#include "unicode/utypes.h"
#include "usettest.h"
#include "unicode/uniset.h"
#include "unicode/unicode.h"


UnicodeString operator+(const UnicodeString& left, const UnicodeSet& set) {
    UnicodeString pat;
    set.toPattern(pat);
    return left + UnicodeSetTest::escape(pat);
}

#define CASE(id,test) case id:                          \
                          name = #test;                 \
                          if (exec) {                   \
                              logln(#test "---");       \
                              logln((UnicodeString)""); \
                              test();                   \
                          }                             \
                          break

void
UnicodeSetTest::runIndexedTest(int32_t index, UBool exec,
                               const char* &name, char* /*par*/) {
    // if (exec) logln((UnicodeString)"TestSuite UnicodeSetTest");
    switch (index) {
        CASE(0,TestPatterns);
        CASE(1,TestAddRemove);
        CASE(2,TestCategories);
        CASE(3,TestCloneEqualHash);
        CASE(4,TestMinimalRep);
        CASE(5,TestAPI);
        CASE(6,TestScriptSet);
        CASE(7,TestPropertySet);
        CASE(8,TestClone);
        CASE(9,TestExhaustive);
        CASE(10,TestToPattern);
        CASE(11,TestIndexOf);
        default: name = ""; break;
    }
}

/**
 * Test that toPattern() round trips with syntax characters and
 * whitespace.
 */
void UnicodeSetTest::TestToPattern() {
    static const char* OTHER_TOPATTERN_TESTS[] = {
        "[[:latin:]&[:greek:]]", 
        "[[:latin:]-[:greek:]]",
        "[:nonspacing mark:]",
        NULL
    };

    for (int32_t j=0; OTHER_TOPATTERN_TESTS[j]!=NULL; ++j) {
        UErrorCode ec = U_ZERO_ERROR;
        UnicodeSet s(OTHER_TOPATTERN_TESTS[j], ec);
        if (U_FAILURE(ec)) {
            errln((UnicodeString)"FAIL: bad pattern " + OTHER_TOPATTERN_TESTS[j]);
            continue;
        }
        checkPat(OTHER_TOPATTERN_TESTS[j], s);
    }
    
    for (UChar32 i = 0; i <= 0x10FFFF; ++i) {
        if ((i <= 0xFF && !u_isalpha(i)) || u_isspace(i)) {

            // check various combinations to make sure they all work.
            if (i != 0 && !toPatternAux(i, i)){
                continue;
            }
            if (!toPatternAux(0, i)){
                continue;
            }
            if (!toPatternAux(i, 0xFFFF)){
                continue;
            }
        }
    }
}
    
UBool UnicodeSetTest::toPatternAux(UChar32 start, UChar32 end) {

    // use Integer.toString because Utility.hex doesn't handle ints
    UnicodeString pat = "";
    // TODO do these in hex
    //String source = "0x" + Integer.toString(start,16).toUpperCase();
    //if (start != end) source += "..0x" + Integer.toString(end,16).toUpperCase();
    UnicodeString source;
    source = source + (uint32_t)start;
    if (start != end) 
        source = source + ".." + (uint32_t)end;
    UnicodeSet testSet;
    testSet.add(start, end);
    return checkPat(source, testSet);
}
    
UBool UnicodeSetTest::checkPat(const UnicodeString& source,
                               const UnicodeSet& testSet) {
    // What we want to make sure of is that a pattern generated
    // by toPattern(), with or without escaped unprintables, can
    // be passed back into the UnicodeSet constructor.
    UnicodeString pat0;

    testSet.toPattern(pat0, TRUE);
    
    if (!checkPat(source + " (escaped)", testSet, pat0)) return FALSE;
    
    //String pat1 = unescapeLeniently(pat0);
    //if (!checkPat(source + " (in code)", testSet, pat1)) return false;
    
    UnicodeString pat2; 
    testSet.toPattern(pat2, FALSE);
    if (!checkPat(source, testSet, pat2)) return FALSE;
    
    //String pat3 = unescapeLeniently(pat2);
    // if (!checkPat(source + " (in code)", testSet, pat3)) return false;
    
    //logln(source + " => " + pat0 + ", " + pat1 + ", " + pat2 + ", " + pat3);
    logln((UnicodeString)source + " => " + pat0 + ", " + pat2);
    return TRUE;
}

UBool UnicodeSetTest::checkPat(const UnicodeString& source,
                               const UnicodeSet& testSet,
                               const UnicodeString& pat) {
    UErrorCode ec = U_ZERO_ERROR;
    UnicodeSet testSet2(pat, ec);
    if (testSet2 != testSet) {
        errln((UnicodeString)"Fail toPattern: " + source + " => " + pat);
        return FALSE;
    }
    return TRUE;
}

void
UnicodeSetTest::TestPatterns(void) {
    UnicodeSet set;
    expectPattern(set, UnicodeString("[[a-m]&[d-z]&[k-y]]", ""),  "km");
    expectPattern(set, UnicodeString("[[a-z]-[m-y]-[d-r]]", ""),  "aczz");
    expectPattern(set, UnicodeString("[a\\-z]", ""),  "--aazz");
    expectPattern(set, UnicodeString("[-az]", ""),  "--aazz");
    expectPattern(set, UnicodeString("[az-]", ""),  "--aazz");
    expectPattern(set, UnicodeString("[[[a-z]-[aeiou]i]]", ""), "bdfnptvz");

    // Throw in a test of complement
    set.complement();
    UnicodeString exp;
    exp.append((UChar)0x0000).append("aeeoouu").append((UChar)(0x007a+1)).append((UChar)0xFFFF);
    expectPairs(set, exp);
}

void
UnicodeSetTest::TestCategories(void) {
    UErrorCode status = U_ZERO_ERROR;
    const char* pat = " [:Lu:] "; // Whitespace ok outside [:..:]
    UnicodeSet set(pat, status);
    if (U_FAILURE(status)) {
        errln((UnicodeString)"Fail: Can't construct set with " + pat);
    } else {
        expectContainment(set, pat, "ABC", "abc");
    }

    UChar32 i;
    int32_t failures = 0;
    // Make sure generation of L doesn't pollute cached Lu set
    // First generate L, then Lu
    set.applyPattern("[:L:]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    for (i=0; i<0x200; ++i) {
        UBool l = Unicode::isLetter((UChar)i);
        if (l != set.contains(i)) {
            errln((UnicodeString)"FAIL: L contains " + (unsigned short)i + " = " + 
                  set.contains(i));
            if (++failures == 10) break;
        }
    }
    
    set.applyPattern("[:Lu:]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    for (i=0; i<0x200; ++i) {
        UBool lu = (Unicode::getType((UChar)i) == Unicode::UPPERCASE_LETTER);
        if (lu != set.contains(i)) {
            errln((UnicodeString)"FAIL: Lu contains " + (unsigned short)i + " = " + 
                  set.contains(i));
            if (++failures == 20) break;
        }
    }
}
void
UnicodeSetTest::TestCloneEqualHash(void) {
    UErrorCode status = U_ZERO_ERROR;
    int8_t category=Unicode::LOWERCASE_LETTER;
    UnicodeSet *set1=new UnicodeSet(category, status); //  :Ll: Letter, lowercase
    UnicodeSet *set1a=new UnicodeSet("[:Ll:]", status); //  Letter, lowercase
    if (U_FAILURE(status)){
        errln((UnicodeString)"FAIL: Can't construst set with category->Ll");
        return;
    }
    category=Unicode::DECIMAL_DIGIT_NUMBER;
    UnicodeSet *set2=new UnicodeSet(category, status);   //Number, Decimal digit
    UnicodeSet *set2a=new UnicodeSet("[:Nd:]", status);   //Number, Decimal digit
    if (U_FAILURE(status)){
        errln((UnicodeString)"FAIL: Can't construct set with category->Nd");
        return;
    }

    if (*set1 != *set1a) {
        errln("FAIL: category constructor for Ll broken");
    }
    if (*set2 != *set2a) {
        errln("FAIL: category constructor for Nd broken");
    }
    delete set1a;
    delete set2a;

    logln("Testing copy construction");
    UnicodeSet *set1copy=new UnicodeSet(*set1);
    if(*set1 != *set1copy || *set1 == *set2 || 
        getPairs(*set1) != getPairs(*set1copy) ||
        set1->hashCode() != set1copy->hashCode()){
        errln("FAIL : Error in copy construction");
        return;
    }

    logln("Testing =operator");
    UnicodeSet set1equal=*set1;
    UnicodeSet set2equal=*set2;
    if(set1equal != *set1 || set1equal != *set1copy || set2equal != *set2 || 
        set2equal == *set1 || set2equal == *set1copy || set2equal == set1equal){
        errln("FAIL: Error in =operator");
    }

    logln("Testing clone()");
    UnicodeSet *set1clone=(UnicodeSet*)set1->clone();
    UnicodeSet *set2clone=(UnicodeSet*)set2->clone();
    if(*set1clone != *set1 || *set1clone != *set1copy || *set1clone != set1equal || 
        *set2clone != *set2 || *set2clone == *set1copy || *set2clone != set2equal || 
        *set2clone == *set1 || *set2clone == set1equal || *set2clone == *set1clone){
        errln("FAIL: Error in clone");
    }

    logln("Testing hashcode");
    if(set1->hashCode() != set1equal.hashCode() || set1->hashCode() != set1clone->hashCode() ||
        set2->hashCode() != set2equal.hashCode() || set2->hashCode() != set2clone->hashCode() ||
        set1copy->hashCode() != set1equal.hashCode() || set1copy->hashCode() != set1clone->hashCode() ||
        set1->hashCode() == set2->hashCode()  || set1copy->hashCode() == set2->hashCode() ||
        set2->hashCode() == set1clone->hashCode() || set2->hashCode() == set1equal.hashCode() ){
        errln("FAIL: Error in hashCode()");
    }

    delete set1;
    delete set1copy;
    delete set2;
    delete set1clone;
    delete set2clone;


}
void
UnicodeSetTest::TestAddRemove(void) {
    UnicodeSet set; // Construct empty set
    doAssert(set.isEmpty() == TRUE, "set should be empty");
    doAssert(set.size() == 0, "size should be 0");
    set.add(0x0061, 0x007a);
    expectPairs(set, "az");
    doAssert(set.isEmpty() == FALSE, "set should not be empty");
    doAssert(set.size() != 0, "size should not be equal to 0");
    doAssert(set.size() == 26, "size should be equal to 26");
    set.remove(0x006d, 0x0070);
    expectPairs(set, "alqz");
    doAssert(set.size() == 22, "size should be equal to 22");
    set.remove(0x0065, 0x0067);
    expectPairs(set, "adhlqz");
    doAssert(set.size() == 19, "size should be equal to 19");
    set.remove(0x0064, 0x0069);
    expectPairs(set, "acjlqz");
    doAssert(set.size() == 16, "size should be equal to 16");
    set.remove(0x0063, 0x0072);
    expectPairs(set, "absz");
    doAssert(set.size() == 10, "size should be equal to 10");
    set.add(0x0066, 0x0071);
    expectPairs(set, "abfqsz");
    doAssert(set.size() == 22, "size should be equal to 22");
    set.remove(0x0061, 0x0067);
    expectPairs(set, "hqsz");
    set.remove(0x0061, 0x007a);
    expectPairs(set, "");
    doAssert(set.isEmpty() == TRUE, "set should be empty");
    doAssert(set.size() == 0, "size should be 0");
    set.add(0x0061);
    doAssert(set.isEmpty() == FALSE, "set should not be empty");
    doAssert(set.size() == 1, "size should not be equal to 1");
    set.add(0x0062);
    set.add(0x0063);
    expectPairs(set, "ac");
    doAssert(set.size() == 3, "size should not be equal to 3");
    set.add(0x0070);
    set.add(0x0071);
    expectPairs(set, "acpq");
    doAssert(set.size() == 5, "size should not be equal to 5");
    set.clear();
    expectPairs(set, "");
    doAssert(set.isEmpty() == TRUE, "set should be empty");
    doAssert(set.size() == 0, "size should be 0");

    // Try removing an entire set from another set
    expectPattern(set, "[c-x]", "cx");
    UnicodeSet set2;
    expectPattern(set2, "[f-ky-za-bc[vw]]", "acfkvwyz");
    set.removeAll(set2);
    expectPairs(set, "deluxx");

    // Try adding an entire set to another set
    expectPattern(set, "[jackiemclean]", "aacceein");
    expectPattern(set2, "[hitoshinamekatajamesanderson]", "aadehkmort");
    set.addAll(set2);
    expectPairs(set, "aacehort");
    doAssert(set.containsAll(set2) == TRUE, "set should contain all the elements in set2");

    // Try retaining an set of elements contained in another set (intersection)
    UnicodeSet set3;
    expectPattern(set3, "[a-c]", "ac");
    doAssert(set.containsAll(set3) == FALSE, "set doesn't contain all the elements in set3");
    set3.remove(0x0062);
    expectPairs(set3, "aacc");
    doAssert(set.containsAll(set3) == TRUE, "set should contain all the elements in set3");
    set.retainAll(set3);
    expectPairs(set, "aacc");
    doAssert(set.size() == set3.size(), "set.size() should be set3.size()");
    doAssert(set.containsAll(set3) == TRUE, "set should contain all the elements in set3");
    set.clear();
    doAssert(set.size() != set3.size(), "set.size() != set3.size()");

    // Test commutativity
    expectPattern(set, "[hitoshinamekatajamesanderson]", "aadehkmort");
    expectPattern(set2, "[jackiemclean]", "aacceein");
    set.addAll(set2);
    expectPairs(set, "aacehort");
    doAssert(set.containsAll(set2) == TRUE, "set should contain all the elements in set2");




}

/**
 * Make sure minimal representation is maintained.
 */
void UnicodeSetTest::TestMinimalRep() {
    UErrorCode status = U_ZERO_ERROR;
    // This is pretty thoroughly tested by checkCanonicalRep()
    // run against the exhaustive operation results.  Use the code
    // here for debugging specific spot problems.

    // 1 overlap against 2
    UnicodeSet set("[h-km-q]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    UnicodeSet set2("[i-o]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.addAll(set2);
    expectPairs(set, "hq");
    // right
    set.applyPattern("[a-m]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set2.applyPattern("[e-o]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.addAll(set2);
    expectPairs(set, "ao");
    // left
    set.applyPattern("[e-o]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set2.applyPattern("[a-m]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.addAll(set2);
    expectPairs(set, "ao");
    // 1 overlap against 3
    set.applyPattern("[a-eg-mo-w]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set2.applyPattern("[d-q]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.addAll(set2);
    expectPairs(set, "aw");
}

void UnicodeSetTest::TestAPI() {
    UErrorCode status = U_ZERO_ERROR;
    // default ct
    UnicodeSet set;
    if (!set.isEmpty() || set.getRangeCount() != 0) {
        errln((UnicodeString)"FAIL, set should be empty but isn't: " +
              set);
    }

    // clear(), isEmpty()
    set.add(0x0061);
    if (set.isEmpty()) {
        errln((UnicodeString)"FAIL, set shouldn't be empty but is: " +
              set);
    }
    set.clear();
    if (!set.isEmpty()) {
        errln((UnicodeString)"FAIL, set should be empty but isn't: " +
              set);
    }

    // size()
    set.clear();
    if (set.size() != 0) {
        errln((UnicodeString)"FAIL, size should be 0, but is " + set.size() +
              ": " + set);
    }
    set.add(0x0061);
    if (set.size() != 1) {
        errln((UnicodeString)"FAIL, size should be 1, but is " + set.size() +
              ": " + set);
    }
    set.add(0x0031, 0x0039);
    if (set.size() != 10) {
        errln((UnicodeString)"FAIL, size should be 10, but is " + set.size() +
              ": " + set);
    }

    // contains(first, last)
    set.clear();
    set.applyPattern("[A-Y 1-8 b-d l-y]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    for (int32_t i = 0; i<set.getRangeCount(); ++i) {
        UChar32 a = set.getRangeStart(i);
        UChar32 b = set.getRangeEnd(i);
        if (!set.contains(a, b)) {
            errln((UnicodeString)"FAIL, should contain " + (unsigned short)a + '-' + (unsigned short)b +
                  " but doesn't: " + set);
        }
        if (set.contains((UChar32)(a-1), b)) {
            errln((UnicodeString)"FAIL, shouldn't contain " +
                  (unsigned short)(a-1) + '-' + (unsigned short)b +
                  " but does: " + set);
        }
        if (set.contains(a, (UChar32)(b+1))) {
            errln((UnicodeString)"FAIL, shouldn't contain " +
                  (unsigned short)a + '-' + (unsigned short)(b+1) +
                  " but does: " + set);
        }
    }

    // Ported InversionList test.
    UnicodeSet a((UChar32)3,(UChar32)10);
    UnicodeSet b((UChar32)7,(UChar32)15);
    UnicodeSet c;

    logln((UnicodeString)"a [3-10]: " + a);
    logln((UnicodeString)"b [7-15]: " + b);
    c = a; c.addAll(b);
    UnicodeSet exp((UChar32)3,(UChar32)15);
    if (c == exp) {
        logln((UnicodeString)"c.set(a).add(b): " + c);
    } else {
        errln((UnicodeString)"FAIL: c.set(a).add(b) = " + c + ", expect " + exp);
    }
    c.complement();
    exp.set((UChar32)0, (UChar32)2);
    exp.add((UChar32)16, UnicodeSet::MAX_VALUE);
    if (c == exp) {
        logln((UnicodeString)"c.complement(): " + c);
    } else {
        errln((UnicodeString)"FAIL: c.complement() = " + c + ", expect " + exp);
    }
    c.complement();
    exp.set((UChar32)3, (UChar32)15);
    if (c == exp) {
        logln((UnicodeString)"c.complement(): " + c);
    } else {
        errln((UnicodeString)"FAIL: c.complement() = " + c + ", expect " + exp);
    }
    c = a; c.complementAll(b);
    exp.set((UChar32)3,(UChar32)6);
    exp.add((UChar32)11,(UChar32) 15);
    if (c == exp) {
        logln((UnicodeString)"c.set(a).exclusiveOr(b): " + c);
    } else {
        errln((UnicodeString)"FAIL: c.set(a).exclusiveOr(b) = " + c + ", expect " + exp);
    }

    exp = c;
    bitsToSet(setToBits(c), c);
    if (c == exp) {
        logln((UnicodeString)"bitsToSet(setToBits(c)): " + c);
    } else {
        errln((UnicodeString)"FAIL: bitsToSet(setToBits(c)) = " + c + ", expect " + exp);
    }
}

/**
 * Test the [:Latin:] syntax.
 */
void UnicodeSetTest::TestScriptSet() {
    UErrorCode status = U_ZERO_ERROR;
    UnicodeSet set("[:Latin:]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    expectContainment(set, "[:Latin:]", "aA", CharsToUnicodeString("\\u0391\\u03B1"));

    UnicodeSet set2("[:Greek:]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    expectContainment(set2, "[:Greek:]", CharsToUnicodeString("\\u0391\\u03B1"), "aA");
    
    /* Jitterbug 1423 */
    UnicodeSet set3("[[:Common:][:Inherited:]]",status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    expectContainment(set3, "[[:Common:][:Inherited:]]", CharsToUnicodeString("\\U00003099\\U0001D169\\u0000"), "aA");

}

/**
 * Test the [:Latin:] syntax.
 */
void UnicodeSetTest::TestPropertySet() {
    UErrorCode status = U_ZERO_ERROR;
    UnicodeSet set("[:Latin:]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    expectContainment(set, "aA", CharsToUnicodeString("\\u0391\\u03B1"));
    set.applyPattern("[\\p{Greek}]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    expectContainment(set, CharsToUnicodeString("\\u0391\\u03B1"), "aA");
    set.applyPattern("\\P{ GENERAL Category = upper case letter }", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    expectContainment(set, "abc", "ABC");
}

/**
 * Test cloning of UnicodeSet.  For C++, we test the copy constructor.
 */
void UnicodeSetTest::TestClone() {
    UErrorCode ec = U_ZERO_ERROR;
    UnicodeSet s("[abcxyz]", ec);
    UnicodeSet t(s);
    expectContainment(t, "abc", "def");
}

/**
 * Test the indexOf() and charAt() methods.
 */
void UnicodeSetTest::TestIndexOf() {
    UErrorCode ec = U_ZERO_ERROR;
    UnicodeSet set("[a-cx-y3578]", ec);
    if (U_FAILURE(ec)) {
        errln("FAIL: UnicodeSet constructor");
        return;
    }
    for (int32_t i=0; i<set.size(); ++i) {
        UChar32 c = set.charAt(i);
        if (set.indexOf(c) != i) {
            errln((UnicodeString)"FAIL: charAt(" + i + ") = " + c +
                  " => indexOf() => " + set.indexOf(c));
        }
    }
    UChar32 c = set.charAt(set.size());
    if (c != -1) {
        errln((UnicodeString)"FAIL: charAt(<out of range>) = " + c);
    }
    int32_t j = set.indexOf((UChar32)0x71/*'q'*/);
    if (j != -1) {
        errln((UnicodeString)"FAIL: indexOf('q') = " + j);
    }
}

void UnicodeSetTest::TestExhaustive() {
    // exhaustive tests. Simulate UnicodeSets with integers.
    // That gives us very solid tests (except for large memory tests).

    UChar32 limit = (UChar32)128;

    UnicodeSet x, y, z;

    for (UChar32 i = 0; i < limit; ++i) {
        bitsToSet(i, x);
        logln((UnicodeString)"Testing " + i + ", " + x);
        _testComplement(i, x, y);
        for (UChar32 j = 0; j < limit; ++j) {
            _testAdd(i,j,  x,y,z);
            _testXor(i,j,  x,y,z);
            _testRetain(i,j,  x,y,z);
            _testRemove(i,j,  x,y,z);
        }
    }
}

void UnicodeSetTest::_testComplement(int32_t a, UnicodeSet& x, UnicodeSet& z) {
    bitsToSet(a, x);
    z = x;
    z.complement();
    int32_t c = setToBits(z);
    if (c != (~a)) {
        errln((UnicodeString)"FAILED: add: ~" + x +  " != " + z);
        errln((UnicodeString)"FAILED: add: ~" + a + " != " + c);
    }
    checkCanonicalRep(z, (UnicodeString)"complement " + a);
}

void UnicodeSetTest::_testAdd(int32_t a, int32_t b, UnicodeSet& x, UnicodeSet& y, UnicodeSet& z) {
    bitsToSet(a, x);
    bitsToSet(b, y);
    z = x;
    z.addAll(y);
    int32_t c = setToBits(z);
    if (c != (a | b)) {
        errln((UnicodeString)"FAILED: add: " + x + " | " + y + " != " + z);
        errln((UnicodeString)"FAILED: add: " + a + " | " + b + " != " + c);
    }
    checkCanonicalRep(z, (UnicodeString)"add " + a + "," + b);
}

void UnicodeSetTest::_testRetain(int32_t a, int32_t b, UnicodeSet& x, UnicodeSet& y, UnicodeSet& z) {
    bitsToSet(a, x);
    bitsToSet(b, y);
    z = x;
    z.retainAll(y);
    int32_t c = setToBits(z);
    if (c != (a & b)) {
        errln((UnicodeString)"FAILED: retain: " + x + " & " + y + " != " + z);
        errln((UnicodeString)"FAILED: retain: " + a + " & " + b + " != " + c);
    }
    checkCanonicalRep(z, (UnicodeString)"retain " + a + "," + b);
}

void UnicodeSetTest::_testRemove(int32_t a, int32_t b, UnicodeSet& x, UnicodeSet& y, UnicodeSet& z) {
    bitsToSet(a, x);
    bitsToSet(b, y);
    z = x;
    z.removeAll(y);
    int32_t c = setToBits(z);
    if (c != (a &~ b)) {
        errln((UnicodeString)"FAILED: remove: " + x + " &~ " + y + " != " + z);
        errln((UnicodeString)"FAILED: remove: " + a + " &~ " + b + " != " + c);
    }
    checkCanonicalRep(z, (UnicodeString)"remove " + a + "," + b);
}

void UnicodeSetTest::_testXor(int32_t a, int32_t b, UnicodeSet& x, UnicodeSet& y, UnicodeSet& z) {
    bitsToSet(a, x);
    bitsToSet(b, y);
    z = x;
    z.complementAll(y);
    int32_t c = setToBits(z);
    if (c != (a ^ b)) {
        errln((UnicodeString)"FAILED: complement: " + x + " ^ " + y + " != " + z);
        errln((UnicodeString)"FAILED: complement: " + a + " ^ " + b + " != " + c);
    }
    checkCanonicalRep(z, (UnicodeString)"complement " + a + "," + b);
}

/**
 * Check that ranges are monotonically increasing and non-
 * overlapping.
 */
void UnicodeSetTest::checkCanonicalRep(const UnicodeSet& set, const UnicodeString& msg) {
    int32_t n = set.getRangeCount();
    if (n < 0) {
        errln((UnicodeString)"FAIL result of " + msg +
              ": range count should be >= 0 but is " +
              n /*+ " for " + set.toPattern())*/);
        return;
    }
    UChar32 last = 0;
    for (int32_t i=0; i<n; ++i) {
        UChar32 start = set.getRangeStart(i);
        UChar32 end = set.getRangeEnd(i);
        if (start > end) {
            errln((UnicodeString)"FAIL result of " + msg +
                  ": range " + (i+1) +
                  " start > end: " + start + ", " + end +
                  " for " + set);
        }
        if (i > 0 && start <= last) {
            errln((UnicodeString)"FAIL result of " + msg +
                  ": range " + (i+1) +
                  " overlaps previous range: " + start + ", " + end +
                  " for " + set);
        }
        last = end;
    }
}

/**
 * Convert a bitmask to a UnicodeSet.
 */
void UnicodeSetTest::bitsToSet(int32_t a, UnicodeSet& result) {
    result.clear();
    for (UChar32 i = 0; i < 32; ++i) {
        if ((a & (1<<i)) != 0) {
            result.add(i);
        }
    }
}

/**
 * Convert a UnicodeSet to a bitmask.  Only the characters
 * U+0000 to U+0020 are represented in the bitmask.
 */
int32_t UnicodeSetTest::setToBits(const UnicodeSet& x) {
    int32_t result = 0;
    for (int32_t i = 0; i < 32; ++i) {
        if (x.contains((UChar32)i)) {
            result |= (1<<i);
        }
    }
    return result;
}

/**
 * Return the representation of an inversion list based UnicodeSet
 * as a pairs list.  Ranges are listed in ascending Unicode order.
 * For example, the set [a-zA-M3] is represented as "33AMaz".
 */
UnicodeString UnicodeSetTest::getPairs(const UnicodeSet& set) {
    UnicodeString pairs;
    for (int32_t i=0; i<set.getRangeCount(); ++i) {
        UChar32 start = set.getRangeStart(i);
        UChar32 end = set.getRangeEnd(i);
        if (end > 0xFFFF) {
            end = 0xFFFF;
            i = set.getRangeCount(); // Should be unnecessary
        }
        pairs.append((UChar)start).append((UChar)end);
    }
    return pairs;
}

void
UnicodeSetTest::expectContainment(const UnicodeSet& set,
                                  const UnicodeString& charsIn,
                                  const UnicodeString& charsOut) {
    UnicodeString pat;
    set.toPattern(pat);
    expectContainment(set, pat, charsIn, charsOut);
}

void
UnicodeSetTest::expectContainment(const UnicodeSet& set,
                                  const UnicodeString& setName,
                                  const UnicodeString& charsIn,
                                  const UnicodeString& charsOut) {
    UnicodeString bad;
    int32_t i;
    for (i=0; i<charsIn.length(); ++i) {
        UChar32 c = charsIn.char32At(i);
        if(c>0xFFFF) i++;

        if (!set.contains(c)) {
            bad.append(c);
        }
    }
    if (bad.length() > 0) {
        errln((UnicodeString)"Fail: set " + setName + " does not contain " + prettify(bad) +
              ", expected containment of " + prettify(charsIn));
    } else {
        logln((UnicodeString)"Ok: set " + setName + " contains " + charsIn);
    }

    bad.truncate(0);
    for (i=0; i<charsOut.length(); ++i) {
        UChar c = charsOut.charAt(i);
        if (set.contains(c)) {
            bad.append(c);
        }
    }
    if (bad.length() > 0) {
        logln((UnicodeString)"Fail: set " + setName + " contains " + bad +
              ", expected non-containment of " + charsOut);
    } else {
        logln((UnicodeString)"Ok: set " + setName + " does not contain " + charsOut);
    }
}

void
UnicodeSetTest::expectPattern(UnicodeSet& set,
                              const UnicodeString& pattern,
                              const UnicodeString& expectedPairs){
    UErrorCode status = U_ZERO_ERROR;
    set.applyPattern(pattern, status);
    if (U_FAILURE(status)) {
        errln(UnicodeString("FAIL: applyPattern(\"") + pattern +
              "\") failed");
        return;
    } else {
        if (getPairs(set) != expectedPairs ) {
            errln(UnicodeString("FAIL: applyPattern(\"") + pattern +
                  "\") => pairs \"" +
                  escape(getPairs(set)) + "\", expected \"" +
                  escape(expectedPairs) + "\"");
        } else {
            logln(UnicodeString("Ok:   applyPattern(\"") + pattern +
                  "\") => pairs \"" +
                  escape(getPairs(set)) + "\"");
        }
    }
    // the result of calling set.toPattern(), which is the string representation of
    // this set(set), is passed to a  UnicodeSet constructor, and tested that it 
    // will produce another set that is equal to this one.
    UnicodeString temppattern;
    set.toPattern(temppattern);
    UnicodeSet *tempset=new UnicodeSet(temppattern, status);
    if (U_FAILURE(status)) {
        errln(UnicodeString("FAIL: applyPattern(\""+ pattern + "\").toPattern() => " + temppattern + " => invalid pattern"));
        return;
    }
    if(*tempset != set || getPairs(*tempset) != getPairs(set)){
        errln(UnicodeString("FAIL: applyPattern(\""+ pattern + "\").toPattern() => " + temppattern + " => pairs \""+ escape(getPairs(*tempset)) + "\", expected pairs \"" +
            escape(getPairs(set)) + "\""));
    } else{
        logln(UnicodeString("Ok:   applyPattern(\""+ pattern + "\").toPattern() => " + temppattern + " => pairs \"" + escape(getPairs(*tempset)) + "\""));
    }

    delete tempset;

}

void
UnicodeSetTest::expectPairs(const UnicodeSet& set, const UnicodeString& expectedPairs) {
    if (getPairs(set) != expectedPairs) {
        errln(UnicodeString("FAIL: Expected pair list \"") +
              escape(expectedPairs) + "\", got \"" +
              escape(getPairs(set)) + "\"");
    }
}

static UChar toHexString(int32_t i) { return (UChar)(i + (i < 10 ? 0x30 : (0x41 - 10))); }

void
UnicodeSetTest::doAssert(UBool condition, const char *message)
{
    if (!condition) {
        errln(UnicodeString("ERROR : ") + message);
    }
}

UnicodeString
UnicodeSetTest::escape(const UnicodeString& s) {
    UnicodeString buf;
    for (int32_t i=0; i<s.length(); ++i)
    {
        UChar c = s[(UTextOffset)i];
        if (0x0020 <= c && c <= 0x007F) {
            buf += c;
        } else {
            buf += (UChar)0x5c; buf += (UChar)0x55;
            buf += toHexString((c & 0xF000) >> 12);
            buf += toHexString((c & 0x0F00) >> 8);
            buf += toHexString((c & 0x00F0) >> 4);
            buf += toHexString(c & 0x000F);
        }
    }
    return buf;
}
