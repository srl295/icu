/*
*******************************************************************************
*
*   Copyright (C) 1999-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  store.c
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2001may25
*   created by: Markus W. Scherer
*
*   Store Unicode normalization data in a memory-mappable file.
*/

#include <stdio.h>
#include <stdlib.h>
#include "unicode/utypes.h"
#include "unicode/uchar.h"
#include "cmemory.h"
#include "cstring.h"
#include "filestrm.h"
#include "unicode/udata.h"
#include "utrie.h"
#include "unewdata.h"
#include "unormimp.h"
#include "gennorm.h"

#ifdef WIN32
#   pragma warning(disable: 4100)
#endif

#define DO_DEBUG_OUT 0

/*
 * The new implementation of the normalization code loads its data from
 * unorm.dat, which is generated with this gennorm tool.
 * The format of that file is described in unormimp.h .
 */

/* file data ---------------------------------------------------------------- */

/* UDataInfo cf. udata.h */
static UDataInfo dataInfo={
    sizeof(UDataInfo),
    0,

    U_IS_BIG_ENDIAN,
    U_CHARSET_FAMILY,
    U_SIZEOF_UCHAR,
    0,

    { 0x4e, 0x6f, 0x72, 0x6d },   /* dataFormat="Norm" */
    { 2, 0, UTRIE_SHIFT, UTRIE_INDEX_SHIFT },   /* formatVersion */
    { 3, 1, 0, 0 }                /* dataVersion (Unicode version) */
};

extern void
setUnicodeVersion(const char *v) {
    UVersionInfo version;
    u_versionFromString(version, v);
    uprv_memcpy(dataInfo.dataVersion, version, 4);
}

static int32_t indexes[_NORM_INDEX_TOP]={ 0 };

/* tool memory helper ------------------------------------------------------- */

/*
 * UToolMemory is used for generic, custom memory management.
 * It is allocated with enough space for count*size bytes starting
 * at array.
 * The array is declared with a union of large data types so
 * that its base address is aligned for any types.
 * If size is a multiple of a data type size, then such items
 * can be safely allocated inside the array, at offsets that
 * are themselves multiples of size.
 */
typedef struct UToolMemory {
    char name[64];
    uint32_t count, size, index;
    union {
        uint32_t u;
        double d;
        void *p;
    } array[1];
} UToolMemory;

static UToolMemory *
utm_open(const char *name, uint32_t count, uint32_t size) {
    UToolMemory *mem=(UToolMemory *)uprv_malloc(sizeof(UToolMemory)+count*size);
    if(mem==NULL) {
        fprintf(stderr, "error: %s - out of memory\n", name);
        exit(U_MEMORY_ALLOCATION_ERROR);
    }
    uprv_strcpy(mem->name, name);
    mem->count=count;
    mem->size=size;
    mem->index=0;
    return mem;
}

static void
utm_close(UToolMemory *mem) {
    if(mem!=NULL) {
        uprv_free(mem);
    }
}



static void *
utm_getStart(UToolMemory *mem) {
    return (char *)mem->array;
}

static void *
utm_alloc(UToolMemory *mem) {
    char *p=(char *)mem->array+mem->index*mem->size;
    if(++mem->index<=mem->count) {
        uprv_memset(p, 0, mem->size);
        return p;
    } else {
        fprintf(stderr, "error: %s - trying to use more than %ld preallocated units\n",
                mem->name, (long)mem->count);
        exit(U_MEMORY_ALLOCATION_ERROR);
    }
}

static void *
utm_allocN(UToolMemory *mem, int32_t n) {
    char *p=(char *)mem->array+mem->index*mem->size;
    if((mem->index+=(uint32_t)n)<=mem->count) {
        uprv_memset(p, 0, n*mem->size);
        return p;
    } else {
        fprintf(stderr, "error: %s - trying to use more than %ld preallocated units\n",
                mem->name, (long)mem->count);
        exit(U_MEMORY_ALLOCATION_ERROR);
    }
}

/* builder data ------------------------------------------------------------- */

typedef void EnumTrieFn(void *context, uint32_t code, Norm *norm);

static UNewTrie normTrie={ 0 }, fcdTrie={ 0 };

static UToolMemory *normMem, *utf32Mem, *extraMem, *combiningTriplesMem;

static Norm *norms;

/*
 * set a flag for each code point that was seen in decompositions -
 * avoid to decompose ones that have not been used before
 */
static uint32_t haveSeenFlags[256];

static uint32_t combiningCPs[2000];
static uint16_t combiningIndexes[2000];
static uint16_t combineFwdTop=0, combineBothTop=0, combineBackTop=0;

typedef struct CombiningTriple {
    uint16_t leadIndex, trailIndex;
    uint32_t lead, trail, combined;
} CombiningTriple;

/* 15b in the combining index -> <=0x8000 uint16_t values in the combining table */
static uint16_t combiningTable[0x8000];
static uint16_t combiningTableTop=0;

