/*
*****************************************************************************************
*   Copyright (C) 1996-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
*****************************************************************************************
*/

#include "unicode/ubrk.h"

#include "unicode/brkiter.h"
#include "unicode/uloc.h"
#include "unicode/ustring.h"
#include "unicode/uchriter.h"

U_NAMESPACE_USE

U_CAPI UBreakIterator* U_EXPORT2
ubrk_open(UBreakIteratorType type,
      const char *locale,
      const UChar *text,
      int32_t textLength,
      UErrorCode *status)
{

  if(U_FAILURE(*status)) return 0;

  BreakIterator *result = 0;

  switch(type) {

  case UBRK_CHARACTER:
    result = BreakIterator::createCharacterInstance(Locale(locale), *status);
    break;

  case UBRK_WORD:
    result = BreakIterator::createWordInstance(Locale(locale), *status);
    break;

  case UBRK_LINE:
    result = BreakIterator::createLineInstance(Locale(locale), *status);
    break;

  case UBRK_SENTENCE:
    result = BreakIterator::createSentenceInstance(Locale(locale), *status);
    break;

  case UBRK_TITLE:
    result = BreakIterator::createTitleInstance(Locale(locale), *status);
    break;
  }

  // check for allocation error
  if (U_FAILURE(*status)) {
     return 0;
  }
  if(result == 0) {
    *status = U_MEMORY_ALLOCATION_ERROR;
    return 0;
  }

  int32_t textLen = (textLength == -1 ? u_strlen(text) : textLength);
  UCharCharacterIterator *iter = 0;
  iter = new UCharCharacterIterator(text, textLen);
  if(iter == 0) {
    *status = U_MEMORY_ALLOCATION_ERROR;
    delete result;
    return 0;
  }
  result->adoptText(iter);

  return (UBreakIterator*)result;
}

U_CAPI UBreakIterator* U_EXPORT2
ubrk_openRules(const UChar *rules,
           int32_t rulesLength,
           const UChar *text,
           int32_t textLength,
           UErrorCode *status)
{
  if(U_FAILURE(*status)) return 0;
  *status = U_UNSUPPORTED_ERROR;
  return 0;
}

U_CAPI UBreakIterator * U_EXPORT2
ubrk_safeClone(
          const UBreakIterator *bi,
          void *stackBuffer,
          int32_t *pBufferSize,
          UErrorCode *status)
{
    if (status == NULL || U_FAILURE(*status)){
        return 0;
    }
    if (!pBufferSize || !bi){
       *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    return (UBreakIterator *)(((BreakIterator*)bi)->
        createBufferClone(stackBuffer, *pBufferSize, *status));
}

U_CAPI void U_EXPORT2
ubrk_close(UBreakIterator *bi)
{

    if (bi && !((BreakIterator*) bi)->isBufferClone())
    {
        delete (BreakIterator*) bi;
    }
}

U_CAPI void U_EXPORT2
ubrk_setText(UBreakIterator* bi,
             const UChar*    text,
             int32_t         textLength,
             UErrorCode*     status)
{

  if (U_FAILURE(*status)) return;

  const CharacterIterator& biText = ((BreakIterator*)bi)->getText();

  int32_t textLen = (textLength == -1 ? u_strlen(text) : textLength);
  if (biText.getDynamicClassID() == UCharCharacterIterator::getStaticClassID()) {
      ((UCharCharacterIterator&)biText).setText(text, textLen);
  }
  else {
      UCharCharacterIterator *iter = 0;
      iter = new UCharCharacterIterator(text, textLen);
      if(iter == 0) {
        *status = U_MEMORY_ALLOCATION_ERROR;
        return;
      }
      ((BreakIterator*)bi)->adoptText(iter);
  }
}

U_CAPI UTextOffset U_EXPORT2
ubrk_current(const UBreakIterator *bi)
{

  return ((BreakIterator*)bi)->current();
}

U_CAPI UTextOffset U_EXPORT2
ubrk_next(UBreakIterator *bi)
{

  return ((BreakIterator*)bi)->next();
}

U_CAPI UTextOffset U_EXPORT2
ubrk_previous(UBreakIterator *bi)
{

  return ((BreakIterator*)bi)->previous();
}

U_CAPI UTextOffset U_EXPORT2
ubrk_first(UBreakIterator *bi)
{

  return ((BreakIterator*)bi)->first();
}

U_CAPI UTextOffset U_EXPORT2
ubrk_last(UBreakIterator *bi)
{

  return ((BreakIterator*)bi)->last();
}

U_CAPI UTextOffset U_EXPORT2
ubrk_preceding(UBreakIterator *bi,
           UTextOffset offset)
{

  return ((BreakIterator*)bi)->preceding(offset);
}

U_CAPI UTextOffset U_EXPORT2
ubrk_following(UBreakIterator *bi,
           UTextOffset offset)
{

  return ((BreakIterator*)bi)->following(offset);
}

U_CAPI const char* U_EXPORT2
ubrk_getAvailable(int32_t index)
{

  return uloc_getAvailable(index);
}

U_CAPI int32_t U_EXPORT2
ubrk_countAvailable()
{

  return uloc_countAvailable();
}
