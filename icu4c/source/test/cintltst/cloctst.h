/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2003, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File CLOCTST.H
*
* Modification History:
*        Name                     Description
*     Madhu Katragadda            Converted to C
*********************************************************************************
*/
#ifndef _CLOCTEST
#define _CLOCTEST

#include "cintltst.h"
/*C API TEST FOR LOCALE */

/**
 * Test functions to set and get data fields
 **/
static void TestBasicGetters(void);
static void TestPrefixes(void);
/**
 * Use Locale to access Resource file data and compare against expected values
 **/
static void TestSimpleResourceInfo(void);
/**
 * Use Locale to access Resource file display names and compare against expected values
 **/
static  void TestDisplayNames(void);
/**
 * Test getAvailableLocales
 **/
 static  void TestGetAvailableLocales(void);
/**
 * Test functions to set and access a custom data directory
 **/
 static void TestDataDirectory(void);
/**
 * Test functions to test get ISO countries and Languages
 **/
 static void TestISOFunctions(void);
/**
 * Test functions to test get ISO3 countries and Languages Fallback
 **/
 static void TestISO3Fallback(void);
/**
 * Test functions to test get ISO3 countries and Languages for Uninstalled locales
 **/
 static void TestUninstalledISO3Names(void);
 static void TestObsoleteNames(void);
/**
 * Test functions uloc_getDisplaynames()
 **/
 static void TestSimpleDisplayNames(void);
/**
 * Test functions uloc_getDisplaynames()
 **/
 static void TestVariantParsing(void);

 /* Make sure that the locale data is good. */
 static void TestLocaleStructure(void);

 /* Make sure that Country information is the same across locales, within reason. */
 static void TestConsistentCountryInfo(void);

 /* Make sure we can pass "de_DE@Collation=PHONEBOOK" */
 static void MoreVariants(void);
 
 /* Test getting keyword enumeratin */
 static void TestKeywordVariants(void);

 /* Test getting keyword values */
 static void TestKeywordVariantParsing(void);

/**
 * routine to perform subtests, used by TestDisplayNames
 */
 static void doTestDisplayNames(const char* inLocale, int32_t compareIndex);

static void TestCanonicalization(void);

/**
 * additional intialization for datatables storing expected values
 */
static void setUpDataTable(void);
static void cleanUpDataTable(void);
void displayDataTable(void);

#endif