extern void
init() {
    /* initialize the two tries */
    if(NULL==utrie_open(&normTrie, NULL, 30000, 0, FALSE)) {
        fprintf(stderr, "error: failed to initialize tries\n");
        exit(U_MEMORY_ALLOCATION_ERROR);
    }

    /* allocate Norm structures and reset the first one */
    normMem=utm_open("gennorm normalization structs", 20000, sizeof(Norm));
    norms=utm_alloc(normMem);

    /* allocate UTF-32 string memory */
    utf32Mem=utm_open("gennorm UTF-32 strings", 30000, 4);

    /* reset all "have seen" flags */
    uprv_memset(haveSeenFlags, 0, sizeof(haveSeenFlags));

    /* allocate extra data memory for UTF-16 decomposition strings and other values */
    extraMem=utm_open("gennorm extra 16-bit memory", _NORM_EXTRA_INDEX_TOP, 2);

    /* allocate temporary memory for combining triples */
    combiningTriplesMem=utm_open("gennorm combining triples", 0x4000, sizeof(CombiningTriple));

    /* set the minimum code points for no/maybe quick check values to the end of the BMP */
    indexes[_NORM_INDEX_MIN_NFC_NO_MAYBE]=0xffff;
    indexes[_NORM_INDEX_MIN_NFKC_NO_MAYBE]=0xffff;
    indexes[_NORM_INDEX_MIN_NFD_NO_MAYBE]=0xffff;
    indexes[_NORM_INDEX_MIN_NFKD_NO_MAYBE]=0xffff;
}

/*
 * get or create a Norm unit;
 * get or create the intermediate trie entries for it as well
 */
static Norm *
createNorm(uint32_t code) {
    Norm *p;
    uint32_t i;

    i=utrie_get32(&normTrie, (UChar32)code, NULL);
    if(i!=0) {
        p=norms+i;
    } else {
        /* allocate Norm */
        p=(Norm *)utm_alloc(normMem);
        if(!utrie_set32(&normTrie, (UChar32)code, (uint32_t)(p-norms))) {
            fprintf(stderr, "error: too many normalization entries\n");
            exit(U_BUFFER_OVERFLOW_ERROR);
        }
    }
    return p;
}

/* get an existing Norm unit */
static Norm *
getNorm(uint32_t code) {
    uint32_t i;

    i=utrie_get32(&normTrie, (UChar32)code, NULL);
    if(i==0) {
        return NULL;
    }
    return norms+i;
}

/* get the canonical combining class of a character */
static uint8_t
getCCFromCP(uint32_t code) {
    Norm *norm=getNorm(code);
    if(norm==NULL) {
        return 0;
    } else {
        return norm->udataCC;
    }
}

/*
 * enumerate all code points with their Norm structs and call a function for each
 * return the number of code points with data
 */
static uint32_t
enumTrie(EnumTrieFn *fn, void *context) {
    uint32_t count, i;
    UChar32 code;
    UBool isInBlockZero;

    for(code=0; code<=0x10ffff;) {
        i=utrie_get32(&normTrie, code, &isInBlockZero);
        if(isInBlockZero) {
            code+=UTRIE_DATA_BLOCK_LENGTH;
        } else {
            if(i!=0) {
                fn(context, (uint32_t)code, norms+i);
                ++count;
            }
            ++code;
        }
    }
    return count;
}

static void
setHaveSeenString(const uint32_t *s, int32_t length) {
    uint32_t c;

    while(length>0) {
        c=*s++;
        haveSeenFlags[(c>>5)&0xff]|=(1<<(c&0x1f));
        --length;
    }
}

#define HAVE_SEEN(c) (haveSeenFlags[((c)>>5)&0xff]&(1<<((c)&0x1f)))

/* handle combining data ---------------------------------------------------- */

static void
addCombiningCP(uint32_t code, uint8_t flags) {
    uint32_t newEntry;
    uint16_t i;

    newEntry=code|((uint32_t)flags<<24);

    /* search for this code point */
    for(i=0; i<combineBackTop; ++i) {
        if(code==(combiningCPs[i]&0xffffff)) {
            /* found it */
            if(newEntry==combiningCPs[i]) {
                return; /* no change */
            }

            /* combine the flags, remove the old entry from the old place, and insert the new one */
            newEntry|=combiningCPs[i];
            if(i!=--combineBackTop) {
                uprv_memmove(combiningCPs+i, combiningCPs+i+1, (combineBackTop-i)*4);
            }
            if(i<combineBothTop) {
                --combineBothTop;
            }
            if(i<combineFwdTop) {
                --combineFwdTop;
            }
            break;
        }
    }

    /* not found or modified, insert it */
    if(combineBackTop>=sizeof(combiningCPs)/4) {
        fprintf(stderr, "error: gennorm combining code points - trying to use more than %ld units\n",
                (long)(sizeof(combiningCPs)/4));
        exit(U_MEMORY_ALLOCATION_ERROR);
    }

    /* set i to the insertion point */
    flags=(uint8_t)(newEntry>>24);
    if(flags==1) {
        i=combineFwdTop++;
        ++combineBothTop;
    } else if(flags==3) {
        i=combineBothTop++;
    } else /* flags==2 */ {
        i=combineBackTop;
    }

    /* move the following code points up one and insert newEntry at i */
    if(i<combineBackTop) {
        uprv_memmove(combiningCPs+i+1, combiningCPs+i, (combineBackTop-i)*4);
    }
    combiningCPs[i]=newEntry;

    /* finally increment the total counter */
    ++combineBackTop;
}

static uint16_t
findCombiningCP(uint32_t code, UBool isLead) {
    uint16_t i, limit;

    if(isLead) {
        i=0;
        limit=combineBothTop;
    } else {
        i=combineFwdTop;
        limit=combineBackTop;
    }

    /* search for this code point */
    for(; i<limit; ++i) {
        if(code==(combiningCPs[i]&0xffffff)) {
            /* found it */
            return i;
        }
    }

    /* not found */
    return 0xffff;
}

static void
addCombiningTriple(uint32_t lead, uint32_t trail, uint32_t combined) {
    CombiningTriple *triple;

    /*
     * set combiningFlags for the two code points
     * do this after decomposition so that getNorm() above returns NULL
     * if we do not have actual sub-decomposition data for the initial NFD here
     */
    createNorm(lead)->combiningFlags|=1;    /* combines forward */
    createNorm(trail)->combiningFlags|=2;    /* combines backward */

    addCombiningCP(lead, 1);
    addCombiningCP(trail, 2);

    triple=(CombiningTriple *)utm_alloc(combiningTriplesMem);
    triple->lead=lead;
    triple->trail=trail;
    triple->combined=combined;
}

