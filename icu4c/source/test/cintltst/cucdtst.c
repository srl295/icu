/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-1999, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File CUCDTST.C
*
* Modification History:
*        Name                     Description            
*     Madhu Katragadda            Ported for C API, added tests for string functions
*********************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "unicode/uchar.h"
#include "unicode/utypes.h"
#include "cstring.h"
#include "cintltst.h"
#include "cucdtst.h"
#include "unicode/utypes.h"
#include "unicode/ustring.h"
#include "unicode/uloc.h"


/* prototypes --------------------------------------------------------------- */

static void
TestCharNames();

static void
TestMirroring();

static void
TestUnescape();

/* test data ---------------------------------------------------------------- */
#define MIN(a,b) (a < b ? a : b)

UChar*** dataTable = 0;
const UChar  LAST_CHAR_CODE_IN_FILE = 0xFFFD;
const char tagStrings[] = "MnMcMeNdNlNoZsZlZpCcCfCsCoCnLuLlLtLmLoPcPdPsPePoSmScSkSoPiPf";
const int32_t tagValues[] =
    {
    /* Mn */ U_NON_SPACING_MARK,
    /* Mc */ U_COMBINING_SPACING_MARK,
    /* Me */ U_ENCLOSING_MARK,
    /* Nd */ U_DECIMAL_DIGIT_NUMBER,
    /* Nl */ U_LETTER_NUMBER,
    /* No */ U_OTHER_NUMBER,
    /* Zs */ U_SPACE_SEPARATOR,
    /* Zl */ U_LINE_SEPARATOR,
    /* Zp */ U_PARAGRAPH_SEPARATOR,
    /* Cc */ U_CONTROL_CHAR,
    /* Cf */ U_FORMAT_CHAR,
    /* Cs */ U_SURROGATE,
    /* Co */ U_PRIVATE_USE_CHAR,
    /* Cn */ U_UNASSIGNED,
    /* Lu */ U_UPPERCASE_LETTER,
    /* Ll */ U_LOWERCASE_LETTER,
    /* Lt */ U_TITLECASE_LETTER,
    /* Lm */ U_MODIFIER_LETTER,
    /* Lo */ U_OTHER_LETTER,
    /* Pc */ U_CONNECTOR_PUNCTUATION,
    /* Pd */ U_DASH_PUNCTUATION,
    /* Ps */ U_START_PUNCTUATION,
    /* Pe */ U_END_PUNCTUATION,
    /* Po */ U_OTHER_PUNCTUATION,
    /* Sm */ U_MATH_SYMBOL,
    /* Sc */ U_CURRENCY_SYMBOL,
    /* Sk */ U_MODIFIER_SYMBOL,
    /* So */ U_OTHER_SYMBOL,
    /* Pi */ U_INITIAL_PUNCTUATION,
    /* Pf */ U_FINAL_PUNCTUATION
    };
const char dirStrings[][5] = {
    "L",
    "R",
    "EN",
    "ES",
    "ET",   
    "AN",
    "CS",
    "B",
    "S",
    "WS",
    "ON",
    "LRE",
    "LRO",
    "AL",
    "RLE",
    "RLO",
    "PDF",
    "NSM",
    "BN"
};


void addUnicodeTest(TestNode** root)
{
    setUpDataTable();
    addTest(root, &TestUpperLower, "tsutil/cucdtst/TestUpperLower");
    addTest(root, &TestLetterNumber, "tsutil/cucdtst/TestLetterNumber");
    addTest(root, &TestMisc, "tsutil/cucdtst/TestMisc");
    addTest(root, &TestControlPrint, "tsutil/cucdtst/TestControlPrint");
    addTest(root, &TestIdentifier, "tsutil/cucdtst/TestIdentifier");
    addTest(root, &TestUnicodeData, "tsutil/cucdtst/TestUnicodeData");
    addTest(root, &TestStringFunctions, "tsutil/cucdtst/TestStringFunctions");
    addTest(root, &TestCharNames, "tsutil/cucdtst/TestCharNames");
    addTest(root, &TestMirroring, "tsutil/cucdtst/TestMirroring");
    addTest(root, &TestUnescape, "tsutil/cucdtst/TestUnescape");
}

