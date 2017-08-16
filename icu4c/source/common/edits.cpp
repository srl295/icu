// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

// edits.cpp
// created: 2017feb08 Markus W. Scherer

#include "unicode/utypes.h"
#include "unicode/edits.h"
#include "cmemory.h"
#include "uassert.h"

U_NAMESPACE_BEGIN

namespace {

// 0000uuuuuuuuuuuu records u+1 unchanged text units.
const int32_t MAX_UNCHANGED_LENGTH = 0x1000;
const int32_t MAX_UNCHANGED = MAX_UNCHANGED_LENGTH - 1;

// 0wwwcccccccccccc with w=1..6 records ccc+1 replacements of w:w text units.
// No length change.
const int32_t MAX_SHORT_WIDTH = 6;
const int32_t MAX_SHORT_CHANGE_LENGTH = 0xfff;
const int32_t MAX_SHORT_CHANGE = 0x6fff;

// 0111mmmmmmnnnnnn records a replacement of m text units with n.
// m or n = 61: actual length follows in the next edits array unit.
// m or n = 62..63: actual length follows in the next two edits array units.
// Bit 30 of the actual length is in the head unit.
// Trailing units have bit 15 set.
const int32_t LENGTH_IN_1TRAIL = 61;
const int32_t LENGTH_IN_2TRAIL = 62;

}  // namespace

void Edits::releaseArray() U_NOEXCEPT {
    if (array != stackArray) {
        uprv_free(array);
    }
}

Edits &Edits::copyArray(const Edits &other) {
    if (U_FAILURE(errorCode_)) {
        length = delta = numChanges = 0;
        return *this;
    }
    if (length > capacity) {
        uint16_t *newArray = (uint16_t *)uprv_malloc((size_t)length * 2);
        if (newArray == nullptr) {
            length = delta = numChanges = 0;
            errorCode_ = U_MEMORY_ALLOCATION_ERROR;
            return *this;
        }
        releaseArray();
        array = newArray;
        capacity = length;
    }
    if (length > 0) {
        uprv_memcpy(array, other.array, (size_t)length * 2);
    }
    return *this;
}

Edits &Edits::moveArray(Edits &src) U_NOEXCEPT {
    if (U_FAILURE(errorCode_)) {
        length = delta = numChanges = 0;
        return *this;
    }
    releaseArray();
    if (length > STACK_CAPACITY) {
        array = src.array;
        capacity = src.capacity;
        src.array = src.stackArray;
        src.capacity = STACK_CAPACITY;
        src.reset();
        return *this;
    }
    array = stackArray;
    capacity = STACK_CAPACITY;
    if (length > 0) {
        uprv_memcpy(array, src.array, (size_t)length * 2);
    }
    return *this;
}

Edits &Edits::operator=(const Edits &other) {
    length = other.length;
    delta = other.delta;
    numChanges = other.numChanges;
    errorCode_ = other.errorCode_;
    return copyArray(other);
}

Edits &Edits::operator=(Edits &&src) U_NOEXCEPT {
    length = src.length;
    delta = src.delta;
    numChanges = src.numChanges;
    errorCode_ = src.errorCode_;
    return moveArray(src);
}

Edits::~Edits() {
    releaseArray();
}

void Edits::reset() U_NOEXCEPT {
    length = delta = numChanges = 0;
    errorCode_ = U_ZERO_ERROR;
}

