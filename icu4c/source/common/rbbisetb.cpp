//
//  rbbisetb.cpp
//
/*
**********************************************************************
*   Copyright (C) 2002 International Business Machines Corporation   *
*   and others. All rights reserved.                                 *
**********************************************************************
*/
//
//  RBBISetBuilder   Handles processing of Unicode Sets from RBBI rules
//                   (part of the rule building process.)
//
//      Starting with the rules parse tree from the scanner,
//
//                   -  Enumerate the set of UnicodeSets that are referenced
//                      by the RBBI rules.
//                   -  compute a set of non-overlapping character ranges
//                      with all characters within a range belonging to the same
//                      set of input uniocde sets.
//                   -  Derive a set of non-overlapping UnicodeSet (like things)
//                      that will correspond to columns in the state table for
//                      the RBBI execution engine.  All characters within one
//                      of these sets belong to the same set of the original
//                      UnicodeSets from the user's rules.
//                   -  construct the trie table that maps input characters
//                      to the index of the matching non-overlapping set of set from
//                      the previous step.
//

#include "unicode/uniset.h"
#include "utrie.h"
#include "cmemory.h"
#include "uvector.h"
#include "assert.h"
#include <stdio.h>

#include "rbbisetb.h"
#include "rbbinode.h"


U_NAMESPACE_BEGIN

const char RBBISetBuilder::fgClassID=0;

//------------------------------------------------------------------------
//
//   Constructor
//
//------------------------------------------------------------------------
RBBISetBuilder::RBBISetBuilder(RBBIRuleBuilder *rb)
{
    fRB             = rb;
    fStatus         = rb->fStatus;
    fRangeList      = 0;
    fTrie           = 0;
    fTrieSize       = 0;
    fGroupCount     = 0;
}


//------------------------------------------------------------------------
//
//   Destructor
//
//------------------------------------------------------------------------
RBBISetBuilder::~RBBISetBuilder()
{
    RangeDescriptor   *nextRangeDesc;

    // Walk through & delete the linked list of RangeDescriptors
    for (nextRangeDesc = fRangeList; nextRangeDesc!=NULL;) {
        RangeDescriptor *r = nextRangeDesc;
        nextRangeDesc      = r->fNext;
        delete r;
    }

    utrie_close(fTrie);
}




//------------------------------------------------------------------------
//
//   getFoldedRBBIValue        Call-back function used during building of Trie table.
//                             Folding value: just store the offset (16 bits)
//                             if there is any non-0 entry.
//                             (It'd really be nice if the Trie builder would provide a
//                             simple default, so this function could go away from here.)
//
//------------------------------------------------------------------------
/* folding value: just store the offset (16 bits) if there is any non-0 entry */
U_CAPI uint32_t U_EXPORT2
getFoldedRBBIValue(UNewTrie *trie, UChar32 start, int32_t offset) {
    uint32_t value;
    UChar32 limit;
    UBool inBlockZero;

    limit=start+0x400;
    while(start<limit) {
        value=utrie_get32(trie, start, &inBlockZero);
        if(inBlockZero) {
            start+=UTRIE_DATA_BLOCK_LENGTH;
        } else if(value!=0) {
            return (uint32_t)(offset|0x8000);
        } else {
            ++start;
        }
    }
    return 0;
}



/* if bit 15 is set, then the folding offset is in bits 14..0 of the 16-bit trie result */
static int32_t U_CALLCONV
getFoldingRBBIOffset(uint32_t data) {
    if(data&0x8000) {
        return (int32_t)(data&0x7fff);
    } else {
        return 0;
    }
}