/*==================================================== */
/* test u_toupper() and u_tolower()                    */
/*==================================================== */
void TestUpperLower()
{
    const UChar upper[] = {0x41, 0x42, 0x00b2, 0x01c4, 0x01c6, 0x01c9, 0x01c8, 0x01c9, 0x000c, 0x0000};
    const UChar lower[] = {0x61, 0x62, 0x00b2, 0x01c6, 0x01c6, 0x01c9, 0x01c9, 0x01c9, 0x000c, 0x0000};
    U_STRING_DECL(upperTest, "abcdefg123hij.?:klmno", 21);
    U_STRING_DECL(lowerTest, "ABCDEFG123HIJ.?:KLMNO", 21);
    int i;

    U_STRING_INIT(upperTest, "abcdefg123hij.?:klmno", 21);
    U_STRING_INIT(lowerTest, "ABCDEFG123HIJ.?:KLMNO", 21);

    
    for(i=0; i < u_strlen(upper); i++){
        if(u_tolower(upper[i]) != lower[i]){
            log_err("FAILED u_tolower() for %lx Expected %lx Got %lx\n", upper[i], lower[i], u_tolower(upper[i]));
        }
       
    }
    log_verbose("testing upper lower\n");
    for (i = 0; i < 21; i++) {
        
        log_verbose("testing to upper to lower\n");
        if (u_isalpha(upperTest[i]) && !u_islower(upperTest[i]))
        {
            log_err("Failed isLowerCase test at  %c\n", upperTest[i]);
        }  
        else if (u_isalpha(lowerTest[i]) && !u_isupper(lowerTest[i]))
         {
            log_err("Failed isUpperCase test at %c\n", lowerTest[i]);
        }  
        else if (upperTest[i] != u_tolower(lowerTest[i]))
        {
            log_err("Failed case conversion from %c  To %c :\n", lowerTest[i], upperTest[i]);
            
        }
        else if (lowerTest[i] != u_toupper(upperTest[i]))
         {
            log_err("Failed case conversion : %c To %c \n", upperTest[i], lowerTest[i]);
        }
        else if (upperTest[i] != u_tolower(upperTest[i]))
        {
            log_err("Failed case conversion with itself: %c\n", upperTest[i]);
        }
        else if (lowerTest[i] != u_toupper(lowerTest[i]))
        {   
            log_err("Failed case conversion with itself: %c\n", lowerTest[i]);
        }
    }
    log_verbose("done testing upper Lower\n");

}

 
/* test isLetter(u_isapha()) and isDigit(u_isdigit()) */
void TestLetterNumber()
{
    UChar i = 0x0000;

    for (i = 0x0041; i < 0x005B; i++) {
        log_verbose("Testing for isalpha\n");    
        if (!u_isalpha(i))
        {
            log_err("Failed isLetter test at  %.4X\n", i);
            
        }
    }
    for (i = 0x0660; i < 0x066A; i++) {
        log_verbose("Testing for isalpha\n"); 
        if (u_isalpha(i))
        {
            log_err("Failed isLetter test with numbers at %.4X\n", i);
            
        }
    }
    for (i = 0x0660; i < 0x066A; i++) {
        log_verbose("Testing for isdigit\n");
        if (!u_isdigit(i))
        {
            log_verbose("Failed isNumber test at %.4X\n", i);
        }
    }
    for (i = 0x0041; i < 0x005B; i++) {
        log_verbose("Testing for isalnum\n");    
        if (!u_isalnum(i))
        {
            log_err("Failed isAlNum test at  %.4X\n", i);
            
        }
    }
    for (i = 0x0660; i < 0x066A; i++) {
        log_verbose("Testing for isalnum\n");    
        if (!u_isalnum(i))
        {
            log_err("Failed isAlNum test at  %.4X\n", i);
        }
    }

}

