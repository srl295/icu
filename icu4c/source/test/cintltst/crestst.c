/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/*******************************************************************************
*
* File CRESTST.C
*
* Modification History:
*        Name                     Description
*     Madhu Katragadda            Ported for C API
*  06/14/99     stephen           Updated for RB API changes (no suffix).
********************************************************************************
*/


#include "unicode/utypes.h"
#include "cintltst.h"
#include "unicode/ustring.h"
#include "cstring.h"

#define RESTEST_HEAP_CHECK 0

#include "unicode/uloc.h"
#include "unicode/ures.h"
#include "crestst.h"
#include "unicode/ctest.h"

#ifdef WIN32
/* Get the private functions. This is a hack! [grhoten] */
#include "locmap.c"
#endif

static void TestOpenDirect(void);
static void TestFallback(void);
static void TestLocaleStructure(void);

/*****************************************************************************/

const UChar kERROR[] = { 0x0045 /*E*/, 0x0052 /*'R'*/, 0x0052 /*'R'*/,
             0x004F /*'O'*/, 0x0052/*'R'*/, 0x0000 /*'\0'*/};

/*****************************************************************************/

enum E_Where
{
  e_Root,
  e_te,
  e_te_IN,
  e_Where_count
};
typedef enum E_Where E_Where;
/*****************************************************************************/

#define CONFIRM_EQ(actual,expected) if (u_strcmp(expected,actual)==0){ record_pass(); } else { record_fail(); log_err("%s  returned  %s  instead of %s\n", action, austrdup(actual), austrdup(expected)); }

#define CONFIRM_ErrorCode(actual,expected) if ((expected)==(actual)) { record_pass(); } else { record_fail();  log_err("%s returned  %s  instead of %s\n", action, myErrorName(actual), myErrorName(expected)); }


/* Array of our test objects */

static struct
{
  const char* name;
  UErrorCode expected_constructor_status;
  E_Where where;
  UBool like[e_Where_count];
  UBool inherits[e_Where_count];
} param[] =
{
  /* "te" means test */
  /* "IN" means inherits */
  /* "NE" or "ne" means "does not exist" */

  { "root",                U_ZERO_ERROR,             e_Root,      { TRUE, FALSE, FALSE }, { TRUE, FALSE, FALSE } },
  { "te",                  U_ZERO_ERROR,             e_te,           { FALSE, TRUE, FALSE }, { TRUE, TRUE, FALSE } },
  { "te_IN",               U_ZERO_ERROR,             e_te_IN,        { FALSE, FALSE, TRUE }, { TRUE, TRUE, TRUE } },
  { "te_NE",               U_USING_FALLBACK_ERROR,   e_te,           { FALSE, TRUE, FALSE }, { TRUE, TRUE, FALSE } },
  { "te_IN_NE",            U_USING_FALLBACK_ERROR,   e_te_IN,        { FALSE, FALSE, TRUE }, { TRUE, TRUE, TRUE } },
  { "ne",                  U_USING_DEFAULT_ERROR,    e_Root,      { TRUE, FALSE, FALSE }, { TRUE, FALSE, FALSE } }
};

static int32_t bundles_count = sizeof(param) / sizeof(param[0]);



/***************************************************************************************/

/* Array of our test objects */

void addResourceBundleTest(TestNode** root);

void addResourceBundleTest(TestNode** root)
{
    addTest(root, &TestConstruction1, "tsutil/crestst/TestConstruction1");
    addTest(root, &TestConstruction2, "tsutil/crestst/TestConstruction2");
    addTest(root, &TestOpenDirect, "tsutil/crestst/TestOpenDirect");
    addTest(root, &TestResourceBundles, "tsutil/crestst/TestResourceBundle");
    addTest(root, &TestFallback, "tsutil/crestst/TestFallback");
    addTest(root, &TestAliasConflict, "tsutil/crestst/TestAliasConflict");
    /*addTest(root, &TestLocaleStructure, "tsutil/crestst/TestLocaleStructure");*/
}


