/*
*******************************************************************************
*   Copyright (C) 1996-1999, International Business Machines
*   Corporation and others.  All Rights Reserved.
*******************************************************************************
*/

#include "unicode/ucol.h"

#include "unicode/uloc.h"
#include "unicode/coll.h"
#include "unicode/tblcoll.h"
#include "unicode/coleitr.h"
#include "unicode/ustring.h"
#include "unicode/normlzr.h"
#include "cpputils.h"

U_CAPI int32_t
u_normalize(const UChar*            source,
        int32_t                 sourceLength, 
        UNormalizationMode      mode, 
        int32_t                 option,
        UChar*                  result,
        int32_t                 resultLength,
        UErrorCode*             status)
{
  if(U_FAILURE(*status)) return -1;

  Normalizer::EMode normMode;
  switch(mode) {
  case UCOL_NO_NORMALIZATION:
    normMode = Normalizer::NO_OP;
    break;
  case UCOL_DECOMP_CAN:
    normMode = Normalizer::DECOMP;
    break;
  case UCOL_DECOMP_COMPAT:
    normMode = Normalizer::DECOMP_COMPAT;
    break;
  case UCOL_DECOMP_CAN_COMP_COMPAT:
    normMode = Normalizer::COMPOSE;
    break;
  case UCOL_DECOMP_COMPAT_COMP_CAN:
    normMode = Normalizer::COMPOSE_COMPAT;
    break;
  default:
    *status = U_ILLEGAL_ARGUMENT_ERROR;
    return -1;
  }

  int32_t len = (sourceLength == -1 ? u_strlen(source) : sourceLength);
  const UnicodeString src((UChar*)source, len, len);
  UnicodeString dst(result, 0, resultLength);
  Normalizer::normalize(src, normMode, option, dst, *status);
  int32_t actualLen;
  T_fillOutputParams(&dst, result, resultLength, &actualLen, status);
  return actualLen;
}

U_CAPI UCollator*
ucol_open(    const    char         *loc,
        UErrorCode      *status)
{
  if(U_FAILURE(*status)) return 0;

  Collator *col = 0;

  if(loc == 0) 
    col = Collator::createInstance(*status);
  else
    col = Collator::createInstance(Locale(loc), *status);

  if(col == 0) {
    *status = U_MEMORY_ALLOCATION_ERROR;
    return 0;
  }

  return (UCollator*)col;
}

U_CAPI UCollator*
ucol_openRules(    const    UChar                  *rules,
        int32_t                 rulesLength,
        UNormalizationMode      mode,
        UCollationStrength      strength,
        UErrorCode              *status)
{
  if(U_FAILURE(*status)) return 0;

  int32_t len = (rulesLength == -1 ? u_strlen(rules) : rulesLength);
  const UnicodeString ruleString((UChar*)rules, len, len);

  Normalizer::EMode normMode;
  switch(mode) {
  case UCOL_NO_NORMALIZATION:
    normMode = Normalizer::NO_OP;
    break;
  case UCOL_DECOMP_CAN:
    normMode = Normalizer::DECOMP;
    break;
  case UCOL_DECOMP_COMPAT:
    normMode = Normalizer::DECOMP_COMPAT;
    break;
  case UCOL_DECOMP_CAN_COMP_COMPAT:
    normMode = Normalizer::COMPOSE;
    break;
  case UCOL_DECOMP_COMPAT_COMP_CAN:
    normMode = Normalizer::COMPOSE_COMPAT;
    break;
  default:
    *status = U_ILLEGAL_ARGUMENT_ERROR;
    return 0;
  }

  RuleBasedCollator *col = 0;
  col = new RuleBasedCollator(ruleString, 
                  (Collator::ECollationStrength) strength, 
                  normMode, 
                  *status);

  if(col == 0) {
    *status = U_MEMORY_ALLOCATION_ERROR;
    return 0;
  }

  return (UCollator*) col;
}

U_CAPI void
ucol_close(UCollator *coll)
{
  delete (Collator*)coll;
}