/* Tests for isDefined(u_isdefined)(, isBaseForm(u_isbase()), isSpaceChar(u_isspace()), isWhiteSpace(), u_CharDigitValue(),u_CharCellWidth() */
void TestMisc()
{
    const UChar sampleSpaces[] = {0x0020, 0x00a0, 0x2000, 0x2001, 0x2005};
    const UChar sampleNonSpaces[] = {0x61, 0x62, 0x63, 0x64, 0x74};
    const UChar sampleUndefined[] = {0xfff1, 0xfff7, 0xfa30};
    const UChar sampleDefined[] = {0x523E, 0x4f88, 0xfffd};
    const UChar sampleBase[] = {0x0061, 0x0031, 0x03d2};
    const UChar sampleNonBase[] = {0x002B, 0x0020, 0x203B};
    const UChar sampleChars[] = {0x000a, 0x0045, 0x4e00, 0xDC00};
    const UChar sampleDigits[]= {0x0030, 0x0662, 0x0F23, 0x0ED5};
    const UChar sample2Digits[]= {0x3007, 0x4e00, 0x4e8c, 0x4e09, 0x56d8, 0x4e94, 0x516d, 0x4e03, 0x516b, 0x4e5d}; /*sp characters not in the proptable*/
    const UChar sampleNonDigits[] = {0x0010, 0x0041, 0x0122, 0x68FE};
    const UChar sampleWhiteSpaces[] = {0x2008, 0x2009, 0x200a, 0x001c, 0x000c};
    const UChar sampleNonWhiteSpaces[] = {0x61, 0x62, 0x3c, 0x28, 0x3f};
                        

    const int32_t sampleDigitValues[] = {0, 2, 3, 5};
    const int32_t sample2DigitValues[]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; /*special characters not in the properties table*/
                                         

    enum ECellWidths         /* pasted in here from unicode.h */
    {
        ZERO_WIDTH              = 0,
        HALF_WIDTH              = 1,
        FULL_WIDTH              = 2,
        NEUTRAL                 = 3
    };

    const uint16_t sampleCellWidth[] = { ZERO_WIDTH, 
                                         HALF_WIDTH, 
                                         FULL_WIDTH, 
                                         NEUTRAL};
    int i;
    char icuVersion[U_MAX_VERSION_STRING_LENGTH];
    UVersionInfo realVersion;

    memset(icuVersion, 0, U_MAX_VERSION_STRING_LENGTH);
    for (i = 0; i < 5; i++) {
      log_verbose("Testing for isspace and nonspaces\n");
        if (!(u_isspace(sampleSpaces[i])) ||
                (u_isspace(sampleNonSpaces[i])))
        {
            log_err("Space char test error : %d or %d \n", (int32_t)sampleSpaces[i], (int32_t)sampleNonSpaces[i]);
        }
    }
    for (i = 0; i < 5; i++) {
      log_verbose("Testing for isspace and nonspaces\n");
        if (!(u_isWhitespace(sampleWhiteSpaces[i])) ||
                (u_isWhitespace(sampleNonWhiteSpaces[i])))
        {
            log_err("White Space char test error : %lx or %lx \n", sampleWhiteSpaces[i], sampleNonWhiteSpaces[i]);
        }
    }
    for (i = 0; i < 3; i++) {
      log_verbose("Testing for isdefined\n");
        if ((u_isdefined(sampleUndefined[i])) ||
                !(u_isdefined(sampleDefined[i])))
        {
            log_err("Undefined char test error : %d  or  %d\n", (int32_t)sampleUndefined[i], (int32_t)sampleDefined[i]);
        }
    }
    for (i = 0; i < 3; i++) {
      log_verbose("Testing for isbase\n");
        if ((u_isbase(sampleNonBase[i])) ||
                !(u_isbase(sampleBase[i])))
        {
            log_err("Non-baseform char test error : %d or %d",(int32_t)sampleNonBase[i], (int32_t)sampleBase[i]);
        }
    }
    for (i = 0; i < 4; i++) {
      log_verbose("Testing for charcellwidth\n");
        if (u_charCellWidth(sampleChars[i]) != sampleCellWidth[i])
        {
            log_err("Cell width char test error : %d  \n", (int32_t)sampleChars[i]);
        }
    }
    for (i = 0; i < 4; i++) {
       log_verbose("Testing for isdigit \n"); 
        if ((u_isdigit(sampleDigits[i]) && 
            (u_charDigitValue(sampleDigits[i])!= sampleDigitValues[i])) ||
            (u_isdigit(sampleNonDigits[i]))) {
            log_err("Digit char test error : %lx   or   %lx\n", sampleDigits[i], sampleNonDigits[i]);
        }
    }
    for (i = 0; i < 10; i++) {
       log_verbose("Testing for u_charDigitValue for special values not existing in prop table %lx \n",  sample2Digits[i]); 
        if (u_charDigitValue(sample2Digits[i])!= sample2DigitValues[i]) 
        {
            log_err("Digit char test error : %lx\n", sample2Digits[i]);
        }
    }
    /* Tests the ICU version #*/
    u_getVersion(realVersion);
    u_versionToString(realVersion, icuVersion);
    if (strncmp(icuVersion, U_ICU_VERSION, MIN(strlen(icuVersion), strlen(U_ICU_VERSION))) != 0)
    {
        log_err("ICU version test failed. Header says=%s, got=%s \n", U_ICU_VERSION, icuVersion);
    }
#if defined(ICU_VERSION)
    /* test only happens where we have configure.in with VERSION - sanity check. */
    if(strcmp(U_ICU_VERSION, ICU_VERSION))
      {
        log_err("ICU version mismatch: Header says %s, build environment says %s.\n",  U_ICU_VERSION, ICU_VERSION);
      }
#endif

}


