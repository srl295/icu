/*
**********************************************************************
*   Copyright (C) 1999-2000 IBM Corp. All rights reserved.
**********************************************************************
*   Date        Name        Description
*   12/1/99    rgillam     Complete port from Java.
*   01/13/2000 helena      Added UErrorCode to ctors.
**********************************************************************
*/

#include "ucmp8.h"
#include "dbbi_tbl.h"
#include "unicode/dbbi.h"

U_NAMESPACE_BEGIN

//=======================================================================
// constructor
//=======================================================================

DictionaryBasedBreakIteratorTables::DictionaryBasedBreakIteratorTables(
                                 UDataMemory* tablesMemory,
                                 char* dictionaryFilename, 
                                 UErrorCode &status)
: RuleBasedBreakIteratorTables(tablesMemory),
  dictionary(dictionaryFilename, status)
{
  if(tablesMemory != 0) {
    const void* tablesImage = udata_getMemory(tablesMemory);
      if(tablesImage != 0) {
	if (U_FAILURE(status)) return;
	const int32_t* tablesIdx = (int32_t*) tablesImage;
	const int8_t* dbbiImage = ((const int8_t*)tablesImage + tablesIdx[8]);
	// we know the offset into the memory image where the DBBI stuff
	// starts is stored in element 8 of the array.  There should be
	// a way for the RBBI constructor to give us this, but there's
	// isn't a good one.
	const int32_t* dbbiIdx = (const int32_t*)dbbiImage;

	categoryFlags = (int8_t*)((const int8_t*)dbbiImage + (int32_t)dbbiIdx[0]);
      }
  }
}

//=======================================================================
// boilerplate
//=======================================================================

/**
 * Destructor
 */
DictionaryBasedBreakIteratorTables::~DictionaryBasedBreakIteratorTables() {
    if (ownTables)
        delete [] categoryFlags;
}

int32_t
DictionaryBasedBreakIteratorTables::lookupCategory(UChar c,
                                                   BreakIterator* bi) const {
    // this override of lookupCategory() exists only to keep track of whether we've
    // passed over any dictionary characters.  It calls the inherited lookupCategory()
    // to do the real work, and then checks whether its return value is one of the
    // categories represented in the dictionary.  If it is, bump the dictionary-
    // character count.
    int32_t result = RuleBasedBreakIteratorTables::lookupCategory(c, bi);
    if (result != RuleBasedBreakIterator::IGNORE && categoryFlags[result]) {
        ((DictionaryBasedBreakIterator*)bi)->bumpDictionaryCharCount();
    }
    return result;
}

U_NAMESPACE_END

/* eof */
