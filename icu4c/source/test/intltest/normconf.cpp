/*
************************************************************************
* Copyright (c) 1997-2001, International Business Machines
* Corporation and others.  All Rights Reserved.
************************************************************************
*/

#include "normconf.h"
#include "unicode/normlzr.h"
#include "unicode/unicode.h"
#include "cstring.h"
#include "unicode/putil.h"
#include "filestrm.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

#define CASE(id,test) case id:                          \
                          name = #test;                 \
                          if (exec) {                   \
                              logln(#test "---");       \
                              logln((UnicodeString)""); \
                              test();                   \
                          }                             \
                          break

void NormalizerConformanceTest::runIndexedTest(int32_t index, UBool exec, const char* &name, char* /*par*/) {
    switch (index) {
        CASE(0,TestConformance);
        // CASE(1,TestCase6);
        default: name = ""; break;
    }
}

#define FIELD_COUNT 5

NormalizerConformanceTest::NormalizerConformanceTest() :
    normalizer(UnicodeString("", ""), Normalizer::COMPOSE) {}

NormalizerConformanceTest::~NormalizerConformanceTest() {}

/**
 * Test the conformance of Normalizer to
 * http://www.unicode.org/unicode/reports/tr15/conformance/Draft-TestSuite.txt.
 * This file must be located at the path specified as TEST_SUITE_FILE.
 */
void NormalizerConformanceTest::TestConformance(void) {
    enum { BUF_SIZE = 1024 };
    char lineBuf[BUF_SIZE];
    UnicodeString fields[FIELD_COUNT];
    UnicodeString buf;
    int32_t passCount = 0;
    int32_t failCount = 0;
    char newPath[256];
    char backupPath[256];
    FileStream *input = NULL;

    /* Look inside ICU_DATA first */
    strcpy(newPath, u_getDataDirectory());
    strcat(newPath, "unidata" U_FILE_SEP_STRING );
    strcat(newPath, TEST_SUITE_FILE);

#if defined (U_SRCDATADIR) /* points to the icu/data directory */
    /* If defined, we'll try an alternate directory second */
    strcpy(backupPath, U_SRCDATADIR);
#else
    strcpy(backupPath, u_getDataDirectory());
    strcat(backupPath, ".." U_FILE_SEP_STRING ".." U_FILE_SEP_STRING "data");
    strcat(backupPath, U_FILE_SEP_STRING);
#endif
    strcat(backupPath, "unidata" U_FILE_SEP_STRING );
    strcat(backupPath, TEST_SUITE_FILE);

      
    input = T_FileStream_open(newPath, "rb");

    if (input == 0) {
      input = T_FileStream_open(backupPath, "rb");
      if (input == 0) {
        errln("Failed to open either " + UnicodeString(newPath) + " or " + UnicodeString(backupPath) );
        return;
      }
    }


    for (int32_t count = 0;;++count) {
        if (T_FileStream_eof(input)) {
            break;
        }
        T_FileStream_readLine(input, lineBuf, (int32_t)sizeof(lineBuf));
        UnicodeString line(lineBuf, "");
        if (line.length() == 0) continue;
        
        // Expect 5 columns of this format:
        // 1E0C;1E0C;0044 0323;1E0C;0044 0323; # <comments>
        
        // Parse out the comment.
        if (line.charAt(0) == 0x0023/*'#'*/) continue;
        
        // Parse out the fields
        if (!hexsplit(line, (UChar)0x003B/*';'*/, fields, FIELD_COUNT, buf)) {
            errln((UnicodeString)"Unable to parse line " + count);
            break; // Syntax error
        }
        if (checkConformance(fields, line)) {
            ++passCount;
        } else {
            ++failCount;
        }
        if ((count % 1000) == 999) {
            logln((UnicodeString)"Line " + (count+1));
        }
    }

    T_FileStream_close(input);

    if (failCount != 0) {
        errln((UnicodeString)"Total: " + failCount + " lines failed, " +
              passCount + " lines passed");
    } else {
        logln((UnicodeString)"Total: " + passCount + " lines passed");
    }
}

/**
 * Verify the conformance of the given line of the Unicode
 * normalization (UTR 15) test suite file.  For each line,
 * there are five columns, corresponding to field[0]..field[4].
 *
 * The following invariants must be true for all conformant implementations
 *  c2 == NFC(c1) == NFC(c2) == NFC(c3)
 *  c3 == NFD(c1) == NFD(c2) == NFD(c3)
 *  c4 == NFKC(c1) == NFKC(c2) == NFKC(c3) == NFKC(c4) == NFKC(c5)
 *  c5 == NFKD(c1) == NFKD(c2) == NFKD(c3) == NFKD(c4) == NFKD(c5)
 *
 * @param field the 5 columns
 * @param line the source line from the test suite file
 * @return true if the test passes
 */
UBool NormalizerConformanceTest::checkConformance(const UnicodeString* field,
                                                  const UnicodeString& line) {
    UBool pass = TRUE;
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString out;
    int32_t fieldNum;

    for (int32_t i=0; i<FIELD_COUNT; ++i) {
        fieldNum = i+1;
        if (i<3) {
            Normalizer::normalize(field[i], Normalizer::COMPOSE, 0, out, status);
            pass &= assertEqual("C", field[i], out, field[1], "c2!=C(c", fieldNum);
            iterativeNorm(field[i], Normalizer::COMPOSE, out, +1);
            pass &= assertEqual("C(+1)", field[i], out, field[1], "c2!=C(c", fieldNum);
            iterativeNorm(field[i], Normalizer::COMPOSE, out, -1);
            pass &= assertEqual("C(-1)", field[i], out, field[1], "c2!=C(c", fieldNum);

            Normalizer::normalize(field[i], Normalizer::DECOMP, 0, out, status);
            pass &= assertEqual("D", field[i], out, field[2], "c3!=D(c", fieldNum);
            iterativeNorm(field[i], Normalizer::DECOMP, out, +1);
            pass &= assertEqual("D(+1)", field[i], out, field[2], "c3!=D(c", fieldNum);
            iterativeNorm(field[i], Normalizer::DECOMP, out, -1);
            pass &= assertEqual("D(-1)", field[i], out, field[2], "c3!=D(c", fieldNum);
        }
        Normalizer::normalize(field[i], Normalizer::COMPOSE_COMPAT, 0, out, status);
        pass &= assertEqual("KC", field[i], out, field[3], "c4!=KC(c", fieldNum);
        iterativeNorm(field[i], Normalizer::COMPOSE_COMPAT, out, +1);
        pass &= assertEqual("KC(+1)", field[i], out, field[3], "c4!=KC(c", fieldNum);
        iterativeNorm(field[i], Normalizer::COMPOSE_COMPAT, out, -1);
        pass &= assertEqual("KC(-1)", field[i], out, field[3], "c4!=KC(c", fieldNum);

        Normalizer::normalize(field[i], Normalizer::DECOMP_COMPAT, 0, out, status);
        pass &= assertEqual("KD", field[i], out, field[4], "c5!=KD(c", fieldNum);
        iterativeNorm(field[i], Normalizer::DECOMP_COMPAT, out, +1);
        pass &= assertEqual("KD(+1)", field[i], out, field[4], "c5!=KD(c", fieldNum);
        iterativeNorm(field[i], Normalizer::DECOMP_COMPAT, out, -1);
        pass &= assertEqual("KD(-1)", field[i], out, field[4], "c5!=KD(c", fieldNum);
    }
    if (U_FAILURE(status)) {
        errln("Normalizer::normalize returned error status");
        return FALSE;
    }
    if (!pass) {
        errln((UnicodeString)"FAIL: " + line);
    }
    return pass;
}

/**
 * Do a normalization using the iterative API in the given direction.
 * @param dir either +1 or -1
 */
void NormalizerConformanceTest::iterativeNorm(const UnicodeString& str,
                                              Normalizer::EMode mode,
                                              UnicodeString& result,
                                              int8_t dir) {
    UErrorCode status = U_ZERO_ERROR;
    normalizer.setText(str, status);
    normalizer.setMode(mode);
    result.truncate(0);
    if (U_FAILURE(status)) {
        return;
    }
    UChar32 ch;
    if (dir > 0) {
        for (ch = normalizer.first(); ch != Normalizer::DONE;
             ch = normalizer.next()) {
            result.append((UChar)ch);
        }
    } else {
        for (ch = normalizer.last(); ch != Normalizer::DONE;
             ch = normalizer.previous()) {
            result.insert(0, (UChar)ch);
        }
    }
}

/**
 * @param op name of normalization form, e.g., "KC"
 * @param s string being normalized
 * @param got value received
 * @param exp expected value
 * @param msg description of this test
 * @param return true if got == exp
 */
UBool NormalizerConformanceTest::assertEqual(const char *op,
                                             const UnicodeString& s,
                                             const UnicodeString& got,
                                             const UnicodeString& exp,
                                             const char *msg,
                                             int32_t field)
{
    if (exp == got)
        return TRUE;

    char *sChars, *gotChars, *expChars;
    UnicodeString sPretty(prettify(s));
    UnicodeString gotPretty(prettify(got));
    UnicodeString expPretty(prettify(exp));

    sChars = new char[sPretty.length() + 1];
    gotChars = new char[gotPretty.length() + 1];
    expChars = new char[expPretty.length() + 1];

    sPretty.extract(0, sPretty.length(), sChars, sPretty.length() + 1);
    sChars[sPretty.length()] = 0;
    gotPretty.extract(0, gotPretty.length(), gotChars, gotPretty.length() + 1);
    gotChars[gotPretty.length()] = 0;
    expPretty.extract(0, expPretty.length(), expChars, expPretty.length() + 1);
    expChars[expPretty.length()] = 0;

    errln("    %s%d)%s(%s)=%s, exp. %s", msg, field, op, sChars, gotChars, expChars);

    delete []sChars;
    delete []gotChars;
    delete []expChars;
    return FALSE;
}

/**
 * Parse 4 hex digits at pos.
 */
static UChar parseInt(const UnicodeString& s, int32_t pos) {
    UChar value = 0;
    int32_t limit = pos+4;
    while (pos < limit) {
        int8_t digit = Unicode::digit(s.charAt(pos++), 16);
        if (digit < 0) {
            return (UChar) -1; // Bogus hex digit -- shouldn't happen
        }
        value = (UChar)((value << 4) | digit);
    }
    return value;
}

/**
 * Split a string into pieces based on the given delimiter
 * character.  Then, parse the resultant fields from hex into
 * characters.  That is, "0040 0400;0C00;0899" -> new String[] {
 * "\u0040\u0400", "\u0C00", "\u0899" }.  The output is assumed to
 * be of the proper length already, and exactly output.length
 * fields are parsed.  If there are too few an exception is
 * thrown.  If there are too many the extras are ignored.
 *
 * @param buf scratch buffer
 * @return FALSE upon failure
 */
UBool NormalizerConformanceTest::hexsplit(const UnicodeString& s, UChar delimiter,
                                          UnicodeString* output, int32_t outputLength,
                                          UnicodeString& buf) {
    int32_t i;
    int32_t pos = 0;
    for (i=0; i<outputLength; ++i) {
        int32_t delim = s.indexOf(delimiter, pos);
        if (delim < 0) {
            errln((UnicodeString)"Missing field in " + s);
            return FALSE;
        }
        // Our field is from pos..delim-1.
        buf.remove();
        while (pos < delim) {
            if (s.charAt(pos) == 0x0020/*' '*/) {
                ++pos;
            } else if (pos+4 > delim) {
                errln((UnicodeString)"Premature eol in " + s);
                return FALSE;
            } else {
                UChar hex = parseInt(s, pos);
                buf.append(hex);
                pos += 4;
            }
        }
        if (buf.length() < 1) {
            errln((UnicodeString)"Empty field " + i + " in " + s);
            return FALSE;
        }
        output[i] = buf;
        ++pos; // Skip over delim
    }
    return TRUE;
}

// Specific tests for debugging.  These are generally failures taken from
// the conformance file, but culled out to make debugging easier.

void NormalizerConformanceTest::TestCase6(void) {
    _testOneLine("0385;0385;00A8 0301;0020 0308 0301;0020 0308 0301;");
}

void NormalizerConformanceTest::_testOneLine(const UnicodeString& line) {
    UnicodeString fields[FIELD_COUNT];
    UnicodeString buf;
    if (!hexsplit(line, (UChar)0x003B/*';'*/, fields, FIELD_COUNT, buf)) {
        errln((UnicodeString)"Unable to parse line " + line);
    } else {
        checkConformance(fields, line);
    }
}