/* Tests for isControl(u_iscntrl()) and isPrintable(u_isprint()) */
void TestControlPrint()
{
    const UChar sampleControl[] = {0x001b, 0x0097, 0x0082};
    const UChar sampleNonControl[] = {0x61, 0x0031, 0x00e2};
    const UChar samplePrintable[] = {0x0042, 0x005f, 0x2014};
    const UChar sampleNonPrintable[] = {0x200c, 0x009f, 0x001b};
    int i;
    for (i = 0; i < 3; i++) {
        log_verbose("Testing for iscontrol\n");
        if (!u_iscntrl(sampleControl[i]))
        {
            log_err("Control char test error : %d should be control but is not\n", (int32_t)sampleControl[i]);
        }
        if (u_iscntrl(sampleNonControl[i]))
        {
            log_err("Control char test error : %d should not be control but is\n", (int32_t)sampleNonControl[i]);
        }
    }
    for (i = 0; i < 3; i++) {
        log_verbose("testing for isprintable\n");
        if (!u_isprint(samplePrintable[i]))
        {
            log_err("Printable char test error : %d should be printable but is not\n", (int32_t)samplePrintable[i]);
        }
        if (u_isprint(sampleNonPrintable[i]))
        {
            log_err("Printable char test error : %d should not be printable but is\n", (int32_t)sampleNonPrintable[i]);
        }
    }
}
/* u_isJavaIDStart, u_isJavaIDPart, u_isIDStart(), u_isIDPart(), u_isIDIgnorable()*/
void TestIdentifier()
{
    const UChar sampleJavaIDStart[] = {0x0071, 0x00e4, 0x005f};
    const UChar sampleNonJavaIDStart[] = {0x0020, 0x2030, 0x0082};
    const UChar sampleJavaIDPart[] = {0x005f, 0x0032, 0x0045};
    const UChar sampleNonJavaIDPart[] = {0x2030, 0x2020, 0x0020};
    const UChar sampleUnicodeIDStart[] = {0x0250, 0x00e2, 0x0061};
    const UChar sampleNonUnicodeIDStart[] = {0x2000, 0x000a, 0x2019};
    const UChar sampleUnicodeIDPart[] = {0x005f, 0x0032, 0x0045};
    const UChar sampleNonUnicodeIDPart[] = {0x2030, 0x00a3, 0x0020};
    const UChar sampleIDIgnore[] = {0x0006, 0x0010, 0x206b};
    const UChar sampleNonIDIgnore[] = {0x0075, 0x00a3, 0x0061};

    int i;
    for (i = 0; i < 3; i++) {
        log_verbose("Testing sampleJavaID start \n");
        if (!(u_isJavaIDStart(sampleJavaIDStart[i])) ||
                (u_isJavaIDStart(sampleNonJavaIDStart[i])))
            log_err("Java ID Start char test error : %lx or %lx\n",
            sampleJavaIDStart[i], sampleNonJavaIDStart[i]);
    }
    for (i = 0; i < 3; i++) {
        log_verbose("Testing sampleJavaID part \n");
        if (!(u_isJavaIDPart(sampleJavaIDPart[i])) ||
                (u_isJavaIDPart(sampleNonJavaIDPart[i])))
            log_err("Java ID Part char test error : %lx or %lx\n",
             sampleJavaIDPart[i], sampleNonJavaIDPart[i]);
    }
    for (i = 0; i < 3; i++) {
        log_verbose("Testing sampleUnicodeID start \n");
        /* T_test_logln_ustr((int32_t)i); */
        if (!(u_isIDStart(sampleUnicodeIDStart[i])) ||
                (u_isIDStart(sampleNonUnicodeIDStart[i])))
        {
            log_err("Unicode ID Start char test error : %lx  or  %lx\n", sampleUnicodeIDStart[i],
                                    sampleNonUnicodeIDStart[i]);
        }
    }
    for (i = 2; i < 3; i++) {   /* nos *** starts with 2 instead of 0, until clarified */
        log_verbose("Testing sample unicode ID part \n");
        /* T_test_logln_ustr((int32_t)i); */
        if (!(u_isIDPart(sampleUnicodeIDPart[i])) ||
                (u_isIDPart(sampleNonUnicodeIDPart[i])))
           {
            log_err("Unicode ID Part char test error : %lx  or  %lx", sampleUnicodeIDPart[i], sampleNonUnicodeIDPart[i]);
            }
    }
    for (i = 0; i < 3; i++) {
        log_verbose("Testing  sampleId ignore\n");
        /*T_test_logln_ustr((int32_t)i); */
        if (!(u_isIDIgnorable(sampleIDIgnore[i])) ||
                (u_isIDIgnorable(sampleNonIDIgnore[i])))
        {
            log_verbose("ID ignorable char test error : %d  or  %d\n", sampleIDIgnore[i], sampleNonIDIgnore[i]);
        }
    }
}

