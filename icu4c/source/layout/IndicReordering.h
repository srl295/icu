/*
 * (C) Copyright IBM Corp. 1998, 1999, 2000 - All Rights Reserved
 *
 * $Source: /xsrl/Nsvn/icu/icu/source/layout/IndicReordering.h,v $
 * $Date: 2003/01/04 02:52:23 $
 * $Revision: 1.6 $
 *
 */

#ifndef __INDICREORDERING_H
#define __INDICREORDERING_H

#include "LETypes.h"
#include "OpenTypeTables.h"

U_NAMESPACE_BEGIN

// Characters that get refered to by name...
enum
{
    C_SIGN_ZWNJ     = 0x200C,
    C_SIGN_ZWJ      = 0x200D
};

typedef LEUnicode SplitMatra[3];

class  MPreFixups;

struct IndicClassTable
{
    enum CharClassValues
    {
        CC_RESERVED             = 0,
        CC_MODIFYING_MARK_ABOVE = 1,
        CC_MODIFYING_MARK_POST  = 2,
        CC_INDEPENDENT_VOWEL    = 3,
        CC_CONSONANT            = 4,
        CC_CONSONANT_WITH_NUKTA = 5,
        CC_NUKTA                = 6,
        CC_DEPENDENT_VOWEL      = 7,
        CC_VIRAMA               = 8,
        CC_ZERO_WIDTH_MARK      = 9,
        CC_COUNT                = 10
    };

    enum CharClassFlags
    {
        CF_CLASS_MASK   = 0x0000FFFF,

        CF_CONSONANT    = 0x80000000,

        CF_REPH         = 0x40000000,
        CF_VATTU        = 0x20000000,
        CF_BELOW_BASE   = 0x10000000,
        CF_POST_BASE    = 0x08000000,

        CF_MATRA_PRE    = 0x04000000,
        CF_MATRA_BELOW  = 0x02000000,
        CF_MATRA_ABOVE  = 0x01000000,
        CF_MATRA_POST   = 0x00800000,
        CF_LENGTH_MARK  = 0x00400000,
        CF_INDEX_MASK   = 0x000F0000,
        CF_INDEX_SHIFT  = 16
    };

    typedef le_int32 CharClass;

    enum ScriptFlagBits
    {
        SF_MATRAS_AFTER_BASE    = 0x80000000,
        SF_REPH_AFTER_BELOW     = 0x40000000,
        SF_EYELASH_RA           = 0x20000000,
        SF_MPRE_FIXUP           = 0x10000000,

        SF_POST_BASE_LIMIT_MASK = 0x0000FFFF,
        SF_NO_POST_BASE_LIMIT   = 0x00007FFF
    };

    typedef le_int32 ScriptFlags;

    LEUnicode firstChar;
    LEUnicode lastChar;
    le_int32 worstCaseExpansion;
    ScriptFlags scriptFlags;
    const CharClass *classTable;
    const SplitMatra *splitMatraTable;

    le_int32 getWorstCaseExpansion() const;

    CharClass getCharClass(LEUnicode ch) const;
    const SplitMatra *getSplitMatra(CharClass charClass) const;

    le_bool isVMabove(LEUnicode ch) const;
    le_bool isVMpost(LEUnicode ch) const;
    le_bool isConsonant(LEUnicode ch) const;
    le_bool isReph(LEUnicode ch) const;
    le_bool isVirama(LEUnicode ch) const;
    le_bool isNukta(LEUnicode ch) const;
    le_bool isVattu(LEUnicode ch) const;
    le_bool isMatra(LEUnicode ch) const;
    le_bool isSplitMatra(LEUnicode ch) const;
    le_bool isMpre(LEUnicode ch) const;
    le_bool isMbelow(LEUnicode ch) const;
    le_bool isMabove(LEUnicode ch) const;
    le_bool isMpost(LEUnicode ch) const;
    le_bool isLengthMark(LEUnicode ch) const;
    le_bool hasPostOrBelowBaseForm(LEUnicode ch) const;
    le_bool hasPostBaseForm(LEUnicode ch) const;
    le_bool hasBelowBaseForm(LEUnicode ch) const;

    static le_bool isVMabove(CharClass charClass);
    static le_bool isVMpost(CharClass charClass);
    static le_bool isConsonant(CharClass charClass);
    static le_bool isReph(CharClass charClass);
    static le_bool isVirama(CharClass charClass);
    static le_bool isNukta(CharClass charClass);
    static le_bool isVattu(CharClass charClass);
    static le_bool isMatra(CharClass charClass);
    static le_bool isSplitMatra(CharClass charClass);
    static le_bool isMpre(CharClass charClass);
    static le_bool isMbelow(CharClass charClass);
    static le_bool isMabove(CharClass charClass);
    static le_bool isMpost(CharClass charClass);
    static le_bool isLengthMark(CharClass charClass);
    static le_bool hasPostOrBelowBaseForm(CharClass charClass);
    static le_bool hasPostBaseForm(CharClass charClass);
    static le_bool hasBelowBaseForm(CharClass charClass);

    static const IndicClassTable *getScriptClassTable(le_int32 scriptCode);
};

class IndicReordering /* not : public UObject because all methods are static */ {
public:
    static le_int32 getWorstCaseExpansion(le_int32 scriptCode);

    static le_int32 reorder(const LEUnicode *theChars, le_int32 charCount, le_int32 scriptCode,
        LEUnicode *outChars, le_int32 *charIndices, const LETag **charTags,
        MPreFixups **outMPreFixups);

    static void adjustMPres(MPreFixups *mpreFixups, LEGlyphID *glyphs, le_int32 *charIndices);

