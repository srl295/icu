/*
 *******************************************************************************
 * Copyright (C) 1996-2000, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 *
 * $Source: /xsrl/Nsvn/icu/icu4j/src/com/ibm/icu/impl/StringUCharacterIterator.java,v $ 
 * $Date: 2003/10/08 21:51:43 $ 
 * $Revision: 1.1 $
 *
 *******************************************************************************
 */
package com.ibm.icu.impl;

import com.ibm.icu.text.UCharacterIterator;
import com.ibm.icu.text.UTF16;    

/**
 * Used by Collation. UCharacterIterator on Strings. Can't use 
 * ReplaceableUCharacterIterator because it is not easy to do a fast setText. 
 * @author synwee
 */
public final class StringUCharacterIterator extends UCharacterIterator 
{

    // public constructor ------------------------------------------------------
    
    /**
     * Public constructor
     * @param str text which the iterator will be based on
     */
    public StringUCharacterIterator(String str)
    {
        if (str == null) {
            throw new IllegalArgumentException();
        }
        m_text_ = str;
        m_currentIndex_ = 0;
    }
    
    /**
     * Public default constructor
     */
    public StringUCharacterIterator()
    {
        m_text_ = "";
        m_currentIndex_ = 0;
    }
    
    // public methods ----------------------------------------------------------
    
    /**
     * Creates a copy of this iterator, does not clone the underlying 
     * <code>String</code>object
     * @return copy of this iterator
     */
    public Object clone()
    {
		try {
		    return super.clone();
		} catch (CloneNotSupportedException e) {
		    return null; // never invoked
		}
    }
    
    /**
     * Returns the current UTF16 character.
     * @return current UTF16 character
     */
    public int current()
    {
        if (m_currentIndex_ < m_text_.length()) {
            return m_text_.charAt(m_currentIndex_);
        }
        return DONE;
    }
    
    /**
     * Returns the current codepoint
     * @return current codepoint
     */
    public int currentCodePoint()
    {
        // cannot use charAt due to it different 
        // behaviour when index is pointing at a
        // trail surrogate, check for surrogates
         
        if (m_currentIndex_ >= m_text_.length()) {
            return DONE;
        }
        char ch = m_text_.charAt(m_currentIndex_);
        if (UTF16.isLeadSurrogate(ch)) {
            // advance the index to get the next code point
            m_currentIndex_ ++;
            if (m_currentIndex_ < m_text_.length()) {
                // due to post increment semantics current() after next() 
                // actually returns the next char which is what we want
                char ch2 = m_text_.charAt(m_currentIndex_);
                
                if (UTF16.isTrailSurrogate(ch2)) {
                    // we found a surrogate pair
                    return UCharacterProperty.getRawSupplementary(ch, ch2);
                }
            }
            // current should never change the current index so back off
            m_currentIndex_ --;
        }
        return ch;
    }
    
    /**
     * Returns the length of the text
     * @return length of the text
     */
    public int getLength()
    {
        return m_text_.length();
    }
    
    /**
     * Gets the current currentIndex in text.
     * @return current currentIndex in text.
     */
    public int getIndex()
    {
        return m_currentIndex_;
    }
        
    /**
     * Returns next UTF16 character and increments the iterator's currentIndex 
     * by 1. 
     * If the resulting currentIndex is greater or equal to the text length, 
     * the currentIndex is reset to the text length and a value of DONE is 
     * returned. 
     * @return next UTF16 character in text or DONE if the new currentIndex is 
     *         off the end of the text range.
     */
    public int next()
    {
        if (m_currentIndex_ < m_text_.length()) 
        {
            return m_text_.charAt(m_currentIndex_ ++);
        }
        return DONE;
    }
    
                
    /**
     * Returns previous UTF16 character and decrements the iterator's 
     * currentIndex by 1. 
     * If the resulting currentIndex is less than 0, the currentIndex is reset 
     * to 0 and a value of DONE is returned. 
     * @return next UTF16 character in text or DONE if the new currentIndex is 
     *         off the start of the text range.
     */
    public int previous()
    {
        if (m_currentIndex_ > 0) {
            return m_text_.charAt(-- m_currentIndex_);
        }
        return DONE;
    }

    /**
     * <p>Sets the currentIndex to the specified currentIndex in the text and 
     * returns that single UTF16 character at currentIndex. 
     * This assumes the text is stored as 16-bit code units.</p>
     * @param currentIndex the currentIndex within the text. 
     * @exception IllegalArgumentException is thrown if an invalid currentIndex 
     *            is supplied. i.e. currentIndex is out of bounds.
     * @return the character at the specified currentIndex or DONE if the 
     *         specified currentIndex is equal to the end of the text.
     */
    public void setIndex(int currentIndex) throws IndexOutOfBoundsException
    {
        if (currentIndex < 0 || currentIndex > m_text_.length()) {
            throw new IndexOutOfBoundsException();
        }
        m_currentIndex_ = currentIndex;
    }
    
    /**
     * Fills the buffer with the underlying text storage of the iterator
     * If the buffer capacity is not enough a exception is thrown. The capacity
     * of the fill in buffer should at least be equal to length of text in the 
     * iterator obtained by calling <code>getLength()</code).
     * <b>Usage:</b>
     * 
     * <code>
     * <pre>
     *         UChacterIterator iter = new UCharacterIterator.getInstance(text);
     *         char[] buf = new char[iter.getLength()];
     *         iter.getText(buf);
     *         
     *         OR
     *         char[] buf= new char[1];
     *         int len = 0;
     *         for(;;){
     *             try{
     *                 len = iter.getText(buf);
     *                 break;
     *             }catch(IndexOutOfBoundsException e){
     *                 buf = new char[iter.getLength()];
     *             }
     *         }
     * </pre>
     * </code>
     *             
     * @param fillIn an array of chars to fill with the underlying UTF-16 code 
     *         units.
     * @param offset the position within the array to start putting the data.
     * @return the number of code units added to fillIn, as a convenience
     * @exception IndexOutOfBounds exception if there is not enough
     *            room after offset in the array, or if offset &lt; 0.
     */
    public int getText(char[] fillIn, int offset)
    {
    	int length = m_text_.length();
        if (offset < 0 || offset + length > fillIn.length) {
            throw new IndexOutOfBoundsException(Integer.toString(length));
        }
        m_text_.getChars(0, length, fillIn, offset);
        return length;
    }
    
    /**
     * Convenience method for returning the underlying text storage as as 
     * string
     * @return the underlying text storage in the iterator as a string
     */
    public String getText() 
    {
        return m_text_;
    }       
    
    /**
     * Reset this iterator to point to a new string. This method is used by 
     * other classes that want to avoid allocating new 
     * ReplaceableCharacterIterator objects every time their setText method
     * is called.
     * @param text The String to be iterated over 
     */
    public void setText(String text) 
    {
        if (text == null) {
            throw new NullPointerException();
        }
        m_text_ = text;
        m_currentIndex_ = 0;
    }
        
    // private data members ----------------------------------------------------
    
    /**
     * Text string object
     */
    private String m_text_;
    /**
     * Current currentIndex
     */
    private int m_currentIndex_;

}
