/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/*******************************************************************************
*
* File cmsccoll.C
*
*******************************************************************************/
/**
 * These are the tests specific to ICU 1.8 and above, that I didn't know where
 * to fit.
 */

#include <stdlib.h>
#include <stdio.h>
#include "unicode/utypes.h"
#include "unicode/ucol.h"
#include "unicode/ucoleitr.h"
#include "unicode/uloc.h"
#include "cintltst.h"
#include "ccolltst.h"
#include "callcoll.h"
#include "unicode/ustring.h"
#include "string.h"
#include "ucol_imp.h"
#include "ucol_tok.h"
#include "cmemory.h"
#include "ucmp32.h"

#ifdef WIN32
#define UNICODE
#include <windows.h>
#else 
#define LCID uint32_t
#define LOCALE_SYSTEM_DEFAULT 0
#endif 

#define MAX_TOKEN_LEN 16

typedef int tst_strcoll(void *collator, const int object, 
                        const UChar *source, const int sLen, 
                        const UChar *target, const int tLen);

static UCollator *myCollation;
const static UChar gRules[MAX_TOKEN_LEN] =
/*" & 0 < 1,\u2461<a,A"*/
{ 0x0026, 0x0030, 0x003C, 0x0031, 0x002C, 0x2460, 0x003C, 0x0061, 0x002C, 0x0041, 0x0000 };

const static UChar testCase[][MAX_TOKEN_LEN] =
{
    /*0*/ {0x0031 /*'1'*/, 0x0061/*'a'*/, 0x0000},
    /*1*/ {0x0031 /*'1'*/, 0x0041/*'A'*/, 0x0000},
    /*2*/ {0x2460 /*circ'1'*/, 0x0061/*'a'*/, 0x0000},
    /*3*/ {0x2460 /*circ'1'*/, 0x0041/*'A'*/, 0x0000}
};

const static UCollationResult caseTestResults[][9] =
{
        { UCOL_LESS, UCOL_LESS, UCOL_LESS, 0, UCOL_LESS, UCOL_LESS, 0, 0, UCOL_LESS },
        { UCOL_GREATER, UCOL_LESS, UCOL_LESS, 0, UCOL_LESS, UCOL_LESS, 0, 0, UCOL_GREATER },
        { UCOL_LESS, UCOL_LESS, UCOL_LESS, 0, UCOL_GREATER, UCOL_LESS, 0, 0, UCOL_LESS },
        { UCOL_GREATER, UCOL_LESS, UCOL_GREATER, 0, UCOL_LESS, UCOL_LESS, 0, 0, UCOL_GREATER }

};

const static UColAttributeValue caseTestAttributes[][2] =
{
        { UCOL_LOWER_FIRST, UCOL_OFF},
        { UCOL_UPPER_FIRST, UCOL_OFF},
        { UCOL_LOWER_FIRST, UCOL_ON},
        { UCOL_UPPER_FIRST, UCOL_ON}

};

static void TestCase( )
{
    int32_t i,j,k;
    UErrorCode status = U_ZERO_ERROR;
    myCollation = ucol_open("en_US", &status);
    if(U_FAILURE(status)){
        log_err("ERROR: in creation of rule based collator: %s\n", myErrorName(status));
        return;
    }
    log_verbose("Testing different case settings\n");
    ucol_setStrength(myCollation, UCOL_TERTIARY);

    for(k = 0; k<4; k++) {
      ucol_setAttribute(myCollation, UCOL_CASE_FIRST, caseTestAttributes[k][0], &status);
      ucol_setAttribute(myCollation, UCOL_CASE_LEVEL, caseTestAttributes[k][1], &status);
      for (i = 0; i < 3 ; i++) {
        for(j = i+1; j<4; j++) {
          doTest(myCollation, testCase[i], testCase[j], caseTestResults[k][3*i+j-1]);
        }
      }
    }
    ucol_close(myCollation);

    myCollation = ucol_openRules(gRules, u_strlen(gRules), UNORM_NONE, UCOL_TERTIARY, &status);
    if(U_FAILURE(status)){
        log_err("ERROR: in creation of rule based collator: %s\n", myErrorName(status));
        return;
    }
    log_verbose("Testing different case settings with custom rules\n");
    ucol_setStrength(myCollation, UCOL_TERTIARY);

    for(k = 0; k<4; k++) {
      ucol_setAttribute(myCollation, UCOL_CASE_FIRST, caseTestAttributes[k][0], &status);
      ucol_setAttribute(myCollation, UCOL_CASE_LEVEL, caseTestAttributes[k][1], &status);
      for (i = 0; i < 3 ; i++) {
        for(j = i+1; j<4; j++) {
          doTest(myCollation, testCase[i], testCase[j], caseTestResults[k][3*i+j-1]);
        }
      }
    }
    ucol_close(myCollation);    

}

/**
 * Return an integer array containing all of the collation orders
 * returned by calls to next on the specified iterator
 */
static int32_t* getOrders(UCollationElements *iter, int32_t *orderLength)
{
    UErrorCode status;
    int32_t order;
    int32_t maxSize = 100;
    int32_t size = 0;
    int32_t *temp;
    int32_t *orders =(int32_t*)malloc(sizeof(int32_t) * maxSize);
    status= U_ZERO_ERROR;


    while ((order=ucol_next(iter, &status)) != UCOL_NULLORDER)
    {
        if (size == maxSize)
        {
            maxSize *= 2;
            temp = (int32_t*)malloc(sizeof(int32_t) * maxSize);

            memcpy(temp, orders, size * sizeof(int32_t));
            free(orders);
            orders = temp;
            
        }

        orders[size++] = order;
    }

    if (maxSize > size)
    {
        temp = (int32_t*)malloc(sizeof(int32_t) * size);

        memcpy(temp, orders, size * sizeof(int32_t));
        free(orders);
        orders = temp;


    }

    *orderLength = size;
    return orders;
}

static void backAndForth(UCollationElements *iter)
{
    /* Run through the iterator forwards and stick it into an array */
    int32_t index, o;
    UErrorCode status = U_ZERO_ERROR;
    int32_t orderLength = 0;
    int32_t *orders;
    orders= getOrders(iter, &orderLength);
    
    
    /* Now go through it backwards and make sure we get the same values */
    index = orderLength;
    ucol_reset(iter);
    
    /* synwee : changed */
    while ((o = ucol_previous(iter, &status)) != UCOL_NULLORDER)
    {
      if (o != orders[-- index])
      {
        if (o == 0)
          index ++;
        else
        {
          while (index > 0 && orders[-- index] == 0)
          {
          }
          if (o != orders[index])
          {
            log_err("Mismatch at index : %d\n", index);
            break;
          }
        }
      }
    }

    while (index != 0 && orders[index - 1] == 0) {
      index --;
    }

    if (index != 0)
    {
        log_err("Didn't get back to beginning - index is %d\n", index);

        ucol_reset(iter);
        log_err("\nnext: ");
        while ((o = ucol_next(iter, &status)) != UCOL_NULLORDER)
        {
            log_err("Error at %d\n", o);
        }
        log_err("\nprev: ");
        while ((o = ucol_previous(iter, &status)) != UCOL_NULLORDER)
        {
            log_err("Error at %d\n", o);
        }
        log_verbose("\n");
    }

    free(orders);    
}

const static char cnt1[][10] = {
  "AA",
  "AC",
  "AZ",
  "AQ",
  "AB",
  "ABZ",
  "ABQ",
  "Z",
  "ABC",
  "Q",
  "B"
};

const static char cnt2[][10] = {
  "DA",
  "DAD",
  "DAZ",
  "MAR",
  "Z",
  "DAVIS",
  "MARK",
  "DAV",
  "DAVI"
};

