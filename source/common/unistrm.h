/*
**********************************************************************
*   Copyright (C) 1997-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*  FILE NAME : unistrm.h
*
*   Modification History:
*
*   Date        Name        Description
*   02/05/97    aliu        Added UnicodeString streamIn and streamOut methods.
*   03/26/97    aliu        Added indexOf(UChar,).
*   04/24/97    aliu        Numerous changes per code review.
*   05/06/97    helena      Added isBogus().
******************************************************************************
*/         
#ifndef UNISTRM_H
#define UNISTRM_H

#include "filestrm.h"
#include "umemstrm.h"
#include "unicode/unistr.h"


class U_COMMON_API UnicodeStringStreamer
{
public:
    static void streamIn(UnicodeString* string, FileStream* is);
    static void streamOut(const UnicodeString* string, FileStream* os);
    static void streamIn(UnicodeString* string, UMemoryStream* is);
    static void streamOut(const UnicodeString* string, UMemoryStream* os);
};


#endif