//------------------------------------------------------------------------
//
//   build          Build the list of non-overlapping character ranges
//                  from the Unicode Sets.
//
//------------------------------------------------------------------------
void RBBISetBuilder::build() {
    RBBINode        *usetNode;
    RangeDescriptor *rlRange;

    if (fRB->fDebugEnv && strstr(fRB->fDebugEnv, "usets")) {printSets();}

    //
    //  Initialize the process by creating a single range encompassing all characters
    //  that is in no sets.
    //
    fRangeList                = new RangeDescriptor(*fStatus);
    fRangeList->fStartChar    = 0;
    fRangeList->fEndChar      = 0x10ffff;


    //
    //  Find the set of non-overlapping ranges of characters
    //
    for (usetNode=fRB->fSetsListHead; usetNode!=NULL; usetNode=usetNode->fRightChild) {
        UnicodeSet      *inputSet             = usetNode->fInputSet;
        int32_t          inputSetRangeCount   = inputSet->getRangeCount();
        int              inputSetRangeIndex   = 0;
                         rlRange              = fRangeList;

        for (;;) {
            if (inputSetRangeIndex >= inputSetRangeCount) {
                break;
            }
            UChar32      inputSetRangeBegin  = inputSet->getRangeStart(inputSetRangeIndex);
            UChar32      inputSetRangeEnd    = inputSet->getRangeEnd(inputSetRangeIndex);

            // skip over ranges from the range list that are completely
            //   below the current range from the input unicode set.
            while (rlRange->fEndChar < inputSetRangeBegin) {
                rlRange = rlRange->fNext;
            }

            // If the start of the range from the range list is before with
            //   the start of the range from the unicode set, split the range list range
            //   in two, with one part being before (wholly outside of) the unicode set
            //   and the other containing the rest.
            //   Then continue the loop; the post-split current range will then be skipped
            //     over
            if (rlRange->fStartChar < inputSetRangeBegin) {
                rlRange->split(inputSetRangeBegin, *fStatus);
                continue;
            }

            // Same thing at the end of the ranges...
            // If the end of the range from the range list doesn't coincide with
            //   the end of the range from the unicode set, split the range list
            //   range in two.  The first part of the split range will be
            //   wholly inside the Unicode set.
            if (rlRange->fEndChar > inputSetRangeEnd) {
                rlRange->split(inputSetRangeEnd+1, *fStatus);
            }

            // The current rlRange is now entirely within the UnicodeSet range.
            // Add this unicode set to the list of sets for this rlRange
            if (rlRange->fIncludesSets->indexOf(usetNode) == -1) {
                rlRange->fIncludesSets->addElement(usetNode, *fStatus);
            }

            // Advance over ranges that we are finished with.
            if (inputSetRangeEnd == rlRange->fEndChar) {
                inputSetRangeIndex++;
            }
            rlRange = rlRange->fNext;
        }
    }

    if (fRB->fDebugEnv && strstr(fRB->fDebugEnv, "range")) { printRanges();}

    //
    //  Group the above ranges, with each group consisting of one or more
    //    ranges that are in exactly the same set of original UnicodeSets.
    //    The groups are numbered, and these group numbers are the set of
    //    input symbols recognized by the run-time state machine.
    //
    RangeDescriptor *rlSearchRange;
    for (rlRange = fRangeList; rlRange!=0; rlRange=rlRange->fNext) {
        for (rlSearchRange=fRangeList; rlSearchRange != rlRange; rlSearchRange=rlSearchRange->fNext) {
            if (rlRange->fIncludesSets->equals(*rlSearchRange->fIncludesSets)) {
                rlRange->fNum = rlSearchRange->fNum;
                break;
            }
        }
        if (rlRange->fNum == 0) {
            fGroupCount ++;
            rlRange->fNum = fGroupCount;
            rlRange->setDictionaryFlag();
            addValToSets(rlRange->fIncludesSets, fGroupCount);
        }
    }

    if (fRB->fDebugEnv && strstr(fRB->fDebugEnv, "rgroup")) {printRangeGroups();}
    if (fRB->fDebugEnv && strstr(fRB->fDebugEnv, "esets")) {printSets();}

    //
    // Build the Trie table for mapping UChar32 values to the corresponding
    //   range group number
    //
    fTrie = utrie_open(NULL,    //  Pre-existing trie to be filled in
                      NULL,    //  Data array  (utrie will allocate one)
                      100000,  //  Max Data Length
                      0,       //  Initial value for all code points
                      TRUE);   //  Keep Latin 1 in separately


    for (rlRange = fRangeList; rlRange!=0; rlRange=rlRange->fNext) {
        utrie_setRange32(fTrie, rlRange->fStartChar, rlRange->fEndChar+1, rlRange->fNum, TRUE);
    }
}



//-----------------------------------------------------------------------------------
//
//  getTrieSize()    Return the size that will be required to serialize the Trie.
//
//-----------------------------------------------------------------------------------
int32_t RBBISetBuilder::getTrieSize() {
    fTrieSize  = utrie_serialize(fTrie,
                                    NULL,                // Buffer
                                    0,                   // Capacity
                                    getFoldedRBBIValue,
                                    TRUE,                // Reduce to 16 bits
                                    fStatus);
    // printf("Trie table size is %d\n", trieSize);
    return fTrieSize;
}


//-----------------------------------------------------------------------------------
//
//  serializeTrie()   Put the serialized trie at the specified address.
//                    Trust the caller to have given us enough memory.
//                    getTrieSize() MUST be called first.
//
//-----------------------------------------------------------------------------------
void RBBISetBuilder::serializeTrie(uint8_t *where) {
utrie_serialize(fTrie,
                where,                   // Buffer
                fTrieSize,               // Capacity
                getFoldedRBBIValue,
                TRUE,                    // Reduce to 16 bits
                fStatus);
}