static void IncompleteCntTest( )
{
  UErrorCode status = U_ZERO_ERROR;
  UChar *temp=(UChar*)malloc(sizeof(UChar) * 90);
  UChar *t1 =(UChar*)malloc(sizeof(UChar) * 90);
  UChar *t2 =(UChar*)malloc(sizeof(UChar) * 90);

  UCollator *coll =  NULL;
  uint32_t i = 0, j = 0;
  uint32_t size = 0;
  
  u_uastrcpy(temp, " & Z < ABC < Q < B");

  coll = ucol_openRules(temp, u_strlen(temp), UCOL_NO_NORMALIZATION, 
                                                UCOL_DEFAULT_STRENGTH, &status);

  if(U_SUCCESS(status)) {
    size = sizeof(cnt1)/sizeof(cnt1[0]);
    for(i = 0; i < size-1; i++) {
      for(j = i+1; j < size; j++) {
        UCollationElements *iter;
        u_uastrcpy(t1, cnt1[i]);
        u_uastrcpy(t2, cnt1[j]);
        doTest(coll, t1, t2, UCOL_LESS);
        /* synwee : added collation element iterator test */
        iter = ucol_openElements(coll, t2, u_strlen(t2), &status);
        if (U_FAILURE(status)) {
          log_err("Creation of iterator failed\n");
          break;
        }
        backAndForth(iter);
        free(iter);
      }
    }
  } 

  ucol_close(coll);


  u_uastrcpy(temp, " & Z < DAVIS < MARK <DAV");
  coll = ucol_openRules(temp, u_strlen(temp), UCOL_NO_NORMALIZATION, 
                                                UCOL_DEFAULT_STRENGTH, &status);

  if(U_SUCCESS(status)) {
    size = sizeof(cnt2)/sizeof(cnt2[0]);
    for(i = 0; i < size-1; i++) {
      for(j = i+1; j < size; j++) {
        UCollationElements *iter;
        u_uastrcpy(t1, cnt2[i]);
        u_uastrcpy(t2, cnt2[j]);
        doTest(coll, t1, t2, UCOL_LESS);

        /* synwee : added collation element iterator test */
        iter = ucol_openElements(coll, t2, u_strlen(t2), &status);
        if (U_FAILURE(status)) {
          log_err("Creation of iterator failed\n");
          break;
        }
        backAndForth(iter);
        free(iter);
      }
    }
  } 

  ucol_close(coll);


}

const static char shifted[][20] = {
  "black bird",
  "black-bird",
  "blackbird",
  "black Bird",
  "black-Bird",
  "blackBird",
  "black birds",
  "black-birds",
  "blackbirds"
};

const static UCollationResult shiftedTert[] = {
  0,
  UCOL_EQUAL,
  UCOL_EQUAL,
  UCOL_LESS,
  UCOL_EQUAL,
  UCOL_EQUAL,
  UCOL_LESS,
  UCOL_EQUAL,
  UCOL_EQUAL
};

const static char nonignorable[][20] = {
  "black bird",
  "black Bird",
  "black birds",
  "black-bird",
  "black-Bird",
  "black-birds",
  "blackbird",
  "blackBird",
  "blackbirds"
};

static void BlackBirdTest( ) {
  UErrorCode status = U_ZERO_ERROR;
  UChar *t1 =(UChar*)malloc(sizeof(UChar) * 90);
  UChar *t2 =(UChar*)malloc(sizeof(UChar) * 90);

  uint32_t i = 0, j = 0;
  uint32_t size = 0;
  UCollator *coll = ucol_open("en_US", &status);

  ucol_setAttribute(coll, UCOL_NORMALIZATION_MODE, UCOL_OFF, &status);
  ucol_setAttribute(coll, UCOL_ALTERNATE_HANDLING, UCOL_NON_IGNORABLE, &status);

  if(U_SUCCESS(status)) {
    size = sizeof(nonignorable)/sizeof(nonignorable[0]);
    for(i = 0; i < size-1; i++) {
      for(j = i+1; j < size; j++) {
        u_uastrcpy(t1, nonignorable[i]);
        u_uastrcpy(t2, nonignorable[j]);
        doTest(coll, t1, t2, UCOL_LESS);
      }
    }
  } 

  ucol_setAttribute(coll, UCOL_ALTERNATE_HANDLING, UCOL_SHIFTED, &status);
  ucol_setAttribute(coll, UCOL_STRENGTH, UCOL_QUATERNARY, &status);

  if(U_SUCCESS(status)) {
    size = sizeof(shifted)/sizeof(shifted[0]);
    for(i = 0; i < size-1; i++) {
      for(j = i+1; j < size; j++) {
        u_uastrcpy(t1, shifted[i]);
        u_uastrcpy(t2, shifted[j]);
        doTest(coll, t1, t2, UCOL_LESS);
      }
    }
  } 

  ucol_setAttribute(coll, UCOL_STRENGTH, UCOL_TERTIARY, &status);
  if(U_SUCCESS(status)) {
    size = sizeof(shifted)/sizeof(shifted[0]);
    for(i = 1; i < size; i++) {
      u_uastrcpy(t1, shifted[i-1]);
      u_uastrcpy(t2, shifted[i]);
      doTest(coll, t1, t2, shiftedTert[i]);
    }
  } 

  ucol_close(coll);
}

const static UChar testSourceCases[][MAX_TOKEN_LEN] = {
    {0x0041/*'A'*/, 0x0300, 0x0301, 0x0000},
    {0x0041/*'A'*/, 0x0300, 0x0316, 0x0000},
    {0x0041/*'A'*/, 0x0300, 0x0000},
    {0x00C0, 0x0301, 0x0000},
    /* this would work with forced normalization */
    {0x00C0, 0x0316, 0x0000}
};

const static UChar testTargetCases[][MAX_TOKEN_LEN] = {
    {0x0041/*'A'*/, 0x0301, 0x0300, 0x0000},
    {0x0041/*'A'*/, 0x0316, 0x0300, 0x0000},
    {0x00C0, 0},
    {0x0041/*'A'*/, 0x0301, 0x0300, 0x0000},
    /* this would work with forced normalization */
    {0x0041/*'A'*/, 0x0316, 0x0300, 0x0000}
};

const static UCollationResult results[] = {
    UCOL_GREATER,
    UCOL_EQUAL,
    UCOL_EQUAL,
    UCOL_GREATER,
    UCOL_EQUAL
};

static void FunkyATest( )
{
    
    int32_t i;
    UErrorCode status = U_ZERO_ERROR;
    myCollation = ucol_open("en_US", &status);
    if(U_FAILURE(status)){
        log_err("ERROR: in creation of rule based collator: %s\n", myErrorName(status));
        return;
    }
    log_verbose("Testing some A letters, for some reason\n");
    ucol_setAttribute(myCollation, UCOL_NORMALIZATION_MODE, UCOL_ON, &status);
    ucol_setStrength(myCollation, UCOL_TERTIARY);
    for (i = 0; i < 4 ; i++)
    {
        doTest(myCollation, testSourceCases[i], testTargetCases[i], results[i]);
    }
    ucol_close(myCollation);
}

UColAttributeValue caseFirst[] = {
    UCOL_OFF,
    UCOL_LOWER_FIRST,
    UCOL_UPPER_FIRST
};


UColAttributeValue alternateHandling[] = {
    UCOL_NON_IGNORABLE,
    UCOL_SHIFTED
};

UColAttributeValue caseLevel[] = {
    UCOL_OFF,
    UCOL_ON
};

UColAttributeValue strengths[] = {
    UCOL_PRIMARY,
    UCOL_SECONDARY,
    UCOL_TERTIARY,
    UCOL_QUATERNARY,
    UCOL_IDENTICAL
};

