
/*
 * @(#)LEFontInstance.h	1.3 00/03/15
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001, 2002 - All Rights Reserved
 *
 */

#ifndef __LEFONTINSTANCE_H
#define __LEFONTINSTANCE_H

#include "LETypes.h"

U_NAMESPACE_BEGIN

/**
 * Instances of this class are used by LEFontInstance::mapCharsToGlyphs and
 * LEFontInstance::mapCharToGlyph to adjust character codes before the character
 * to glyph mapping process. Examples of this are filtering out control charcters
 * and character mirroring - replacing a character which has both a left and a right
 * hand form with the opposite form.
 *
 * @draft ICU 2.2
 */
class LECharMapper /* not : public UObject because this is an interface/mixin class */ {
public:
    /**
     * Destructor.
     * @draft ICU 2.4
     */
    virtual inline ~LECharMapper() {};

    /**
     * This method does the adjustments.
     *
     * @param ch - the input charcter
     *
     * @return the adjusted character
     *
     * @draft ICU 2.2
     */
    virtual LEUnicode32 mapChar(LEUnicode32 ch) const = 0;
};

/**
 * This is a pure virtual base class that servers as the interface between a LayoutEngine
 * and the platform font environment. It allows a LayoutEngine to access font tables, do
 * character to glyph mapping, and obtain metrics information without knowing any platform
 * specific details. There are also a few utility methods for converting between points,
 * pixels and funits. (font design units)
 *
 * Each instance of an LEFontInstance represents a renerable instance of a font. (i.e. a
 * single font at a particular point size, with a particular transform)
 *
 * @draft ICU 2.2
 */
class U_LAYOUT_API LEFontInstance /* not : public UObject because this is an interface/mixin class */ {
public:

    /**
     * This virtual destructor is here so that the subclass
     * destructors can be invoked through the base class.
     *
     * @draft ICU 2.2
     */
    virtual inline ~LEFontInstance() {};

    /**
     * This method is provided so that clients can tell if
     * a given LEFontInstance is an instance of a composite
     * font.
     * 
     * @return <code>true</code> if the instance represents a composite font, <code>false</code> otherwise.
     *
     * @draft ICU 2.6
     */
    virtual le_bool isComposite() const;

    /**
     * Get a sub-font for a run of text from a composite font. This method examines the
     * given text, finding a run of text which can all be rendered
     * using the same sub-font. Subclassers should try to keep all the text in a single
     * sub-font if they can.
     *
     * @param chars  - the array of unicode characters
     * @param offset - a pointer to the starting offset in the text. This will be
     *                 set to the limit offset of the run on exit.
     * @param count  - the number of characters in the array. Can be used as a hint for selecting a sub-font.
     * @param script - the script of the characters.
     *
     * @return an <code>LEFontInstance</code> for the sub font which can render the characters.
     *
     * @draft ICU 2.6
     */
    virtual const LEFontInstance *getSubFont(const LEUnicode chars[], le_int32 *offset, le_int32 count, le_int32 script) const;

    //
    // Font file access
    //

    /**
     * This method reads a table from the font.
     *
     * @param tableTag - the four byte table tag
     *
     * @return the address of the table in memory
     *
     * @draft ICU 2.2
     */
    virtual const void *getFontTable(LETag tableTag) const = 0;

    /**
     * This method is used to determine if the font can
     * render the given character. This can usually be done
     * by looking the character up in the font's character
     * to glyph mapping.
     *
     * @param ch - the character to be tested
     *
     * @return true if the font can render ch.
     *
     * @draft ICU 2.6
     */
    virtual le_bool canDisplay(LEUnicode32 ch) const;

    /**
     * This method returns the number of design units in
     * the font's EM square.
     *
     * @return the number of design units pre EM.
     *
     * @draft ICU 2.2
     */
    virtual le_int32 getUnitsPerEM() const = 0;