static int
compareTriples(const void *l, const void *r) {
    int diff;
    diff=(int)((CombiningTriple *)l)->leadIndex-
         (int)((CombiningTriple *)r)->leadIndex;
    if(diff==0) {
        diff=(int)((CombiningTriple *)l)->trailIndex-
             (int)((CombiningTriple *)r)->trailIndex;
    }
    return diff;
}

static void
processCombining() {
    CombiningTriple *triples;
    uint16_t *p;
    uint32_t combined;
    uint16_t i, j, count, tableTop, finalIndex, combinesFwd;

    triples=utm_getStart(combiningTriplesMem);

    /* add lead and trail indexes to the triples for sorting */
    count=(uint16_t)combiningTriplesMem->index;
    for(i=0; i<count; ++i) {
        /* findCombiningCP() must always find the code point */
        triples[i].leadIndex=findCombiningCP(triples[i].lead, TRUE);
        triples[i].trailIndex=findCombiningCP(triples[i].trail, FALSE);
    }

    /* sort them by leadIndex, trailIndex */
    qsort(triples, count, sizeof(CombiningTriple), compareTriples);

    /* calculate final combining indexes and store them in the Norm entries */
    tableTop=0;
    j=0; /* triples counter */

    /* first, combining indexes of fwd/both characters are indexes into the combiningTable */
    for(i=0; i<combineBothTop; ++i) {
        /* start a new table */

        /* assign combining index */
        createNorm(combiningCPs[i]&0xffffff)->combiningIndex=combiningIndexes[i]=tableTop;

        /* calculate the length of the combining data for this lead code point in the combiningTable */
        while(j<count && i==triples[j].leadIndex) {
            /* count 2 to 3 16-bit units per composition entry (back-index, code point) */
            combined=triples[j++].combined;
            if(combined<=0x1fff) {
                tableTop+=2;
            } else {
                tableTop+=3;
            }
        }
    }

    /* second, combining indexes of back-only characters are simply incremented from here to be unique */
    finalIndex=tableTop;
    for(; i<combineBackTop; ++i) {
        createNorm(combiningCPs[i]&0xffffff)->combiningIndex=combiningIndexes[i]=finalIndex++;
    }

    /* it must be finalIndex<=0x8000 because bit 15 is used in combiningTable as an end-for-this-lead marker */
    if(finalIndex>0x8000) {
        fprintf(stderr, "error: gennorm combining table - trying to use %u units, more than the %ld units available\n",
                tableTop, (long)(sizeof(combiningTable)/4));
        exit(U_MEMORY_ALLOCATION_ERROR);
    }

    combiningTableTop=tableTop;

    /* store the combining data in the combiningTable, with the final indexes from above */
    p=combiningTable;
    j=0; /* triples counter */

    /*
     * this is essentially the same loop as above, but
     * it writes the table data instead of calculating and setting the final indexes;
     * it is necessary to have two passes so that all the final indexes are known before
     * they are written into the table
     */
    for(i=0; i<combineBothTop; ++i) {
        /* start a new table */

        combined=0; /* avoid compiler warning */

        /* store the combining data for this lead code point in the combiningTable */
        while(j<count && i==triples[j].leadIndex) {
            finalIndex=combiningIndexes[triples[j].trailIndex];
            combined=triples[j++].combined;

            /* is combined a starter? (i.e., cc==0 && combines forward) */
            combinesFwd=(uint16_t)((getNorm(combined)->combiningFlags&1)<<13);

            *p++=finalIndex;
            if(combined<=0x1fff) {
                *p++=(uint16_t)(combinesFwd|combined);
            } else if(combined<=0xffff) {
                *p++=(uint16_t)(0x8000|combinesFwd);
                *p++=(uint16_t)combined;
            } else {
                *p++=(uint16_t)(0xc000|combinesFwd|((combined-0x10000)>>10));
                *p++=(uint16_t)(0xdc00|(combined&0x3ff));
            }
        }

        /* set a marker on the last final trail index in this lead's table */
        if(combined<=0x1ffff) {
            *(p-2)|=0x8000;
        } else {
            *(p-3)|=0x8000;
        }
    }

    /* post condition: tableTop==(p-combiningTable) */
}

/* processing incoming normalization data ----------------------------------- */

/*
 * decompose the one decomposition further, may generate two decompositions
 * apply all previous characters' decompositions to this one
 */
