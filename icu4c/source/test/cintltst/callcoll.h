/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File CALLCOLL.H
*
* Modification History:
*        Name                     Description            
*     Madhu Katragadda              Ported to C
*********************************************************************************
*/
/**
 * CollationDummyTest is a third level test class.  This tests creation of 
 * a customized collator object.  For example, number 1 to be sorted 
 * equlivalent to word 'one'.
 */
#ifndef _CALLCOLLTST
#define _CALLCOLLTST

#include "cintltst.h"



    /* static constants */
#define MAX_TOKEN_LEN 128
      
    /* tests comparison of custom collation with different strengths */
void doTest(UCollator*, const UChar* source, const UChar* target, UCollationResult result);

    /* perform test with strength PRIMARY */
static    void TestPrimary(void);

    /* perform test with strength SECONDARY */
static void TestSecondary(void);

    /* perform test with strength tertiary */
    static void TestTertiary(void);

    /*perform tests with strength Identical */
static    void TestIdentical(void);

    /* perform extra tests */
    static void TestExtra(void);

    /* Test jitterbug 581 */
    static void TestJB581(void);
   



#endif