/* tests for u_charType(), u_isTitle(), and u_toTitle(),u_charDirection and u_charScript()*/
void TestUnicodeData()
{
    FILE*   input = 0;
    char    buffer[1000];
    char*   bufferPtr = 0, *dirPtr = 0;
    int32_t unicode;
    int8_t  type;
    char newPath[256];
    const char *expectVersion = U_UNICODE_VERSION;  /* NOTE: this purposely breaks to force the tests to stay in sync with the unicodedata */
    /* expectVersionArray must be filled from u_versionFromString(expectVersionArray, U_UNICODE_VERSION)
       once this function is public. */
    UVersionInfo expectVersionArray;
    UVersionInfo versionArray;
    char expectString[256];

    strcpy(newPath, u_getDataDirectory());
    strcat(newPath, "UnicodeData-");
    strcat(newPath, expectVersion);
    strcat(newPath, ".txt");

    u_versionFromString(expectVersionArray, expectVersion);
    strcpy(expectString, "Unicode Version ");
    strcat(expectString, expectVersion);
    u_getUnicodeVersion(versionArray);

    if(memcmp(versionArray, expectVersionArray, U_MAX_VERSION_LENGTH) != 0)
      {
        log_err("Testing u_getUnicodeVersion() - expected %s got %d.%d.%d.%d\n", expectString, 
        versionArray[0], versionArray[1], versionArray[2], versionArray[3]);
      }

#if defined(ICU_UNICODE_VERSION)
    /* test only happens where we have configure.in with UNICODE_VERSION - sanity check. */
    if(strcmp(expectVersion, ICU_UNICODE_VERSION))
      {
         log_err("Testing configure.in's UNICODE_VERSION - expected %s got %s\n",  expectVersion, ICU_UNICODE_VERSION);
      }
#endif

    input = fopen(newPath, "r");

    if (input == 0) {
      log_err("Unicode C API test 'fopen' (%s) error\n", newPath);
      return;
    }


        for(;;) {
            bufferPtr = fgets(buffer, 999, input);
            if (bufferPtr == NULL) break;
            if (bufferPtr[0] == '#' || bufferPtr[0] == '\n' || bufferPtr[0] == 0) continue;
            sscanf(bufferPtr, "%X", &unicode);
            if (!(0 <= unicode && unicode < 65536)) {
                log_err("Unicode C API test precondition '(0 <= unicode && unicode < 65536)' failed\n");
                return;
            }
            if (unicode == LAST_CHAR_CODE_IN_FILE)
                break;
            bufferPtr = strchr(bufferPtr, ';');
            if (!(bufferPtr != NULL)) {
                log_err("Unicode C API test condition '(bufferPtr != NULL)' failed\n");
                return;
            }
            bufferPtr = strchr(bufferPtr + 1, ';'); /* go to start of third field */
            if (!(bufferPtr != NULL)) {
                log_err("Unicode C API test condition '(bufferPtr != NULL)' failed\n");
                return;
            }
            dirPtr = bufferPtr;
            dirPtr = strchr(dirPtr + 1, ';');
            if (!(dirPtr != NULL)) {
                log_err("Unicode C API test precondition '(dirPtr != NULL)' failed\n");
                return;
            }
            dirPtr = strchr(dirPtr + 1, ';');
            if (!(dirPtr != NULL)) {
                log_err("Unicode C API test precondition '(dirPtr != NULL)' failed\n");
                return;
            }
            bufferPtr++;
            bufferPtr[2] = 0;

            /* we override the general category of some control characters */
            switch(unicode) {
            case 9:
            case 0xb:
            case 0x1f:
                type = U_SPACE_SEPARATOR;
                break;
            case 0xc:
                type = U_LINE_SEPARATOR;
                break;
            case 0xa:
            case 0xd:
            case 0x1c:
            case 0x1d:
            case 0x1e:
            case 0x85:
                type = U_PARAGRAPH_SEPARATOR;
                break;
            default:
                type = (int8_t)tagValues[MakeProp(bufferPtr)];
                break;
            }
            if (u_charType((UChar)unicode) != type)
            {
                log_err("Unicode character type failed at %d\n",unicode);
            }

            /* test title case */
            if ((u_totitle((UChar)unicode) != u_toupper((UChar)unicode)) &&
                !(u_istitle(u_totitle((UChar)unicode))))
            {
                log_err("Title case test failed at %d \n", unicode);
            }
            bufferPtr = strchr(dirPtr + 1, ';');
            dirPtr++;
            bufferPtr[0] = 0;
            if (u_charDirection((UChar)unicode) != MakeDir(dirPtr)) 
            {
                log_err("Unicode character directionality failed at %d\n", unicode);
                
            }
            
        }

        if (u_charScript((UChar)0x0041 != U_BASIC_LATIN)) {
            log_err("Unicode character script property failed !\n");
        }
        if (input) 
            fclose(input);


}
/*internal functions ----*/
int32_t MakeProp(char* str) 
{
    int32_t result = 0;
    char* matchPosition =0;
    
    matchPosition = strstr(tagStrings, str);
    if (matchPosition == 0) 
    {
        log_err("unrecognized type letter ");
        log_err(str);
    }
    else result = ((matchPosition - tagStrings) / 2);
    return result;
}

int32_t MakeDir(char* str) 
{
    int32_t pos = 0;
    for (pos = 0; pos < 19; pos++) {
        if (strcmp(str, dirStrings[pos]) == 0) {
            return pos;
        }
    }
    return -1;
}
/*----------------*/


static const char* raw[3][4] = {
   
    /* First String */
    {   "English_",  "French_",   "Croatian_", "English_"},
    /* Second String */
    {   "United States",    "France",   "Croatia",  "Unites States"},
   
   /* Concatenated string */
    {   "English_United States", "French_France", "Croatian_Croatia", "English_United States"}
};
   
void setUpDataTable()
{
    int32_t i,j;
    if(dataTable == NULL) {
        dataTable = (UChar***)calloc(sizeof(UChar**),3);

            for (i = 0; i < 3; i++) {
              dataTable[i] = (UChar**)calloc(sizeof(UChar*),4);
                for (j = 0; j < 4; j++){
                    dataTable[i][j] = (UChar*) malloc(sizeof(UChar)*(strlen(raw[i][j])+1));
                    u_uastrcpy(dataTable[i][j],raw[i][j]);
                }
            }
    }
    
}