    /**
     * This method maps an array of character codes to an array of glyph
     * indices, using the font's character to glyph map.
     *
     * @param chars - the character array
     * @param offset - the index of the first charcter
     * @param count - the number of charcters
     * @param reverse - if true, store the glyph indices in reverse order.
     * @param mapper - the character mapper.
     * @param glyphs - the output glyph array
     *
     * @see LECharMapper
     *
     * @draft ICU 2.6
     */
    virtual void mapCharsToGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_bool reverse, const LECharMapper *mapper, LEGlyphID glyphs[]) const;

    /**
     * This method maps a single character to a glyph index, using the
     * font's charcter to glyph map. The default implementation of this
     * method calls the mapper, and then calls <code>mapCharToGlyph(mappedCh)</code>.
     *
     * @param ch - the character
     * @param mapper - the character mapper
     *
     * @return the glyph index
     *
     * @draft ICU 2.6
     */
    virtual LEGlyphID mapCharToGlyph(LEUnicode32 ch, const LECharMapper *mapper) const;

    /**
     * This method maps a single character to a glyph index, using the
     * font's charcter to glyph map.
     *
     * @param ch - the character
     *
     * @return the glyph index
     *
     * @see LECharMapper
     *
     * @draft ICU 2.6
     */
    virtual LEGlyphID mapCharToGlyph(LEUnicode32 ch) const = 0;

    /**
     * This method gets a name from the font. (e.g. the family name) The encoding
     * of the name is specified by the platform, the script, and the language.
     *
     * @param platformID - the platform id
     * @param scriptID - the script id
     * @param langaugeID - the language id
     * @param name - the destination character array (can be null)
     *
     * @return the number of characters in the name
     *
     * @draft ICU 2.6
     */
    virtual le_int32 getName(le_uint16 platformID, le_uint16 scriptID, le_uint16 languageID, le_uint16 nameID, LEUnicode *name) const;

    //
    // Metrics
    //

    /**
     * This method gets the X and Y advance of a particular glyph, in pixels.
     *
     * @param glyph - the glyph index
     * @param advance - the X and Y pixel values will be stored here
     *
     * @draft ICU 2.2
     */
    virtual void getGlyphAdvance(LEGlyphID glyph, LEPoint &advance) const = 0;

    /**
     * This method gets the hinted X and Y pixel coordinates of a particular
     * point in the outline of the given glyph.
     *
     * @param glyph - the glyph index
     * @param pointNumber - the number of the point
     * @param point - the point's X and Y pixel values will be stored here
     *
     * @return true if the point coordinates could be stored.
     *
     * @draft ICU 2.2
     */
    virtual le_bool getGlyphPoint(LEGlyphID glyph, le_int32 pointNumber, LEPoint &point) const = 0;

    /**
     * This method returns the width of the font's EM square
     * in pixels.
     *
     * @return the pixel width of the EM square
     *
     * @draft ICU 2.2
     */
    virtual float getXPixelsPerEm() const = 0;

    /**
     * This method returns the height of the font's EM square
     * in pixels.
     *
     * @return the pixel height of the EM square
     *
     * @draft ICU 2.2
     */
    virtual float getYPixelsPerEm() const = 0;

    /**
     * This method converts font design units in the
     * X direction to points.
     *
     * @param xUnits - design units in the X direction
     *
     * @return points in the X direction
     *
     * @draft ICU 2.6
     */
    virtual float xUnitsToPoints(float xUnits) const;

    /**
     * This method converts font design units in the
     * Y direction to points.
     *
     * @param yUnits - design units in the Y direction
     *
     * @return points in the Y direction
     *
     * @draft ICU 2.6
     */
    virtual float yUnitsToPoints(float yUnits) const;

    /**
     * This method converts font design units to points.
     *
     * @param units - X and Y design units
     * @param points - set to X and Y points
     *
     * @draft ICU 2.6
     */
    virtual void unitsToPoints(LEPoint &units, LEPoint &points) const;

    /**
     * This method converts pixels in the
     * X direction to font design units.
     *
     * @param xPixels - pixels in the X direction
     *
     * @return font design units in the X direction
     *
     * @draft ICU 2.6
     */
    virtual float xPixelsToUnits(float xPixels) const;

    /**
     * This method converts pixels in the
     * Y direction to font design units.
     *
     * @param yPixels - pixels in the Y direction
     *
     * @return font design units in the Y direction
     *
     * @draft ICU 2.6
     */
    virtual float yPixelsToUnits(float yPixels) const;

    /**
     * This method converts pixels to font design units.
     *
     * @param pixels - X and Y pixel
     * @param units - set to X and Y font design units
     *
     * @draft ICU 2.6
     */
    virtual void pixelsToUnits(LEPoint &pixels, LEPoint &units) const;

    /**
     * Get the X scale factor from the font's transform. The default
     * implementation of <code>transformFunits()</code> will call this method.
     *
     * @return the X scale factor.
     *
     *
     * @see transformFunits
     *
     * @draft ICU 2.6
     */
    virtual float getScaleFactorX() const = 0;

    /**
     * Get the Y scale factor from the font's transform. The default
     * implementation of <code>transformFunits()</code> will call this method.
     *
     * @return the Yscale factor.
     *
     * @see transformFunits
     *
     * @draft ICU 2.6
     */
    virtual float getScaleFactorY() const = 0;

    /**
     * This method transforms an X, Y point in font design units to a
     * pixel coordinate, applying the font's transform. The default
     * implementation of this method calls <code>getScaleFactorX()</code>
     * and <code>getScaleFactorY()</code>.
     *
     * @param xFunits - the X coordinate in font design units
     * @param yFunits - the Y coordinate in font design units
     * @param pixels - the tranformed co-ordinate in pixels
     *
     * @see getScaleFactorX
     * @see getScaleFactorY
     *
     * @draft ICU 2.6
     */
    virtual void transformFunits(float xFunits, float yFunits, LEPoint &pixels) const;

    /**
     * This is a convenience method used to convert
     * values in a 16.16 fixed point format to floating point.
     *
     * @param fixed - the fixed point value
     *
     * @return the floating point value
     *
     * @draft ICU 2.2
     */
    static float fixedToFloat(le_int32 fixed);

    /**
     * This is a convenience method used to convert
     * floating point values to 16.16 fixed point format.
     *
     * @param theFloat - the floating point value
     *
     * @return the fixed point value
     *
     * @draft ICU 2.2
     */
    static le_int32 floatToFixed(float theFloat);

    //
    // These methods won't ever be called by the LayoutEngine,
    // but are useful for cleints of <code>LEFontInstance</code> who
    // need to render text.
    //

    /**
     * Get the font's ascent.
     *
     * @return the font's ascent, in points.
     *
     * @draft ICU 2.6
     */
    virtual le_int32 getAscent() const = 0;

    /**
     * Get the font's descent.
     *
     * @return the font's descent, in points.
     *
     * @draft ICU 2.6
     */
    virtual le_int32 getDescent() const = 0;

    /**
     * Get the font's leading.
     *
     * @return the font's leading, in points.
     *
     * @draft ICU 2.6
     */
    virtual le_int32 getLeading() const = 0;

    /**
     * Get the line height required to display text in
     * this font. The value returned is just the sum of
     * the ascent, descent, and leading.
     *
     * @return the line height, in points
     *
     * @draft ICU 2.6
     */
    virtual le_int32 getLineHeight() const;
};

