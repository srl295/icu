
/*
*****************************************************************************************
*                                                                                       *
* COPYRIGHT:                                                                            *
*   (C) Copyright Taligent, Inc.,  1996                                                 *
*   (C) Copyright International Business Machines Corporation,  1996-1998               *
*   Licensed Material - Program-Property of IBM - All Rights Reserved.                  *
*   US Government Users Restricted Rights - Use, duplication, or disclosure             *
*   restricted by GSA ADP Schedule Contract with IBM Corp.                              *
*                                                                                       *
*****************************************************************************************
*/

//===============================================================================
//
// File colrules.cpp
//
// Created by: Helena Shih (originally colrules.h)
//
// WARNING: THIS FILE IS MACHINE GENERATED. DO NOT HAND EDIT IT UNLESS YOU REALLY
// KNOW WHAT YOU'RE DOING.
//
// Modification History:
//
//  Date        Name        Description
//  2/13/97     aliu        Moved into TableCollation class.
//  8/13/98        erm            Changed to machine generated, added Normalizer tables.
//
//===============================================================================

#ifndef _TBLCOLL
#include "tblcoll.h"
#endif

#ifndef _UNISTR
#include "unistr.h"
#endif

#define ARRAY_LENGTH(array) (sizeof array / sizeof array[0])