static const char * caseFirstC[] = {
    "UCOL_OFF",
    "UCOL_LOWER_FIRST",
    "UCOL_UPPER_FIRST"
};


static const char * alternateHandlingC[] = {
    "UCOL_NON_IGNORABLE",
    "UCOL_SHIFTED"
};

static const char * caseLevelC[] = {
    "UCOL_OFF",
    "UCOL_ON"
};

static const char * strengthsC[] = {
    "UCOL_PRIMARY",
    "UCOL_SECONDARY",
    "UCOL_TERTIARY",
    "UCOL_QUATERNARY",
    "UCOL_IDENTICAL"
};


static void PrintMarkDavis( )
{
  UErrorCode status = U_ZERO_ERROR;
  UChar m[256];
  uint8_t sortkey[256];
  UCollator *coll = ucol_open("en_US", &status);
  uint32_t h,i,j,k, sortkeysize;
  uint32_t sizem = 0;
  char buffer[512];
  uint32_t len = 512;

  log_verbose("PrintMarkDavis");

  u_uastrcpy(m, "Mark Davis");
  sizem = u_strlen(m);


  m[1] = 0xe4;

  for(i = 0; i<sizem; i++) {
    fprintf(stderr, "\\u%04X ", m[i]);
  }
  fprintf(stderr, "\n");

  for(h = 0; h<sizeof(caseFirst)/sizeof(caseFirst[0]); h++) {
    ucol_setAttribute(coll, UCOL_CASE_FIRST, caseFirst[i], &status);
    fprintf(stderr, "caseFirst: %s\n", caseFirstC[h]);

    for(i = 0; i<sizeof(alternateHandling)/sizeof(alternateHandling[0]); i++) {
      ucol_setAttribute(coll, UCOL_ALTERNATE_HANDLING, alternateHandling[i], &status);
      fprintf(stderr, "  AltHandling: %s\n", alternateHandlingC[i]);

      for(j = 0; j<sizeof(caseLevel)/sizeof(caseLevel[0]); j++) {
        ucol_setAttribute(coll, UCOL_CASE_LEVEL, caseLevel[j], &status);
        fprintf(stderr, "    caseLevel: %s\n", caseLevelC[j]);

        for(k = 0; k<sizeof(strengths)/sizeof(strengths[0]); k++) {
          ucol_setAttribute(coll, UCOL_STRENGTH, strengths[k], &status);
          sortkeysize = ucol_getSortKey(coll, m, sizem, sortkey, 256);
          fprintf(stderr, "      strength: %s\n      Sortkey: ", strengthsC[k]);
          fprintf(stderr, "%s\n", ucol_sortKeyToString(coll, sortkey, buffer, &len));
        }

      }

    }

  }
}

static void BillFairmanTest( ) {
/*
** check for actual locale via ICU resource bundles
**
** lp points to the original locale ("fr_FR_....")
*/

    UResourceBundle *lr,*cr;
    UErrorCode              lec = U_ZERO_ERROR;
    const char *lp = "fr_FR_you_ll_never_find_this_locale";

    log_verbose("BillFairmanTest\n");

    lr = ures_open(NULL,lp,&lec);
    if (lr) {
        cr = ures_getByKey(lr,"CollationElements",0,&lec);
        if (cr) {
            lp = ures_getLocale(cr,&lec);
            if (lp) {
                if (U_SUCCESS(lec)) {
                    if(strcmp(lp, "fr") != 0) {
                        log_err("Wrong locale for French Collation Data, expected \"fr\" got %s", lp);
                    }
                }
            }
            ures_close(cr);
        }
        ures_close(lr);
    }
}

static void testPrimary(UCollator* col, const UChar* p,const UChar* q){
    UChar source[256] = { '\0'};
    UChar target[256] = { '\0'};
    UChar preP = 0x31a3;
    UChar preQ = 0x310d;
/*
    UChar preP = (*p>0x0400 && *p<0x0500)?0x00e1:0x491;
    UChar preQ = (*p>0x0400 && *p<0x0500)?0x0041:0x413;
*/
    /*log_verbose("Testing primary\n");*/

    doTest(col, p, q, UCOL_LESS);
/*
    UCollationResult result = ucol_strcoll(col,p,u_strlen(p),q,u_strlen(q));

    if(result!=UCOL_LESS){
       aescstrdup(p,utfSource,256);
       aescstrdup(q,utfTarget,256);
       fprintf(file,"Primary failed  source: %s target: %s \n", utfSource,utfTarget);
    }
*/
    source[0] = preP;
    u_strcpy(source+1,p);
    target[0] = preQ;
    u_strcpy(target+1,q);
    doTest(col, source, target, UCOL_LESS);
/*
    fprintf(file,"Primary swamps 2nd failed  source: %s target: %s \n", utfSource,utfTarget);
*/
}
   
static void testSecondary(UCollator* col, const UChar* p,const UChar* q){
    UChar source[256] = { '\0'};
    UChar target[256] = { '\0'};

    /*log_verbose("Testing secondary\n");*/

    doTest(col, p, q, UCOL_LESS);
/*
    fprintf(file,"secondary failed  source: %s target: %s \n", utfSource,utfTarget);
*/
    source[0] = 0x0053;
    u_strcpy(source+1,p);
    target[0]= 0x0073;
    u_strcpy(target+1,q);

    doTest(col, source, target, UCOL_LESS);
/*
    fprintf(file,"secondary swamps 3rd failed  source: %s target: %s \n",utfSource,utfTarget);
*/


    u_strcpy(source,p);
    source[u_strlen(p)] = 0x62;
    source[u_strlen(p)+1] = 0;


    u_strcpy(target,q);
    target[u_strlen(q)] = 0x61;
    target[u_strlen(q)+1] = 0;

    doTest(col, source, target, UCOL_GREATER);

/*
    fprintf(file,"secondary is swamped by 1  failed  source: %s target: %s \n",utfSource,utfTarget);
*/
}

static void testTertiary(UCollator* col, const UChar* p,const UChar* q){
    UChar source[256] = { '\0'};
    UChar target[256] = { '\0'};

    /*log_verbose("Testing tertiary\n");*/

    doTest(col, p, q, UCOL_LESS);
/*
    fprintf(file,"Tertiary failed  source: %s target: %s \n",utfSource,utfTarget);
*/
    source[0] = 0x0020;
    u_strcpy(source+1,p);
    target[0]= 0x002D;
    u_strcpy(target+1,q);

    doTest(col, source, target, UCOL_LESS);
/*
    fprintf(file,"Tertiary swamps 4th failed  source: %s target: %s \n", utfSource,utfTarget);
*/

    u_strcpy(source,p);
    source[u_strlen(p)] = 0xE0;
    source[u_strlen(p)+1] = 0;

    u_strcpy(target,q);
    target[u_strlen(q)] = 0x61;
    target[u_strlen(q)+1] = 0;

    doTest(col, source, target, UCOL_GREATER);

/*
    fprintf(file,"Tertiary is swamped by 3rd failed  source: %s target: %s \n",utfSource,utfTarget);
*/
}

static void testEquality(UCollator* col, const UChar* p,const UChar* q){
/*
    UChar source[256] = { '\0'};
    UChar target[256] = { '\0'};
*/

    doTest(col, p, q, UCOL_EQUAL);
/*
    fprintf(file,"Primary failed  source: %s target: %s \n", utfSource,utfTarget);
*/
}