static void
decompStoreNewNF(uint32_t code, Norm *norm) {
    uint32_t nfd[40], nfkd[40];
    uint32_t *s32;
    Norm *p;
    uint32_t c;
    int32_t i, length;
    uint8_t lenNFD=0, lenNFKD=0;
    UBool changedNFD=FALSE, changedNFKD=FALSE;

    if((length=norm->lenNFD)!=0) {
        /* always allocate the original string */
        changedNFD=TRUE;
        s32=norm->nfd;
    } else if((length=norm->lenNFKD)!=0) {
        /* always allocate the original string */
        changedNFKD=TRUE;
        s32=norm->nfkd;
    } else {
        /* no decomposition here, nothing to do */
        return;
    }

    /* decompose each code point */
    for(i=0; i<length; ++i) {
        c=s32[i];
        p=getNorm(c);
        if(p==NULL) {
            /* no data, no decomposition */
            nfd[lenNFD++]=c;
            nfkd[lenNFKD++]=c;
            continue;
        }

        /* canonically decompose c */
        if(changedNFD) {
            if(p->lenNFD!=0) {
                uprv_memcpy(nfd+lenNFD, p->nfd, p->lenNFD*4);
                lenNFD+=p->lenNFD;
            } else {
                nfd[lenNFD++]=c;
            }
        }

        /* compatibility-decompose c */
        if(p->lenNFKD!=0) {
            uprv_memcpy(nfkd+lenNFKD, p->nfkd, p->lenNFKD*4);
            lenNFKD+=p->lenNFKD;
            changedNFKD=TRUE;
        } else if(p->lenNFD!=0) {
            uprv_memcpy(nfkd+lenNFKD, p->nfd, p->lenNFD*4);
            lenNFKD+=p->lenNFD;
            changedNFKD=TRUE;
        } else {
            nfkd[lenNFKD++]=c;
        }
    }

    /* assume that norm->lenNFD==1 or ==2 */
    if(norm->lenNFD==2 && !(norm->combiningFlags&0x80)) {
        addCombiningTriple(s32[0], s32[1], code);
    }

    if(changedNFD) {
        if(lenNFD!=0) {
            s32=utm_allocN(utf32Mem, lenNFD);
            uprv_memcpy(s32, nfd, lenNFD*4);
        } else {
            s32=NULL;
        }
        norm->lenNFD=lenNFD;
        norm->nfd=s32;
        setHaveSeenString(nfd, lenNFD);
    }
    if(changedNFKD) {
        if(lenNFKD!=0) {
            s32=utm_allocN(utf32Mem, lenNFKD);
            uprv_memcpy(s32, nfkd, lenNFKD*4);
        } else {
            s32=NULL;
        }
        norm->lenNFKD=lenNFKD;
        norm->nfkd=s32;
        setHaveSeenString(nfkd, lenNFKD);
    }
}

typedef struct DecompSingle {
    uint32_t c;
    Norm *norm;
} DecompSingle;

/*
 * apply this one character's decompositions (there is at least one!) to
 * all previous characters' decompositions to decompose them further
 */
static void
decompWithSingleFn(void *context, uint32_t code, Norm *norm) {
    uint32_t nfd[40], nfkd[40];
    uint32_t *s32;
    DecompSingle *me=(DecompSingle *)context;
    uint32_t c, myC;
    int32_t i, length;
    uint8_t lenNFD=0, lenNFKD=0, myLenNFD, myLenNFKD;
    UBool changedNFD=FALSE, changedNFKD=FALSE;

    /* get the new character's data */
    myC=me->c;
    myLenNFD=me->norm->lenNFD;
    myLenNFKD=me->norm->lenNFKD;
    /* assume that myC has at least one decomposition */

    if((length=norm->lenNFD)!=0 && myLenNFD!=0) {
        /* apply NFD(myC) to norm->nfd */
        s32=norm->nfd;
        for(i=0; i<length; ++i) {
            c=s32[i];
            if(c==myC) {
                uprv_memcpy(nfd+lenNFD, me->norm->nfd, myLenNFD*4);
                lenNFD+=myLenNFD;
                changedNFD=TRUE;
            } else {
                nfd[lenNFD++]=c;
            }
        }
    }

    if((length=norm->lenNFKD)!=0) {
        /* apply NFD(myC) and NFKD(myC) to norm->nfkd */
        s32=norm->nfkd;
        for(i=0; i<length; ++i) {
            c=s32[i];
            if(c==myC) {
                if(myLenNFKD!=0) {
                    uprv_memcpy(nfkd+lenNFKD, me->norm->nfkd, myLenNFKD*4);
                    lenNFKD+=myLenNFKD;
                } else /* assume myLenNFD!=0 */ {
                    uprv_memcpy(nfkd+lenNFKD, me->norm->nfd, myLenNFD*4);
                    lenNFKD+=myLenNFD;
                }
                changedNFKD=TRUE;
            } else {
                nfkd[lenNFKD++]=c;
            }
        }
    } else if((length=norm->lenNFD)!=0 && myLenNFKD!=0) {
        /* apply NFKD(myC) to norm->nfd, forming a new norm->nfkd */
        s32=norm->nfd;
        for(i=0; i<length; ++i) {
            c=s32[i];
            if(c==myC) {
                uprv_memcpy(nfkd+lenNFKD, me->norm->nfkd, myLenNFKD*4);
                lenNFKD+=myLenNFKD;
                changedNFKD=TRUE;
            } else {
                nfkd[lenNFKD++]=c;
            }
        }
    }

    /* set the new decompositions, forget the old ones */
    if(changedNFD) {
        if(lenNFD!=0) {
            if(lenNFD>norm->lenNFD) {
                s32=utm_allocN(utf32Mem, lenNFD);
            } else {
                s32=norm->nfd;
            }
            uprv_memcpy(s32, nfd, lenNFD*4);
        } else {
            s32=NULL;
        }
        norm->lenNFD=lenNFD;
        norm->nfd=s32;
    }
    if(changedNFKD) {
        if(lenNFKD!=0) {
            if(lenNFKD>norm->lenNFKD) {
                s32=utm_allocN(utf32Mem, lenNFKD);
            } else {
                s32=norm->nfkd;
            }
            uprv_memcpy(s32, nfkd, lenNFKD*4);
        } else {
            s32=NULL;
        }
        norm->lenNFKD=lenNFKD;
        norm->nfkd=s32;
    }
}

/*
 * process the data for one code point listed in UnicodeData;
 * UnicodeData itself never maps a code point to both NFD and NFKD
 */