//------------------------------------------------------------------------
//
//  addValToSets     Add a runtime-mapped input value to each uset from a
//                   list of uset nodes.
//                   For each of the original Unicode sets - which correspond
//                   directly to uset nodes - a logically equivalent expression
//                   is constructed in terms of the remapped runtime input
//                   symbol set.  This function adds one runtime input symbol to
//                   a list of sets.
//
//                   The "logically equivalent expression" is the tree for an
//                   or-ing together of all of the symbols that go into the set.
//
//------------------------------------------------------------------------
void  RBBISetBuilder::addValToSets(UVector *sets, uint32_t val) {
    int32_t       ix;

    for (ix=0; ix<sets->size(); ix++) {
        RBBINode *usetNode = (RBBINode *)sets->elementAt(ix);
        RBBINode *leafNode = new RBBINode(RBBINode::leafChar);
        leafNode->fVal = (unsigned short)val;
        if (usetNode->fLeftChild == NULL) {
            usetNode->fLeftChild = leafNode;
            leafNode->fParent    = usetNode;
        } else {
            // There are already input symbols present for this set.
            // Set up an OR node, with the previous stuff as the left child
            //   and the new value as the right child.
            RBBINode *orNode = new RBBINode(RBBINode::opOr);
            orNode->fLeftChild  = usetNode->fLeftChild;
            orNode->fRightChild = leafNode;
            orNode->fLeftChild->fParent  = orNode;
            orNode->fRightChild->fParent = orNode;
            usetNode->fLeftChild = orNode;
            orNode->fParent = usetNode;
        }
    }
}



//------------------------------------------------------------------------
//
//   getNumOutputSets
//
//------------------------------------------------------------------------
int32_t  RBBISetBuilder::getNumCharCategories() {
    return fGroupCount + 1;
}



//------------------------------------------------------------------------
//
//   printRanges        A debugging function.
//                      dump out all of the range definitions.
//
//------------------------------------------------------------------------
void RBBISetBuilder::printRanges() {
    RangeDescriptor       *rlRange;
    int                    i;

    printf("\n\n Nonoverlapping Ranges ...\n");
    for (rlRange = fRangeList; rlRange!=0; rlRange=rlRange->fNext) {
        printf("%2i  %4x-%4x  ", rlRange->fNum, rlRange->fStartChar, rlRange->fEndChar);

        for (i=0; i<rlRange->fIncludesSets->size(); i++) {
            RBBINode       *usetNode    = (RBBINode *)rlRange->fIncludesSets->elementAt(i);
            UnicodeString   setName = "anon";   //  TODO:  no string literals.
            RBBINode       *setRef = usetNode->fParent;
            if (setRef != NULL) {
                RBBINode *varRef = setRef->fParent;
                if (varRef != NULL  &&  varRef->fType == RBBINode::varRef) {
                    setName = varRef->fText;
                }
            }
            RBBINode::printUnicodeString(setName); printf("  ");
        }
        printf("\n");
    }
}


//------------------------------------------------------------------------
//
//   printRangeGroups     A debugging function.
//                        dump out all of the range groups.
//
//------------------------------------------------------------------------
void RBBISetBuilder::printRangeGroups() {
    RangeDescriptor       *rlRange;
    RangeDescriptor       *tRange;
    int                    i;
    int                    lastPrintedGroupNum = 0;

    printf("\nRanges grouped by Unicode Set Membership...\n");
    for (rlRange = fRangeList; rlRange!=0; rlRange=rlRange->fNext) {
        int groupNum = rlRange->fNum & 0xbfff;
        if (groupNum > lastPrintedGroupNum) {
            lastPrintedGroupNum = groupNum;
            printf("%2i  ", groupNum);

            if (rlRange->fNum & 0x4000) { printf(" <DICT> ");};

            for (i=0; i<rlRange->fIncludesSets->size(); i++) {
                RBBINode       *usetNode    = (RBBINode *)rlRange->fIncludesSets->elementAt(i);
                UnicodeString   setName = "anon";
                RBBINode       *setRef = usetNode->fParent;
                if (setRef != NULL) {
                    RBBINode *varRef = setRef->fParent;
                    if (varRef != NULL  &&  varRef->fType == RBBINode::varRef) {
                        setName = varRef->fText;
                    }
                }
                RBBINode::printUnicodeString(setName); printf(" ");
            }

            i = 0;
            for (tRange = rlRange; tRange != 0; tRange = tRange->fNext) {
                if (tRange->fNum == rlRange->fNum) {
                    if (i++ % 5 == 0) {
                        printf("\n    ");
                    }
                    printf("  %05x-%05x", tRange->fStartChar, tRange->fEndChar);
                }
            }
            printf("\n");
        }
    }
    printf("\n");
}