    static const LETag *getFeatureOrder();

private:
    // do not instantiate
    IndicReordering();

    static le_int32 findSyllable(const IndicClassTable *classTable, const LEUnicode *chars, le_int32 prev, le_int32 charCount);

};

inline le_int32 IndicClassTable::getWorstCaseExpansion() const
{
    return worstCaseExpansion;
}

inline const SplitMatra *IndicClassTable::getSplitMatra(CharClass charClass) const
{
    le_int32 index = (charClass & CF_INDEX_MASK) >> CF_INDEX_SHIFT;

    return &splitMatraTable[index - 1];
}

inline le_bool IndicClassTable::isVMabove(LEUnicode ch) const
{
    return isVMabove(getCharClass(ch));
}

inline le_bool IndicClassTable::isVMpost(LEUnicode ch) const
{
    return isVMpost(getCharClass(ch));
}

inline le_bool IndicClassTable::isConsonant(LEUnicode ch) const
{
    return isConsonant(getCharClass(ch));
}

inline le_bool IndicClassTable::isReph(LEUnicode ch) const
{
    return isReph(getCharClass(ch));
}

inline le_bool IndicClassTable::isVirama(LEUnicode ch) const
{
    return isVirama(getCharClass(ch));
}

inline le_bool IndicClassTable::isNukta(LEUnicode ch) const
{
    return isNukta(getCharClass(ch));
}

inline le_bool IndicClassTable::isVattu(LEUnicode ch) const
{
    return isVattu(getCharClass(ch));
}

inline le_bool IndicClassTable::isMatra(LEUnicode ch) const
{
    return isMatra(getCharClass(ch));
}

inline le_bool IndicClassTable::isSplitMatra(LEUnicode ch) const
{
    return isSplitMatra(getCharClass(ch));
}

inline le_bool IndicClassTable::isMpre(LEUnicode ch) const
{
    return isMpre(getCharClass(ch));
}

inline le_bool IndicClassTable::isMbelow(LEUnicode ch) const
{
    return isMbelow(getCharClass(ch));
}

inline le_bool IndicClassTable::isMabove(LEUnicode ch) const
{
    return isMabove(getCharClass(ch));
}

inline le_bool IndicClassTable::isMpost(LEUnicode ch) const
{
    return isMpost(getCharClass(ch));
}

inline le_bool IndicClassTable::isLengthMark(LEUnicode ch) const
{
    return isLengthMark(getCharClass(ch));
}

inline le_bool IndicClassTable::hasPostOrBelowBaseForm(LEUnicode ch) const
{
    return hasPostOrBelowBaseForm(getCharClass(ch));
}

inline le_bool IndicClassTable::hasPostBaseForm(LEUnicode ch) const
{
    return hasPostBaseForm(getCharClass(ch));
}

inline le_bool IndicClassTable::hasBelowBaseForm(LEUnicode ch) const
{
    return hasBelowBaseForm(getCharClass(ch));
}

inline le_bool IndicClassTable::isVMabove(CharClass charClass)
{
    return (charClass & CF_CLASS_MASK) == CC_MODIFYING_MARK_ABOVE;
}

inline le_bool IndicClassTable::isVMpost(CharClass charClass)
{
    return (charClass & CF_CLASS_MASK) == CC_MODIFYING_MARK_POST;
}

inline le_bool IndicClassTable::isConsonant(CharClass charClass)
{
    return (charClass & CF_CONSONANT) != 0;
}

inline le_bool IndicClassTable::isReph(CharClass charClass)
{
    return (charClass & CF_REPH) != 0;
}

inline le_bool IndicClassTable::isNukta(CharClass charClass)
{
    return (charClass & CF_CLASS_MASK) == CC_NUKTA;
}

inline le_bool IndicClassTable::isVirama(CharClass charClass)
{
    return (charClass & CF_CLASS_MASK) == CC_VIRAMA;
}

inline le_bool IndicClassTable::isVattu(CharClass charClass)
{
    return (charClass & CF_VATTU) != 0;
}

inline le_bool IndicClassTable::isMatra(CharClass charClass)
{
    return (charClass & CF_CLASS_MASK) == CC_DEPENDENT_VOWEL;
}

inline le_bool IndicClassTable::isSplitMatra(CharClass charClass)
{
    return (charClass & CF_INDEX_MASK) != 0;
}

inline le_bool IndicClassTable::isMpre(CharClass charClass)
{
    return (charClass & CF_MATRA_PRE) != 0;
}

inline le_bool IndicClassTable::isMbelow(CharClass charClass)
{
    return (charClass & CF_MATRA_BELOW) != 0;
}

inline le_bool IndicClassTable::isMabove(CharClass charClass)
{
    return (charClass & CF_MATRA_ABOVE) != 0;
}

inline le_bool IndicClassTable::isMpost(CharClass charClass)
{
    return (charClass & CF_MATRA_POST) != 0;
}

inline le_bool IndicClassTable::isLengthMark(CharClass charClass)
{
    return (charClass & CF_LENGTH_MARK) != 0;
}

inline le_bool IndicClassTable::hasPostOrBelowBaseForm(CharClass charClass)
{
    return (charClass & (CF_POST_BASE | CF_BELOW_BASE)) != 0;
}

inline le_bool IndicClassTable::hasPostBaseForm(CharClass charClass)
{
    return (charClass & CF_POST_BASE) != 0;
}

inline le_bool IndicClassTable::hasBelowBaseForm(CharClass charClass)
{
    return (charClass & CF_BELOW_BASE) != 0;
}

U_NAMESPACE_END
#endif
