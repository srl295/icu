/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File CALLCOLL.C
*
* Modification History:
*        Name                     Description            
*     Madhu Katragadda              Ported for C API
*********************************************************************************
*/

/*
 * Important: This file is included into intltest/allcoll.cpp so that the
 * test data is shared. This makes it easier to maintain the test data,
 * especially since the Unicode data must be portable and quoted character
 * literals will not work.
 * If it is included, then there will be a #define INCLUDE_CALLCOLL_C
 * that must prevent the actual code in here from being part of the
 * allcoll.cpp compilation.
 */

/**
 * CollationDummyTest is a third level test class.  This tests creation of 
 * a customized collator object.  For example, number 1 to be sorted 
 * equlivalent to word 'one'. 
 */

#include <string.h>
#include <stdlib.h>

#include "unicode/utypes.h"

#if !UCONFIG_NO_COLLATION

#ifndef INCLUDE_CALLCOLL_C

#include "unicode/ucol.h"
#include "unicode/uloc.h"
#include "cstring.h"
#include "cintltst.h"
#include "ccolltst.h"
#include "callcoll.h"
#include "unicode/ustring.h"
#include "cmemory.h"
#include "unicode/ucoleitr.h"
#include "ucol_imp.h"

/* perform test with strength PRIMARY */
static void TestPrimary(void);

/* perform test with strength SECONDARY */
static void TestSecondary(void);

/* perform test with strength tertiary */
static void TestTertiary(void);

/*perform tests with strength Identical */
static void TestIdentical(void);

/* perform extra tests */
static void TestExtra(void);

/* Test jitterbug 581 */
static void TestJB581(void);

/* Test jitterbug 1401 */
static void TestJB1401(void);

/* Test [variable top] in the rule syntax */
static void TestVariableTop(void);

/* Test surrogates */
static void TestSurrogates(void);

static void TestInvalidRules(void);

#endif

const UChar testSourceCases[][16] = {
    {0x61, 0x62, 0x27, 0x63, 0},
    {0x63, 0x6f, 0x2d, 0x6f, 0x70, 0},
    {0x61, 0x62, 0},
    {0x61, 0x6d, 0x70, 0x65, 0x72, 0x73, 0x61, 0x64, 0},
    {0x61, 0x6c, 0x6c, 0},
    {0x66, 0x6f, 0x75, 0x72, 0},
    {0x66, 0x69, 0x76, 0x65, 0},
    {0x31, 0},
    {0x31, 0},
    {0x31, 0},                                            /*  10 */
    {0x32, 0},
    {0x32, 0},
    {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0},
    {0x61, 0x3c, 0x62, 0},
    {0x61, 0x3c, 0x62, 0},
    {0x61, 0x63, 0x63, 0},
    {0x61, 0x63, 0x48, 0x63, 0},  /*  simple test */
    {0x70, 0x00EA, 0x63, 0x68, 0x65, 0},
    {0x61, 0x62, 0x63, 0},
    {0x61, 0x62, 0x63, 0},                                  /*  20 */
    {0x61, 0x62, 0x63, 0},
    {0x61, 0x62, 0x63, 0},
    {0x61, 0x62, 0x63, 0},
    {0x61, 0x00E6, 0x63, 0},
    {0x61, 0x63, 0x48, 0x63, 0},  /*  primary test */
    {0x62, 0x6c, 0x61, 0x63, 0x6b, 0},
    {0x66, 0x6f, 0x75, 0x72, 0},
    {0x66, 0x69, 0x76, 0x65, 0},
    {0x31, 0},
    {0x61, 0x62, 0x63, 0},                                        /*  30 */
    {0x61, 0x62, 0x63, 0},                                  
    {0x61, 0x62, 0x63, 0x48, 0},
    {0x61, 0x62, 0x63, 0},
    {0x61, 0x63, 0x48, 0x63, 0},                              /*  34 */
    {0x61, 0x63, 0x65, 0x30},
    {0x31, 0x30},
    {0x70, 0x00EA,0x30}                                    /* 37     */
};