extern void
storeNorm(uint32_t code, Norm *norm) {
    DecompSingle decompSingle;
    Norm *p;

    /* copy existing derived normalization properties */
    p=createNorm(code);
    norm->qcFlags=p->qcFlags;
    norm->combiningFlags=p->combiningFlags;

    /* process the decomposition if if there is at one here */
    if((norm->lenNFD|norm->lenNFKD)!=0) {
        /* decompose this one decomposition further, may generate two decompositions */
        decompStoreNewNF(code, norm);

        /* has this code point been used in previous decompositions? */
        if(HAVE_SEEN(code)) {
            /* use this decomposition to decompose other decompositions further */
            decompSingle.c=code;
            decompSingle.norm=norm;
            enumTrie(decompWithSingleFn, &decompSingle);
        }
    }

    /* store the data */
    uprv_memcpy(p, norm, sizeof(Norm));
}

extern void
setQCFlags(uint32_t code, uint8_t qcFlags) {
    createNorm(code)->qcFlags|=qcFlags;

    /* adjust the minimum code point for quick check no/maybe */
    if(code<0xffff) {
        if((qcFlags&_NORM_QC_NFC) && (uint16_t)code<indexes[_NORM_INDEX_MIN_NFC_NO_MAYBE]) {
            indexes[_NORM_INDEX_MIN_NFC_NO_MAYBE]=(uint16_t)code;
        }
        if((qcFlags&_NORM_QC_NFKC) && (uint16_t)code<indexes[_NORM_INDEX_MIN_NFKC_NO_MAYBE]) {
            indexes[_NORM_INDEX_MIN_NFKC_NO_MAYBE]=(uint16_t)code;
        }
        if((qcFlags&_NORM_QC_NFD) && (uint16_t)code<indexes[_NORM_INDEX_MIN_NFD_NO_MAYBE]) {
            indexes[_NORM_INDEX_MIN_NFD_NO_MAYBE]=(uint16_t)code;
        }
        if((qcFlags&_NORM_QC_NFKD) && (uint16_t)code<indexes[_NORM_INDEX_MIN_NFKD_NO_MAYBE]) {
            indexes[_NORM_INDEX_MIN_NFKD_NO_MAYBE]=(uint16_t)code;
        }
    }
}

extern void
setCompositionExclusion(uint32_t code) {
    createNorm(code)->combiningFlags|=0x80;
}

static void
setHangulJamoSpecials() {
    Norm *norm;
    uint32_t c;

    /*
     * Hangul syllables are algorithmically decomposed into Jamos,
     * and Jamos are algorithmically composed into Hangul syllables.
     * The quick check flags are parsed, except for Hangul.
     */

    /* set Jamo L specials */
    for(c=0x1100; c<=0x1112; ++c) {
        norm=createNorm(c);
        norm->specialTag=_NORM_EXTRA_INDEX_TOP+_NORM_EXTRA_JAMO_L;
        norm->combiningFlags=1;
    }

    /* set Jamo V specials */
    for(c=0x1161; c<=0x1175; ++c) {
        norm=createNorm(c);
        norm->specialTag=_NORM_EXTRA_INDEX_TOP+_NORM_EXTRA_JAMO_V;
        norm->combiningFlags=2;
    }

    /* set Jamo T specials */
    for(c=0x11a8; c<=0x11c2; ++c) {
        norm=createNorm(c);
        norm->specialTag=_NORM_EXTRA_INDEX_TOP+_NORM_EXTRA_JAMO_T;
        norm->combiningFlags=2;
    }

    /* set Hangul specials, precompacted */
    norm=(Norm *)utm_alloc(normMem);
    norm->specialTag=_NORM_EXTRA_INDEX_TOP+_NORM_EXTRA_HANGUL;
    norm->qcFlags=_NORM_QC_NFD|_NORM_QC_NFKD;

    if(!utrie_setRange32(&normTrie, 0xac00, 0xd7a4, (uint32_t)(norm-norms), TRUE)) {
        fprintf(stderr, "error: too many normalization entries (setting Hangul)\n");
        exit(U_BUFFER_OVERFLOW_ERROR);
    }
}

/* build runtime structures ------------------------------------------------- */

/* canonically reorder a UTF-32 string; return { leadCC, trailCC } */
static uint16_t
reorderString(uint32_t *s, int32_t length) {
    uint8_t ccs[40];
    uint32_t c;
    int32_t i, j;
    uint8_t cc, prevCC;

    if(length<=0) {
        return 0;
    }

    for(i=0; i<length; ++i) {
        /* get the i-th code point and its combining class */
        c=s[i];
        cc=getCCFromCP(c);
        if(cc!=0 && i!=0) {
            /* it is a combining mark, see if it needs to be moved back */
            j=i;
            do {
                prevCC=ccs[j-1];
                if(prevCC<=cc) {
                    break;  /* found the right place */
                }
                /* move the previous code point here and go back */
                s[j]=s[j-1];
                ccs[j]=prevCC;
            } while(--j!=0);
            s[j]=c;
            ccs[j]=cc;
        } else {
            /* just store the combining class */
            ccs[i]=cc;
        }
    }

    return (uint16_t)(((uint16_t)ccs[0]<<8)|ccs[length-1]);
}

static UBool combineAndQC[64]={ 0 };

/*
 * canonically reorder the up to two decompositions
 * and store the leading and trailing combining classes accordingly
 */