/***************************************************************************************/
void TestAliasConflict(void) {
    UErrorCode status = U_ZERO_ERROR;
    UResourceBundle *he = NULL;
    UResourceBundle *iw = NULL;
    const UChar *result = NULL;
    int32_t resultLen;

    he = ures_open(NULL, "he", &status);
    iw = ures_open(NULL, "iw", &status);
    if(U_FAILURE(status)) { 
        log_err("Failed to get resource with %s\n", myErrorName(status));
    }
    ures_close(iw);
    result = ures_getStringByKey(he, "localPatternChars", &resultLen, &status);
    if(U_FAILURE(status) || result == NULL) { 
        log_err("Failed to get resource with %s\n", myErrorName(status));
    }
    ures_close(he);
}


void TestResourceBundles()
{
    testTag("only_in_Root", TRUE, FALSE, FALSE);
    testTag("in_Root_te", TRUE, TRUE, FALSE);
    testTag("in_Root_te_te_IN", TRUE, TRUE, TRUE);
    testTag("in_Root_te_IN", TRUE, FALSE, TRUE);
    testTag("only_in_te", FALSE, TRUE, FALSE);
    testTag("only_in_te_IN", FALSE, FALSE, TRUE);
    testTag("in_te_te_IN", FALSE, TRUE, TRUE);
    testTag("nonexistent", FALSE, FALSE, FALSE);

    log_verbose("Passed:=  %d   Failed=   %d \n", pass, fail);
}

void TestConstruction1()
{
    UResourceBundle *test1 = 0, *test2 = 0;
    const UChar *result1, *result2;
    int32_t resultLen;

    UErrorCode   err = U_ZERO_ERROR;
    char testdatapath[256] ;
    const char*      locale="te_IN";

    log_verbose("Testing ures_open()......\n");
    

    loadTestData(testdatapath,256,&err);
    if(U_FAILURE(err))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(err));
        return;
    }
    
    test1=ures_open(testdatapath, NULL, &err);
    if(U_FAILURE(err))
    {
        log_err("construction of %s did not succeed :  %s \n",NULL, myErrorName(err));
        return;
    }

    
    test2=ures_open(testdatapath, locale, &err);
    if(U_FAILURE(err))
    {
        log_err("construction of %s did not succeed :  %s \n",locale, myErrorName(err));
        return;
    }
    result1= ures_getStringByKey(test1, "string_in_Root_te_te_IN", &resultLen, &err);
    result2= ures_getStringByKey(test2, "string_in_Root_te_te_IN", &resultLen, &err);
    
    
    if (U_FAILURE(err)) {
        log_err("Something threw an error in TestConstruction(): %s\n", myErrorName(err));
        return;
    }
    
    
    log_verbose("for string_in_Root_te_te_IN, default.txt had  %s\n", austrdup(result1));
    log_verbose("for string_in_Root_te_te_IN, te_IN.txt had %s\n", austrdup(result2));
    
    /* Test getVersionNumber*/
    log_verbose("Testing version number\n");
    log_verbose("for getVersionNumber :  %s\n", ures_getVersionNumber(test1));
    
    ures_close(test1);
    ures_close(test2);
}

void TestConstruction2()
{
  int n;
  int32_t resultLen;
  UChar temp[7];
  UResourceBundle *test4 = 0;
  const UChar*   result4;
  UErrorCode   err = U_ZERO_ERROR;
  const char*     directory;
  const char*    locale="te_IN";
  wchar_t widedirectory[256];
  char testdatapath[256];

  directory= u_getDataDirectory();
  uprv_strcpy(testdatapath, directory);
  uprv_strcat(testdatapath, "testdata");
  mbstowcs(widedirectory, testdatapath, 256);

  log_verbose("Testing ures_openW().......\n");

  test4=ures_openW(widedirectory, locale, &err);
  if(U_FAILURE(err)){
    log_err("Error in the construction using ures_openW():  %s\n", myErrorName(err));
    return;
  }

  result4=ures_getStringByKey(test4, "string_in_Root_te_te_IN", &resultLen, &err);

  if (U_FAILURE(err)) {
    log_err("Something threw an error in TestConstruction()  %s\n", myErrorName(err));
    return;
  }

  log_verbose("for string_in_Root_te_te_IN, te_IN.txt had  %s\n", austrdup(result4));
  u_uastrcpy(temp, "TE_IN");

  if(u_strcmp(result4, temp)!=0)
  {

    log_err("Construction test failed for ures_openW();\n");
    if(!VERBOSITY)
         log_info("(run verbose for more information)\n");

      log_verbose("\nGot->");
    for(n=0;result4[n];n++)
       {
         log_verbose("%04X ",result4[n]);
       }
    log_verbose("<\n");

    log_verbose("\nWant>");
    for(n=0;temp[n];n++)
       {
         log_verbose("%04X ",temp[n]);
       }
    log_verbose("<\n");

  }

  ures_close(test4);
}