const UChar testTargetCases[][16] = {
    {0x61, 0x62, 0x63, 0x27, 0},
    {0x43, 0x4f, 0x4f, 0x50, 0},
    {0x61, 0x62, 0x63, 0},
    {0x26, 0},
    {0x26, 0},
    {0x34, 0},
    {0x35, 0},
    {0x6f, 0x6e, 0x65, 0},
    {0x6e, 0x6e, 0x65, 0},
    {0x70, 0x6e, 0x65, 0},                                  /*  10 */
    {0x74, 0x77, 0x6f, 0},
    {0x75, 0x77, 0x6f, 0},
    {0x68, 0x65, 0x6c, 0x6c, 0x4f, 0},
    {0x61, 0x3c, 0x3d, 0x62, 0},
    {0x61, 0x62, 0x63, 0},
    {0x61, 0x43, 0x48, 0x63, 0},
    {0x61, 0x43, 0x48, 0x63, 0},  /*  simple test */
    {0x70, (UChar)0x00E9, 0x63, 0x68, 0x00E9, 0},
    {0x61, 0x62, 0x63, 0},
    {0x61, 0x42, 0x43, 0},                                  /*  20 */
    {0x61, 0x62, 0x63, 0x68, 0},
    {0x61, 0x62, 0x64, 0},
    {(UChar)0x00E4, 0x62, 0x63, 0},
    {0x61, (UChar)0x00C6, 0x63, 0},
    {0x61, 0x43, 0x48, 0x63, 0},  /*  primary test */
    {0x62, 0x6c, 0x61, 0x63, 0x6b, 0x2d, 0x62, 0x69, 0x72, 0x64, 0},
    {0x34, 0},
    {0x35, 0},
    {0x6f, 0x6e, 0x65, 0},
    {0x61, 0x62, 0x63, 0},
    {0x61, 0x42, 0x63, 0},                                  /*  30 */
    {0x61, 0x62, 0x63, 0x68, 0},
    {0x61, 0x62, 0x64, 0},
    {0x61, 0x43, 0x48, 0x63, 0},                                /*  34 */
    {0x61, 0x63, 0x65, 0x30},
    {0x31, 0x30},
    {0x70, (UChar)0x00EB,0x30}                                    /* 37 */
};

#ifndef INCLUDE_CALLCOLL_C

const UCollationResult results[] = {
    UCOL_LESS,
    UCOL_LESS, /*UCOL_GREATER,*/
    UCOL_LESS,
    UCOL_LESS,
    UCOL_LESS,
    UCOL_LESS,
    UCOL_LESS,
    UCOL_GREATER,
    UCOL_GREATER,
    UCOL_LESS,                                     /*  10 */
    UCOL_GREATER,
    UCOL_LESS,
    UCOL_GREATER,
    UCOL_GREATER,
    UCOL_LESS,
    UCOL_LESS,
    UCOL_LESS,
    /*  test primary > 17 */
    UCOL_EQUAL,
    UCOL_EQUAL,
    UCOL_EQUAL,                                    /*  20 */
    UCOL_LESS,
    UCOL_LESS,
    UCOL_EQUAL,
    UCOL_EQUAL,
    UCOL_EQUAL,
    UCOL_LESS,
    /*  test secondary > 26 */
    UCOL_EQUAL,
    UCOL_EQUAL,
    UCOL_EQUAL,
    UCOL_EQUAL,
    UCOL_EQUAL,                                    /*  30 */
    UCOL_EQUAL,
    UCOL_LESS,
    UCOL_EQUAL,                                     /*  34 */
    UCOL_EQUAL,
    UCOL_EQUAL,
    UCOL_LESS                                        /* 37 */
};

#endif

const UChar testCases[][4] =
{
    {0x61, 0},
    {0x41, 0},
    {0x00e4, 0},
    {0x00c4, 0},
    {0x61, 0x65, 0},
    {0x61, 0x45, 0},
    {0x41, 0x65, 0},
    {0x41, 0x45, 0},
    {(UChar)0x00e6, 0},
    {(UChar)0x00c6, 0},
    {0x62, 0},
    {0x63, 0},
    {0x7a, 0}
};

#define COUNT_TEST_CASES 13

#ifndef INCLUDE_CALLCOLL_C

static void TestJitterbug1098(void);