void Edits::addUnchanged(int32_t unchangedLength) {
    if(U_FAILURE(errorCode_) || unchangedLength == 0) { return; }
    if(unchangedLength < 0) {
        errorCode_ = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    // Merge into previous unchanged-text record, if any.
    int32_t last = lastUnit();
    if(last < MAX_UNCHANGED) {
        int32_t remaining = MAX_UNCHANGED - last;
        if (remaining >= unchangedLength) {
            setLastUnit(last + unchangedLength);
            return;
        }
        setLastUnit(MAX_UNCHANGED);
        unchangedLength -= remaining;
    }
    // Split large lengths into multiple units.
    while(unchangedLength >= MAX_UNCHANGED_LENGTH) {
        append(MAX_UNCHANGED);
        unchangedLength -= MAX_UNCHANGED_LENGTH;
    }
    // Write a small (remaining) length.
    if(unchangedLength > 0) {
        append(unchangedLength - 1);
    }
}

void Edits::addReplace(int32_t oldLength, int32_t newLength) {
    if(U_FAILURE(errorCode_)) { return; }
    if(oldLength == newLength && 0 < oldLength && oldLength <= MAX_SHORT_WIDTH) {
        // Replacement of short oldLength text units by same-length new text.
        // Merge into previous short-replacement record, if any.
        ++numChanges;
        int32_t last = lastUnit();
        if(MAX_UNCHANGED < last && last < MAX_SHORT_CHANGE &&
                (last >> 12) == oldLength && (last & 0xfff) < MAX_SHORT_CHANGE_LENGTH) {
            setLastUnit(last + 1);
            return;
        }
        append(oldLength << 12);
        return;
    }

    if(oldLength < 0 || newLength < 0) {
        errorCode_ = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    if (oldLength == 0 && newLength == 0) {
        return;
    }
    ++numChanges;
    int32_t newDelta = newLength - oldLength;
    if (newDelta != 0) {
        if ((newDelta > 0 && delta >= 0 && newDelta > (INT32_MAX - delta)) ||
                (newDelta < 0 && delta < 0 && newDelta < (INT32_MIN - delta))) {
            // Integer overflow or underflow.
            errorCode_ = U_INDEX_OUTOFBOUNDS_ERROR;
            return;
        }
        delta += newDelta;
    }

    int32_t head = 0x7000;
    if (oldLength < LENGTH_IN_1TRAIL && newLength < LENGTH_IN_1TRAIL) {
        head |= oldLength << 6;
        head |= newLength;
        append(head);
    } else if ((capacity - length) >= 5 || growArray()) {
        int32_t limit = length + 1;
        if(oldLength < LENGTH_IN_1TRAIL) {
            head |= oldLength << 6;
        } else if(oldLength <= 0x7fff) {
            head |= LENGTH_IN_1TRAIL << 6;
            array[limit++] = (uint16_t)(0x8000 | oldLength);
        } else {
            head |= (LENGTH_IN_2TRAIL + (oldLength >> 30)) << 6;
            array[limit++] = (uint16_t)(0x8000 | (oldLength >> 15));
            array[limit++] = (uint16_t)(0x8000 | oldLength);
        }
        if(newLength < LENGTH_IN_1TRAIL) {
            head |= newLength;
        } else if(newLength <= 0x7fff) {
            head |= LENGTH_IN_1TRAIL;
            array[limit++] = (uint16_t)(0x8000 | newLength);
        } else {
            head |= LENGTH_IN_2TRAIL + (newLength >> 30);
            array[limit++] = (uint16_t)(0x8000 | (newLength >> 15));
            array[limit++] = (uint16_t)(0x8000 | newLength);
        }
        array[length] = (uint16_t)head;
        length = limit;
    }
}

void Edits::append(int32_t r) {
    if(length < capacity || growArray()) {
        array[length++] = (uint16_t)r;
    }
}

UBool Edits::growArray() {
    int32_t newCapacity;
    if (array == stackArray) {
        newCapacity = 2000;
    } else if (capacity == INT32_MAX) {
        // Not U_BUFFER_OVERFLOW_ERROR because that could be confused on a string transform API
        // with a result-string-buffer overflow.
        errorCode_ = U_INDEX_OUTOFBOUNDS_ERROR;
        return FALSE;
    } else if (capacity >= (INT32_MAX / 2)) {
        newCapacity = INT32_MAX;
    } else {
        newCapacity = 2 * capacity;
    }
    // Grow by at least 5 units so that a maximal change record will fit.
    if ((newCapacity - capacity) < 5) {
        errorCode_ = U_INDEX_OUTOFBOUNDS_ERROR;
        return FALSE;
    }
    uint16_t *newArray = (uint16_t *)uprv_malloc((size_t)newCapacity * 2);
    if (newArray == NULL) {
        errorCode_ = U_MEMORY_ALLOCATION_ERROR;
        return FALSE;
    }
    uprv_memcpy(newArray, array, (size_t)length * 2);
    releaseArray();
    array = newArray;
    capacity = newCapacity;
    return TRUE;
}

UBool Edits::copyErrorTo(UErrorCode &outErrorCode) {
    if (U_FAILURE(outErrorCode)) { return TRUE; }
    if (U_SUCCESS(errorCode_)) { return FALSE; }
    outErrorCode = errorCode_;
    return TRUE;
}

Edits &Edits::mergeAndAppend(const Edits &ab, const Edits &bc, UErrorCode &errorCode) {
    if (copyErrorTo(errorCode)) { return *this; }
    // Picture string a --(Edits ab)--> string b --(Edits bc)--> string c.
    // Parallel iteration over both Edits.
    Iterator abIter = ab.getFineIterator();
    Iterator bcIter = bc.getFineIterator();
    UBool abHasNext = TRUE, bcHasNext = TRUE;
    // Copy iterator state into local variables, so that we can modify and subdivide spans.
    // ab old & new length, bc old & new length
    int32_t aLength = 0, ab_bLength = 0, bc_bLength = 0, cLength = 0;
    // When we have different-intermediate-length changes, we accumulate a larger change.
    int32_t pending_aLength = 0, pending_cLength = 0;
    for (;;) {
        // At this point, for each of the two iterators:
        // Either we are done with the locally cached current edit,
        // and its intermediate-string length has been reset,
        // or we will continue to work with a truncated remainder of this edit.
        //
        // If the current edit is done, and the iterator has not yet reached the end,
        // then we fetch the next edit. This is true for at least one of the iterators.
        //
        // Normally it does not matter whether we fetch from ab and then bc or vice versa.
        // However, the result is observably different when
        // ab deletions meet bc insertions at the same intermediate-string index.
        // Some users expect the bc insertions to come first, so we fetch from bc first.
        if (bc_bLength == 0) {
            if (bcHasNext && (bcHasNext = bcIter.next(errorCode))) {
                bc_bLength = bcIter.oldLength();
                cLength = bcIter.newLength();
                if (bc_bLength == 0) {
                    // insertion
                    if (ab_bLength == 0 || !abIter.hasChange()) {
                        addReplace(pending_aLength, pending_cLength + cLength);
                        pending_aLength = pending_cLength = 0;
                    } else {
                        pending_cLength += cLength;
                    }
                    continue;
                }
            }
            // else see if the other iterator is done, too.
        }
        if (ab_bLength == 0) {
            if (abHasNext && (abHasNext = abIter.next(errorCode))) {
                aLength = abIter.oldLength();
                ab_bLength = abIter.newLength();
                if (ab_bLength == 0) {
                    // deletion
                    if (bc_bLength == bcIter.oldLength() || !bcIter.hasChange()) {
                        addReplace(pending_aLength + aLength, pending_cLength);
                        pending_aLength = pending_cLength = 0;
                    } else {
                        pending_aLength += aLength;
                    }
                    continue;
                }
            } else if (bc_bLength == 0) {
                // Both iterators are done at the same time:
                // The intermediate-string lengths match.
                break;
            } else {
                // The ab output string is shorter than the bc input string.
                if (!copyErrorTo(errorCode)) {
                    errorCode = U_ILLEGAL_ARGUMENT_ERROR;
                }
                return *this;
            }
        }
        if (bc_bLength == 0) {
            // The bc input string is shorter than the ab output string.
            if (!copyErrorTo(errorCode)) {
                errorCode = U_ILLEGAL_ARGUMENT_ERROR;
            }
            return *this;
        }
        //  Done fetching: ab_bLength > 0 && bc_bLength > 0

        // The current state has two parts:
        // - Past: We accumulate a longer ac edit in the "pending" variables.
        // - Current: We have copies of the current ab/bc edits in local variables.
        //   At least one side is newly fetched.
        //   One side might be a truncated remainder of an edit we fetched earlier.

        if (!abIter.hasChange() && !bcIter.hasChange()) {
            // An unchanged span all the way from string a to string c.
            if (pending_aLength != 0 || pending_cLength != 0) {
                addReplace(pending_aLength, pending_cLength);
                pending_aLength = pending_cLength = 0;
            }
            int32_t unchangedLength = aLength <= cLength ? aLength : cLength;
            addUnchanged(unchangedLength);
            ab_bLength = aLength -= unchangedLength;
            bc_bLength = cLength -= unchangedLength;
            // At least one of the unchanged spans is now empty.
            continue;
        }
        if (!abIter.hasChange() && bcIter.hasChange()) {
            // Unchanged a->b but changed b->c.
            if (ab_bLength >= bc_bLength) {
                // Split the longer unchanged span into change + remainder.
                addReplace(pending_aLength + bc_bLength, pending_cLength + cLength);
                pending_aLength = pending_cLength = 0;
                aLength = ab_bLength -= bc_bLength;
                bc_bLength = 0;
                continue;
            }
            // Handle the shorter unchanged span below like a change.
        } else if (abIter.hasChange() && !bcIter.hasChange()) {
            // Changed a->b and then unchanged b->c.
            if (ab_bLength <= bc_bLength) {
                // Split the longer unchanged span into change + remainder.
                addReplace(pending_aLength + aLength, pending_cLength + ab_bLength);
                pending_aLength = pending_cLength = 0;
                cLength = bc_bLength -= ab_bLength;
                ab_bLength = 0;
                continue;
            }
            // Handle the shorter unchanged span below like a change.
        } else {  // both abIter.hasChange() && bcIter.hasChange()
            if (ab_bLength == bc_bLength) {
                // Changes on both sides up to the same position. Emit & reset.
                addReplace(pending_aLength + aLength, pending_cLength + cLength);
                pending_aLength = pending_cLength = 0;
                ab_bLength = bc_bLength = 0;
                continue;
            }
        }
        // Accumulate the a->c change, reset the shorter side,
        // keep a remainder of the longer one.
        pending_aLength += aLength;
        pending_cLength += cLength;
        if (ab_bLength < bc_bLength) {
            bc_bLength -= ab_bLength;
            cLength = ab_bLength = 0;
        } else {  // ab_bLength > bc_bLength
            ab_bLength -= bc_bLength;
            aLength = bc_bLength = 0;
        }
    }
    if (pending_aLength != 0 || pending_cLength != 0) {
        addReplace(pending_aLength, pending_cLength);
    }
    copyErrorTo(errorCode);
    return *this;
}

Edits::Iterator::Iterator(const uint16_t *a, int32_t len, UBool oc, UBool crs) :
        array(a), index(0), length(len), remaining(0),
        onlyChanges_(oc), coarse(crs),
        changed(FALSE), oldLength_(0), newLength_(0),
        srcIndex(0), replIndex(0), destIndex(0) {}

int32_t Edits::Iterator::readLength(int32_t head) {
    if (head < LENGTH_IN_1TRAIL) {
        return head;
    } else if (head < LENGTH_IN_2TRAIL) {
        U_ASSERT(index < length);
        U_ASSERT(array[index] >= 0x8000);
        return array[index++] & 0x7fff;
    } else {
        U_ASSERT((index + 2) <= length);
        U_ASSERT(array[index] >= 0x8000);
        U_ASSERT(array[index + 1] >= 0x8000);
        int32_t len = ((head & 1) << 30) |
                ((int32_t)(array[index] & 0x7fff) << 15) |
                (array[index + 1] & 0x7fff);
        index += 2;
        return len;
    }
}

void Edits::Iterator::updateIndexes() {
    srcIndex += oldLength_;
    if (changed) {
        replIndex += newLength_;
    }
    destIndex += newLength_;
}

UBool Edits::Iterator::noNext() {
    // No change beyond the string.
    changed = FALSE;
    oldLength_ = newLength_ = 0;
    return FALSE;
}

UBool Edits::Iterator::next(UBool onlyChanges, UErrorCode &errorCode) {
    if (U_FAILURE(errorCode)) { return FALSE; }
    // We have an errorCode in case we need to start guarding against integer overflows.
    // It is also convenient for caller loops if we bail out when an error was set elsewhere.
    updateIndexes();
    if (remaining > 0) {
        // Fine-grained iterator: Continue a sequence of equal-length changes.
        --remaining;
        return TRUE;
    }
    if (index >= length) {
        return noNext();
    }
    int32_t u = array[index++];
    if (u <= MAX_UNCHANGED) {
        // Combine adjacent unchanged ranges.
        changed = FALSE;
        oldLength_ = u + 1;
        while (index < length && (u = array[index]) <= MAX_UNCHANGED) {
            ++index;
            oldLength_ += u + 1;
        }
        newLength_ = oldLength_;
        if (onlyChanges) {
            updateIndexes();
            if (index >= length) {
                return noNext();
            }
            // already fetched u > MAX_UNCHANGED at index
            ++index;
        } else {
            return TRUE;
        }
    }
    changed = TRUE;
    if (u <= MAX_SHORT_CHANGE) {
        if (coarse) {
            int32_t w = u >> 12;
            int32_t len = (u & 0xfff) + 1;
            oldLength_ = newLength_ = len * w;
        } else {
            // Split a sequence of equal-length changes that was compressed into one unit.
            oldLength_ = newLength_ = u >> 12;
            remaining = u & 0xfff;
            return TRUE;
        }
    } else {
        U_ASSERT(u <= 0x7fff);
        oldLength_ = readLength((u >> 6) & 0x3f);
        newLength_ = readLength(u & 0x3f);
        if (!coarse) {
            return TRUE;
        }
    }
    // Combine adjacent changes.
    while (index < length && (u = array[index]) > MAX_UNCHANGED) {
        ++index;
        if (u <= MAX_SHORT_CHANGE) {
            int32_t w = u >> 12;
            int32_t len = (u & 0xfff) + 1;
            len = len * w;
            oldLength_ += len;
            newLength_ += len;
        } else {
            U_ASSERT(u <= 0x7fff);
            int32_t oldLen = readLength((u >> 6) & 0x3f);
            int32_t newLen = readLength(u & 0x3f);
            oldLength_ += oldLen;
            newLength_ += newLen;
        }
    }
    return TRUE;
}

int32_t Edits::Iterator::findIndex(int32_t i, UBool findSource, UErrorCode &errorCode) {
    if (U_FAILURE(errorCode) || i < 0) { return -1; }
    int32_t spanStart, spanLength;
    if (findSource) {  // find source index
        spanStart = srcIndex;
        spanLength = oldLength_;
    } else {  // find destination index
        spanStart = destIndex;
        spanLength = newLength_;
    }
    if (i < spanStart) {
        // Reset the iterator to the start.
        index = remaining = oldLength_ = newLength_ = srcIndex = replIndex = destIndex = 0;
    } else if (i < (spanStart + spanLength)) {
        // The index is in the current span.
        return 0;
    }
    while (next(FALSE, errorCode)) {
        if (findSource) {
            spanStart = srcIndex;
            spanLength = oldLength_;
        } else {
            spanStart = destIndex;
            spanLength = newLength_;
        }
        if (i < (spanStart + spanLength)) {
            // The index is in the current span.
            return 0;
        }
        if (remaining > 0) {
            // Is the index in one of the remaining compressed edits?
            // spanStart is the start of the current span, before the remaining ones.
            int32_t len = (remaining + 1) * spanLength;
            if (i < (spanStart + len)) {
                int32_t n = (i - spanStart) / spanLength;  // 1 <= n <= remaining
                len = n * spanLength;
                srcIndex += len;
                replIndex += len;
                destIndex += len;
                remaining -= n;
                return 0;
            }
            // Make next() skip all of these edits at once.
            oldLength_ = newLength_ = len;
            remaining = 0;
        }
    }
    return 1;
}

int32_t Edits::Iterator::destinationIndexFromSourceIndex(int32_t i, UErrorCode &errorCode) {
    int32_t where = findIndex(i, TRUE, errorCode);
    if (where < 0) {
        // Error or before the string.
        return 0;
    }
    if (where > 0 || i == srcIndex) {
        // At or after string length, or at start of the found span.
        return destIndex;
    }
    if (changed) {
        // In a change span, map to its end.
        return destIndex + newLength_;
    } else {
        // In an unchanged span, offset 1:1 within it.
        return destIndex + (i - srcIndex);
    }
}

int32_t Edits::Iterator::sourceIndexFromDestinationIndex(int32_t i, UErrorCode &errorCode) {
    int32_t where = findIndex(i, FALSE, errorCode);
    if (where < 0) {
        // Error or before the string.
        return 0;
    }
    if (where > 0 || i == destIndex) {
        // At or after string length, or at start of the found span.
        return srcIndex;
    }
    if (changed) {
        // In a change span, map to its end.
        return srcIndex + oldLength_;
    } else {
        // In an unchanged span, offset within it.
        return srcIndex + (i - destIndex);
    }
}

U_NAMESPACE_END
