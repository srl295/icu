/*
*******************************************************************************
*   Copyright (C) 2010-2011, International Business Machines
*   Corporation and others.  All Rights Reserved.
*******************************************************************************
*   file name:  bytestriebuilder.cpp
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2010sep25
*   created by: Markus W. Scherer
*/

#include "unicode/utypes.h"
#include "unicode/stringpiece.h"
#include "bytestrie.h"
#include "bytestriebuilder.h"
#include "charstr.h"
#include "cmemory.h"
#include "uarrsort.h"

U_NAMESPACE_BEGIN

/*
 * Note: This builder implementation stores (bytes, value) pairs with full copies
 * of the byte sequences, until the BytesTrie is built.
 * It might(!) take less memory if we collected the data in a temporary, dynamic trie.
 */

class BytesTrieElement : public UMemory {
public:
    // Use compiler's default constructor, initializes nothing.

    void setTo(const StringPiece &s, int32_t val, CharString &strings, UErrorCode &errorCode);

    StringPiece getString(const CharString &strings) const {
        int32_t offset=stringOffset;
        int32_t length;
        if(offset>=0) {
            length=(uint8_t)strings[offset++];
        } else {
            offset=~offset;
            length=((int32_t)(uint8_t)strings[offset]<<8)|(uint8_t)strings[offset+1];
            offset+=2;
        }
        return StringPiece(strings.data()+offset, length);
    }
    int32_t getStringLength(const CharString &strings) const {
        int32_t offset=stringOffset;
        if(offset>=0) {
            return (uint8_t)strings[offset];
        } else {
            offset=~offset;
            return ((int32_t)(uint8_t)strings[offset]<<8)|(uint8_t)strings[offset+1];
        }
    }

    char charAt(int32_t index, const CharString &strings) const { return data(strings)[index]; }

    int32_t getValue() const { return value; }

    int32_t compareStringTo(const BytesTrieElement &o, const CharString &strings) const;

private:
    const char *data(const CharString &strings) const {
        int32_t offset=stringOffset;
        if(offset>=0) {
            ++offset;
        } else {
            offset=~offset+2;
        }
        return strings.data()+offset;
    }

    // If the stringOffset is non-negative, then the first strings byte contains
    // the string length.
    // If the stringOffset is negative, then the first two strings bytes contain
    // the string length (big-endian), and the offset needs to be bit-inverted.
    // (Compared with a stringLength field here, this saves 3 bytes per string for most strings.)
    int32_t stringOffset;
    int32_t value;
};

void
BytesTrieElement::setTo(const StringPiece &s, int32_t val,
                        CharString &strings, UErrorCode &errorCode) {
    if(U_FAILURE(errorCode)) {
        return;
    }
    int32_t length=s.length();
    if(length>0xffff) {
        // Too long: We store the length in 1 or 2 bytes.
        errorCode=U_INDEX_OUTOFBOUNDS_ERROR;
        return;
    }
    int32_t offset=strings.length();
    if(length>0xff) {
        offset=~offset;
        strings.append((char)(length>>8), errorCode);
    }
    strings.append((char)length, errorCode);
    stringOffset=offset;
    value=val;
    strings.append(s, errorCode);
}

int32_t
BytesTrieElement::compareStringTo(const BytesTrieElement &other, const CharString &strings) const {
    // TODO: add StringPiece::compare(), see ticket #8187
    StringPiece thisString=getString(strings);
    StringPiece otherString=other.getString(strings);
    int32_t lengthDiff=thisString.length()-otherString.length();
    int32_t commonLength;
    if(lengthDiff<=0) {
        commonLength=thisString.length();
    } else {
        commonLength=otherString.length();
    }
    int32_t diff=uprv_memcmp(thisString.data(), otherString.data(), commonLength);
    return diff!=0 ? diff : lengthDiff;
}

BytesTrieBuilder::~BytesTrieBuilder() {
    delete[] elements;
    uprv_free(bytes);
}

BytesTrieBuilder &
BytesTrieBuilder::add(const StringPiece &s, int32_t value, UErrorCode &errorCode) {
    if(U_FAILURE(errorCode)) {
        return *this;
    }
    if(bytesLength>0) {
        // Cannot add elements after building.
        errorCode=U_NO_WRITE_PERMISSION;
        return *this;
    }
    bytesCapacity+=s.length()+1;  // Crude bytes preallocation estimate.
    if(elementsLength==elementsCapacity) {
        int32_t newCapacity;
        if(elementsCapacity==0) {
            newCapacity=1024;
        } else {
            newCapacity=4*elementsCapacity;
        }
        BytesTrieElement *newElements=new BytesTrieElement[newCapacity];
        if(newElements==NULL) {
            errorCode=U_MEMORY_ALLOCATION_ERROR;
        }
        if(elementsLength>0) {
            uprv_memcpy(newElements, elements, elementsLength*sizeof(BytesTrieElement));
        }
        delete[] elements;
        elements=newElements;
        elementsCapacity=newCapacity;
    }
    elements[elementsLength++].setTo(s, value, strings, errorCode);
    return *this;
}