//------------------------------------------------------------------------
//
//   printSets          A debugging function.
//                      dump out all of the set definitions.
//
//------------------------------------------------------------------------
void RBBISetBuilder::printSets() {
    RBBINode             *usetNode;
    int                   i;
    UnicodeSet            inputSet;

    printf("\n\nUnicode Sets List\n------------------\n");
    i = 0;
    for (usetNode=fRB->fSetsListHead; usetNode!=NULL; usetNode=usetNode->fRightChild) {
        RBBINode       *setRef;
        RBBINode       *varRef;
        UnicodeString   setName;

        i++;
        printf("%3d    ", i);
        setName = "anonymous";
        setRef = usetNode->fParent;
        if (setRef != NULL) {
            varRef = setRef->fParent;
            if (varRef != NULL  &&  varRef->fType == RBBINode::varRef) {
                setName = varRef->fText;
            }
        }
        RBBINode::printUnicodeString(setName);
        printf("   ");
        RBBINode::printUnicodeString(usetNode->fText);
        printf("\n");
        if (usetNode->fLeftChild != NULL) {
            usetNode->fLeftChild->printTree();
        }
    }
    printf("\n");
}



//-------------------------------------------------------------------------------------
//
//  RangeDesriptor copy constructor
//
//-------------------------------------------------------------------------------------
RangeDescriptor::RangeDescriptor(const RangeDescriptor &other, UErrorCode &status) {
    int  i;

    this->fStartChar    = other.fStartChar;
    this->fEndChar      = other.fEndChar;
    this->fNum          = other.fNum;
    this->fNext         = NULL;
    this->fIncludesSets = new UVector(status);
    /* test for NULL */
    if (this->fIncludesSets == 0) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return;
    }
    
    for (i=0; i<other.fIncludesSets->size(); i++) {
        this->fIncludesSets->addElement(other.fIncludesSets->elementAt(i), status);
    }
}


//-------------------------------------------------------------------------------------
//
//  RangeDesriptor default constructor
//
//-------------------------------------------------------------------------------------
RangeDescriptor::RangeDescriptor(UErrorCode &status) {
    this->fStartChar    = 0;
    this->fEndChar      = 0;
    this->fNum          = 0;
    this->fNext         = NULL;
    this->fIncludesSets = new UVector(status);
    /* test for NULL */
    if(this->fIncludesSets == 0) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return;
    }
    
}


//-------------------------------------------------------------------------------------
//
//  RangeDesriptor Destructor
//
//-------------------------------------------------------------------------------------
RangeDescriptor::~RangeDescriptor() {
    delete  fIncludesSets;
    fIncludesSets = NULL;
}

//-------------------------------------------------------------------------------------
//
//  RangeDesriptor::split()
//
//-------------------------------------------------------------------------------------
void RangeDescriptor::split(UChar32 where, UErrorCode &status) {
    /* test for buffer overflows */
    if (U_FAILURE(status)) {
        return;
    }
    assert(where>fStartChar && where<=fEndChar);
    RangeDescriptor *nr = new RangeDescriptor(*this, status);
    /* test for NULL */
    if(nr == 0) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return;
    }
    
    //  RangeDescriptor copy constructor copies all fields.
    //  Only need to update those that are different after the split.
    nr->fStartChar = where;
    this->fEndChar = where-1;
    nr->fNext      = this->fNext;
    this->fNext    = nr;
}


//-------------------------------------------------------------------------------------
//
//   RangeDescriptor::setDictionaryFlag
//
//            Character Category Numbers that include characters from
//            the original Unicode Set named "dictionary" have bit 14
//            set to 1.  The RBBI runtime engine uses this to trigger
//            use of the word dictionary.
//
//            This function looks through the Unicode Sets that it
//            (the range) includes, and sets the bit in fNum when
//            "dictionary" is among them.
//
//            TODO:  a faster way would be to find the set node for
//                   "dictionary" just once, rather than looking it
//                   up by name every time.
//
//-------------------------------------------------------------------------------------
void RangeDescriptor::setDictionaryFlag() {
    int i;

    for (i=0; i<this->fIncludesSets->size(); i++) {
        RBBINode       *usetNode    = (RBBINode *)fIncludesSets->elementAt(i);
        UnicodeString   setName;
        RBBINode       *setRef = usetNode->fParent;
        if (setRef != NULL) {
            RBBINode *varRef = setRef->fParent;
            if (varRef != NULL  &&  varRef->fType == RBBINode::varRef) {
                setName = varRef->fText;
            }
        }
        if (setName.compare("dictionary") == 0) {   // TODO:  no string literals.
            this->fNum |= 0x4000;
            break;
        }
    }
}



U_NAMESPACE_END