static void
postParseFn(void *context, uint32_t code, Norm *norm) {
    int32_t length;

    /* canonically order the NFD */
    length=norm->lenNFD;
    if(length>0) {
        norm->canonBothCCs=reorderString(norm->nfd, length);
    }

    /* canonically reorder the NFKD */
    length=norm->lenNFKD;
    if(length>0) {
        norm->compatBothCCs=reorderString(norm->nfkd, length);
    }

    /* verify that code has a decomposition if and only if the quick check flags say "no" on NF(K)D */
    if((norm->lenNFD!=0) != ((norm->qcFlags&_NORM_QC_NFD)!=0)) {
        fprintf(stderr, "gennorm warning: U+%04lx has NFD[%d] but quick check 0x%02x\n", (long)code, norm->lenNFD, norm->qcFlags);
    }
    if(((norm->lenNFD|norm->lenNFKD)!=0) != ((norm->qcFlags&(_NORM_QC_NFD|_NORM_QC_NFKD))!=0)) {
        fprintf(stderr, "gennorm warning: U+%04lx has NFD[%d] NFKD[%d] but quick check 0x%02x\n", (long)code, norm->lenNFD, norm->lenNFKD, norm->qcFlags);
    }

    /* ### see which combinations of combiningFlags and qcFlags are used for NFC/NFKC */
    combineAndQC[(norm->qcFlags&0x33)|((norm->combiningFlags&3)<<2)]=1;

    if(norm->combiningFlags&1) {
        if(norm->udataCC!=0) {
            /* illegal - data-derivable composition exclusion */
            fprintf(stderr, "gennorm warning: U+%04lx combines forward but udataCC==%u\n", (long)code, norm->udataCC);
        }
    }
    if(norm->combiningFlags&2) {
        if((norm->qcFlags&0x11)==0) {
            fprintf(stderr, "gennorm warning: U+%04lx combines backward but qcNF?C==0\n", (long)code);
        }
#if 0
        /* occurs sometimes, this one is ok (therefore #if 0) - still here for documentation */
        if(norm->udataCC==0) {
            printf("U+%04lx combines backward but udataCC==0\n", (long)code);
        }
#endif
    }
    if((norm->combiningFlags&3)==3 && beVerbose) {
        printf("U+%04lx combines both ways\n", (long)code);
    }
}

/* ### debug */
static uint32_t countCCSame=0, countCCTrail=0, countCCTwo=0;

static uint32_t
make32BitNorm(Norm *norm) {
    UChar extra[100];
    const Norm *other;
    uint32_t word;
    int32_t i, length, beforeZero=0, count, start;

    /*
     * Check for assumptions:
     *
     * Test that if a "true starter" (cc==0 && NF*C_YES) decomposes,
     * then the decomposition also begins with a true starter.
     */
    if(norm->udataCC==0) {
        /* this is a starter */
        if((norm->qcFlags&_NORM_QC_NFC)==0 && norm->lenNFD>0) {
            /* a "true" NFC starter with a canonical decomposition */
            if( norm->canonBothCCs>=0x100 || /* lead cc!=0 or */
                ((other=getNorm(norm->nfd[0]))!=NULL && (other->qcFlags&_NORM_QC_NFC)!=0) /* nfd[0] not NFC_YES */
            ) {
                fprintf(stderr,
                    "error: true NFC starter canonical decomposition[%u] does not begin\n"
                    "    with a true NFC starter: U+%04lx U+%04lx%s\n",
                    norm->lenNFD, (long)norm->nfd[0], (long)norm->nfd[1],
                    norm->lenNFD<=2 ? "" : " ...");
                exit(U_INVALID_TABLE_FILE);
            }
        }

        if((norm->qcFlags&_NORM_QC_NFKC)==0) {
            if(norm->lenNFKD>0) {
                /* a "true" NFKC starter with a compatibility decomposition */
                if( norm->compatBothCCs>=0x100 || /* lead cc!=0 or */
                    ((other=getNorm(norm->nfkd[0]))!=NULL && (other->qcFlags&_NORM_QC_NFKC)!=0) /* nfkd[0] not NFC_YES */
                ) {
                    fprintf(stderr,
                        "error: true NFKC starter compatibility decomposition[%u] does not begin\n"
                        "    with a true NFKC starter: U+%04lx U+%04lx%s\n",
                        norm->lenNFKD, (long)norm->nfkd[0], (long)norm->nfkd[1],                        norm->lenNFKD<=2 ? "" : " ...");
                    exit(U_INVALID_TABLE_FILE);
                }
            } else if(norm->lenNFD>0) {
                /* a "true" NFKC starter with only a canonical decomposition */
                if( norm->canonBothCCs>=0x100 || /* lead cc!=0 or */
                    ((other=getNorm(norm->nfd[0]))!=NULL && (other->qcFlags&_NORM_QC_NFKC)!=0) /* nfd[0] not NFC_YES */
                ) {
                    fprintf(stderr,
                        "error: true NFKC starter canonical decomposition[%u] does not begin\n"
                        "    with a true NFKC starter: U+%04lx U+%04lx%s\n",
                        norm->lenNFD, (long)norm->nfd[0], (long)norm->nfd[1],
                        norm->lenNFD<=2 ? "" : " ...");
                    exit(U_INVALID_TABLE_FILE);
                }
            }
        }
    }

    /* reset the 32-bit word and set the quick check flags */
    word=norm->qcFlags;

    /* set the UnicodeData combining class */
    word|=(uint32_t)norm->udataCC<<_NORM_CC_SHIFT;

    /* set the combining flag and index */
    if(norm->combiningFlags&3) {
        word|=(uint32_t)(norm->combiningFlags&3)<<6;
    }

    /* set the combining index value into the extra data */
    if(norm->combiningIndex!=0) {
        extra[0]=norm->combiningIndex;
        beforeZero=1;
    }

    count=beforeZero;

    /* write the decompositions */
    if((norm->lenNFD|norm->lenNFKD)!=0) {
        extra[count++]=0; /* set the pieces when available, into extra[beforeZero] */

        length=norm->lenNFD;
        if(length>0) {
            if(norm->canonBothCCs!=0) {
                extra[beforeZero]|=0x80;
                extra[count++]=norm->canonBothCCs;
            }
            start=count;
            for(i=0; i<length; ++i) {
                UTF_APPEND_CHAR_UNSAFE(extra, count, norm->nfd[i]);
            }
            extra[beforeZero]|=(UChar)(count-start); /* set the decomp length as the number of UTF-16 code units */
        }

        length=norm->lenNFKD;
        if(length>0) {
            if(norm->compatBothCCs!=0) {
                extra[beforeZero]|=0x8000;
                extra[count++]=norm->compatBothCCs;
            }
            start=count;
            for(i=0; i<length; ++i) {
                UTF_APPEND_CHAR_UNSAFE(extra, count, norm->nfkd[i]);
            }
            extra[beforeZero]|=(UChar)((count-start)<<8); /* set the decomp length as the number of UTF-16 code units */
        }
    }

    /* allocate and copy the extra data */
    if(count!=0) {
        UChar *p;

        if(norm->specialTag!=0) {
            fprintf(stderr, "error: gennorm - illegal to have both extra data and a special tag (0x%x)\n", norm->specialTag);
            exit(U_ILLEGAL_ARGUMENT_ERROR);
        }

        p=(UChar *)utm_allocN(extraMem, count);
        uprv_memcpy(p, extra, count*2);

        /* set the extra index, offset by beforeZero */
        word|=(uint32_t)(beforeZero+(p-(UChar *)utm_getStart(extraMem)))<<_NORM_EXTRA_SHIFT;
    } else if(norm->specialTag!=0) {
        /* set a special tag instead of an extra index */
        word|=(uint32_t)norm->specialTag<<_NORM_EXTRA_SHIFT;
    }

    return word;
}