static void testCollator(UCollator *coll, UErrorCode *status) {
  const UChar *rules = NULL, *current = NULL;
  int32_t ruleLen = 0;
  uint32_t strength = 0;
  uint32_t chOffset = 0; uint32_t chLen = 0;
  uint32_t exOffset = 0; uint32_t exLen = 0;
  uint32_t firstEx = 0;
/*  uint32_t rExpsLen = 0; */
  uint32_t firstLen = 0;
  UBool varT = FALSE; UBool top_ = TRUE;
  UBool startOfRules = TRUE;
  UBool lastReset = FALSE;
  UColTokenParser src;
  UColOptionSet opts;

  UChar first[256];
  UChar second[256];
  UChar *rulesCopy = NULL;

  src.opts = &opts;

  rules = ucol_getRules(coll, &ruleLen);
  if(U_SUCCESS(*status) && ruleLen > 0) {
    rulesCopy = (UChar *)uprv_malloc((ruleLen+UCOL_TOK_EXTRA_RULE_SPACE_SIZE)*sizeof(UChar));
    uprv_memcpy(rulesCopy, rules, ruleLen*sizeof(UChar));
    src.source = src.current = rulesCopy;
    src.end = rulesCopy+ruleLen;
    src.extraCurrent = src.end;
    src.extraEnd = src.end+UCOL_TOK_EXTRA_RULE_SPACE_SIZE;
    *first = *second = 0;

    while ((current = ucol_tok_parseNextToken(&src, &strength, 
                      &chOffset, &chLen, &exOffset, &exLen,
                      &varT, &top_, startOfRules, status)) != NULL) {
      startOfRules = FALSE;

      u_strncpy(second,rulesCopy+chOffset, chLen);
      second[chLen] = 0;

      if(exLen > 0 && firstEx == 0) {
        u_strncat(first, rulesCopy+exOffset, exLen);
        first[firstLen+exLen] = 0;
      } 

      lastReset = FALSE;

      switch(strength){
      case UCOL_IDENTICAL:
          testEquality(coll,first,second);
          break;
      case UCOL_PRIMARY:
          testPrimary(coll,first,second);
          break;
      case UCOL_SECONDARY:
          testSecondary(coll,first,second);
          break;
      case UCOL_TERTIARY:
          testTertiary(coll,first,second);
          break;
      case UCOL_TOK_RESET:
      default:
          break;
      }

      firstLen = chLen;
      firstEx = exLen;
      u_strcpy(first, second);

    }
    uprv_free(rulesCopy);
  }
}

int ucaTest(void *collator, const int object, const UChar *source, const int sLen, const UChar *target, const int tLen) {
  UCollator *UCA = (UCollator *)collator;
  return ucol_strcoll(UCA, source, sLen, target, tLen);
}

int winTest(void *collator, const int object, const UChar *source, const int sLen, const UChar *target, const int tLen) {
#ifdef WIN32
  LCID lcid = (LCID)collator;
  return CompareString(lcid, 0, source, sLen, target, tLen);
#else
  return 0;
#endif
}

static UCollationResult swampEarlier(tst_strcoll* func, void *collator, int opts, 
                                     UChar s1, UChar s2, 
                                     const UChar *s, const uint32_t sLen, 
                                     const UChar *t, const uint32_t tLen) {
  UChar source[256] = {0};
  UChar target[256] = {0};
  
  source[0] = s1;
  u_strcpy(source+1, s);
  target[0] = s2;
  u_strcpy(target+1, t);

  return func(collator, opts, source, sLen+1, target, tLen+1);
}

static UCollationResult swampLater(tst_strcoll* func, void *collator, int opts, 
                                   UChar s1, UChar s2, 
                                   const UChar *s, const uint32_t sLen, 
                                   const UChar *t, const uint32_t tLen) {
  UChar source[256] = {0};
  UChar target[256] = {0};
  
  u_strcpy(source, s);
  source[sLen] = s1;
  u_strcpy(target, t);
  target[tLen] = s2;

  return func(collator, opts, source, sLen+1, target, tLen+1);
}

static uint32_t probeStrength(tst_strcoll* func, void *collator, int opts, 
                              const UChar *s, const uint32_t sLen, 
                              const UChar *t, const uint32_t tLen,
                              UCollationResult result) {
  UChar fPrimary = 0x6d;
  UChar sPrimary = 0x6e;
  UChar fSecondary = 0x310d;
  UChar sSecondary = 0x31a3;
  UChar fTertiary = 0x310f;
  UChar sTertiary = 0x31b7;

  UCollationResult oposite;
  if(result == UCOL_EQUAL) {
    return UCOL_IDENTICAL;
  } else if(result == UCOL_GREATER) {
    oposite = UCOL_LESS;
  } else {
    oposite = UCOL_GREATER;
  }

  if(swampEarlier(func, collator, opts, sSecondary, fSecondary, s, sLen, t, tLen) == result) {
    return UCOL_PRIMARY;
  } else if((swampEarlier(func, collator, opts, sTertiary, 0x310f, s, sLen, t, tLen) == result) && 
    (swampEarlier(func, collator, opts, 0x310f, sTertiary, s, sLen, t, tLen) == result)) {
    return UCOL_SECONDARY;
  } else if((swampLater(func, collator, opts, sTertiary, fTertiary, s, sLen, t, tLen) == result) && 
    (swampLater(func, collator, opts, fTertiary, sTertiary, s, sLen, t, tLen) == result)) {
    return UCOL_TERTIARY;
  } else if((swampLater(func, collator, opts, sTertiary, 0x310f, s, sLen, t, tLen) == oposite) && 
    (swampLater(func, collator, opts, fTertiary, sTertiary, s, sLen, t, tLen) == oposite)) {
    return UCOL_QUATERNARY;
  } else {
    return UCOL_IDENTICAL;
  }
}

static char *getRelationSymbol(UCollationResult res, uint32_t strength, char *buffer) {
  uint32_t i = 0;

  if(res == UCOL_EQUAL || strength == 0xdeadbeef) {
    buffer[0] = '=';
    buffer[1] = '=';
    buffer[2] = '\0';
  } else if(res == UCOL_GREATER) {
    for(i = 0; i<strength+1; i++) {
      buffer[i] = '>';
    }
    buffer[strength+1] = '\0';
  } else {
    for(i = 0; i<strength+1; i++) {
      buffer[i] = '<';
    }
    buffer[strength+1] = '\0';
  }

  return buffer;
}
  


static void logFailure (const char *platform, const char *test, 
                        const UChar *source, const uint32_t sLen,
                        const UChar *target, const uint32_t tLen,
                        UCollationResult realRes, uint32_t realStrength,
                        UCollationResult expRes, uint32_t expStrength) { 

  uint32_t i = 0;

  char sEsc[256], s[256], tEsc[256], t[256], b[256], output[256], relation[256];

  *sEsc = *tEsc = *s = *t = 0;
  for(i = 0; i<sLen; i++) {
    sprintf(b, "%04X", source[i]);
    strcat(sEsc, "\\u");
    strcat(sEsc, b);
    strcat(s, b);
    strcat(s, " ");
  }
  for(i = 0; i<tLen; i++) {
    sprintf(b, "%04X", target[i]);
    strcat(tEsc, "\\u");
    strcat(tEsc, b);
    strcat(t, b);
    strcat(t, " ");
  }
/*
  strcpy(output, "[[ ");
  strcat(output, sEsc);
  strcat(output, getRelationSymbol(expRes, expStrength, relation));
  strcat(output, tEsc);

  strcat(output, " : ");

  strcat(output, sEsc);
  strcat(output, getRelationSymbol(realRes, realStrength, relation));
  strcat(output, tEsc);
  strcat(output, " ]] ");

  log_verbose("%s", output);
*/


  strcpy(output, "DIFF: ");

  strcat(output, s);
  strcat(output, " : ");
  strcat(output, t);

  strcat(output, test);
  strcat(output, ": ");

  strcat(output, sEsc);
  strcat(output, getRelationSymbol(expRes, expStrength, relation));
  strcat(output, tEsc);

  strcat(output, " ");

  strcat(output, platform);
  strcat(output, ": ");

  strcat(output, sEsc);
  strcat(output, getRelationSymbol(realRes, realStrength, relation));
  strcat(output, tEsc);

  log_verbose("%s\n", output);

}