U_CAPI UCollationResult
ucol_strcoll(    const    UCollator    *coll,
        const    UChar        *source,
        int32_t            sourceLength,
        const    UChar        *target,
        int32_t            targetLength)
{
    if (coll == NULL) return UCOL_EQUAL;
    if (sourceLength == -1) sourceLength = u_strlen(source);
    if (targetLength == -1) targetLength = u_strlen(target);
        return (UCollationResult) ((Collator*)coll)->compare(source,sourceLength,target,targetLength);
}

U_CAPI UBool
ucol_greater(    const    UCollator        *coll,
        const    UChar            *source,
        int32_t            sourceLength,
        const    UChar            *target,
        int32_t            targetLength)
{
  return (ucol_strcoll(coll, source, sourceLength, target, targetLength) 
      == UCOL_GREATER);
}

U_CAPI UBool
ucol_greaterOrEqual(    const    UCollator    *coll,
            const    UChar        *source,
            int32_t        sourceLength,
            const    UChar        *target,
            int32_t        targetLength)
{
  return (ucol_strcoll(coll, source, sourceLength, target, targetLength) 
      != UCOL_LESS);
}

U_CAPI UBool
ucol_equal(        const    UCollator        *coll,
            const    UChar            *source,
            int32_t            sourceLength,
            const    UChar            *target,
            int32_t            targetLength)
{
  return (ucol_strcoll(coll, source, sourceLength, target, targetLength) 
      == UCOL_EQUAL);
}

U_CAPI UCollationStrength
ucol_getStrength(const UCollator *coll)
{
  return (UCollationStrength) ((Collator*)coll)->getStrength();
}

U_CAPI void
ucol_setStrength(    UCollator                *coll,
            UCollationStrength        strength)
{
  ((Collator*)coll)->setStrength((Collator::ECollationStrength)strength);
}

U_CAPI UNormalizationMode
ucol_getNormalization(const UCollator* coll)
{
  switch(((Collator*)coll)->getDecomposition()) {
  case Normalizer::NO_OP:
    return UCOL_NO_NORMALIZATION;

  case Normalizer::COMPOSE:
    return UCOL_DECOMP_COMPAT_COMP_CAN;

  case Normalizer::COMPOSE_COMPAT:
    return UCOL_DECOMP_CAN_COMP_COMPAT;

  case Normalizer::DECOMP:
    return UCOL_DECOMP_COMPAT;

  case Normalizer::DECOMP_COMPAT:
    return UCOL_DECOMP_COMPAT;

  }
  return UCOL_NO_NORMALIZATION;
}

U_CAPI void
ucol_setNormalization(  UCollator            *coll,
            UNormalizationMode    mode)
{
  Normalizer::EMode normMode;
  switch(mode) {
  case UCOL_NO_NORMALIZATION:
    normMode = Normalizer::NO_OP;
    break;
  case UCOL_DECOMP_CAN:
    normMode = Normalizer::DECOMP;
    break;
  case UCOL_DECOMP_COMPAT:
    normMode = Normalizer::DECOMP_COMPAT;
    break;
  case UCOL_DECOMP_COMPAT_COMP_CAN:
    normMode = Normalizer::COMPOSE;
    break;
  case UCOL_DECOMP_CAN_COMP_COMPAT:
    normMode = Normalizer::COMPOSE_COMPAT;
    break;
  default:
    /* Shouldn't get here. */
    /* *status = U_ILLEGAL_ARGUMENT_ERROR; */
    return;
  }

  ((Collator*)coll)->setDecomposition(normMode);
}

U_CAPI int32_t
ucol_getDisplayName(    const    char        *objLoc,
            const    char        *dispLoc,
            UChar             *result,
            int32_t         resultLength,
            UErrorCode        *status)
{
  if(U_FAILURE(*status)) return -1;

  UnicodeString dst(result, resultLength, resultLength);
  Collator::getDisplayName(Locale(objLoc), Locale(dispLoc), dst);
  int32_t actLen;
  T_fillOutputParams(&dst, result, resultLength, &actLen, status);
  return actLen;
}

U_CAPI const char*
ucol_getAvailable(int32_t index)
{
  return uloc_getAvailable(index);
}

U_CAPI int32_t
ucol_countAvailable()
{
  return uloc_countAvailable();
}