void addAllCollTest(TestNode** root)
{
    
    
    addTest(root, &TestPrimary, "tscoll/callcoll/TestPrimary");
    addTest(root, &TestSecondary, "tscoll/callcoll/TestSecondary");
    addTest(root, &TestTertiary, "tscoll/callcoll/TestTertiary");
    addTest(root, &TestIdentical, "tscoll/callcoll/TestIdentical");
    addTest(root, &TestExtra, "tscoll/callcoll/TestExtra");
    addTest(root, &TestJB581, "tscoll/callcoll/TestJB581");      
    addTest(root, &TestVariableTop, "tscoll/callcoll/TestVariableTop");      
    addTest(root, &TestSurrogates, "tscoll/callcoll/TestSurrogates");
    addTest(root, &TestInvalidRules, "tscoll/callcoll/TestInvalidRules");
    addTest(root, &TestJB1401, "tscoll/callcoll/TestJB1401");      
    addTest(root, &TestJitterbug1098, "tscoll/callcoll/TestJitterbug1098");      

   }

static void doTestVariant(UCollator* myCollation, const UChar source[], const UChar target[], UCollationResult result)
{
    int32_t sortklen1, sortklen2, sortklenmax, sortklenmin;
    int temp=0, gSortklen1=0,gSortklen2=0;
    UCollationResult compareResult, compareResulta, keyResult, incResult = result;
    uint8_t *sortKey1, *sortKey2, *sortKey1a, *sortKey2a;
    uint32_t sLen = u_strlen(source);
    uint32_t tLen = u_strlen(target);
    char buffer[256];
    uint32_t len;

    
    compareResult  = ucol_strcoll(myCollation, source, sLen, target, tLen);
    compareResulta = ucol_strcoll(myCollation, source, -1,   target, -1); 
    if (compareResult != compareResulta) {
        log_err("ucol_strcoll result from null terminated and explicit length strings differs.\n");
    }

    sortklen1=ucol_getSortKey(myCollation, source, sLen,  NULL, 0);
    sortklen2=ucol_getSortKey(myCollation, target, tLen,  NULL, 0);

    sortklenmax = (sortklen1>sortklen2?sortklen1:sortklen2);
    sortklenmin = (sortklen1<sortklen2?sortklen1:sortklen2);

    sortKey1 =(uint8_t*)malloc(sizeof(uint8_t) * (sortklenmax+1));
    sortKey1a=(uint8_t*)malloc(sizeof(uint8_t) * (sortklenmax+1));
    ucol_getSortKey(myCollation, source, sLen, sortKey1,  sortklen1+1);
    ucol_getSortKey(myCollation, source, -1,   sortKey1a, sortklen1+1);
    
    sortKey2 =(uint8_t*)malloc(sizeof(uint8_t) * (sortklenmax+1));
    sortKey2a=(uint8_t*)malloc(sizeof(uint8_t) * (sortklenmax+1));
    ucol_getSortKey(myCollation, target, tLen, sortKey2,  sortklen2+1);
    ucol_getSortKey(myCollation, target, -1,   sortKey2a, sortklen2+1);

    /* Check that sort key generated with null terminated string is identical  */
    /*  to that generted with a length specified.                              */
    if (uprv_strcmp((const char *)sortKey1, (const char *)sortKey1a) != 0 ||
        uprv_strcmp((const char *)sortKey2, (const char *)sortKey2a) != 0 ) {
        log_err("Sort Keys from null terminated and explicit length strings differ.\n");
    }

    /*memcmp(sortKey1, sortKey2,sortklenmax);*/
    temp= uprv_strcmp((const char *)sortKey1, (const char *)sortKey2);
    gSortklen1 = uprv_strlen((const char *)sortKey1)+1;
    gSortklen2 = uprv_strlen((const char *)sortKey2)+1;
    if(sortklen1 != gSortklen1){
        log_err("SortKey length does not match Expected: %i Got: %i\n",sortklen1, gSortklen1);
        log_verbose("Generated sortkey: %s\n", ucol_sortKeyToString(myCollation, sortKey1, buffer, &len));
    }
    if(sortklen2!= gSortklen2){
        log_err("SortKey length does not match Expected: %i Got: %i\n", sortklen2, gSortklen2);
        log_verbose("Generated sortkey: %s\n", ucol_sortKeyToString(myCollation, sortKey2, buffer, &len));
    }

    if(temp < 0) {
        keyResult=UCOL_LESS;
    }
    else if(temp > 0) {
        keyResult= UCOL_GREATER;
    }
    else {
        keyResult = UCOL_EQUAL;
    }
    reportCResult( source, target, sortKey1, sortKey2, compareResult, keyResult, incResult, result );
    free(sortKey1);
    free(sortKey2);
    free(sortKey1a);
    free(sortKey2a);

}