static printOutRules(const UChar *rules) {
  uint32_t len = u_strlen(rules);
  uint32_t i = 0;
  char toPrint;
  uint32_t line = 0;

  fprintf(stdout, "Rules:");

  for(i = 0; i<len; i++) {
    if(rules[i]<0x7f && rules[i]>=0x20) {
      toPrint = (char)rules[i];
      if(toPrint == '&') {
        line = 1;
        fprintf(stdout, "\n&");
      } else if(toPrint == ';') {
        fprintf(stdout, "<<");
        line+=2;
      } else if(toPrint == ',') {
        fprintf(stdout, "<<<");
        line+=3;
      } else {
        fprintf(stdout, "%c", toPrint);
        line++;
      }
    } else if(rules[i]<0x3400 || rules[i]>=0xa000) {
      fprintf(stdout, "\\u%04X", rules[i]);
      line+=6;
    }
    if(line>72) {
      fprintf(stdout, "\n");
      line = 0;
    }
  }

  log_verbose("\n");

}


static uint32_t testSwitch(tst_strcoll* func, void *collator, int opts, uint32_t strength, const UChar *first, const UChar *second, const char* msg) {
  uint32_t diffs = 0;
  UCollationResult realResult;
  uint32_t realStrength;

  uint32_t sLen = u_strlen(first);
  uint32_t tLen = u_strlen(second);

  realResult = func(collator, opts, first, sLen, second, tLen);
  realStrength = probeStrength(func, collator, opts, first, sLen, second, tLen, realResult);

  if(strength == UCOL_IDENTICAL && realResult != UCOL_IDENTICAL) {
    logFailure(msg, "tailoring", first, sLen, second, tLen, realResult, realStrength, UCOL_EQUAL, strength);
    diffs++;
  } else if(realResult != UCOL_LESS || realStrength != strength) {
    logFailure(msg, "tailoring", first, sLen, second, tLen, realResult, realStrength, UCOL_LESS, strength);
    diffs++;
  } 
  return diffs;
}


static void testAgainstUCA(UCollator *coll, UCollator *UCA, LCID lcid, UErrorCode *status) {
  const UChar *rules = NULL, *current = NULL;
  int32_t ruleLen = 0;
  uint32_t strength = 0;
  uint32_t chOffset = 0; uint32_t chLen = 0;
  uint32_t exOffset = 0; uint32_t exLen = 0;
/*  uint32_t rExpsLen = 0; */
  uint32_t firstLen = 0, secondLen = 0;
  UBool varT = FALSE; UBool top_ = TRUE;
  UBool startOfRules = TRUE;
  UColTokenParser src;
  UColOptionSet opts;

  UChar first[256];
  UChar second[256];
  UChar *rulesCopy = NULL;

  uint32_t UCAdiff = 0;
  uint32_t Windiff = 1;

  src.opts = &opts;

  rules = ucol_getRules(coll, &ruleLen);

  /*printOutRules(rules);*/

  if(U_SUCCESS(*status) && ruleLen > 0) {
    rulesCopy = (UChar *)uprv_malloc((ruleLen+UCOL_TOK_EXTRA_RULE_SPACE_SIZE)*sizeof(UChar));
    uprv_memcpy(rulesCopy, rules, ruleLen*sizeof(UChar));
    src.source = src.current = rulesCopy;
    src.end = rulesCopy+ruleLen;
    src.extraCurrent = src.end;
    src.extraEnd = src.end+UCOL_TOK_EXTRA_RULE_SPACE_SIZE;
    *first = *second = 0;

    while ((current = ucol_tok_parseNextToken(&src, &strength, 
                      &chOffset, &chLen, &exOffset, &exLen,
                      &varT, &top_, startOfRules, status)) != NULL) {
      startOfRules = FALSE;

      u_strncpy(second,rulesCopy+chOffset, chLen);
      second[chLen] = 0;
      secondLen = chLen;

      if(exLen > 0) {
        u_strncat(first, rulesCopy+exOffset, exLen);
        first[firstLen+exLen] = 0;
        firstLen += exLen;
      } 

      if(strength != UCOL_TOK_RESET) {
        if((*first<0x3400 || *first>=0xa000) && (*second<0x3400 || *second>=0xa000)) {
          UCAdiff += testSwitch(&ucaTest, (void *)UCA, 0, strength, first, second, "UCA");
          /*Windiff += testSwitch(&winTest, (void *)lcid, 0, strength, first, second, "Win32");*/
        }
      }


      firstLen = chLen;
      u_strcpy(first, second);

    }
    if(UCAdiff != 0 && Windiff != 0) {
      log_verbose("\n");
    }
    if(UCAdiff == 0) {
      log_verbose("No immediate difference with UCA!\n");
    }
    if(Windiff == 0) {
      log_verbose("No immediate difference with Win32!\n");
    }
    uprv_free(rulesCopy);
  }
}

static void testCEs(UCollator *coll, UErrorCode *status) {

  const UChar *rules = NULL, *current = NULL;
  int32_t ruleLen = 0;

  uint32_t strength = 0;
  uint32_t maxStrength = UCOL_IDENTICAL;
  uint32_t baseCE, baseContCE, nextCE, nextContCE, currCE, currContCE;
  uint32_t lastCE;
  uint32_t lastContCE;

  int32_t result = 0;
  uint32_t chOffset = 0; uint32_t chLen = 0;
  uint32_t exOffset = 0; uint32_t exLen = 0;
  uint32_t oldOffset = 0;

  /* uint32_t rExpsLen = 0; */
  /* uint32_t firstLen = 0; */
  UBool varT = FALSE; UBool top_ = TRUE;
  UBool startOfRules = TRUE;
  UColTokenParser src;
  UColOptionSet opts;

  UChar *rulesCopy = NULL;
  collIterate c;

  baseCE=baseContCE=nextCE=nextContCE=currCE=currContCE=lastCE=lastContCE = UCOL_NOT_FOUND;

  src.opts = &opts;

  rules = ucol_getRules(coll, &ruleLen);

  ucol_initInverseUCA(status);

  if(U_SUCCESS(*status) && ruleLen > 0) {
    rulesCopy = (UChar *)uprv_malloc((ruleLen+UCOL_TOK_EXTRA_RULE_SPACE_SIZE)*sizeof(UChar));
    uprv_memcpy(rulesCopy, rules, ruleLen*sizeof(UChar));
    src.source = src.current = rulesCopy;
    src.end = rulesCopy+ruleLen;
    src.extraCurrent = src.end;
    src.extraEnd = src.end+UCOL_TOK_EXTRA_RULE_SPACE_SIZE;

    while ((current = ucol_tok_parseNextToken(&src, &strength, 
                      &chOffset, &chLen, &exOffset, &exLen,
                      &varT, &top_, startOfRules, status)) != NULL) {
      startOfRules = FALSE;

      init_collIterate(coll, rulesCopy+chOffset, chLen, &c);

      currCE = ucol_getNextCE(coll, &c, status);
      if(currCE == 0 && UCOL_ISTHAIPREVOWEL(*(rulesCopy+chOffset))) {
        log_verbose("Thai prevowel detected. Will pick next CE\n");
        currCE = ucol_getNextCE(coll, &c, status);
      }

      currContCE = ucol_getNextCE(coll, &c, status);
      if(!isContinuation(currContCE)) {
        currContCE = 0;
      }

      if(strength == UCOL_TOK_RESET) {
        if(top_ == TRUE) {
          nextCE = baseCE = currCE = UCOL_RESET_TOP_VALUE;
          nextContCE = baseContCE = currContCE = 0;
        } else {
          nextCE = baseCE = currCE;
          nextContCE = baseContCE = currContCE;
        }
        maxStrength = UCOL_IDENTICAL;
      } else {
        if(strength < maxStrength) {
          maxStrength = strength;
          if(baseCE == UCOL_RESET_TOP_VALUE) {
              log_verbose("Resetting to [top]\n");
              nextCE = UCOL_NEXT_TOP_VALUE;
              nextContCE = 0;
          } else {
            result = ucol_inv_getNextCE(baseCE, baseContCE, &nextCE, &nextContCE, maxStrength);
          }
          if(result < 0) {
            if(isTailored(coll, *(rulesCopy+oldOffset), status)) {
              log_verbose("Reset is tailored codepoint %04X, don't know how to continue, taking next test\n", *(rulesCopy+oldOffset));
              return;
            } else {
              log_err("couldn't find the CE\n");
              return;
            }
          }
        }

        currCE &= 0xFFFFFF3F;
        currContCE &= 0xFFFFFFBF;

        if(maxStrength == UCOL_IDENTICAL) {
          if(baseCE != currCE || baseContCE != currContCE) {
            log_err("current CE  (initial strength UCOL_EQUAL)\n");
          }
        } else {
          if(strength == UCOL_IDENTICAL) {
            if(lastCE != currCE || lastContCE != currContCE) {
              log_err("current CE  (initial strength UCOL_EQUAL)\n");
            }
          } else {
            if(currCE > nextCE || (currCE == nextCE && currContCE >= nextContCE)) {
              log_err("current CE is not less than base CE\n");
            }
            if(currCE < lastCE || (currCE == lastCE && currContCE <= lastContCE)) {
              log_err("sequence of generated CEs is broken\n");
            }
          }
        }

      }

      oldOffset = chOffset;
      lastCE = currCE & 0xFFFFFF3F;
      lastContCE = currContCE & 0xFFFFFFBF;
    }
    uprv_free(rulesCopy);
  }
}