U_CFUNC void cleanUpDataTable()
{
    int32_t i,j;
    if(dataTable != NULL) {
        for (i=0; i<3; i++) {
            for(j = 0; j<4; j++) {
                free(dataTable[i][j]);
            }
            free(dataTable[i]);
        }
        free(dataTable);
    }
    dataTable = NULL;
}

/*Tests  for u_strcat(),u_strcmp(), u_strlen(), u_strcpy(),u_strncat(),u_strncmp(),u_strncpy, u_uastrcpy(),u_austrcpy(), u_uastrncpy(); */
void TestStringFunctions()
{
   
    int32_t i,j,k;
    UChar temp[40];
    char test[40];
    log_verbose("Testing u_strlen()\n");
    if( u_strlen(dataTable[0][0])!= u_strlen(dataTable[0][3]) || u_strlen(dataTable[0][0]) == u_strlen(dataTable[0][2]))
        log_err("There is an error in u_strlen()");

    log_verbose("Testing u_strcpy() and u_strcmp)\n");
      
    for(i=0;i<3;++i){
        for(j=0;j<4;++j)
        {
        log_verbose("Testing  %s  \n", austrdup(dataTable[i][j]));
        u_uastrcpy(temp, "");
        u_strcpy(temp,dataTable[i][j]);
        
        if(u_strcmp(temp,dataTable[i][j])!=0)
        log_err("something threw an error in u_strcpy() or u_strcmp()\n");
        }
    }
    
    log_verbose("testing u_strcat()\n");
    i=0;
    for(j=0; j<2;++j)
    {
        u_uastrcpy(temp, "");
        u_strcpy(temp,dataTable[i][j]);
        u_strcat(temp,dataTable[i+1][j]);
        if(u_strcmp(temp,dataTable[i+2][j])!=0)
            log_err("something threw an error in u_strcat()\n");
    
    }
    log_verbose("Testing u_strncmp()\n");
    for(i=0,j=0;j<4; ++j)
    {
        k=u_strlen(dataTable[i][j]);
        if(u_strncmp(dataTable[i][j],dataTable[i+2][j],k)!=0)
            log_err("Something threw an error in u_strncmp\n");
    }
    
    
    log_verbose("Testing u_strncat \n");
    for(i=0,j=0;j<4; ++j)
    {    
        k=u_strlen(dataTable[i][j]);
        
        u_uastrcpy(temp,"");
        
        if(u_strcmp(u_strncat(temp,dataTable[i+2][j],k),dataTable[i][j])!=0)
            log_err("something threw an error in u_strncat or u_uastrcpy()\n");
        
    }

    log_verbose("Testing u_strncpy()\n");
    for(i=0,j=0;j<4; ++j)
    {
        k=u_strlen(dataTable[i][j]);
        u_uastrcpy(temp,"");
        u_strncpy(temp,dataTable[i+2][j],k);
        
        if(u_strcmp(temp,dataTable[i][j])!=0)
            log_err("something threw an error in u_strncpy()\n");
    }
    
    log_verbose("Testing if u_strchr() works fine\n");
    
    for(i=2,j=0;j<4;j++)
    {
        UChar *findPtr = u_strchr(dataTable[i][j],'_');
        log_verbose("%s ", austrdup(findPtr));

        if (findPtr == NULL || *findPtr != '_') {
            log_err("strchr can't find '_' in the string\n");
        }

        findPtr = u_strchr(dataTable[i][j], 0);
        if (findPtr != (&(dataTable[i][j][u_strlen(dataTable[i][j])]))) {
            log_err("strchr can't find NULL in the string\n");
        }
    }


    log_verbose("Testing u_austrcpy()");
    u_austrcpy(test,dataTable[0][0]);
    if(strcmp(test,raw[0][0])!=0)
        log_err("There is an error in u_austrcpy()");


    log_verbose("Testing u_uastrncpy() and u_uastrcpy()");
    {
    UChar *result=0;
    UChar subString[5];
    UChar uchars[]={0x61, 0x62, 0x63, 0x00};
    u_uastrcpy(temp, "abc");
    if(u_strcmp(temp, uchars) != 0){
        log_err("There is an error in u_uastrcpy() Expected %s Got %s\n", austrdup(uchars), austrdup(temp));
    }

    temp[0] = 0xFB; /* load garbage into it */
    temp[1] = 0xFB;
    temp[2] = 0xFB;
    temp[3] = 0xFB;

    u_uastrncpy(temp, "abcabcabc", 3);
    if(u_strncmp(uchars, temp, 3) != 0){
        log_err("There is an error in u_uastrncpy() Expected %s Got %s\n", austrdup(uchars), austrdup(temp));
    }
    if(temp[3] != 0xFB) {
      log_err("u_austrncpy wrote past it's bounds. Expected undisturbed byte at 3\n");
    }
    /*Testing u_strchr()*/
    log_verbose("Testing u_strchr\n");
    temp[0]=0x42;
    temp[1]=0x62;
    temp[2]=0x62;
    temp[3]=0x63;
    temp[4]=0xd841;
    temp[5]=0xd841;
    temp[6]=0xdc02;
    temp[7]=0;
    result=u_strchr(temp, (UChar)0x62);
    if(result != temp+1){
        log_err("There is an error in u_strchr() Expected match at position 1 Got %ld (pointer 0x%lx)\n", result-temp, result);
    }
    /*Testing u_strstr()*/
    log_verbose("Testing u_strstr\n");
    subString[0]=0x62;
    subString[1]=0x63;
    subString[2]=0;
    result=u_strstr(temp, subString);
    if(result != temp+2){
        log_err("There is an error in u_strstr() Expected match at position 2 Got %ld (pointer 0x%lx)\n", result-temp, result);
    }
    result=u_strstr(temp, subString+2); /* subString+2 is an empty string */
    if(result != temp){
        log_err("There is an error in u_strstr() Expected match at position 0 Got %ld (pointer 0x%lx)\n", result-temp, result);
    }
    result=u_strstr(subString, temp);
    if(result != NULL){
        log_err("There is an error in u_strstr() Expected NULL \"not found\" Got non-NULL \"found\" result\n");
    }
    
    /*Testing u_strchr32*/
    log_verbose("Testing u_strchr32\n");
    result=u_strchr32(temp, (UChar32)0x62);
    if(result != temp+1){
        log_err("There is an error in u_strchr32() Expected match at position 1 Got %ld (pointer 0x%lx)\n", result-temp, result);
    }
    result=u_strchr32(temp, (UChar32)0xfb);
    if(result != NULL){
        log_err("There is an error in u_strchr32() Expected NULL \"not found\" Got non-NULL \"found\" result\n");
    }
    result=u_strchr32(temp, (UChar32)0x20402);
    if(result != temp+5){
        log_err("There is an error in u_strchr32() Expected match at position 5 Got %ld (pointer 0x%lx)\n", result-temp, result);
    }
   
  }
}