/*****************************************************************************/
/*****************************************************************************/

UBool testTag(const char* frag,
           UBool in_Root,
           UBool in_te,
           UBool in_te_IN)
{
    int32_t passNum=pass;

    /* Make array from input params */

    UBool is_in[3];
    const char *NAME[] = { "ROOT", "TE", "TE_IN" };

    /* Now try to load the desired items */
    UResourceBundle* theBundle = NULL;
    char tag[99];
    char action[256];
    UErrorCode status = U_ZERO_ERROR,expected_resource_status = U_ZERO_ERROR;
    UChar* base = NULL;
    UChar* expected_string = NULL;
    const UChar* string = NULL;
    char item_tag[10];
    int32_t i,j;
    int32_t actual_bundle;
    int32_t resultLen;
    char testdatapath[256];
    const char *directory= u_getDataDirectory();

    uprv_strcpy(testdatapath, directory);
    uprv_strcat(testdatapath, "testdata");

    is_in[0] = in_Root;
    is_in[1] = in_te;
    is_in[2] = in_te_IN;

    strcpy(item_tag, "tag");

    status = U_ZERO_ERROR;
    theBundle = ures_open(testdatapath, "root", &status);
    if(U_FAILURE(status))
    {
        ures_close(theBundle);
        log_err("Couldn't open root bundle in %s", testdatapath);
        return FALSE;
    }
    ures_close(theBundle);
    theBundle = NULL;


    for (i=0; i<bundles_count; ++i)
    {
        strcpy(action,"construction for");
        strcat(action, param[i].name);


        status = U_ZERO_ERROR;

        theBundle = ures_open(testdatapath, param[i].name, &status);
        /*theBundle = ures_open("c:\\icu\\icu\\source\\test\\testdata\\testdata", param[i].name, &status);*/

        CONFIRM_ErrorCode(status,param[i].expected_constructor_status);



        if(i == 5)
            actual_bundle = 0; /* ne -> default */
        else if(i == 3)
            actual_bundle = 1; /* te_NE -> te */
        else if(i == 4)
            actual_bundle = 2; /* te_IN_NE -> te_IN */
        else
            actual_bundle = i;

        expected_resource_status = U_MISSING_RESOURCE_ERROR;
        for (j=e_te_IN; j>=e_Root; --j)
        {
            if (is_in[j] && param[i].inherits[j])
            {

                if(j == actual_bundle) /* it's in the same bundle OR it's a nonexistent=default bundle (5) */
                    expected_resource_status = U_ZERO_ERROR;
                else if(j == 0)
                    expected_resource_status = U_USING_DEFAULT_ERROR;
                else
                    expected_resource_status = U_USING_FALLBACK_ERROR;

                log_verbose("%s[%d]::%s: in<%d:%s> inherits<%d:%s>.  actual_bundle=%s\n",
                            param[i].name, 
                            i,
                            frag,
                            j,
                            is_in[j]?"Yes":"No",
                            j,
                            param[i].inherits[j]?"Yes":"No",
                            param[actual_bundle].name);

                break;
            }
        }

        for (j=param[i].where; j>=0; --j)
        {
            if (is_in[j])
            {
                if(base != NULL) {
                    free(base);
                    base = NULL;
                }

                base=(UChar*)malloc(sizeof(UChar)*(strlen(NAME[j]) + 1));
                u_uastrcpy(base,NAME[j]);

                break;
            }
            else {
                if(base != NULL) {
                    free(base);
                    base = NULL;
                }
                base = (UChar*) malloc(sizeof(UChar) * 1);
                *base = 0x0000;
            }
        }

        /*-------------------------------------------------------------------- */
        /* string */

        strcpy(tag,"string_");
        strcat(tag,frag);

        strcpy(action,param[i].name);
        strcat(action, ".ures_get(" );
        strcat(action,tag);
        strcat(action, ")");

        string=    kERROR;

        status = U_ZERO_ERROR;

        ures_getStringByKey(theBundle, tag, &resultLen, &status);
        if(U_SUCCESS(status))
        {
            status = U_ZERO_ERROR;
            string=ures_getStringByKey(theBundle, tag, &resultLen, &status);
        }

        log_verbose("%s got %d, expected %d\n", action, status, expected_resource_status);

        CONFIRM_ErrorCode(status, expected_resource_status);


        if(U_SUCCESS(status)){
            expected_string=(UChar*)malloc(sizeof(UChar)*(u_strlen(base) + 3));
            u_strcpy(expected_string,base);

        }
        else
        {
            expected_string = (UChar*)malloc(sizeof(UChar)*(u_strlen(kERROR) + 1));
            u_strcpy(expected_string,kERROR);

        }

        CONFIRM_EQ(string, expected_string);

        free(expected_string);
        ures_close(theBundle);
    }
    free(base);
    return (UBool)(passNum == pass);
}

