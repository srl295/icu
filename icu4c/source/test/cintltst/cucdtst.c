/*
*****************************************************************************************
*                                                                                       *
* COPYRIGHT:                                                                            *
*   (C) Copyright Taligent, Inc.,  1996                                                 *
*   (C) Copyright International Business Machines Corporation,  1999                    *
*   Licensed Material - Program-Property of IBM - All Rights Reserved.                  *
*   US Government Users Restricted Rights - Use, duplication, or disclosure             *
*   restricted by GSA ADP Schedule Contract with IBM Corp.                              *
*                                                                                       *
*****************************************************************************************
********************************************************************************
*
* File CUCDTST.C
*
* Modification History:
*        Name                     Description            
*     Madhu Katragadda            Ported for CAPI, added tests for string functions
*********************************************************************************
*/
#include "uchar.h"
#include "utypes.h"
#include "cintltst.h"
#include "cucdtst.h"
#include "utypes.h"
#include "ustring.h"
#include "uloc.h"

#include <stdio.h>
#include<string.h>
UChar*** dataTable = 0;
const UChar  LAST_CHAR_CODE_IN_FILE = 0xFFFD;
const char tagStrings[] = "MnMcMeNdNlNoZsZlZpCcCfCsCoCnLuLlLtLmLoPcPdPsPePoSmScSkSoPiPf";
const int32_t tagValues[] =
    {
    /* Mn */ NON_SPACING_MARK,
	/* Mc */ COMBINING_SPACING_MARK,
	/* Me */ ENCLOSING_MARK,
	/* Nd */ DECIMAL_DIGIT_NUMBER,
	/* Nl */ LETTER_NUMBER,
	/* No */ OTHER_NUMBER,
	/* Zs */ SPACE_SEPARATOR,
	/* Zl */ LINE_SEPARATOR,
	/* Zp */ PARAGRAPH_SEPARATOR,
	/* Cc */ CONTROL,
	/* Cf */ FORMAT,
	/* Cs */ SURROGATE,
	/* Co */ PRIVATE_USE,
	/* Cn */ UNASSIGNED,
	/* Lu */ UPPERCASE_LETTER,
	/* Ll */ LOWERCASE_LETTER,
	/* Lt */ TITLECASE_LETTER,
	/* Lm */ MODIFIER_LETTER,
	/* Lo */ OTHER_LETTER,
	/* Pc */ CONNECTOR_PUNCTUATION,
	/* Pd */ DASH_PUNCTUATION,
	/* Ps */ START_PUNCTUATION,
	/* Pe */ END_PUNCTUATION,
	/* Po */ OTHER_PUNCTUATION,
	/* Sm */ MATH_SYMBOL,
	/* Sc */ CURRENCY_SYMBOL,
	/* Sk */ MODIFIER_SYMBOL,
	/* So */ OTHER_SYMBOL,
	/* Pi */ INITIAL_PUNCTUATION,
	/* Pf */ FINAL_PUNCTUATION
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

}
/*==================================================== */
/* test u_toupper() and u_tolower()                    */
/*==================================================== */
void TestUpperLower()
{
    static char* upperTest = "abcdefg123hij.?:klmno";
    static char* lowerTest = "ABCDEFG123HIJ.?:KLMNO";
    int i;
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
}