static const char* localesToTest[] = {
"ar", "bg", "ca", "cs", "da",
"el", "en_BE", "en_US_POSIX", 
"es", "et", "fi", "fr", "hi", 
"hr", "hu", "is", "iw", "ja", 
"ko", "lt", "lv", "mk", "mt", 
"nb", "nn", "nn_NO", "pl", "ro", 
"ru", "sh", "sk", "sl", "sq",
"sr", "sv", "th", "tr", "uk", 
"vi", "zh", "zh_TW"
};

static const char* rulesToTest[] = {
  /*"& Z < p, P",*/
    /* Cui Mins rules */
    "<o,O<p,P<q,Q<'?'/u<r,R<u,U", /*"<o,O<p,P<q,Q<r,R<u,U & Qu<'?'",*/
    "<o,O<p,P<q,Q;'?'/u<r,R<u,U", /*"<o,O<p,P<q,Q<r,R<u,U & Qu;'?'",*/
    "<o,O<p,P<q,Q,'?'/u<r,R<u,U", /*"<o,O<p,P<q,Q<r,R<u,U&'Qu','?'",*/
    "<3<4<5<c,C<f,F<m,M<o,O<p,P<q,Q;'?'/u<r,R<u,U",  /*"<'?'<3<4<5<a,A<f,F<m,M<o,O<p,P<q,Q<r,R<u,U & Qu;'?'",*/
    "<'?';Qu<3<4<5<c,C<f,F<m,M<o,O<p,P<q,Q<r,R<u,U",  /*"<'?'<3<4<5<a,A<f,F<m,M<o,O<p,P<q,Q<r,R<u,U & '?';Qu",*/
    "<3<4<5<c,C<f,F<m,M<o,O<p,P<q,Q;'?'/um<r,R<u,U", /*"<'?'<3<4<5<a,A<f,F<m,M<o,O<p,P<q,Q<r,R<u,U & Qum;'?'",*/
    "<'?';Qum<3<4<5<c,C<f,F<m,M<o,O<p,P<q,Q<r,R<u,U"  /*"<'?'<3<4<5<a,A<f,F<m,M<o,O<p,P<q,Q<r,R<u,U & '?';Qum"*/
};

static UBool hasCollationElements(const char *locName) {

  UErrorCode status = U_ZERO_ERROR;
  UResourceBundle *ColEl = NULL;

  UResourceBundle *loc = ures_open(NULL, locName, &status);;

  if(U_SUCCESS(status)) {
    status = U_ZERO_ERROR;
    ColEl = ures_getByKey(loc, "CollationElements", ColEl, &status);
    if(status == U_ZERO_ERROR) { /* do the test - there are real elements */
      ures_close(ColEl);
      ures_close(loc);
      return TRUE;
    }
    ures_close(ColEl);
    ures_close(loc);
  }
  return FALSE;
}


static void TestCollations( ) {
  int32_t noOfLoc = uloc_countAvailable();
  int32_t i = 0, j = 0;
  LCID lcid = LOCALE_SYSTEM_DEFAULT;

  UErrorCode status = U_ZERO_ERROR;
  char cName[256];
  UChar name[256];
  int32_t nameSize;
  

  const char *locName = NULL;
  UCollator *coll = NULL;
  UCollator *UCA = ucol_open("", &status);
  UColAttributeValue oldStrength = ucol_getAttribute(UCA, UCOL_STRENGTH, &status);
  ucol_setAttribute(UCA, UCOL_STRENGTH, UCOL_QUATERNARY, &status);

  for(i = 0; i<noOfLoc; i++) {
    status = U_ZERO_ERROR;
    locName = uloc_getAvailable(i);
    if(hasCollationElements(locName)) {
        lcid = uloc_getLCID(locName);
        nameSize = uloc_getDisplayName(locName, NULL, name, 256, &status);
        for(j = 0; j<nameSize; j++) {
          cName[j] = (char)name[j];
        }
        cName[nameSize] = 0;
        log_verbose("\nTesting locale %s (%s), LCID %04X\n", locName, cName, lcid);
        coll = ucol_open(locName, &status);
        testAgainstUCA(coll, UCA, lcid, &status);
        ucol_close(coll);
    }
  }
  ucol_setAttribute(UCA, UCOL_STRENGTH, oldStrength, &status);
  ucol_close(UCA);
}

static void RamsRulesTest( ) {
  UErrorCode status = U_ZERO_ERROR;
  int32_t i = 0;
  UCollator *coll = NULL;
  UCollator *UCA = ucol_open("", &status);
  UChar rule[2048];
  uint32_t ruleLen;
  int32_t noOfLoc = uloc_countAvailable();
  const char *locName = NULL;

  log_verbose("RamsRulesTest\n");

  for(i = 0; i<noOfLoc; i++) {
    status = U_ZERO_ERROR;
    locName = uloc_getAvailable(i);
    if(hasCollationElements(locName)) {
      log_verbose("Testing locale %s\n", locName);
      coll = ucol_open(locName, &status);
      if(U_SUCCESS(status)) {
        ucol_setAttribute(coll, UCOL_CASE_FIRST, UCOL_OFF, &status);
        testCollator(coll, &status);
        testCEs(coll, &status);
        ucol_close(coll);
      }
    }
  }

  for(i = 0; i<sizeof(rulesToTest)/sizeof(rulesToTest[0]); i++) {
    log_verbose("Testing rule: %s\n", rulesToTest[i]);
    u_uastrcpy(rule, rulesToTest[i]);
    ruleLen = u_strlen(rule);
    coll = ucol_openRules(rule, ruleLen, UCOL_NO_NORMALIZATION, UCOL_TERTIARY, &status);
    if(U_SUCCESS(status)) {
      testCollator(coll, &status);
      testCEs(coll, &status);
      ucol_close(coll);
    }
  }

}

