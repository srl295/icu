/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

/**
 * CollationGermanTest is a third level test class.  This tests the locale
 * specific primary, secondary and tertiary rules.  For example, o-umlaut
 * is sorted with expanding char e.
 */
#ifndef _DECOLL
#define _DECOLL

#ifndef _UTYPES
#include "unicode/utypes.h"
#endif

#ifndef _COLL
#include "unicode/coll.h"
#endif

#ifndef _INTLTEST
#include "intltest.h"
#endif

class CollationGermanTest: public IntlTest {
public:
    // static constants
    enum EToken_Len { MAX_TOKEN_LEN = 128 };

    CollationGermanTest();
    virtual ~CollationGermanTest();
    void runIndexedTest( int32_t index, UBool exec, const char* &name, char* par = NULL );

    //main test routine, tests rules specific to germa locale
    void doTest( UnicodeString source, UnicodeString target, Collator::EComparisonResult result);

    // perform test with strength PRIMARY
    void TestPrimary(/* char* par */);

    // perform test with strength SECONDARY
    void TestSecondary(/* char* par */);

    // perform tests with strength TERTIARY
    void TestTertiary(/* char* par */);

private:
    static const UChar testSourceCases[][MAX_TOKEN_LEN];
    static const UChar testTargetCases[][MAX_TOKEN_LEN];
    static const Collator::EComparisonResult results[][2];

    Collator *myCollation;
};

#endif