U_CDECL_BEGIN

static int32_t U_CALLCONV
compareElementStrings(const void *context, const void *left, const void *right) {
    const CharString *strings=reinterpret_cast<const CharString *>(context);
    const BytesTrieElement *leftElement=reinterpret_cast<const BytesTrieElement *>(left);
    const BytesTrieElement *rightElement=reinterpret_cast<const BytesTrieElement *>(right);
    return leftElement->compareStringTo(*rightElement, *strings);
}

U_CDECL_END

StringPiece
BytesTrieBuilder::build(UStringTrieBuildOption buildOption, UErrorCode &errorCode) {
    StringPiece result;
    if(U_FAILURE(errorCode)) {
        return result;
    }
    if(bytesLength>0) {
        // Already built.
        result.set(bytes+(bytesCapacity-bytesLength), bytesLength);
        return result;
    }
    if(elementsLength==0) {
        errorCode=U_INDEX_OUTOFBOUNDS_ERROR;
        return result;
    }
    uprv_sortArray(elements, elementsLength, (int32_t)sizeof(BytesTrieElement),
                   compareElementStrings, &strings,
                   FALSE,  // need not be a stable sort
                   &errorCode);
    if(U_FAILURE(errorCode)) {
        return result;
    }
    // Duplicate strings are not allowed.
    StringPiece prev=elements[0].getString(strings);
    for(int32_t i=1; i<elementsLength; ++i) {
        StringPiece current=elements[i].getString(strings);
        if(prev==current) {
            errorCode=U_ILLEGAL_ARGUMENT_ERROR;
            return result;
        }
        prev=current;
    }
    // Create and byte-serialize the trie for the elements.
    if(bytesCapacity<1024) {
        bytesCapacity=1024;
    }
    bytes=reinterpret_cast<char *>(uprv_malloc(bytesCapacity));
    if(bytes==NULL) {
        errorCode=U_MEMORY_ALLOCATION_ERROR;
        return result;
    }
    StringTrieBuilder::build(buildOption, elementsLength, errorCode);
    if(bytes==NULL) {
        errorCode=U_MEMORY_ALLOCATION_ERROR;
    } else {
        result.set(bytes+(bytesCapacity-bytesLength), bytesLength);
    }
    return result;
}

int32_t
BytesTrieBuilder::getElementStringLength(int32_t i) const {
    return elements[i].getStringLength(strings);
}

UChar
BytesTrieBuilder::getElementUnit(int32_t i, int32_t byteIndex) const {
    return (uint8_t)elements[i].charAt(byteIndex, strings);
}

int32_t
BytesTrieBuilder::getElementValue(int32_t i) const {
    return elements[i].getValue();
}

int32_t
BytesTrieBuilder::getLimitOfLinearMatch(int32_t first, int32_t last, int32_t byteIndex) const {
    const BytesTrieElement &firstElement=elements[first];
    const BytesTrieElement &lastElement=elements[last];
    int32_t minStringLength=firstElement.getStringLength(strings);
    while(++byteIndex<minStringLength &&
            firstElement.charAt(byteIndex, strings)==
            lastElement.charAt(byteIndex, strings)) {}
    return byteIndex;
}

int32_t
BytesTrieBuilder::countElementUnits(int32_t start, int32_t limit, int32_t byteIndex) const {
    int32_t length=0;  // Number of different bytes at byteIndex.
    int32_t i=start;
    do {
        char byte=elements[i++].charAt(byteIndex, strings);
        while(i<limit && byte==elements[i].charAt(byteIndex, strings)) {
            ++i;
        }
        ++length;
    } while(i<limit);
    return length;
}

int32_t
BytesTrieBuilder::skipElementsBySomeUnits(int32_t i, int32_t byteIndex, int32_t count) const {
    do {
        char byte=elements[i++].charAt(byteIndex, strings);
        while(byte==elements[i].charAt(byteIndex, strings)) {
            ++i;
        }
    } while(--count>0);
    return i;
}

int32_t
BytesTrieBuilder::indexOfElementWithNextUnit(int32_t i, int32_t byteIndex, UChar byte) const {
    char b=(char)byte;
    while(b==elements[i].charAt(byteIndex, strings)) {
        ++i;
    }
    return i;
}

BytesTrieBuilder::BTLinearMatchNode::BTLinearMatchNode(const char *bytes, int32_t len, Node *nextNode)
        : LinearMatchNode(len, nextNode), s(bytes) {
    hash=hash*37+uhash_hashCharsN(bytes, len);
}

UBool
BytesTrieBuilder::BTLinearMatchNode::operator==(const Node &other) const {
    if(this==&other) {
        return TRUE;
    }
    if(!LinearMatchNode::operator==(other)) {
        return FALSE;
    }
    const BTLinearMatchNode &o=(const BTLinearMatchNode &)other;
    return 0==uprv_memcmp(s, o.s, length);
}

void
BytesTrieBuilder::BTLinearMatchNode::write(StringTrieBuilder &builder) {
    BytesTrieBuilder &b=(BytesTrieBuilder &)builder;
    next->write(builder);
    b.write(s, length);
    offset=b.write(b.getMinLinearMatch()+length-1);
}