/* turn all Norm structs into corresponding 32-bit norm values */
static void
makeAll32() {
    uint32_t *pNormData;
    uint32_t n;
    int32_t i, normLength, count;

    count=(int32_t)normMem->index;
    for(i=0; i<count; ++i) {
        norms[i].value32=make32BitNorm(norms+i);
    }

    pNormData=utrie_getData(&normTrie, &normLength);

    count=0;
    for(i=0; i<normLength; ++i) {
        n=pNormData[i];
        if(0!=(pNormData[i]=norms[n].value32)) {
            ++count;
        }
    }

    if(beVerbose) {
        printf("count of 16-bit extra data: %lu\n", (long)extraMem->index);
        printf("count of (uncompacted) non-zero 32-bit words: %lu\n", (long)count);
        printf("count CC frequencies: same %lu  trail %lu  two %lu\n",
                (long)countCCSame, (long)countCCTrail, (long)countCCTwo);
    }
}

/*
 * extract all Norm.canonBothCCs into the FCD table
 * set 32-bit values to use the common fold and compact functions
 */
static void
makeFCD() {
    uint32_t *pFCDData;
    uint32_t n;
    int32_t i, count, fcdLength;
    uint16_t bothCCs;

    count=(int32_t)normMem->index;
    for(i=0; i<count; ++i) {
        bothCCs=norms[i].canonBothCCs;
        if(bothCCs==0) {
            /* if there are no decomposition cc's then use the udataCC twice */
            bothCCs=norms[i].udataCC;
            bothCCs|=bothCCs<<8;
        }
        norms[i].value32=bothCCs;
    }

    pFCDData=utrie_getData(&fcdTrie, &fcdLength);

    for(i=0; i<fcdLength; ++i) {
        n=pFCDData[i];
        pFCDData[i]=norms[n].value32;
    }
}

/* folding value for normalization: just store the offset (16 bits) if there is any non-0 entry */
static uint32_t U_CALLCONV
getFoldedNormValue(UNewTrie *trie, UChar32 start, int32_t offset) {
    uint32_t value, leadNorm32=0;
    UChar32 limit;
    UBool inBlockZero;

    limit=start+0x400;
    while(start<limit) {
        value=utrie_get32(trie, start, &inBlockZero);
        if(inBlockZero) {
            start+=UTRIE_DATA_BLOCK_LENGTH;
        } else {
            if(value!=0) {
                leadNorm32|=value;
            }
            ++start;
        }
    }

    /* turn multi-bit fields into the worst-case value */
    if(leadNorm32&_NORM_CC_MASK) {
        leadNorm32|=_NORM_CC_MASK;
    }

    /* clean up unnecessarily ored bit fields */
    leadNorm32&=~((uint32_t)0xffffffff<<_NORM_EXTRA_SHIFT);

    if(leadNorm32==0) {
        /* nothing to do (only composition exclusions?) */
        return 0;
    }

    /* add the extra surrogate index, offset by the BMP top, for the new stage 1 location */
    leadNorm32|=(
        (uint32_t)_NORM_EXTRA_INDEX_TOP+
        (uint32_t)((offset-UTRIE_BMP_INDEX_LENGTH)>>UTRIE_SURROGATE_BLOCK_BITS)
    )<<_NORM_EXTRA_SHIFT;

    return leadNorm32;
}

/* folding value for FCD: just store the offset (16 bits) if there is any non-0 entry */
static uint32_t U_CALLCONV
getFoldedFCDValue(UNewTrie *trie, UChar32 start, int32_t offset) {
    uint32_t value;
    UChar32 limit;
    UBool inBlockZero;

    limit=start+0x400;
    while(start<limit) {
        value=utrie_get32(trie, start, &inBlockZero);
        if(inBlockZero) {
            start+=UTRIE_DATA_BLOCK_LENGTH;
        } else if(value!=0) {
            return (uint32_t)offset;
        } else {
            ++start;
        }
    }
    return 0;
}