void doTest(UCollator* myCollation, const UChar source[], const UChar target[], UCollationResult result)
{
  doTestVariant(myCollation, source, target, result);
  if(result == UCOL_LESS) {
    doTestVariant(myCollation, target, source, UCOL_GREATER);
  } else if(result == UCOL_GREATER) {
    doTestVariant(myCollation, target, source, UCOL_LESS);
  } else {
    doTestVariant(myCollation, target, source, UCOL_EQUAL);
  }


}

static void TestTertiary()
{
    int32_t len,i;
    UChar *rules;
    UCollator *myCollation;
    UErrorCode status=U_ZERO_ERROR;
    const char* str="& C < ch, cH, Ch, CH & Five, 5 & Four, 4 & one, 1 & Ampersand; '&' & Two, 2 ";
    len = strlen(str);
    rules=(UChar*)malloc(sizeof(UChar*) * (len+1));
    u_uastrcpy(rules, str);

    myCollation=ucol_openRules(rules, len, UCOL_OFF, UCOL_DEFAULT_STRENGTH, NULL, &status);
    if(U_FAILURE(status)){
        log_err("ERROR: in creation of rule based collator :%s\n", myErrorName(status));
    }
   
    ucol_setStrength(myCollation, UCOL_TERTIARY);
    for (i = 0; i < 17 ; i++)
    {
        doTest(myCollation, testSourceCases[i], testTargetCases[i], results[i]);
    }
    free(rules);
    ucol_close(myCollation);
    myCollation = 0;
}

static void TestPrimary( )
{
    int32_t len,i;
    UChar *rules;
    UCollator *myCollation;
    UErrorCode status=U_ZERO_ERROR;
    const char* str="& C < ch, cH, Ch, CH & Five, 5 & Four, 4 & one, 1 & Ampersand; '&' & Two, 2 ";   
    len = strlen(str);
    rules=(UChar*)malloc(sizeof(UChar*) * (len+1));
    u_uastrcpy(rules, str);

    myCollation=ucol_openRules(rules, len, UCOL_OFF, UCOL_DEFAULT_STRENGTH,NULL, &status);
    if(U_FAILURE(status)){
        log_err("ERROR: in creation of rule based collator :%s\n", myErrorName(status));
    }
    ucol_setStrength(myCollation, UCOL_PRIMARY);
    
    for (i = 17; i < 26 ; i++)
    {
        
        doTest(myCollation, testSourceCases[i], testTargetCases[i], results[i]);
    }
    free(rules);
    ucol_close(myCollation);
    myCollation = 0;
}

static void TestSecondary()
{
    int32_t i;
    int32_t len;
    UChar *rules;
    UCollator *myCollation;
    UErrorCode status=U_ZERO_ERROR;
    const char* str="& C < ch, cH, Ch, CH & Five, 5 & Four, 4 & one, 1 & Ampersand; '&' & Two, 2 ";
    len = strlen(str);
    rules=(UChar*)malloc(sizeof(UChar*) * (len+1));
    u_uastrcpy(rules, str);

    myCollation=ucol_openRules(rules, len, UCOL_OFF, UCOL_DEFAULT_STRENGTH,NULL, &status);
    if(U_FAILURE(status)){
        log_err("ERROR: in creation of rule based collator :%s\n", myErrorName(status));
    }
    ucol_setStrength(myCollation, UCOL_SECONDARY);
    for (i = 26; i < 34 ; i++)
    {
        doTest(myCollation, testSourceCases[i], testTargetCases[i], results[i]);
    }
    free(rules);
    ucol_close(myCollation);
    myCollation = 0;
}