/* test u_charName() -------------------------------------------------------- */

static const struct {
    uint32_t code;
    const char *name, *oldName;
} names[]={
    0x0061, "LATIN SMALL LETTER A", "",
    0x0284, "LATIN SMALL LETTER DOTLESS J WITH STROKE AND HOOK", "LATIN SMALL LETTER DOTLESS J BAR HOOK",
    0x3401, "CJK UNIFIED IDEOGRAPH-3401", "",
    0x7fed, "CJK UNIFIED IDEOGRAPH-7FED", "",
    0xac00, "HANGUL SYLLABLE GA", "",
    0xd7a3, "HANGUL SYLLABLE HIH", "",
    0xff08, "FULLWIDTH LEFT PARENTHESIS", "FULLWIDTH OPENING PARENTHESIS",
    0xffe5, "FULLWIDTH YEN SIGN", ""
};

static UBool
enumCharNamesFn(void *context,
                UChar32 code, UCharNameChoice nameChoice,
                const char *name, UTextOffset length) {
    UTextOffset *pCount=(UTextOffset *)context;
    int i;

    if(length<=0 || length!=(UTextOffset)uprv_strlen(name)) {
        /* should not be called with an empty string or invalid length */
        log_err("u_enumCharName(0x%lx)=%s but length=%ld\n", name, length);
        return TRUE;
    }

    ++*pCount;
    for(i=0; i<sizeof(names)/sizeof(names[0]); ++i) {
        if(code==names[i].code) {
            if(nameChoice==U_UNICODE_CHAR_NAME) {
                if(0!=uprv_strcmp(name, names[i].name)) {
                    log_err("u_enumCharName(0x%lx)=%s instead of %s\n", code, name, names[i].name);
                }
            } else {
                if(names[i].oldName[0]==0 || 0!=uprv_strcmp(name, names[i].oldName)) {
                    log_err("u_enumCharName(0x%lx - 1.0)=%s instead of %s\n", code, name, names[i].oldName);
                }
            }
            break;
        }
    }
    return TRUE;
}

