/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/*
* File stdnmtst.c
*
* Modification History:
*
*   Date          Name        Description
*   08/05/2000    Yves       Creation 
*******************************************************************************
*/

#include "unicode/ucnv.h"
#include "cstring.h"
#include "cintltst.h"

static void TestStandardNames(void);

void addStandardNamesTest(TestNode** root);


void
addStandardNamesTest(TestNode** root)
{
  addTest(root, &TestStandardNames,    "stdnmtst/TestStandardNames");
}

static int dotestname(const char *name, const char *standard, const char *expected) {
    int res = 1;

    UErrorCode error;
    const char *tag;

    error = U_ZERO_ERROR;
    tag = ucnv_getStandardName(name, standard, &error);
    if (!tag) {
        log_err("FAIL: could not find %s standard name for %s\n", standard, name);
        res = 0;
    } else if (expected && uprv_stricmp(expected, tag)) {
        log_err("FAIL: expected %s for %s standard name for %s, got %s\n", expected, standard, name, tag);
        res = 0;
    }

    return res;
}

static void TestStandardNames()
{
    int res = 1;

    uint16_t i, count;
    UErrorCode err;

    /* Iterate over all standards. */

    for (i = 0, count = ucnv_countStandards(); i < count; ++i) {
        const char *std;

        err = U_ZERO_ERROR;
        std = ucnv_getStandard(i, &err);
        if (U_FAILURE(err)) {
            log_err("FAIL: ucnv_getStandard(%d), error=%s\n", i, u_errorName(err));
            res = 0;
        } else if (!std || !*std) {
            log_err("FAIL: %s standard name at index %d\n", (std ? "empty" :
                "null"), i);
            res = 0;
        }
    }
    err = U_ZERO_ERROR;
    if (ucnv_getStandard(i, &err)) {
        log_err("FAIL: ucnv_getStandard(%d) should return NULL\n", i);
        res = 0;
    }

    if (res) {
        log_verbose("PASS: iterating over standard names works\n");
    }

    /* Test for some expected results. */

    if (dotestname("ibm-1208", "MIME", "utf-8") &&
        /*dotestname("cp1252", "MIME", "windows-1252") &&*/
        dotestname("ascii", "MIME", "us-ascii") &&
        dotestname("ascii", "IANA", "ANSI_X3.4-1968") &&
        dotestname("cp850", "IANA", "IBM850")) {
        log_verbose("PASS: getting IANA and MIME stadard names works\n");
    }
}