static void IsTailoredTest( ) {
  UErrorCode status = U_ZERO_ERROR;
  uint32_t i = 0;
  UCollator *coll = NULL;
  UChar rule[2048];
  UChar tailored[2048];
  UChar notTailored[2048];
  uint32_t ruleLen, tailoredLen, notTailoredLen;

  log_verbose("IsTailoredTest\n");
 
  u_uastrcpy(rule, "&Z < A, B, C;c < d");
  ruleLen = u_strlen(rule);

  u_uastrcpy(tailored, "ABCcd");
  tailoredLen = u_strlen(tailored);

  u_uastrcpy(notTailored, "ZabD");
  notTailoredLen = u_strlen(notTailored);

  coll = ucol_openRules(rule, ruleLen, UCOL_NO_NORMALIZATION, UCOL_TERTIARY, &status);
  if(U_SUCCESS(status)) {
    for(i = 0; i<tailoredLen; i++) {
      if(!isTailored(coll, tailored[i], &status)) {
        log_err("%i: %04X should be tailored - it is reported as not\n", i, tailored[i]);
      }
    }
    for(i = 0; i<notTailoredLen; i++) {
      if(isTailored(coll, notTailored[i], &status)) {
        log_err("%i: %04X should not be tailored - it is reported as it is\n", i, notTailored[i]);
      }
    }
    ucol_close(coll);
  }
}