static void
TestCharNames() {
    static char name[80];
    UErrorCode errorCode=U_ZERO_ERROR;
    UTextOffset length;
    UChar32 c;
    int i;

    log_verbose("Testing u_charName()\n");
    for(i=0; i<sizeof(names)/sizeof(names[0]); ++i) {
        /* modern Unicode character name */
        length=u_charName(names[i].code, U_UNICODE_CHAR_NAME, name, sizeof(name), &errorCode);
        if(U_FAILURE(errorCode)) {
            log_err("u_charName(0x%lx) error %s\n", names[i].code, u_errorName(errorCode));
            return;
        }
        if(length<=0 || 0!=uprv_strcmp(name, names[i].name) || length!=(uint16_t)uprv_strlen(name)) {
            log_err("u_charName(0x%lx) gets %s length %ld instead of %s\n", names[i].code, name, length, names[i].name);
        }

        /* find the modern name */
        c=u_charFromName(U_UNICODE_CHAR_NAME, names[i].name, &errorCode);
        if(U_FAILURE(errorCode)) {
            log_err("u_charFromName(%s) error %s\n", names[i].name, u_errorName(errorCode));
            return;
        }
        if(c!=names[i].code) {
            log_err("u_charFromName(%s) gets 0x%lx instead of 0x%lx\n", names[i].name, c, names[i].code);
        }

        /* Unicode 1.0 character name */
        length=u_charName(names[i].code, U_UNICODE_10_CHAR_NAME, name, sizeof(name), &errorCode);
        if(U_FAILURE(errorCode)) {
            log_err("u_charName(0x%lx - 1.0) error %s\n", names[i].code, u_errorName(errorCode));
            return;
        }
        if(length<0 || length>0 && 0!=uprv_strcmp(name, names[i].oldName) || length!=(uint16_t)uprv_strlen(name)) {
            log_err("u_charName(0x%lx - 1.0) gets %s length %ld instead of nothing or %s\n", names[i].code, name, length, names[i].oldName);
        }

        /* find the Unicode 1.0 name if it is stored (length>0 means that we could read it) */
        if(names[i].oldName[0]!=0 && length>0) {
            c=u_charFromName(U_UNICODE_10_CHAR_NAME, names[i].oldName, &errorCode);
            if(U_FAILURE(errorCode)) {
                log_err("u_charFromName(%s - 1.0) error %s\n", names[i].oldName, u_errorName(errorCode));
                return;
            }
            if(c!=names[i].code) {
                log_err("u_charFromName(%s - 1.0) gets 0x%lx instead of 0x%lx\n", names[i].oldName, c, names[i].code);
            }
        }
    }

    /* test u_enumCharNames() */
    length=0;
    errorCode=U_ZERO_ERROR;
    u_enumCharNames(0, 0x110000, enumCharNamesFn, &length, U_UNICODE_CHAR_NAME, &errorCode);
    if(U_FAILURE(errorCode) || length<49194) {
        log_err("u_enumCharNames(0..0x1100000) error %s names count=%ld\n", u_errorName(errorCode), length);
    }

    /* ### TODO: test error cases and other interesting things */
}

/* test u_isMirrored() and u_charMirror() ----------------------------------- */

static void
TestMirroring() {
    log_verbose("Testing u_isMirrored()\n");
    if(!(u_isMirrored(0x28) && u_isMirrored(0xbb) && u_isMirrored(0x2045) && u_isMirrored(0x232a) &&
         !u_isMirrored(0x27) && !u_isMirrored(0x61) && !u_isMirrored(0x284) && !u_isMirrored(0x3400)
        )
    ) {
        log_err("u_isMirrored() does not work correctly\n");
    }

    log_verbose("Testing u_charMirror()\n");
    if(!(u_charMirror(0x3c)==0x3e && u_charMirror(0x5d)==0x5b && u_charMirror(0x208d)==0x208e && u_charMirror(0x3017)==0x3016 &&
         u_charMirror(0x2e)==0x2e && u_charMirror(0x6f3)==0x6f3 && u_charMirror(0x301c)==0x301c && u_charMirror(0xa4ab)==0xa4ab 
         )
    ) {
        log_err("u_charMirror() does not work correctly\n");
    }
}

/* test u_unescape() and u_unescapeAt() ------------------------------------- */

static void
TestUnescape() {
    static UChar buffer[200];
    static const UChar expect[]={
        0x53, 0x63, 0x68, 0xf6, 0x6e, 0x65, 0x73, 0x20, 0x41, 0x75, 0x74, 0x6f, 0x3a, 0x20,
        0x20ac, 0x20, 0x31, 0x31, 0x32, 0x34, 0x30, 0x2e, 0x0c,
        0x50, 0x72, 0x69, 0x76, 0x61, 0x74, 0x65, 0x73, 0x20,
        0x5a, 0x65, 0x69, 0x63, 0x68, 0x65, 0x6e, 0x3a, 0x20, 0xdbc8, 0xdf45, 0x0a, 0
    };
    int32_t length;

    /* test u_unescape() */
    length=u_unescape(
        "Sch\\u00f6nes Auto: \\u20ac 11240.\\fPrivates Zeichen: \\U00102345\\n",
        buffer, sizeof(buffer)/sizeof(buffer[0]));
    if(length!=45 || u_strcmp(buffer, expect)!=0) {
        log_err("failure in u_unescape(): length %d!=45 and/or incorrect result string\n", length);
    }

    /* try preflighting */
    length=u_unescape(
        "Sch\\u00f6nes Auto: \\u20ac 11240.\\fPrivates Zeichen: \\U00102345\\n",
        NULL, sizeof(buffer)/sizeof(buffer[0]));
    if(length!=45 || u_strcmp(buffer, expect)!=0) {
        log_err("failure in u_unescape(preflighting): length %d!=45\n", length);
    }

    /* ### TODO: test u_unescapeAt() */
}