static void TestIdentical()
{
    int32_t i;
    int32_t len;
    UChar *rules = 0;
    UCollator *myCollation;
    UErrorCode status=U_ZERO_ERROR;
    const char* str="& C < ch, cH, Ch, CH & Five, 5 & Four, 4 & one, 1 & Ampersand; '&' & Two, 2 ";
    len = strlen(str);
    rules=(UChar*)malloc(sizeof(UChar*) * (len+1));
    u_uastrcpy(rules, str);

    myCollation=ucol_openRules(rules, len, UCOL_OFF, UCOL_IDENTICAL, NULL,&status);
    if(U_FAILURE(status)){
        log_err("ERROR: in creation of rule based collator :%s\n", myErrorName(status));
    }
    for(i= 34; i<37; i++)
    {
        doTest(myCollation, testSourceCases[i], testTargetCases[i], results[i]);
    }
    free(rules);
    ucol_close(myCollation);
    myCollation = 0;
}

static void TestExtra()
{
    int32_t i, j;
    int32_t len;
    UChar *rules;
    UCollator *myCollation;
    UErrorCode status = U_ZERO_ERROR;
    const char* str="& C < ch, cH, Ch, CH & Five, 5 & Four, 4 & one, 1 & Ampersand; '&' & Two, 2 ";
    len = strlen(str);
    rules=(UChar*)malloc(sizeof(UChar*) * (len+1));
    u_uastrcpy(rules, str);

    myCollation=ucol_openRules(rules, len, UCOL_OFF, UCOL_DEFAULT_STRENGTH,NULL, &status);
    if(U_FAILURE(status)){
        log_err("ERROR: in creation of rule based collator :%s\n", myErrorName(status));
    }
    ucol_setStrength(myCollation, UCOL_TERTIARY);
    for (i = 0; i < COUNT_TEST_CASES-1 ; i++)
    {
        for (j = i + 1; j < COUNT_TEST_CASES; j += 1)
        {
        
            doTest(myCollation, testCases[i], testCases[j], UCOL_LESS);
        }
    }
    free(rules);
    ucol_close(myCollation);
    myCollation = 0;
}

static void TestJB581(void)
{
    UChar       dispName    [100]; 
    int32_t     bufferLen   = 0;
    UChar       source      [100];
    UChar       target      [100];
    UCollationResult result     = UCOL_EQUAL;
    uint8_t     sourceKeyArray  [100];
    uint8_t     targetKeyArray  [100]; 
    int32_t     sourceKeyOut    = 0, 
                targetKeyOut    = 0;
    UCollator   *myCollator = 0;
    UErrorCode status = U_ZERO_ERROR;

    /*u_uastrcpy(source, "This is a test.");*/
    /*u_uastrcpy(target, "THISISATEST.");*/
    u_uastrcpy(source, "THISISATEST.");
    u_uastrcpy(target, "Thisisatest.");

    myCollator = ucol_open("en_US", &status);
    if (U_FAILURE(status)){
        bufferLen = uloc_getDisplayName("en_US", 0, dispName, 100, &status);
        /*Report the error with display name... */
        log_err("ERROR: Failed to create the collator for : \"%s\"\n", dispName);
        return;
    }
    result = ucol_strcoll(myCollator, source, -1, target, -1);
    /* result is 1, secondary differences only for ignorable space characters*/
    if (result != 1)
    {
        log_err("Comparing two strings with only secondary differences in C failed.\n");
    }
    /* To compare them with just primary differences */
    ucol_setStrength(myCollator, UCOL_PRIMARY);
    result = ucol_strcoll(myCollator, source, -1, target, -1);
    /* result is 0 */
    if (result != 0)
    {
        log_err("Comparing two strings with no differences in C failed.\n");
    }
    /* Now, do the same comparison with keys */
    sourceKeyOut = ucol_getSortKey(myCollator, source, -1, sourceKeyArray, 100);
    targetKeyOut = ucol_getSortKey(myCollator, target, -1, targetKeyArray, 100);
    result = 0;
    bufferLen = ((targetKeyOut > 100) ? 100 : targetKeyOut);
    result = memcmp(sourceKeyArray, targetKeyArray, bufferLen);
    if (result != 0)
    {
        log_err("Comparing two strings with sort keys in C failed.\n");
    }
    ucol_close(myCollator);
}

