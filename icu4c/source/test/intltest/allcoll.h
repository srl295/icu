/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

/**
 * CollationDummyTest is a third level test class.  This tests creation of 
 * a customized collator object.  For example, number 1 to be sorted 
 * equlivalent to word 'one'.
 */

#ifndef _ALLCOLL
#define _ALLCOLL

#ifndef _UTYPES
#include "unicode/utypes.h"
#endif

#ifndef _COLL
#include "unicode/coll.h"
#endif

#ifndef _TBLCOLL
#include "unicode/tblcoll.h"
#endif

#ifndef _INTLTEST
#include "intltest.h"
#endif

class CollationDummyTest: public IntlTest {
public:
    // If this is too small for the test data, just increase it.
    // Just don't make it too large, otherwise the executable will get too big
    enum EToken_Len { MAX_TOKEN_LEN = 16 };

    CollationDummyTest();
    virtual ~CollationDummyTest();
    void runIndexedTest( int32_t index, UBool exec, const char* &name, char* /*par = NULL */);

    // main test method called with different strengths,
    // tests comparison of custum collation with different strengths
    void doTest( UnicodeString source, UnicodeString target, Collator::EComparisonResult result);

    // perform test with strength PRIMARY
    void TestPrimary(/* char* par */);

    // perform test with strength SECONDARY
    void TestSecondary(/* char* par */);

    // perform test with strength tertiary
    void TestTertiary(/* char* par */);

    // perform extra tests
    void TestExtra(/* char* par */);

    void TestIdentical();

    void TestJB581();

private:
    static const Collator::EComparisonResult results[];

    RuleBasedCollator *myCollation;

    void doTestVariant(const UnicodeString source, const UnicodeString target, Collator::EComparisonResult result);
};
#endif