U_CAPI const UChar*
ucol_getRules(    const    UCollator        *coll, 
        int32_t            *length)
{
  const UnicodeString& rules = ((RuleBasedCollator*)coll)->getRules();
  *length = rules.length();
  return rules.getUChars();
}

U_CAPI int32_t
ucol_getSortKey(const    UCollator    *coll,
        const    UChar        *source,
        int32_t        sourceLength,
        uint8_t        *result,
        int32_t        resultLength)
{
  int32_t         count;
  const uint8_t*     bytes = NULL;
  CollationKey         key;
  int32_t         copyLen;
    int32_t         len = (sourceLength == -1 ? u_strlen(source) 
                   : sourceLength);
  //  UnicodeString     string((UChar*)source, len, len);
  UErrorCode         status = U_ZERO_ERROR;

  ((Collator*)coll)->getCollationKey(source, len, key, status);
  if(U_FAILURE(status)) 
    return 0;

  bytes = key.getByteArray(count);
  
  copyLen = uprv_min(count, resultLength);
  uprv_arrayCopy((const int8_t*)bytes, (int8_t*)result, copyLen);

  //  if(count > resultLength) {
  //    *status = U_BUFFER_OVERFLOW_ERROR;
  //  }

  return count;
}

U_CAPI int32_t
ucol_keyHashCode(    const    uint8_t*    key, 
            int32_t        length)
{
  CollationKey newKey(key, length);
  return newKey.hashCode();
}

UCollationElements*
ucol_openElements(    const    UCollator            *coll,
            const    UChar                *text,
            int32_t              textLength,
            UErrorCode *status)
{
  int32_t len = (textLength == -1 ? u_strlen(text) : textLength);
  const UnicodeString src((UChar*)text, len, len);

  CollationElementIterator *iter = 0;
  iter = ((RuleBasedCollator*)coll)->createCollationElementIterator(src);
  if(iter == 0) {
    *status = U_MEMORY_ALLOCATION_ERROR;
    return 0;
  }

  return (UCollationElements*) iter;
}

U_CAPI void
ucol_closeElements(UCollationElements *elems)
{
  delete (CollationElementIterator*)elems;
}

U_CAPI void
ucol_reset(UCollationElements *elems)
{
  ((CollationElementIterator*)elems)->reset();
}

U_CAPI int32_t
ucol_next(    UCollationElements    *elems,
        UErrorCode            *status)
{
  if(U_FAILURE(*status)) return UCOL_NULLORDER;

  return ((CollationElementIterator*)elems)->next(*status);
}

U_CAPI int32_t
ucol_previous(    UCollationElements    *elems,
        UErrorCode            *status)
{
  if(U_FAILURE(*status)) return UCOL_NULLORDER;

  return ((CollationElementIterator*)elems)->previous(*status);
}

U_CAPI int32_t
ucol_getMaxExpansion(    const    UCollationElements    *elems,
            int32_t                order)
{
  return ((CollationElementIterator*)elems)->getMaxExpansion(order);
}

U_CAPI void
ucol_setText(UCollationElements        *elems,
         const    UChar                    *text,
         int32_t                    textLength,
         UErrorCode                *status)
{
  if(U_FAILURE(*status)) return;

  int32_t len = (textLength == -1 ? u_strlen(text) : textLength);
  const UnicodeString src((UChar*)text, len, len);

  ((CollationElementIterator*)elems)->setText(src, *status);
}

U_CAPI UTextOffset
ucol_getOffset(const UCollationElements *elems)
{
  return ((CollationElementIterator*)elems)->getOffset();
}

U_CAPI void
ucol_setOffset(    UCollationElements    *elems,
        UTextOffset            offset,
        UErrorCode            *status)
{
  if(U_FAILURE(*status)) return;
  
  ((CollationElementIterator*)elems)->setOffset(offset, *status);
}

U_CAPI void 
ucol_getVersion(const UCollator* coll, 
                UVersionInfo versionInfo) 
{
    ((Collator*)coll)->getVersion(versionInfo);
}

U_CAPI uint8_t *
ucol_cloneRuleData(UCollator *coll, int32_t *length, UErrorCode *status)
{
  return ((RuleBasedCollator*)coll)->cloneRuleData(*length,*status);
}