static void TestJB1401(void)
{
    UCollator     *myCollator = 0;
    UErrorCode     status = U_ZERO_ERROR;
    static UChar   NFD_UnsafeStartChars[] = {
        0x0f73,          /* Tibetan Vowel Sign II */
        0x0f75,          /* Tibetan Vowel Sign UU */
        0x0f81,          /* Tibetan Vowel Sign Reversed II */
            0
    };
    int            i;

    
    myCollator = ucol_open("en_US", &status);
    if (U_FAILURE(status)){
        int32_t     bufferLen   = 0;
        UChar       dispName    [100]; 
        bufferLen = uloc_getDisplayName("en_US", 0, dispName, 100, &status);
        /*Report the error with display name... */
        log_err("ERROR: Failed to create the collator for : \"%s\"\n", dispName);
        return;
    }
    ucol_setAttribute(myCollator, UCOL_NORMALIZATION_MODE, UCOL_ON, &status);
    if (U_FAILURE(status)){
        log_err("ERROR: Failed to set normalization mode ON for collator.\n");
        return;
    }

    for (i=0; ; i++) {
        UChar    c;
        UChar    X[4];
        UChar    Y[20];
        UChar    Z[20];

        /*  Get the next funny character to be tested, and set up the
         *  three test strings X, Y, Z, consisting of an A-grave + test char,
         *    in original form, NFD, and then NFC form.
         */
        c = NFD_UnsafeStartChars[i];
        if (c==0) {break;}

        X[0]=0xC0; X[1]=c; X[2]=0;   /* \u00C0 is A Grave*/
        
        unorm_normalize(X, -1, UNORM_NFD, 0, Y, 20, &status);
        unorm_normalize(Y, -1, UNORM_NFC, 0, Z, 20, &status);
        if (U_FAILURE(status)){
            log_err("ERROR: Failed to normalize test of character %x\n", c);
            return;
        }

        /* Collation test.  All three strings should be equal.
         *   doTest does both strcoll and sort keys, with params in both orders.
         */
        doTest(myCollator, X, Y, UCOL_EQUAL);
        doTest(myCollator, X, Z, UCOL_EQUAL);
        doTest(myCollator, Y, Z, UCOL_EQUAL);

        /* Run collation element iterators over the three strings.  Results should be same for each.
         */
        {
            UCollationElements *ceiX, *ceiY, *ceiZ;
            int32_t             ceX,   ceY,   ceZ;
            int                 j;

            ceiX = ucol_openElements(myCollator, X, -1, &status);
            ceiY = ucol_openElements(myCollator, Y, -1, &status);
            ceiZ = ucol_openElements(myCollator, Z, -1, &status);
            if (U_FAILURE(status)) {
                log_err("ERROR: uucol_openElements failed.\n");
                return;
            }

            for (j=0;; j++) {
                ceX = ucol_next(ceiX, &status);
                ceY = ucol_next(ceiY, &status);
                ceZ = ucol_next(ceiZ, &status);
                if (U_FAILURE(status)) {
                    log_err("ERROR: ucol_next failed for iteration #%d.\n", j);
                    break;
                }
                if (ceX != ceY || ceY != ceZ) {
                    log_err("ERROR: ucol_next failed for iteration #%d.\n", j);
                    break;
                }
                if (ceX == UCOL_NULLORDER) {
                    break;
                }
            }
            ucol_closeElements(ceiX);
            ucol_closeElements(ceiY);
            ucol_closeElements(ceiZ);
        }
    }
    ucol_close(myCollator);
}



