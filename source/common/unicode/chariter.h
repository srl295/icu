
/*
********************************************************************
*
*   Copyright (C) 1997-1999, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
********************************************************************
*/

#ifndef CHARITER_H
#define CHARITER_H

#include "unicode/utypes.h"
#include "unicode/unistr.h"


/**
 *  Abstract class defining a protcol for accessing characters in a text-storage object.
    <P>Examples:<P>

    Function processing characters, in this example simple output
    <pre>
    .   void processChar( UChar c )
    .   {
    .       cout &lt;&lt; " " &lt;&lt; c;
    .   }
    </pre>
    Traverse the text from start to finish
    <pre> 
    .   void traverseForward(CharacterIterator& iter)
    .   {
    .       for(UChar c = iter.first(); c != CharacterIterator.DONE; c = iter.next()) {
    .           processChar(c);
    .       }
    .   }
    </pre>
    Traverse the text backwards, from end to start
    <pre>
    .   void traverseBackward(CharacterIterator& iter)
    .   {
    .       for(UChar c = iter.last(); c != CharacterIterator.DONE; c = iter.previous()) {
    .           processChar(c);
    .       }
    .   }
    </pre>
    Traverse both forward and backward from a given position in the text. 
    Calls to notBoundary() in this example represents some additional stopping criteria.
    <pre>
    .   void traverseOut(CharacterIterator& iter, UTextOffset pos)
    .   {
    .       UChar c;
    .       for (c = iter.setIndex(pos);
    .       c != CharacterIterator.DONE && (Unicode::isLetter(c) || Unicode::isDigit(c));
    .           c = iter.next()) {}
    .       UTextOffset end = iter.getIndex();
    .       for (c = iter.setIndex(pos);
    .           c != CharacterIterator.DONE && (Unicode::isLetter(c) || Unicode::isDigit(c));
    .           c = iter.previous()) {}
    .       UTextOffset start = iter.getIndex() + 1;
    .   
    .       cout &lt;&lt; "start: " &lt;&lt; start &lt;&lt; " end: " &lt;&lt; end &lt;&lt; endl;
    .       for (c = iter.setIndex(start); iter.getIndex() &lt; end; c = iter.next() ) {
    .           processChar(c);
    .       }
    .    }
    </pre>
    Creating a StringCharacterIteratorand calling the test functions
    <pre>
    .    void CharacterIterator_Example( void )
    .    {
    .        cout &lt;&lt; endl &lt;&lt; "===== CharacterIterator_Example: =====" &lt;&lt; endl;
    .        UnicodeString text("Ein kleiner Satz.");
    .        StringCharacterIterator iterator(text);
    .        cout &lt;&lt; "----- traverseForward: -----------" &lt;&lt; endl;
    .        traverseForward( iterator );
    .        cout &lt;&lt; endl &lt;&lt; endl &lt;&lt; "----- traverseBackward: ----------" &lt;&lt; endl;
    .        traverseBackward( iterator );
    .        cout &lt;&lt; endl &lt;&lt; endl &lt;&lt; "----- traverseOut: ---------------" &lt;&lt; endl;
    .        traverseOut( iterator, 7 );
    .        cout &lt;&lt; endl &lt;&lt; endl &lt;&lt; "-----" &lt;&lt; endl;
    .    }
    </pre>
 */
class U_COMMON_API CharacterIterator
{
public:
  /**
   * Value returned by most of CharacterIterator's functions
   * when the iterator has reached the limits of its iteration.  */
  static const UChar    DONE;

  /**
   * Destructor.  
   * @stable
   */
  virtual ~CharacterIterator();

  /**
   * Returns true when both iterators refer to the same
   * character in the same character-storage object.  
   * @stable
   */
  virtual bool_t          operator==(const CharacterIterator& that) const = 0;
        
  /**
   * Returns true when the iterators refer to different
   * text-storage objects, or to different characters in the
   * same text-storage object.  
   * @stable
   */
  bool_t                  operator!=(const CharacterIterator& that) const { return !operator==(that); }

  /**
   * Returns a pointer to a new CharacterIterator of the same
   * concrete class as this one, and referring to the same
   * character in the same text-storage object as this one.  The
   * caller is responsible for deleting the new clone.  
   * @stable
   */
  virtual CharacterIterator*
  clone(void) const = 0;

  /**
   * Generates a hash code for this iterator.  
   * @stable
   */
  virtual int32_t         hashCode(void) const = 0;
        
  /**
   * Sets the iterator to refer to the first character in its
   * iteration range, and returns that character, 
   * @draft
   */
  virtual UChar         first(void) = 0;
        
  /**
   * Sets the iterator to refer to the last character in its
   * iteration range, and returns that character.  
   * @draft
   */
  virtual UChar         last(void) = 0;
        
  /**
   * Sets the iterator to refer to the "position"-th character
   * in the text-storage object the iterator refers to, and
   * returns that character.  
   * @draft
   */
  virtual UChar         setIndex(UTextOffset position) = 0;

  /**
   * Returns the character the iterator currently refers to.  
   * @draft
   */
  virtual UChar         current(void) const = 0;
        
  /**
   * Advances to the next character in the iteration range
   * (toward last()), and returns that character.  If there are
   * no more characters to return, returns DONE.  
   * @draft
   */
  virtual UChar         next(void) = 0;
        
  /**
   * Advances to the previous character in the iteration rance
   * (toward first()), and returns that character.  If there are
   * no more characters to return, returns DONE.  
   * @draft
   */
  virtual UChar         previous(void) = 0;

  /**
   * Returns the numeric index in the underlying text-storage
   * object of the character returned by first().  Since it's
   * possible to create an iterator that iterates across only
   * part of a text-storage object, this number isn't
   * necessarily 0.  
   * @stable
   */
  virtual UTextOffset      startIndex(void) const = 0;
        
  /**
   * Returns the numeric index in the underlying text-storage
   * object of the position immediately BEYOND the character
   * returned by last().  
   * @stable
   */
  virtual UTextOffset      endIndex(void) const = 0;
        
  /**
   * Returns the numeric index in the underlying text-storage
   * object of the character the iterator currently refers to
   * (i.e., the character returned by current()).  
   * @stable
   */
  virtual UTextOffset      getIndex(void) const = 0;

  /**
   * Copies the text under iteration into the UnicodeString
   * referred to by "result".  
   * @param result Receives a copy of the text under iteration.  
   * @stable
   */
  virtual void            getText(UnicodeString&  result) = 0;

  /**
   * Returns a UClassID for this CharacterIterator ("poor man's
   * RTTI").<P> Despite the fact that this function is public,
   * DO NOT CONSIDER IT PART OF CHARACTERITERATOR'S API!  
   * @stable
   */
  virtual UClassID         getDynamicClassID(void) const = 0;

protected:
  CharacterIterator() {}
  CharacterIterator(const CharacterIterator&) {}
  CharacterIterator&      operator=(const CharacterIterator&) { return *this; }
    
};

#endif