/* Tests for isDefined(u_isdefined)(, isBaseForm(u_isbase()), isSpaceChar(u_isspace()),u_CharDigitValue(),u_CharCellWidth() */
void TestMisc()
{
    const UChar sampleSpaces[] = {0x0020, 0x00a0, 0x2000, 0x2001, 0x2005};
    const UChar sampleNonSpaces[] = {'a', 'b', 'c', 'd', 't'};
    const UChar sampleUndefined[] = {0xfff1, 0xfff7, 0xfa30};
    const UChar sampleDefined[] = {0x523E, 0x4f88, 0xfffd};
    const UChar sampleBase[] = {0x0061, 0x0031, 0x03d2};
    const UChar sampleNonBase[] = {0x002B, 0x0020, 0x203B};
    const UChar sampleChars[] = {0x000a, 0x0045, 0x4e00, 0xDC00};
    const UChar sampleDigits[]= {0x0030, 0x0662, 0x0F23, 0x0ED5};
    const UChar sampleNonDigits[] = {0x0010, 0x0041, 0x0122, 0x68FE};
    const int32_t sampleDigitValues[] = {0, 2, 3, 5};

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
    for (i = 0; i < 5; i++) {
      log_verbose("Testing for isspace and nonspaces\n");
        if (!(u_isspace(sampleSpaces[i])) ||
                (u_isspace(sampleNonSpaces[i])))
        {
            log_err("Space char test error : %d or %d \n", (int32_t)sampleSpaces[i], (int32_t)sampleNonSpaces[i]);
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
       log_verbose("Testing for isdigit\n"); 
        if ((u_isdigit(sampleDigits[i]) && 
            (u_charDigitValue(sampleDigits[i])!= sampleDigitValues[i])) ||
            (u_isdigit(sampleNonDigits[i]))) {
            log_err("Digit char test error : %d   or   %d\n", (int32_t)sampleDigits[i], (int32_t)sampleNonDigits[i]);
        }
    }
}


/* Tests for isControl(u_iscntrl()) and isPrintable(u_isprint()) */
void TestControlPrint()
{
    const UChar sampleControl[] = {0x001b, 0x0097, 0x0082};
    const UChar sampleNonControl[] = {'a', 0x0031, 0x00e2};
    const UChar samplePrintable[] = {0x0042, 0x005f, 0x2014};
    const UChar sampleNonPrintable[] = {0x200c, 0x009f, 0x001c};
    int i;
    for (i = 0; i < 3; i++) {
        log_verbose("Testing for iscontrol\n");
        if (!(u_iscntrl(sampleControl[i])) ||
                (u_iscntrl(sampleNonControl[i])))
        {
            log_err("Control char test error : %d   or  %d\n", (int32_t)sampleControl[i], (int32_t)sampleNonControl[i]);
        }
    }
    for (i = 0; i < 3; i++) {
        log_verbose("testing for isprintable\n");
        if ((u_isprint(sampleNonPrintable[i])) ||
                !(u_isprint(samplePrintable[i])))
        {
            log_err("Printable char test error : %d  or  %d\n", (int32_t)samplePrintable[i], (int32_t)sampleNonPrintable[i]);
        }
    }
}
/* u_isIDStart(), u_isIDPart(), u_isIDIgnorable()*/
void TestIdentifier()
{
    
 
    const UChar sampleUnicodeIDStart[] = {0x0250, 0x00e2, 0x0061};
    const UChar sampleNonUnicodeIDStart[] = {0x2000, 0x000a, 0x2019};
    const UChar sampleUnicodeIDPart[] = {0x005f, 0x0032, 0x0045};
    const UChar sampleNonUnicodeIDPart[] = {0x007f, 0x00a3, 0x0020};
    const UChar sampleIDIgnore[] = {0x0006, 0x0010, 0x206b};
    const UChar sampleNonIDIgnore[] = {0x0075, 0x00a3, 0x0061};

    int i;
    for (i = 0; i < 3; i++) {
        log_verbose("Testing sampleUnicodeID start \n");
        /* T_test_logln_ustr((int32_t)i); */
        if (!(u_isIDStart(sampleUnicodeIDStart[i])) ||
                (u_isIDStart(sampleNonUnicodeIDStart[i])))
        {
            log_err("Unicode ID Start char test error : %d  or  %d\n", (int32_t)sampleUnicodeIDStart[i],
                                    (int32_t)sampleNonUnicodeIDStart[i]);
        }
    }
    for (i = 2; i < 3; i++) {   /* nos *** starts with 2 instead of 0, until clarified */
        log_verbose("Testing sample unicode ID part \n");
        /* T_test_logln_ustr((int32_t)i); */
        if (!(u_isIDPart(sampleUnicodeIDPart[i])) ||
                (u_isIDPart(sampleNonUnicodeIDPart[i])))
           {
            log_err("Unicode ID Part char test error : %d  or  %d", (int32_t)sampleUnicodeIDPart[i], (int32_t)sampleNonUnicodeIDPart[i]);
            }
    }
    for (i = 0; i < 3; i++) {
        log_verbose("Testing  sampleId ignore\n");
        /*T_test_logln_ustr((int32_t)i); */
        if (!(u_isIDIgnorable(sampleIDIgnore[i])) ||
                (u_isIDIgnorable(sampleNonIDIgnore[i])))
        {
            log_verbose("ID ignorable char test error : %d  or  %d\n", (int32_t)sampleIDIgnore[i], (int32_t)sampleNonIDIgnore[i]);
        }
    }
}

/* tests for u_CharType(), u_isTitle(), and u_toTitle(),u_charDirection and u_charScript()*/
void TestUnicodeData()
{
    FILE*   input = 0;
    char    buffer[1000];
    char*   bufferPtr = 0, *dirPtr = 0;
    int32_t unicode;
    char newPath[256];

    strcpy(newPath,icu_getDefaultDataDirectory());
    strcat(newPath, "UnicodeData-3.0.0.txt");
    
    input = fopen(newPath, "r");

    if (input == 0) {
      log_err("Unicode CAPI test 'fopen' (%s) error\n", newPath);
      return;
    }

        
        for(;;) {
            bufferPtr = fgets(buffer, 999, input);
            if (bufferPtr == NULL) break;
            if (bufferPtr[0] == '#' || bufferPtr[0] == '\n' || bufferPtr[0] == 0) continue;
            sscanf(bufferPtr, "%X", &unicode);
            if (!(0 <= unicode && unicode < 65536)) {
                log_err("Unicode CAPI test precondition '(0 <= unicode && unicode < 65536)' failed\n");
                return;
            }
            if (unicode == LAST_CHAR_CODE_IN_FILE)
                break;
            bufferPtr = strchr(bufferPtr, ';');
            if (!(bufferPtr != NULL)) {
                log_err("Unicode CAPI test condition '(bufferPtr != NULL)' failed\n");
                return;
            }
            bufferPtr = strchr(bufferPtr + 1, ';'); /* go to start of third field */
            if (!(bufferPtr != NULL)) {
                log_err("Unicode CAPI test condition '(bufferPtr != NULL)' failed\n");
                return;
            }
            dirPtr = bufferPtr;
            dirPtr = strchr(dirPtr + 1, ';');
            if (!(dirPtr != NULL)) {
                log_err("Unicode CAPI test precondition '(dirPtr != NULL)' failed\n");
                return;
            }
            dirPtr = strchr(dirPtr + 1, ';');
            if (!(dirPtr != NULL)) {
                log_err("Unicode CAPI test precondition '(dirPtr != NULL)' failed\n");
                return;
            }
            bufferPtr++;
            bufferPtr[2] = 0;
            
            if (u_charType((UChar)unicode) != tagValues[MakeProp(bufferPtr)])
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

        if (u_charScript((UChar)0x0041 != BASIC_LATIN)) {
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


static  char* raw[3][4] = {
   
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
    dataTable = calloc(sizeof(UChar**),3);

        for (i = 0; i < 3; i++) {
          dataTable[i] = calloc(sizeof(UChar*),4);
            for (j = 0; j < 4; j++){
                dataTable[i][j] = (UChar*) malloc(sizeof(UChar)*(strlen(raw[i][j])+1));
                u_uastrcpy(dataTable[i][j],raw[i][j]);
            }
        }
    
}
/*Tests  for u_strcat(),u_strcmp(), u_strlen(), u_strcpy(),u_strncat(),u_strncmp(),u_strncpy, u_uastrcpy(),u_austrcpy(); */
void TestStringFunctions()
{
   
    int32_t i,j,k;
    UChar* temp =0;
    char test[20];
    log_verbose("Testing u_strlen()\n");
    if( u_strlen(dataTable[0][0])!= u_strlen(dataTable[0][3]) || u_strlen(dataTable[0][0]) == u_strlen(dataTable[0][2]))
        log_err("There is an error in u_strlen()");

    log_verbose("Testing u_strcpy() and u_strcmp)\n");
    temp=(UChar*)malloc(sizeof(UChar*) * 1);
    
    for(i=0;i<3;++i){
        for(j=0;j<4;++j)
        {
        log_verbose("Testing  %s  \n", austrdup(dataTable[i][j]));
        temp=(UChar*)realloc(temp, sizeof(UChar)*(u_strlen(dataTable[i][j]) + 1));
        u_strcpy(temp,dataTable[i][j]);
        
        if(u_strcmp(temp,dataTable[i][j])!=0)
        log_err("something threw an error in u_strcpy() or u_strcmp()\n");
        }
    }
    
    log_verbose("testing u_strcat()\n");
    i=0;
    for(j=0; j<2;++j)
    {
        temp=(UChar*)realloc(temp, sizeof(UChar)*(u_strlen(dataTable[i+2][j]) +1));
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
        
        log_verbose("%s ", austrdup(u_strchr(dataTable[i][j],'_')));
    }


    log_verbose("Testing u_austrcpy()");
    u_austrcpy(test,dataTable[0][0]);
    if(strcmp(test,raw[0][0])!=0)
        log_err("There is an error in u_austrcpy()");

  
    
}




     