void record_pass()
{
  ++pass;
}

void record_fail()
{
  ++fail;
}

/**
 * Test to make sure that the U_USING_FALLBACK_ERROR and U_USING_DEFAULT_ERROR
 * are set correctly
 */

static void TestFallback()
{
    UErrorCode status = U_ZERO_ERROR;
    UResourceBundle *fr_FR = NULL;
    const UChar *junk; /* ignored */
    int32_t resultLen;
    
    log_verbose("Opening fr_FR..");
    fr_FR = ures_open(NULL, "fr_FR", &status);
    if(U_FAILURE(status))
    {
        log_err("Couldn't open fr_FR - %d\n", status);
        return;
    }
    
    status = U_ZERO_ERROR;
    
    
    /* clear it out..  just do some calls to get the gears turning */
    junk = ures_getStringByKey(fr_FR, "LocaleID", &resultLen, &status);
    status = U_ZERO_ERROR;
    junk = ures_getStringByKey(fr_FR, "LocaleString", &resultLen, &status);
    status = U_ZERO_ERROR;
    junk = ures_getStringByKey(fr_FR, "LocaleID", &resultLen, &status);
    status = U_ZERO_ERROR;
    
    /* OK first one. This should be a Default value. */
    junk = ures_getStringByKey(fr_FR, "%%PREEURO", &resultLen, &status);
    if(status != U_USING_DEFAULT_ERROR)
    {
        log_err("Expected U_USING_DEFAULT_ERROR when trying to get %%PREEURO from fr_FR, got %s\n", 
            u_errorName(status));
    }
    
    status = U_ZERO_ERROR;
    
    /* and this is a Fallback, to fr */
    junk = ures_getStringByKey(fr_FR, "DayNames", &resultLen, &status);
    if(status != U_USING_FALLBACK_ERROR)
    {
        log_err("Expected U_USING_FALLBACK_ERROR when trying to get DayNames from fr_FR, got %s\n", 
            u_errorName(status));
    }
    
    status = U_ZERO_ERROR;
    
    ures_close(fr_FR);
}

static void
TestOpenDirect(void) {
    UResourceBundle *translit_index, *item;
    UErrorCode errorCode;

    /*
     * test that ures_openDirect() opens a resource bundle
     * where one can look up its own items but not fallback items
     * from root or similar
     */
    errorCode=U_ZERO_ERROR;
    translit_index=ures_openDirect(NULL, "translit_index", &errorCode);
    if(U_FAILURE(errorCode)) {
        log_err("ures_openDirect(\"translit_index\") failed: %s\n", u_errorName(errorCode));
        return;
    }

    if(0!=uprv_strcmp("translit_index", ures_getLocale(translit_index, &errorCode))) {
        log_err("ures_openDirect(\"translit_index\").getLocale()!=translit_index\n");
    }
    errorCode=U_ZERO_ERROR;

    /* try an item in translit_index, must work */
    item=ures_getByKey(translit_index, "RuleBasedTransliteratorIDs", NULL, &errorCode);
    if(U_FAILURE(errorCode)) {
        log_err("translit_index.getByKey(local key) failed: %s\n", u_errorName(errorCode));
        errorCode=U_ZERO_ERROR;
    } else {
        ures_close(item);
    }

    /* try an item in root, must fail */
    item=ures_getByKey(translit_index, "Languages", NULL, &errorCode);
    if(U_FAILURE(errorCode)) {
        errorCode=U_ZERO_ERROR;
    } else {
        log_err("translit_index.getByKey(root key) succeeded!\n");
        ures_close(item);
    }
    ures_close(translit_index);

    /* now make sure that "translit_index" will not work with ures_open() */
    errorCode=U_ZERO_ERROR;
    translit_index=ures_open(NULL, "translit_index", &errorCode);
    if(U_FAILURE(errorCode) || errorCode==U_USING_DEFAULT_ERROR || errorCode==U_USING_FALLBACK_ERROR) {
        /* falling back to default or root is ok */
        errorCode=U_ZERO_ERROR;
    } else {
        log_err("ures_open(\"translit_index\") succeeded, should fail!\n");
    }
    ures_close(translit_index);

    /* ures_openDirect("translit_index_WronG") must fail */
    translit_index=ures_openDirect(NULL, "translit_index_WronG", &errorCode);
    if(U_FAILURE(errorCode)) {
        errorCode=U_ZERO_ERROR;
    } else {
        log_err("ures_openDirect(\"translit_index_WronG\") succeeded, should fail!\n");
    }
    ures_close(translit_index);
}