/**
* Tests the [variable top] tag in rule syntax. Since the default [alternate]
* tag has the value shifted, any codepoints before [variable top] should give
* a primary ce of 0.
*/
static void TestVariableTop(void)
{
    const char       *str          = "&z = [variable top]";
          int         len          = strlen(str);
          UChar      *rules;
          UCollator  *myCollation;
          UCollator  *enCollation;
          UErrorCode  status       = U_ZERO_ERROR;
          UChar       source[1];
          UChar       ch;
          uint8_t     result[20];
          uint8_t     expected[20];

    rules = (UChar*)malloc(sizeof(UChar*) * (len + 1));
    u_uastrcpy(rules, str);

    enCollation = ucol_open("en_US", &status);
    myCollation = ucol_openRules(rules, len, UCOL_OFF, 
                                 UCOL_PRIMARY,NULL, &status);
    if (U_FAILURE(status)) {
        log_err("ERROR: in creation of rule based collator :%s\n", 
                myErrorName(status));
        return;
    }

    ucol_setStrength(enCollation, UCOL_PRIMARY);
    ucol_setAttribute(enCollation, UCOL_ALTERNATE_HANDLING, UCOL_SHIFTED,
                      &status);
    ucol_setAttribute(myCollation, UCOL_ALTERNATE_HANDLING, UCOL_SHIFTED,
                      &status);
        
    if (ucol_getAttribute(myCollation, UCOL_ALTERNATE_HANDLING, &status) !=
        UCOL_SHIFTED || U_FAILURE(status)) {
        log_err("ERROR: ALTERNATE_HANDLING value can not be set to SHIFTED\n");
    }

    uprv_memset(expected, 0, 20);

    /* space is supposed to be a variable */
    source[0] = ' ';
    len = ucol_getSortKey(enCollation, source, 1, result, 
                          sizeof(result));

    if (uprv_memcmp(expected, result, len) != 0) {
        log_err("ERROR: SHIFTED alternate does not return 0 for primary of space\n");
    }

    ch = 'a';
    while (ch < 'z') {
        source[0] = ch;
        len = ucol_getSortKey(myCollation, source, 1, result,
                              sizeof(result));
        if (uprv_memcmp(expected, result, len) != 0) {
            log_err("ERROR: SHIFTED alternate does not return 0 for primary of %c\n", 
                    ch);
        }
        ch ++;
    }
  
    free(rules);
    ucol_close(enCollation);
    ucol_close(myCollation);
    enCollation = NULL;
    myCollation = NULL;
}

/**
  * Tests surrogate support.
  * NOTE: This test used \\uD801\\uDC01 pair, which is now assigned to Desseret
  * Therefore, another (unassigned) code point was used for this test.
  */
static void TestSurrogates(void)
{
    const char       *str          = 
                              "&z<'\\uD800\\uDC00'<'\\uD800\\uDC0A\\u0308'<A";
          int         len          = strlen(str);
          int         rlen         = 0;
          UChar      *rules;
          UCollator  *myCollation;
          UCollator  *enCollation;
          UErrorCode  status       = U_ZERO_ERROR;
          UChar       source[][4]    = 
          {{'z', 0, 0}, {0xD800, 0xDC00, 0}, {0xD800, 0xDC0A, 0x0308, 0}, {0xD800, 0xDC02}};
          UChar       target[][4]    = 
          {{0xD800, 0xDC00, 0}, {0xD800, 0xDC0A, 0x0308, 0}, {'A', 0, 0}, {0xD800, 0xDC03}};
          int         count        = 0;
          uint8_t enresult[20], myresult[20];
          int enlen, mylen;
          
    /* tests for open rules with surrogate rules */
    rules = (UChar*)malloc(sizeof(UChar*) * (len + 1));
    rlen = u_unescape(str, rules, len);
    
    enCollation = ucol_open("en_US", &status);
    myCollation = ucol_openRules(rules, rlen, UCOL_OFF, 
                                 UCOL_TERTIARY,NULL, &status);
    if (U_FAILURE(status)) {
        log_err("ERROR: in creation of rule based collator :%s\n", 
                myErrorName(status));
        return;
    }

    /* 
    this test is to verify the supplementary sort key order in the english 
    collator
    */
    log_verbose("start of english collation supplementary characters test\n");
    while (count < 2) {
        doTest(enCollation, source[count], target[count], UCOL_LESS);
        count ++;
    }
    doTest(enCollation, source[count], target[count], UCOL_GREATER);
        
    log_verbose("start of tailored collation supplementary characters test\n");
    count = 0;
    /* tests getting collation elements for surrogates for tailored rules */
    while (count < 4) {
        doTest(myCollation, source[count], target[count], UCOL_LESS);
        count ++;
    }

    /* tests that \uD800\uDC02 still has the same value, not changed */
    enlen = ucol_getSortKey(enCollation, source[3], 2, enresult, 20);
    mylen = ucol_getSortKey(myCollation, source[3], 2, myresult, 20);
    if (enlen != mylen ||
        uprv_memcmp(enresult, myresult, enlen) != 0) {
        log_verbose("Failed : non-tailored supplementary characters should have the same value\n");
    }

    free(rules);
    ucol_close(enCollation);
    ucol_close(myCollation);
    enCollation = NULL;
    myCollation = NULL;
}