/**
* Tests the [variable top] tag in rule syntax. Since the default [alternate]
* tag has the value shifted, any codepoints before [variable top] should give
* a primary ce of 0.
*/
static void TestVariableTop(void) {
    const char       *str          = "[alternate shifted] &B [variable top]";
          int         len          = strlen(str);
          UChar      *rules;
          UCollator  *myCollation;
          UCollator  *enCollation;
          UErrorCode  status       = U_ZERO_ERROR;
          UChar       source[1];
          UChar       ch;
          uint8_t     result[20];

    rules = (UChar*)malloc(sizeof(UChar*) * (len + 1));
    u_uastrcpy(rules, str);

    enCollation = ucol_open("en_US", &status);
    myCollation = ucol_openRules(rules, len, UCOL_NO_NORMALIZATION, 
                                 UCOL_PRIMARY, &status);
    if (U_FAILURE(status)) {
        log_err("ERROR: in creation of rule based collator :%s\n", 
                myErrorName(status));
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

    /* space is supposed to be a variable */
    source[0] = ' ';
    len = ucol_getSortKey(enCollation, source, 1, result, 
                          sizeof(result));

    ch = 'A';
    while (ch < 'z') {
        source[0] = ch;
        len = ucol_getSortKey(enCollation, source, 1, result, 
                              sizeof(result));
        len = ucol_getSortKey(myCollation, source, 1, result,
                              sizeof(result));
        ch ++;
    }
  
    free(rules);
    ucol_close(enCollation);
    ucol_close(myCollation);
    enCollation = NULL;
    myCollation = NULL;
}
 
static void genericOrderingTest(UCollator *coll, const char *s[], uint32_t size) {
  UChar t1[256] = {0};
  UChar t2[256] = {0};

  uint32_t i = 0, j = 0;

  for(i = 0; i < size-1; i++) {
    for(j = i+1; j < size; j++) {
      u_unescape(s[i], t1, 256);
      u_unescape(s[j], t2, 256);
      doTest(coll, t1, t2, UCOL_LESS);
    }
  }
}

static void genericLocaleStarter(const char *locale, const char *s[], uint32_t size) {
  UErrorCode status = U_ZERO_ERROR;
  UCollator *coll = ucol_open(locale, &status);
  if(U_SUCCESS(status)) {
    genericOrderingTest(coll, s, size);
  } else {
    log_err("Unable to open collator for locale %s\n", locale);
  }
}

static void genericRulesStarter(const char *rules, const char *s[], uint32_t size) {
  UErrorCode status = U_ZERO_ERROR;
  UChar rlz[2048] = { 0 };
  UCollator *coll = NULL;
  uint32_t rlen = u_unescape(rules, rlz, 2048);

  ucol_openRules(rlz, rlen, UCOL_DEFAULT, UCOL_DEFAULT, &status);

  if(U_SUCCESS(status)) {
    genericOrderingTest(coll, s, size);
  } else {
    log_err("Unable to open collator with rules %s\n", rules);
  }
}

const static char chTest[][20] = {
  /*"c",
  "C",
  "ca", "cb", "cx", "cy", "CZ",
  "c\\u030C", "C\\u030C",
  "h",
  "H",
  "ha", "Ha", "harly", "hb", "HB", "hx", "HX", "hy", "HY",
  "ch", "cH", "Ch", "CH",
  "cha", "charly", "che", */"chh", "chch"/*, "chr",
  "i", "I", "iarly", 
  "r", "R",
  "r\\u030C", "R\\u030C",
  "s",
  "S",
  "s\\u030C", "S\\u030C",
  "z", "Z",
  "z\\u030C", "Z\\u030C"*/
};

static void TestChMove(void) {
  UChar t1[256] = {0};
  UChar t2[256] = {0};

  uint32_t i = 0, j = 0;
  uint32_t size = 0;
  UErrorCode status = U_ZERO_ERROR;

  UCollator *coll = ucol_open("cs", &status);

  if(U_SUCCESS(status)) {
    size = sizeof(chTest)/sizeof(chTest[0]);
    for(i = 0; i < size-1; i++) {
      for(j = i+1; j < size; j++) {
        u_unescape(chTest[i], t1, 256);
        u_unescape(chTest[j], t2, 256);
        doTest(coll, t1, t2, UCOL_LESS);
      }
    }
  } 
}

const static char impTest[][20] = {
  "\\u4e00",
    "a",
    "A",
    "b",
    "B",
    "\\u4e01"
};


static void TestImplicitTailoring(void) {
  UChar t1[256] = {0};
  UChar t2[256] = {0};

  char *rule = "&\\u4e00 < a <<< A < b <<< B";

  uint32_t i = 0, j = 0;
  uint32_t size = 0;
  uint32_t ruleLen = 0;
  UErrorCode status = U_ZERO_ERROR;
  UCollator *coll = NULL;
  ruleLen = u_unescape(rule, t1, 256);

  coll = ucol_openRules(t1, ruleLen, UCOL_NO_NORMALIZATION, UCOL_TERTIARY, &status);

  if(U_SUCCESS(status)) {
    size = sizeof(impTest)/sizeof(impTest[0]);
    for(i = 0; i < size-1; i++) {
      for(j = i+1; j < size; j++) {
        u_unescape(impTest[i], t1, 256);
        u_unescape(impTest[j], t2, 256);
        doTest(coll, t1, t2, UCOL_LESS);
      }
    }
  } 
}

static void TestFCDProblem(void) {
  UChar t1[256] = {0};
  UChar t2[256] = {0};

  char *s1 = "\\u0430\\u0306\\u0325";
  char *s2 = "\\u04D1\\u0325";

  UErrorCode status = U_ZERO_ERROR;
  UCollator *coll = ucol_open("", &status);
  u_unescape(s1, t1, 256);
  u_unescape(s2, t2, 256);

  ucol_setAttribute(coll, UCOL_NORMALIZATION_MODE, UCOL_OFF, &status);
  doTest(coll, t1, t2, UCOL_EQUAL);

  ucol_setAttribute(coll, UCOL_NORMALIZATION_MODE, UCOL_ON, &status);
  doTest(coll, t1, t2, UCOL_EQUAL);

}

struct tester{
  UChar u;
  UChar NFC[64];
  UChar NFD[64];
};

static void TestComposeDecompose(void) {
  int32_t noOfLoc = uloc_countAvailable();
  int32_t i = 0, j = 0;

  UErrorCode status = U_ZERO_ERROR;
  char cName[256];
  UChar name[256];
  int32_t nameSize;

  const char *locName = NULL;

  UChar u = 0;
  UChar NFC[256] = {0};
  UChar NFD[256] = {0};
  uint32_t nfcSize;
  uint32_t nfdSize;
  struct tester *t[0xFFFF];
  uint32_t noCases = 0;
  UCollator *coll = NULL;

  t[0] = (struct tester *)malloc(sizeof(struct tester));
  
  for(u = 0; u < 0xFFFF; u++) {
    nfcSize = unorm_normalize(&u, 1, UNORM_NFC, 0, t[noCases]->NFC, 64, &status);
    nfdSize = unorm_normalize(&u, 1, UNORM_NFD, 0, t[noCases]->NFD, 64, &status);

    if(nfcSize != nfdSize || (uprv_memcmp(t[noCases]->NFC, t[noCases]->NFD, nfcSize * sizeof(UChar)) != 0)) {
      t[noCases]->u = u;
      noCases++;
      t[noCases] = (struct tester *)malloc(sizeof(struct tester));
    }
  }

  for(i = 0; i<noOfLoc; i++) {
    status = U_ZERO_ERROR;
    locName = uloc_getAvailable(i);
    if(hasCollationElements(locName)) {
        nameSize = uloc_getDisplayName(locName, NULL, name, 256, &status);
        for(j = 0; j<nameSize; j++) {
          cName[j] = (char)name[j];
        }
        cName[nameSize] = 0;
        log_verbose("\nTesting locale %s (%s)\n", locName, cName);

        coll = ucol_open(locName, &status);

        for(u=0; u<noCases; u++) {
          doTest(coll, t[u]->NFC, t[u]->NFD, UCOL_EQUAL);
        }

        ucol_close(coll);

    }
  }
}

static TestUnmappedSpaces(void) {
  UChar u;
  UErrorCode status = U_ZERO_ERROR;
  UCollator *uca = ucol_open("", &status);
  for(u = 0; u < 0xFFFF; u++) {
    if(ucmp32_get(uca->mapping, u) == UCOL_NOT_FOUND) {
      log_verbose("%04X ", u);
    }
  }
  ucol_close(uca);
}

static void TestEmptyRule() {
  UErrorCode status = U_ZERO_ERROR;
  UChar rulez[] = { 0 };
  UCollator *coll = ucol_openRules(rulez, 0, UCOL_NO_NORMALIZATION, UCOL_TERTIARY, &status);

  ucol_close(coll);
}

static void TestUCAZero() {
  UErrorCode status = U_ZERO_ERROR;
  char blah[] =  "\\u9fff";
  uint8_t res[256];
  UChar b[256];
  UCollator *coll = ucol_open("", &status);

  u_unescape(blah, b, 256);
  ucol_getSortKey(coll, b, 1, res, 256);

  

  ucol_close(coll);
}


/* Pinyin tonal order */
/*
    A < .. (\u0101) < .. (\u00e1) < .. (\u01ce) < .. (\u00e0)
          (w/macron)<  (w/acute)<   (w/caron)<   (w/grave)
    E < .. (\u0113) < .. (\u00e9) < .. (\u011b) < .. (\u00e8)
    I < .. (\u012b) < .. (\u00ed) < .. (\u01d0) < .. (\u00ec)
    O < .. (\u014d) < .. (\u00f3) < .. (\u01d2) < .. (\u00f2)
    U < .. (\u016b) < .. (\u00fa) < .. (\u01d4) < .. (\u00f9) 
      < .. (\u01d6) < .. (\u01d8) < .. (\u01da) < .. (\u01dc) <
.. (\u00fc)

However, in testing we got the following order:
    A < .. (\u00e1) < .. (\u00e0) < .. (\u01ce) < .. (\u0101)
          (w/acute)<   (w/grave)<   (w/caron)<   (w/macron)
    E < .. (\u00e9) < .. (\u00e8) < .. (\u00ea) < .. (\u011b) <
.. (\u0113)
    I < .. (\u00ed) < .. (\u00ec) < .. (\u01d0) < .. (\u012b)
    O < .. (\u00f3) < .. (\u00f2) < .. (\u01d2) < .. (\u014d)
    U < .. (\u00fa) < .. (\u00f9) < .. (\u01d4) < .. (\u00fc) <
.. (\u01d8) 
      < .. (\u01dc) < .. (\u01da) < .. (\u01d6) < .. (\u016b)
*/
static void TestJ784() {
  const static char *data[] = {
      "A", "\\u0101", "\\u00e1", "\\u01ce", "\\u00e0",
      "E", "\\u0113", "\\u00e9", "\\u011b", "\\u00e8",
      "I", "\\u012b", "\\u00ed", "\\u01d0", "\\u00ec",
      "O", "\\u014d", "\\u00f3", "\\u01d2", "\\u00f2",
      "U", "\\u016b", "\\u00fa", "\\u01d4", "\\u00f9",
      "\\u00fc",
           "\\u01d6", "\\u01d8", "\\u01da", "\\u01dc"
  };
  genericLocaleStarter("zh", data, sizeof(data)/sizeof(data[0]));
}


static void TestJ831() {
  const static char *data[] = {
    "I",
      "i",
      "Y",
      "y"
  };
  genericLocaleStarter("lv", data, sizeof(data)/sizeof(data[0]));
}

static void TestJ815() {
  const static char *data[] = {
    "aa",
      "Aa",
      "ab",
      "Ab",
      "ad",
      "Ad",
      "ae",
      "Ae",
      "\\u00e6",
      "\\u00c6",
      "af",
      "Af",
      "b",
      "B"
  };
  genericLocaleStarter("fr", data, sizeof(data)/sizeof(data[0]));
}


void addMiscCollTest(TestNode** root)
{ 
    addTest(root, &TestCase, "tscoll/cmsccoll/TestCase");
    addTest(root, &IncompleteCntTest, "tscoll/cmsccoll/IncompleteCntTest");
    addTest(root, &BlackBirdTest, "tscoll/cmsccoll/BlackBirdTest");
    addTest(root, &FunkyATest, "tscoll/cmsccoll/FunkyATest");
    addTest(root, &BillFairmanTest, "tscoll/cmsccoll/BillFairmanTest");
    addTest(root, &RamsRulesTest, "tscoll/cmsccoll/RamsRulesTest");
    addTest(root, &IsTailoredTest, "tscoll/cmsccoll/IsTailoredTest");
    addTest(root, &TestCollations, "tscoll/cmsccoll/TestCollations");
    addTest(root, &TestChMove, "tscoll/cmsccoll/TestChMove");
    addTest(root, &TestImplicitTailoring, "tscoll/cmsccoll/TestImplicitTailoring");
    addTest(root, &TestFCDProblem, "tscoll/cmsccoll/TestFCDProblem");   
    addTest(root, &TestEmptyRule, "tscoll/cmsccoll/TestEmptyRule");   
    addTest(root, &TestJ784, "tscoll/cmsccoll/TestJ784");
    addTest(root, &TestJ815, "tscoll/cmsccoll/TestJ815");
    addTest(root, &TestJ831, "tscoll/cmsccoll/TestJ831");
    /*addTest(root, &TestUCAZero, "tscoll/cmsccoll/TestUCAZero");*/
    /*addTest(root, &TestUnmappedSpaces, "tscoll/cmsccoll/TestUnmappedSpaces");*/   
    /*addTest(root, &PrintMarkDavis, "tscoll/cmsccoll/PrintMarkDavis");*/
    /*addTest(root, &TestVariableTop, "tscoll/cmsccoll/TestVariableTop");*/
    addTest(root, &TestComposeDecompose, "tscoll/cmsccoll/TestComposeDecompose");
}