extern void
processData() {
#if 0
    uint16_t i;
#endif

    processCombining();

    /* canonically reorder decompositions and assign combining classes for decompositions */
    enumTrie(postParseFn, NULL);

#if 0
    for(i=1; i<64; ++i) {
        if(combineAndQC[i]) {
            printf("combiningFlags==0x%02x  qcFlags(NF?C)==0x%02x\n", (i&0xc)>>2, i&0x33);
        }
    }
#endif

    /* add hangul/jamo specials */
    setHangulJamoSpecials();

    /* clone the normalization trie to make the FCD trie */
    if(NULL==utrie_clone(&fcdTrie, &normTrie, NULL, 0)) {
        fprintf(stderr, "error: unable to clone the normalization trie\n");
        exit(U_MEMORY_ALLOCATION_ERROR);
    }

    /* --- finalize data for quick checks & normalization --- */

    /* turn the Norm structs (stage2, norms) into 32-bit data words (norm32Table) */
    makeAll32();

    /* --- finalize data for FCD checks: fcdStage1/fcdTable --- */

    /* FCD data: take Norm.canonBothCCs and store them in the FCD table */
    makeFCD();

    if(beVerbose) {
#if 0
        printf("number of stage 2 entries: %ld\n", stage2Mem->index);
        printf("size of stage 1 (BMP) & 2 (uncompacted) + extra data: %ld bytes\n", _NORM_STAGE_1_BMP_COUNT*2+stage2Mem->index*4+extraMem->index*2);
#endif
        printf("combining CPs tops: fwd %u  both %u  back %u\n", combineFwdTop, combineBothTop, combineBackTop);
        printf("combining table count: %u\n", combiningTableTop);
    }
}

extern void
generateData(const char *dataDir) {
    static uint8_t normTrieBlock[100000], fcdTrieBlock[100000];

    UNewDataMemory *pData;
    UErrorCode errorCode=U_ZERO_ERROR;
    int32_t size, normTrieSize, fcdTrieSize, dataLength;

    normTrieSize=utrie_serialize(&normTrie, normTrieBlock, sizeof(normTrieBlock), getFoldedNormValue, FALSE, &errorCode);
    if(U_FAILURE(errorCode)) {
        fprintf(stderr, "error: utrie_serialize(normalization properties) failed, %s\n", u_errorName(errorCode));
        exit(errorCode);
    }

    fcdTrieSize=utrie_serialize(&fcdTrie, fcdTrieBlock, sizeof(fcdTrieBlock), getFoldedFCDValue, TRUE, &errorCode);
    if(U_FAILURE(errorCode)) {
        fprintf(stderr, "error: utrie_serialize(FCD data) failed, %s\n", u_errorName(errorCode));
        exit(errorCode);
    }

    size=
        _NORM_INDEX_TOP*4+
        normTrieSize+
        extraMem->index*2+
        combiningTableTop*2+
        fcdTrieSize;

    if(beVerbose) {
        printf("size of " DATA_NAME "." DATA_TYPE " contents: %ld bytes\n", (long)size);
    }

    indexes[_NORM_INDEX_TRIE_SIZE]=normTrieSize;
    indexes[_NORM_INDEX_UCHAR_COUNT]=(uint16_t)extraMem->index;

    indexes[_NORM_INDEX_COMBINE_DATA_COUNT]=combiningTableTop;
    indexes[_NORM_INDEX_COMBINE_FWD_COUNT]=combineFwdTop;
    indexes[_NORM_INDEX_COMBINE_BOTH_COUNT]=(uint16_t)(combineBothTop-combineFwdTop);
    indexes[_NORM_INDEX_COMBINE_BACK_COUNT]=(uint16_t)(combineBackTop-combineBothTop);

    /* the quick check minimum code points are already set */

    indexes[_NORM_INDEX_FCD_TRIE_SIZE]=fcdTrieSize;

    /* write the data */
    pData=udata_create(dataDir, DATA_TYPE, DATA_NAME, &dataInfo,
                       haveCopyright ? U_COPYRIGHT_STRING : NULL, &errorCode);
    if(U_FAILURE(errorCode)) {
        fprintf(stderr, "gennorm: unable to create the output file, error %d\n", errorCode);
        exit(errorCode);
    }

    udata_writeBlock(pData, indexes, sizeof(indexes));
    udata_writeBlock(pData, normTrieBlock, normTrieSize);
    udata_writeBlock(pData, utm_getStart(extraMem), extraMem->index*2);
    udata_writeBlock(pData, combiningTable, combiningTableTop*2);
    udata_writeBlock(pData, fcdTrieBlock, fcdTrieSize);

    /* finish up */
    dataLength=udata_finish(pData, &errorCode);
    if(U_FAILURE(errorCode)) {
        fprintf(stderr, "gennorm: error %d writing the output file\n", errorCode);
        exit(errorCode);
    }

    if(dataLength!=size) {
        fprintf(stderr, "gennorm error: data length %ld != calculated size %ld\n",
            (long)dataLength, (long)size);
        exit(U_INTERNAL_PROGRAM_ERROR);
    }
}

extern void
cleanUpData(void) {
    utm_close(normMem);
    utm_close(utf32Mem);
    utm_close(extraMem);
    utm_close(combiningTriplesMem);
    utrie_close(&normTrie);
    utrie_close(&fcdTrie);
}

/*
 * Hey, Emacs, please set the following:
 *
 * Local Variables:
 * indent-tabs-mode: nil
 * End:
 *
 */