static void
TestKeyInRootRecursive(UResourceBundle *root, UResourceBundle *currentBundle, const char *locale) {
    UErrorCode errorCode = U_ZERO_ERROR;
    UResourceBundle *subRootBundle = NULL, *subBundle = NULL;

    ures_resetIterator(root);
    ures_resetIterator(currentBundle);
    while (ures_hasNext(currentBundle)) {
        const char *subBundleKey = NULL;

        errorCode = U_ZERO_ERROR;
        subBundle = ures_getNextResource(currentBundle, NULL, &errorCode);
        if (U_FAILURE(errorCode)) {
            log_err("Can't open a resource for locale %s\n", locale);
            continue;
        }
        subBundleKey = ures_getKey(subBundle);

        subRootBundle = ures_getByKey(root, subBundleKey, NULL, &errorCode);
        if (U_FAILURE(errorCode)) {
/*            if (ures_hasNext(root)) {
                errorCode = U_ZERO_ERROR;
                subRootBundle = ures_getNextResource(root, NULL, &errorCode);
            }
            if (errorCode!=U_ZERO_ERROR) {
                if (ures_getKey(currentBundle) != 0 && strcmp(ures_getKey(currentBundle), "zoneStrings") == 0) {
                    break;
                }
                else {*/
                    if (subBundleKey == NULL
                        || (strcmp(subBundleKey, "TransliterateLATIN") != 0 /* Ignore these special cases */
                        && strcmp(subBundleKey, "BreakDictionaryData") != 0))
                    {
                        log_err("Can't open a resource with key \"%s\" in \"%s\" from root for locale \"%s\"\n",
                                subBundleKey,
                                ures_getKey(currentBundle),
                                locale);
                    }
                    continue;
/*                }
            }*/
        }
        if (ures_getType(subRootBundle) != ures_getType(subBundle)) {
            log_err("key \"%s\" in \"%s\" has a different type from root for locale \"%s\"\n"
                    "\troot=%d, locale=%d\n",
                    subBundleKey,
                    ures_getKey(currentBundle),
                    locale,
                    ures_getType(subRootBundle),
                    ures_getType(subBundle));
            continue;
        }
        else if (ures_getType(subBundle) == RES_ARRAY) {
            UResourceBundle *subSubBundle = ures_getByIndex(subBundle, 0, NULL, &errorCode);
            UResourceBundle *subSubRootBundle = ures_getByIndex(subRootBundle, 0, NULL, &errorCode);

            if (U_SUCCESS(errorCode)
                && ures_getType(subSubBundle) == RES_ARRAY || ures_getType(subSubRootBundle) == RES_ARRAY)
            {
                /* TODO: Properly check for 2D arrays and zoneStrings */
                if (subBundleKey != NULL && strcmp(subBundleKey, "zoneStrings") == 0) {
/*                    int32_t minSize = ures_getSize(subBundle);
                    int32_t idx;

                    for (idx = 0; idx < minSize; idx++) {
                        UResourceBundle *subSubBundleAtIndex = ures_getByIndex(subBundle, idx, NULL, &errorCode);
                        if (ures_getSize(subSubBundleAtIndex) != 6) {
                            log_err("zoneStrings at index %d has wrong size for locale \"%s\". array size=%d\n",
                                    idx,
                                    locale,
                                    ures_getSize(subSubBundleAtIndex));
                        }
                        ures_close(subSubBundleAtIndex);
                    }*/
                }
                else {
                    /* Here is one of the recursive parts */
                    TestKeyInRootRecursive(subRootBundle, subBundle, locale);
                }
            }
            else {
                int32_t minSize = ures_getSize(subRootBundle);
                int32_t idx;
                UBool sameArray = TRUE;

                if (minSize > ures_getSize(subBundle)) {
                    minSize = ures_getSize(subBundle);
                }

                if ((subBundleKey == NULL
                    || (subBundleKey != NULL && strcmp(subBundleKey, "LocaleScript") != 0))
                    && ures_getSize(subRootBundle) != ures_getSize(subBundle))
                {
                    log_err("Different size array with key \"%s\" in \"%s\" from root for locale \"%s\"\n"
                            "\troot array size=%d, locale array size=%d\n",
                            subBundleKey,
                            ures_getKey(currentBundle),
                            locale,
                            ures_getSize(subRootBundle),
                            ures_getSize(subBundle));
                }

                for (idx = 0; idx < minSize && sameArray; idx++) {
                    int32_t rootStrLen, localeStrLen;
                    const UChar *rootStr = ures_getStringByIndex(subRootBundle,idx,&rootStrLen,&errorCode);
                    const UChar *localeStr = ures_getStringByIndex(subBundle,idx,&localeStrLen,&errorCode);
                    if (rootStr && localeStr && U_SUCCESS(errorCode)) {
                        sameArray = (u_strcmp(rootStr, localeStr) == 0);
                    }
                    else {
                        log_err("Got a NULL string with key \"%s\" in \"%s\" at index %d for root or locale \"%s\"\n",
                                subBundleKey,
                                ures_getKey(currentBundle),
                                idx,
                                locale);
                        continue;
                    }
                    if (localeStr[0] == (UChar)0x20) {
                        log_err("key \"%s\" at index %d in \"%s\" starts with a space in locale \"%s\"\n",
                                subBundleKey,
                                idx,
                                ures_getKey(currentBundle),
                                locale);
                    }
                    else if (localeStr[localeStrLen - 1] == (UChar)0x20) {
                        log_err("key \"%s\" at index %d in \"%s\" ends with a space in locale \"%s\"\n",
                                subBundleKey,
                                idx,
                                ures_getKey(currentBundle),
                                locale);
                    }
                    else if (subBundleKey != NULL
                        && strcmp(subBundleKey, "DateTimePatterns") == 0)
                    {
                        int32_t quoted = 0;
                        const UChar *localeStrItr = localeStr;
                        while (*localeStrItr) {
                            if (*localeStrItr == (UChar)0x27 /* ' */) {
                                quoted++;
                            }
                            else if ((quoted % 2) == 0) {
                                /* Search for unquoted characters */
                                if (*localeStrItr == (UChar)0x64 /* d */
                                    && (*localeStrItr == (UChar)0x6B /* k */
                                    || *localeStrItr == (UChar)0x48 /* H */
                                    || *localeStrItr == (UChar)0x6D /* m */
                                    || *localeStrItr == (UChar)0x73 /* s */
                                    || *localeStrItr == (UChar)0x53 /* S */
                                    || *localeStrItr == (UChar)0x61 /* a */
                                    || *localeStrItr == (UChar)0x68 /* h */
                                    || *localeStrItr == (UChar)0x7A /* z */))
                                {
                                    log_err("key \"%s\" at index %d has time pattern chars in date for locale \"%s\"\n",
                                            subBundleKey,
                                            idx,
                                            locale);
                                }
                                else if (*localeStrItr == (UChar)0x6D /* m */
                                    && (*localeStrItr == (UChar)0x47 /* G */
                                    || *localeStrItr == (UChar)0x79 /* y */
                                    || *localeStrItr == (UChar)0x4D /* M */
                                    || *localeStrItr == (UChar)0x64 /* d */
                                    || *localeStrItr == (UChar)0x45 /* E */
                                    || *localeStrItr == (UChar)0x44 /* D */
                                    || *localeStrItr == (UChar)0x46 /* F */
                                    || *localeStrItr == (UChar)0x77 /* w */
                                    || *localeStrItr == (UChar)0x57 /* W */))
                                {
                                    log_err("key \"%s\" at index %d has date pattern chars in time for locale \"%s\"\n",
                                            subBundleKey,
                                            idx,
                                            locale);
                                }
                            }
                            localeStrItr++;
                        }
                    }
                }
                if (sameArray) {
                    log_err("Arrays are the same with key \"%s\" in \"%s\" from root for locale \"%s\"\n",
                            subBundleKey,
                            ures_getKey(currentBundle),
                            locale);
                }
            }
            ures_close(subSubBundle);
            ures_close(subSubRootBundle);
        }
        else if (ures_getType(subBundle) == RES_STRING) {
            int32_t len = 0;
            const UChar *string = ures_getString(subBundle, &len, &errorCode);
            if (U_FAILURE(errorCode) || string == NULL) {
                log_err("Can't open a string with key \"%s\" in \"%s\" for locale \"%s\"\n",
                        subBundleKey,
                        ures_getKey(currentBundle),
                        locale);
            } else if (string[0] == (UChar)0x20) {
                log_err("key \"%s\" in \"%s\" starts with a space in locale \"%s\"\n",
                        subBundleKey,
                        ures_getKey(currentBundle),
                        locale);
            } else if (string[len - 1] == (UChar)0x20) {
                log_err("key \"%s\" in \"%s\" ends with a space in locale \"%s\"\n",
                        subBundleKey,
                        ures_getKey(currentBundle),
                        locale);
            } else if (strcmp(subBundleKey, "localPatternChars") == 0 && len != 20) {
                log_err("key \"%s\" has the wrong number of characters in locale \"%s\"\n",
                        subBundleKey,
                        locale);
            }
            /* No fallback was done. Check for duplicate data */
            /* The ures_* API does not do fallback of sub-resource bundles,
               So we can't do this now. */
            else if (strcmp(locale, "root") != 0 && errorCode == U_ZERO_ERROR) {
            
                const UChar *rootString = ures_getString(subRootBundle, &len, &errorCode);
                if (U_FAILURE(errorCode) || rootString == NULL) {
                    log_err("Can't open a string with key \"%s\" in \"%s\" in root\n",
                            ures_getKey(subRootBundle),
                            ures_getKey(currentBundle));
                    continue;
                } else if (u_strcmp(string, rootString) == 0) {
                    log_err("Found duplicate data with key \"%s\" in \"%s\" in locale \"%s\"\n",
                            ures_getKey(subRootBundle),
                            ures_getKey(currentBundle),
                            locale);
                }
            }
        }
        else if (ures_getType(subBundle) == RES_TABLE) {
            /* Here is one of the recursive parts */
            TestKeyInRootRecursive(subRootBundle, subBundle, locale);
        }
        else if (ures_getType(subBundle) == RES_BINARY) {
            /* Can't do anything to check it */
            /* We'll assume it's all correct */
            log_verbose("Skipping key \"%s\" in \"%s\" for locale \"%s\"\n",
                    subBundleKey,
                    ures_getKey(currentBundle),
                    locale);
        }
        else {
            log_err("Type %d for key \"%s\" in \"%s\" is unknown for locale \"%s\"\n",
                    ures_getType(subBundle),
                    subBundleKey,
                    ures_getKey(currentBundle),
                    locale);
        }
        ures_close(subRootBundle);
        ures_close(subBundle);
    }
}