inline le_bool LEFontInstance::isComposite() const
{
    return false;
}

inline const LEFontInstance *LEFontInstance::getSubFont(const LEUnicode chars[], le_int32 *offset, le_int32 count, le_int32 script) const
{
    *offset += count;
    return this;
}

inline le_bool LEFontInstance::canDisplay(LEUnicode32 ch) const
{
    return mapCharToGlyph(ch) != 0;
}

inline le_int32 LEFontInstance::getName(le_uint16 platformID, le_uint16 scriptID, le_uint16 languageID, le_uint16 nameID, LEUnicode *name) const
{
    if (name != NULL) {
        *name = 0;
    }

    return 0;
}

inline float LEFontInstance::xUnitsToPoints(float xUnits) const
{
    return (xUnits * getXPixelsPerEm()) / (float) getUnitsPerEM();
}

inline float LEFontInstance::yUnitsToPoints(float yUnits) const
{
    return (yUnits * getYPixelsPerEm()) / (float) getUnitsPerEM();
}

inline void LEFontInstance::unitsToPoints(LEPoint &units, LEPoint &points) const
{
    points.fX = xUnitsToPoints(units.fX);
    points.fY = yUnitsToPoints(units.fY);
}

inline float LEFontInstance::xPixelsToUnits(float xPixels) const
{
    return (xPixels * getUnitsPerEM()) / (float) getXPixelsPerEm();
}

inline float LEFontInstance::yPixelsToUnits(float yPixels) const
{
    return (yPixels * getUnitsPerEM()) / (float) getYPixelsPerEm();
}

inline void LEFontInstance::pixelsToUnits(LEPoint &pixels, LEPoint &units) const
{
    units.fX = xPixelsToUnits(pixels.fX);
    units.fY = yPixelsToUnits(pixels.fY);
}

inline void LEFontInstance::transformFunits(float xFunits, float yFunits, LEPoint &pixels) const
{
    pixels.fX = xUnitsToPoints(xFunits) * getScaleFactorX();
    pixels.fY = yUnitsToPoints(yFunits) * getScaleFactorY();
}

inline float LEFontInstance::fixedToFloat(le_int32 fixed)
{
    return (float) (fixed / 65536.0);
}

inline le_int32 LEFontInstance::floatToFixed(float theFloat)
{
    return (le_int32) (theFloat * 65536.0);
}

inline le_int32 LEFontInstance::getLineHeight() const
{
    return getAscent() + getDescent() + getLeading();
}

U_NAMESPACE_END
#endif


