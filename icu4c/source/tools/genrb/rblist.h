/*
*******************************************************************************
*
*   Copyright (C) 1998-1999, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*
* File rblist.h
*
* Modification History:
*
*   Date        Name        Description
*   06/01/99    stephen     Creation.
*******************************************************************************
*/

#ifndef RBLIST_H
#define RBLIST_H 1

#include "utypes.h"
#include "list.h"

/* A resource bundle data item */
struct SRBItem {
  UChar *fTag;
  struct SList *fData;
};

struct SRBItem* make_rbitem(const UChar *tag, const struct SList *data, 
			    UErrorCode *status);

/* A list of RBItems */
struct SRBItemList {
  UChar *fLocale;
  struct SRBItem **fData;
  int32_t fCount;
  int32_t fCapacity;
};

struct SRBItemList* rblist_open(UErrorCode *status);
void rblist_close(struct SRBItemList *list, UErrorCode *status);

void rblist_setlocale(struct SRBItemList *list, const UChar *locale, 
		      UErrorCode *status);
void rblist_add(struct SRBItemList *list, struct SRBItem *s, 
		UErrorCode *status);

#endif
