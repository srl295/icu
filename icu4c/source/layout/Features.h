/*
 * @(#)Features.h	1.4 00/03/15
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001 - All Rights Reserved
 *
 */

#ifndef __FEATURES_H
#define __FEATURES_H

#include "LETypes.h"
#include "OpenTypeTables.h"

U_NAMESPACE_BEGIN

struct FeatureRecord
{
    ATag		featureTag;
    Offset		featureTableOffset;
};

struct FeatureTable
{
    Offset		featureParamsOffset;
    le_uint16	lookupCount;
    le_uint16	lookupListIndexArray[ANY_NUMBER];
};

struct FeatureListTable
{
    le_uint16			featureCount;
    FeatureRecord		featureRecordArray[ANY_NUMBER];

    const FeatureTable	*getFeatureTable(le_uint16 featureIndex, LETag *featureTag) const;

    const FeatureTable *getFeatureTable(LETag featureTag) const;
};

U_NAMESPACE_END
#endif