/*
 *### TODO: Add more invalid rules to test all different scenarios.
 *
 */
static void 
TestInvalidRules(){
#define MAX_ERROR_STATES 2

    static const char* rulesArr[MAX_ERROR_STATES] = {
        "& C < ch, cH, Ch[this should fail]<d",
        "& C < ch, cH, & Ch[variable top]"
    };
    static const char* preContextArr[MAX_ERROR_STATES] = {
        "his should fail",
        "& C < ch, cH, ",

    };
    static const char* postContextArr[MAX_ERROR_STATES] = {
        "<d",
        " Ch[variable t"
    };
    int i;

    for(i = 0;i<MAX_ERROR_STATES;i++){
        UChar rules[1000]       = { '\0' };
        UChar preContextExp[1000]  = { '\0' };
        UChar postContextExp[1000] = { '\0' };
        UParseError parseError;
        UErrorCode status = U_ZERO_ERROR;
        UCollator* coll=0;
        u_charsToUChars(rulesArr[i],rules,uprv_strlen(rulesArr[i])+1);
        u_charsToUChars(preContextArr[i],preContextExp,uprv_strlen(preContextArr[i])+1);
        u_charsToUChars(postContextArr[i],postContextExp,uprv_strlen(postContextArr[i])+1);
        /* clean up stuff in parseError */
        u_memset(parseError.preContext,0x0000,U_PARSE_CONTEXT_LEN);      
        u_memset(parseError.postContext,0x0000,U_PARSE_CONTEXT_LEN);
        /* open the rules and test */
        coll = ucol_openRules(rules,u_strlen(rules),UCOL_OFF,UCOL_DEFAULT_STRENGTH,&parseError,&status);
        if(u_strcmp(parseError.preContext,preContextExp)!=0){
            log_err("preContext in UParseError for ucol_openRules does not match\n");
        }
        if(u_strcmp(parseError.postContext,postContextExp)!=0){
            log_err("postContext in UParseError for ucol_openRules does not match\n");
        }
    }  
}

static void
TestJitterbug1098(){
    UChar rule[1000];
    UCollator* c1 = NULL;
    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;
    char preContext[200]={0};
    char postContext[200]={0};
    int i=0;
    const char* rules[] = {
         "&''<\\\\",
         "&\\'<\\\\",
         "&\\\"<'\\'",
         "&'\"'<\\'",
         '\0'

    };
    const UCollationResult results1098[] = {
        UCOL_LESS,
        UCOL_LESS, 
        UCOL_LESS,
        UCOL_LESS,
    };
    const UChar input[][2]= {
        {0x0027,0x005c},
        {0x0027,0x005c},
        {0x0022,0x005c},
        {0x0022,0x0027},
    };
    UChar X[2] ={0};
    UChar Y[2] ={0};
    u_memset(parseError.preContext,0x0000,U_PARSE_CONTEXT_LEN);      
    u_memset(parseError.postContext,0x0000,U_PARSE_CONTEXT_LEN);
    for(;rules[i]!=0;i++){
        u_uastrcpy(rule, rules[i]);
        c1 = ucol_openRules(rule, u_strlen(rule), UCOL_OFF, UCOL_DEFAULT_STRENGTH, &parseError, &status);
        if(U_FAILURE(status)){
            u_UCharsToChars(parseError.preContext,preContext,20);
            u_UCharsToChars(parseError.postContext,postContext,20);
            log_err("Could not parse the rules syntax. Error: %s ", u_errorName(status));
            log_verbose("\n\tPre-Context: %s \n\tPost-Context:%s \n",preContext,postContext);
            return;
        }
        X[0] = input[i][0];
        Y[0] = input[i][1];
        doTest(c1,X,Y,results1098[i]);
        ucol_close(c1);
    }
}

#endif

#endif /* #if !UCONFIG_NO_COLLATION */