#ifdef WIN32

static void
testLCID(UResourceBundle *currentBundle,
         const char *localeName)
{
    UErrorCode status = U_ZERO_ERROR;
    uint32_t lcid;
    uint32_t expectedLCID;
    char lcidStringC[64] = {0};
    int32_t lcidStringLen = 0;
    const UChar *lcidString = NULL;

    lcidString = ures_getStringByKey(currentBundle, "LocaleID", &lcidStringLen, &status);

    if (U_FAILURE(status)) {
        log_err("ERROR:   %s does not have a LocaleID (%s)\n",
            localeName, u_errorName(status));
        return;
    }

    u_UCharsToChars(lcidString, lcidStringC, lcidStringLen + 1);
    expectedLCID = uprv_strtoul(lcidStringC, NULL, 16);

    lcid = T_convertToLCID(localeName, &status);
    if (U_FAILURE(status)) {
        if (expectedLCID == 0) {
            log_verbose("INFO:    %-5s does not have any LCID mapping\n",
                localeName);
        }
        else {
            log_err("ERROR:   %-5s does not have an LCID mapping to 0x%.4X\n",
                localeName, expectedLCID);
        }
        return;
    }

    status = U_ZERO_ERROR;
    uprv_strcpy(lcidStringC, T_convertToPosix(expectedLCID, &status));
    if (U_FAILURE(status)) {
        log_err("ERROR:   %.4x does not have a POSIX mapping due to %s\n",
            expectedLCID, u_errorName(status));
    }

    if(lcid != expectedLCID) {
        log_err("ERROR:   %-5s wrongfully has 0x%.4x instead of 0x%.4x for LCID\n",
            localeName, expectedLCID, lcid);
    }
    if(strcmp(localeName, lcidStringC) != 0) {
        char langName[1024];
        char langLCID[1024];
        uloc_getLanguage(localeName, langName, sizeof(langName), &status);
        uloc_getLanguage(lcidStringC, langLCID, sizeof(langLCID), &status);

        if (expectedLCID == lcid && strcmp(langName, langLCID) == 0) {
            log_verbose("WARNING: %-5s resolves to %s (0x%.4x)\n",
                localeName, lcidStringC, lcid);
        }
        else if (expectedLCID == lcid) {
            log_err("ERROR:   %-5s has 0x%.4x and the number resolves wrongfully to %s\n",
                localeName, expectedLCID, lcidStringC);
        }
        else {
            log_err("ERROR:   %-5s has 0x%.4x and the number resolves wrongfully to %s. It should be 0x%x.\n",
                localeName, expectedLCID, lcidStringC, lcid);
        }
    }
}