StringTrieBuilder::Node *
BytesTrieBuilder::createLinearMatchNode(int32_t i, int32_t byteIndex, int32_t length,
                                        Node *nextNode) const {
    return new BTLinearMatchNode(
            elements[i].getString(strings).data()+byteIndex,
            length,
            nextNode);
}

UBool
BytesTrieBuilder::ensureCapacity(int32_t length) {
    if(bytes==NULL) {
        return FALSE;  // previous memory allocation had failed
    }
    if(length>bytesCapacity) {
        int32_t newCapacity=bytesCapacity;
        do {
            newCapacity*=2;
        } while(newCapacity<=length);
        char *newBytes=reinterpret_cast<char *>(uprv_malloc(newCapacity));
        if(newBytes==NULL) {
            // unable to allocate memory
            uprv_free(bytes);
            bytes=NULL;
            return FALSE;
        }
        uprv_memcpy(newBytes+(newCapacity-bytesLength),
                    bytes+(bytesCapacity-bytesLength), bytesLength);
        uprv_free(bytes);
        bytes=newBytes;
        bytesCapacity=newCapacity;
    }
    return TRUE;
}

int32_t
BytesTrieBuilder::write(int32_t byte) {
    int32_t newLength=bytesLength+1;
    if(ensureCapacity(newLength)) {
        bytesLength=newLength;
        bytes[bytesCapacity-bytesLength]=(char)byte;
    }
    return bytesLength;
}

int32_t
BytesTrieBuilder::write(const char *b, int32_t length) {
    int32_t newLength=bytesLength+length;
    if(ensureCapacity(newLength)) {
        bytesLength=newLength;
        uprv_memcpy(bytes+(bytesCapacity-bytesLength), b, length);
    }
    return bytesLength;
}

int32_t
BytesTrieBuilder::writeElementUnits(int32_t i, int32_t byteIndex, int32_t length) {
    return write(elements[i].getString(strings).data()+byteIndex, length);
}

int32_t
BytesTrieBuilder::writeValueAndFinal(int32_t i, UBool isFinal) {
    if(0<=i && i<=BytesTrie::kMaxOneByteValue) {
        return write(((BytesTrie::kMinOneByteValueLead+i)<<1)|isFinal);
    }
    char intBytes[5];
    int32_t length=1;
    if(i<0 || i>0xffffff) {
        intBytes[0]=(char)BytesTrie::kFiveByteValueLead;
        intBytes[1]=(char)(i>>24);
        intBytes[2]=(char)(i>>16);
        intBytes[3]=(char)(i>>8);
        intBytes[4]=(char)i;
        length=5;
    // } else if(i<=BytesTrie::kMaxOneByteValue) {
    //     intBytes[0]=(char)(BytesTrie::kMinOneByteValueLead+i);
    } else {
        if(i<=BytesTrie::kMaxTwoByteValue) {
            intBytes[0]=(char)(BytesTrie::kMinTwoByteValueLead+(i>>8));
        } else {
            if(i<=BytesTrie::kMaxThreeByteValue) {
                intBytes[0]=(char)(BytesTrie::kMinThreeByteValueLead+(i>>16));
            } else {
                intBytes[0]=(char)BytesTrie::kFourByteValueLead;
                intBytes[1]=(char)(i>>16);
                length=2;
            }
            intBytes[length++]=(char)(i>>8);
        }
        intBytes[length++]=(char)i;
    }
    intBytes[0]=(char)((intBytes[0]<<1)|isFinal);
    return write(intBytes, length);
}

int32_t
BytesTrieBuilder::writeValueAndType(UBool hasValue, int32_t value, int32_t node) {
    int32_t offset=write(node);
    if(hasValue) {
        offset=writeValueAndFinal(value, FALSE);
    }
    return offset;
}

int32_t
BytesTrieBuilder::writeDeltaTo(int32_t jumpTarget) {
    int32_t i=bytesLength-jumpTarget;
    U_ASSERT(i>=0);
    if(i<=BytesTrie::kMaxOneByteDelta) {
        return write(i);
    }
    char intBytes[5];
    int32_t length;
    if(i<=BytesTrie::kMaxTwoByteDelta) {
        intBytes[0]=(char)(BytesTrie::kMinTwoByteDeltaLead+(i>>8));
        length=1;
    } else {
        if(i<=BytesTrie::kMaxThreeByteDelta) {
            intBytes[0]=(char)(BytesTrie::kMinThreeByteDeltaLead+(i>>16));
            length=2;
        } else {
            if(i<=0xffffff) {
                intBytes[0]=(char)BytesTrie::kFourByteDeltaLead;
                length=3;
            } else {
                intBytes[0]=(char)BytesTrie::kFiveByteDeltaLead;
                intBytes[1]=(char)(i>>24);
                length=4;
            }
            intBytes[1]=(char)(i>>16);
        }
        intBytes[1]=(char)(i>>8);
    }
    intBytes[length++]=(char)i;
    return write(intBytes, length);
}

U_NAMESPACE_END