static const UChar defaultRulesArray[] =
{
    0x003D, 0x0027, 0x200B, 0x0027, 0x003D, 0x200C, 0x003D, 0x200D, 0x003D, 0x200E, 
    0x003D, 0x200F, 0x003D, 0x0000, 0x0020, 0x003D, 0x0001, 0x0020, 0x003D, 0x0002, 
    0x0020, 0x003D, 0x0003, 0x0020, 0x003D, 0x0004, 0x003D, 0x0005, 0x0020, 0x003D, 
    0x0006, 0x0020, 0x003D, 0x0007, 0x0020, 0x003D, 0x0008, 0x0020, 0x003D, 0x0027, 
    0x0009, 0x0027, 0x003D, 0x0027, 0x000B, 0x0027, 0x0020, 0x003D, 0x000E, 0x003D, 
    0x000F, 0x0020, 0x003D, 0x0027, 0x0010, 0x0027, 0x0020, 0x003D, 0x0011, 0x0020, 
    0x003D, 0x0012, 0x0020, 0x003D, 0x0013, 0x003D, 0x0014, 0x0020, 0x003D, 0x0015, 
    0x0020, 0x003D, 0x0016, 0x0020, 0x003D, 0x0017, 0x0020, 0x003D, 0x0018, 0x003D, 
    0x0019, 0x0020, 0x003D, 0x001A, 0x0020, 0x003D, 0x001B, 0x0020, 0x003D, 0x001C, 
    0x0020, 0x003D, 0x001D, 0x003D, 0x001E, 0x0020, 0x003D, 0x001F, 0x0020, 0x003D, 
    0x007F, 0x003D, 0x0080, 0x0020, 0x003D, 0x0081, 0x0020, 0x003D, 0x0082, 0x0020, 
    0x003D, 0x0083, 0x0020, 0x003D, 0x0084, 0x0020, 0x003D, 0x0085, 0x003D, 0x0086, 
    0x0020, 0x003D, 0x0087, 0x0020, 0x003D, 0x0088, 0x0020, 0x003D, 0x0089, 0x0020, 
    0x003D, 0x008A, 0x0020, 0x003D, 0x008B, 0x003D, 0x008C, 0x0020, 0x003D, 0x008D, 
    0x0020, 0x003D, 0x008E, 0x0020, 0x003D, 0x008F, 0x0020, 0x003D, 0x0090, 0x0020, 
    0x003D, 0x0091, 0x003D, 0x0092, 0x0020, 0x003D, 0x0093, 0x0020, 0x003D, 0x0094, 
    0x0020, 0x003D, 0x0095, 0x0020, 0x003D, 0x0096, 0x0020, 0x003D, 0x0097, 0x003D, 
    0x0098, 0x0020, 0x003D, 0x0099, 0x0020, 0x003D, 0x009A, 0x0020, 0x003D, 0x009B, 
    0x0020, 0x003D, 0x009C, 0x0020, 0x003D, 0x009D, 0x003D, 0x009E, 0x0020, 0x003D, 
    0x009F, 0x003B, 0x0027, 0x0020, 0x0027, 0x003B, 0x0027, 0x00A0, 0x0027, 0x003B, 
    0x0027, 0x2000, 0x0027, 0x003B, 0x0027, 0x2001, 0x0027, 0x003B, 0x0027, 0x2002, 
    0x0027, 0x003B, 0x0027, 0x2003, 0x0027, 0x003B, 0x0027, 0x2004, 0x0027, 0x003B, 
    0x0027, 0x2005, 0x0027, 0x003B, 0x0027, 0x2006, 0x0027, 0x003B, 0x0027, 0x2007, 
    0x0027, 0x003B, 0x0027, 0x2008, 0x0027, 0x003B, 0x0027, 0x2009, 0x0027, 0x003B, 
    0x0027, 0x200A, 0x0027, 0x003B, 0x0027, 0x3000, 0x0027, 0x003B, 0x0027, 0xFEFF, 
    0x0027, 0x003B, 0x0027, 0x000D, 0x0027, 0x0020, 0x003B, 0x0027, 0x0009, 0x0027, 
    0x0020, 0x003B, 0x0027, 0x000A, 0x0027, 0x003B, 0x0027, 0x000C, 0x0027, 0x003B, 
    0x0027, 0x000B, 0x0027, 0x003B, 0x0301, 0x003B, 0x0300, 0x003B, 0x0306, 0x003B, 
    0x0302, 0x003B, 0x030C, 0x003B, 0x030A, 0x003B, 0x030D, 0x003B, 0x0308, 0x003B, 
    0x030B, 0x003B, 0x0303, 0x003B, 0x0307, 0x003B, 0x0304, 0x003B, 0x0337, 0x003B, 
    0x0327, 0x003B, 0x0328, 0x003B, 0x0323, 0x003B, 0x0332, 0x003B, 0x0305, 0x003B, 
    0x0309, 0x003B, 0x030E, 0x003B, 0x030F, 0x003B, 0x0310, 0x003B, 0x0311, 0x003B, 
    0x0312, 0x003B, 0x0313, 0x003B, 0x0314, 0x003B, 0x0315, 0x003B, 0x0316, 0x003B, 
    0x0317, 0x003B, 0x0318, 0x003B, 0x0319, 0x003B, 0x031A, 0x003B, 0x031B, 0x003B, 
    0x031C, 0x003B, 0x031D, 0x003B, 0x031E, 0x003B, 0x031F, 0x003B, 0x0320, 0x003B, 
    0x0321, 0x003B, 0x0322, 0x003B, 0x0324, 0x003B, 0x0325, 0x003B, 0x0326, 0x003B, 
    0x0329, 0x003B, 0x032A, 0x003B, 0x032B, 0x003B, 0x032C, 0x003B, 0x032D, 0x003B, 
    0x032E, 0x003B, 0x032F, 0x003B, 0x0330, 0x003B, 0x0331, 0x003B, 0x0333, 0x003B, 
    0x0334, 0x003B, 0x0335, 0x003B, 0x0336, 0x003B, 0x0338, 0x003B, 0x0339, 0x003B, 
    0x033A, 0x003B, 0x033B, 0x003B, 0x033C, 0x003B, 0x033D, 0x003B, 0x033E, 0x003B, 
    0x033F, 0x003B, 0x0342, 0x003B, 0x0343, 0x003B, 0x0344, 0x003B, 0x0345, 0x003B, 
    0x0360, 0x003B, 0x0361, 0x003B, 0x0483, 0x003B, 0x0484, 0x003B, 0x0485, 0x003B, 
    0x0486, 0x003B, 0x20D0, 0x003B, 0x20D1, 0x003B, 0x20D2, 0x003B, 0x20D3, 0x003B, 
    0x20D4, 0x003B, 0x20D5, 0x003B, 0x20D6, 0x003B, 0x20D7, 0x003B, 0x20D8, 0x003B, 
    0x20D9, 0x003B, 0x20DA, 0x003B, 0x20DB, 0x003B, 0x20DC, 0x003B, 0x20DD, 0x003B, 
    0x20DE, 0x003B, 0x20DF, 0x003B, 0x20E0, 0x003B, 0x20E1, 0x002C, 0x0027, 0x002D, 
    0x0027, 0x003B, 0x00AD, 0x003B, 0x2010, 0x003B, 0x2011, 0x003B, 0x2012, 0x003B, 
    0x2013, 0x003B, 0x2014, 0x003B, 0x2015, 0x003B, 0x2212, 0x003C, 0x0027, 0x005F, 
    0x0027, 0x003C, 0x00AF, 0x003C, 0x0027, 0x002C, 0x0027, 0x003C, 0x0027, 0x003B, 
    0x0027, 0x003C, 0x0027, 0x003A, 0x0027, 0x003C, 0x0027, 0x0021, 0x0027, 0x003C, 
    0x00A1, 0x003C, 0x0027, 0x003F, 0x0027, 0x003C, 0x00BF, 0x003C, 0x0027, 0x002F, 
    0x0027, 0x003C, 0x0027, 0x002E, 0x0027, 0x003C, 0x00B4, 0x003C, 0x0027, 0x0060, 
    0x0027, 0x003C, 0x0027, 0x005E, 0x0027, 0x003C, 0x00A8, 0x003C, 0x0027, 0x007E, 
    0x0027, 0x003C, 0x00B7, 0x003C, 0x00B8, 0x003C, 0x0027, 0x0027, 0x0027, 0x003C, 
    0x0027, 0x0022, 0x0027, 0x003C, 0x00AB, 0x003C, 0x00BB, 0x003C, 0x0027, 0x0028, 
    0x0027, 0x003C, 0x0027, 0x0029, 0x0027, 0x003C, 0x0027, 0x005B, 0x0027, 0x003C, 
    0x0027, 0x005D, 0x0027, 0x003C, 0x0027, 0x007B, 0x0027, 0x003C, 0x0027, 0x007D, 
    0x0027, 0x003C, 0x00A7, 0x003C, 0x00B6, 0x003C, 0x00A9, 0x003C, 0x00AE, 0x003C, 
    0x0027, 0x0040, 0x0027, 0x003C, 0x00A4, 0x003C, 0x0E3F, 0x003C, 0x00A2, 0x003C, 
    0x20A1, 0x003C, 0x20A2, 0x003C, 0x0027, 0x0024, 0x0027, 0x003C, 0x20AB, 0x003C, 
    0x20AC, 0x003C, 0x20A3, 0x003C, 0x20A4, 0x003C, 0x20A5, 0x003C, 0x20A6, 0x003C, 
    0x20A7, 0x003C, 0x00A3, 0x003C, 0x20A8, 0x003C, 0x20AA, 0x003C, 0x20A9, 0x003C, 
    0x00A5, 0x003C, 0x0027, 0x002A, 0x0027, 0x003C, 0x0027, 0x005C, 0x0027, 0x003C, 
    0x0027, 0x0026, 0x0027, 0x003C, 0x0027, 0x0023, 0x0027, 0x003C, 0x0027, 0x0025, 
    0x0027, 0x003C, 0x0027, 0x002B, 0x0027, 0x003C, 0x00B1, 0x003C, 0x00F7, 0x003C, 
    0x00D7, 0x003C, 0x0027, 0x003C, 0x0027, 0x003C, 0x0027, 0x003D, 0x0027, 0x003C, 
    0x0027, 0x003E, 0x0027, 0x003C, 0x00AC, 0x003C, 0x0027, 0x007C, 0x0027, 0x003C, 
    0x00A6, 0x003C, 0x00B0, 0x003C, 0x00B5, 0x003C, 0x0030, 0x003C, 0x0031, 0x003C, 
    0x0032, 0x003C, 0x0033, 0x003C, 0x0034, 0x003C, 0x0035, 0x003C, 0x0036, 0x003C, 
    0x0037, 0x003C, 0x0038, 0x003C, 0x0039, 0x003C, 0x00BC, 0x003C, 0x00BD, 0x003C, 
    0x00BE, 0x003C, 0x0061, 0x002C, 0x0041, 0x003C, 0x0062, 0x002C, 0x0042, 0x003C, 
    0x0063, 0x002C, 0x0043, 0x003C, 0x0064, 0x002C, 0x0044, 0x003C, 0x00F0, 0x002C, 
    0x00D0, 0x003C, 0x0065, 0x002C, 0x0045, 0x003C, 0x0066, 0x002C, 0x0046, 0x003C, 
    0x0067, 0x002C, 0x0047, 0x003C, 0x0068, 0x002C, 0x0048, 0x003C, 0x0069, 0x002C, 
    0x0049, 0x003C, 0x006A, 0x002C, 0x004A, 0x003C, 0x006B, 0x002C, 0x004B, 0x003C, 
    0x006C, 0x002C, 0x004C, 0x003C, 0x006D, 0x002C, 0x004D, 0x003C, 0x006E, 0x002C, 
    0x004E, 0x003C, 0x006F, 0x002C, 0x004F, 0x003C, 0x0070, 0x002C, 0x0050, 0x003C, 
    0x0071, 0x002C, 0x0051, 0x003C, 0x0072, 0x002C, 0x0052, 0x003C, 0x0073, 0x002C, 
    0x0020, 0x0053, 0x0020, 0x0026, 0x0020, 0x0053, 0x0053, 0x002C, 0x00DF, 0x003C, 
    0x0074, 0x002C, 0x0054, 0x0026, 0x0020, 0x0054, 0x0048, 0x002C, 0x0020, 0x00DE, 
    0x0020, 0x0026, 0x0054, 0x0048, 0x002C, 0x0020, 0x00FE, 0x0020, 0x003C, 0x0075, 
    0x002C, 0x0055, 0x003C, 0x0076, 0x002C, 0x0056, 0x003C, 0x0077, 0x002C, 0x0057, 
    0x003C, 0x0078, 0x002C, 0x0058, 0x003C, 0x0079, 0x002C, 0x0059, 0x003C, 0x007A, 
    0x002C, 0x005A, 0x0026, 0x0041, 0x0045, 0x002C, 0x00C6, 0x0026, 0x0041, 0x0045, 
    0x002C, 0x00E6, 0x0026, 0x004F, 0x0045, 0x002C, 0x0152, 0x0026, 0x004F, 0x0045, 
    0x002C, 0x0153
};

UnicodeString RuleBasedCollator::DEFAULTRULES(defaultRulesArray, ARRAY_LENGTH(defaultRulesArray));