#endif

static void
TestLocaleStructure(void) {
    UResourceBundle *root, *currentLocale;
    int32_t locCount = uloc_countAvailable();
    int32_t locIndex;
    UErrorCode errorCode = U_ZERO_ERROR;

    /* TODO: Compare against parent's data too. This code can't handle fallbacks that some tools do already. */
/*    char locName[ULOC_FULLNAME_CAPACITY];
    char *locNamePtr;

    for (locIndex = 0; locIndex < locCount; locIndex++) {
        errorCode=U_ZERO_ERROR;
        strcpy(locName, uloc_getAvailable(locIndex));
        locNamePtr = strrchr(locName, '_');
        if (locNamePtr) {
            *locNamePtr = 0;
        }
        else {
            strcpy(locName, "root");
        }

        root = ures_openDirect(NULL, locName, &errorCode);
        if(U_FAILURE(errorCode)) {
            log_err("Can't open %s\n", locName);
            continue;
        }
*/
    root = ures_openDirect(NULL, "root", &errorCode);
    if(U_FAILURE(errorCode)) {
        log_err("Can't open root\n");
        return;
    }
    for (locIndex = 0; locIndex < locCount; locIndex++) {
        errorCode=U_ZERO_ERROR;
        currentLocale = ures_open(NULL, uloc_getAvailable(locIndex), &errorCode);
        if(errorCode != U_ZERO_ERROR) {
            if(U_SUCCESS(errorCode)) {
                if (strcmp(uloc_getAvailable(locIndex),"sv_FI_AL") != 0) {
                    /* It's installed, but there is no data.
                       It's installed for the g18n white paper [grhoten] */
                    log_err("ERROR: Locale %-5s not installed, and it should be!\n",
                        uloc_getAvailable(locIndex));
                }
            } else {
                log_err("%%%%%%% Unexpected error %d in %s %%%%%%%",
                    u_errorName(errorCode),
                    uloc_getAvailable(locIndex));
            }
            continue;
        }
        ures_getStringByKey(currentLocale, "Version", NULL, &errorCode);
        if(errorCode != U_ZERO_ERROR) {
            log_err("No version information is available for locale %s, and it should be!\n",
                uloc_getAvailable(locIndex));
        }
        else if (ures_getStringByKey(currentLocale, "Version", NULL, &errorCode)[0] == (UChar)(0x78)) {
            log_err("The locale %s is experimental! It shouldn't be listed as an installed locale.\n",
                uloc_getAvailable(locIndex));
        }
        TestKeyInRootRecursive(root, currentLocale, uloc_getAvailable(locIndex));
#ifdef WIN32
        testLCID(currentLocale, uloc_getAvailable(locIndex));
#endif
        ures_close(currentLocale);
    }
}