/*
 * @(#)$RCSfile: TextPanelEvent.java,v $ $Revision: 1.1 $ $Date: 2000/04/20 17:51:23 $
 *
 * (C) Copyright IBM Corp. 1998-1999.  All Rights Reserved.
 *
 * The program is provided "as is" without any warranty express or
 * implied, including the warranty of non-infringement and the implied
 * warranties of merchantibility and fitness for a particular purpose.
 * IBM will not be liable for any damages suffered by you as a result
 * of using the Program. In no event will IBM be liable for any
 * special, indirect or consequential damages or lost profits even if
 * IBM has been advised of the possibility of their occurrence. IBM
 * will not be liable for any third party claims against you.
 */
package com.ibm.richtext.textpanel;

import java.util.EventObject;

/**
 * TextPanelEvent is generated by an MTextPanel to notify listeners
 * of changes.  To receive TextPanelEvents from an MTextPanel, clients
 * must implement TextPanelListener and add themselves to the MTextPanel's
 * list of listeners.
 * <p>
 * Some event types are special cases of others.  This is intentional - it
 * allows notifications to be sent less often in certain common cases.  For
 * example, a change in the selection range generates a SELECTION_RANGE_CHANGED 
 * event.  This is a very common occurrance, and if many clients listen for this
 * event, there may be a significant performance penalty.  By
 * listening for a more specialized event (such as SELECTION_EMPTY_CHANGED), clients
 * can reduce the number of notifications sent.
 * 
 * @see MTextPanel
 * @see TextPanelListener
 */
public final class TextPanelEvent extends EventObject {

    static final String COPYRIGHT =
                "(C) Copyright IBM Corp. 1998-1999 - All Rights Reserved";
    
    /**
     * The lower bound of TextPanelEvent ID's.
     */
    public static final int TEXT_PANEL_FIRST = 11;

    /**
     * Events of this type indicate a change in the selection range.
     * This occurs quite often.  Most clients do not need to be 
     * notified every time the selection range changes.
     */
    public static final int SELECTION_RANGE_CHANGED = 11;
    
    /**
     * Events of this type are sent when the selection range becomes
     * 0-length after not being 0-length, or vice versa.  This event
     * is a special case of SELECTION_RANGE_CHANGED.
     */
    public static final int SELECTION_EMPTY_CHANGED = 12;
    
    /**
     * Events of this type indicate that the text in the TextPanel changed.
     * This type of event occurs often.
     */
    public static final int TEXT_CHANGED = 13;
    
    /**
     * Events of this type are sent when the styles in the current
     * selection change.
     */
    public static final int SELECTION_STYLES_CHANGED = 14;
     
    /**
     * Events of this type are sent when the undo/redo state changes.
     */
    public static final int UNDO_STATE_CHANGED = 15;
    
    /**
     * Events of this type are sent when the clipboard state changes.
     */
    public static final int CLIPBOARD_CHANGED = 16;
    
    /**
     * Events of this type are sent when 
     * the wrap width of the text changes.
     */
    public static final int FORMAT_WIDTH_CHANGED = 17;

    /**
     * Events of this type are sent when the key remap changes.
     */
    public static final int KEYREMAP_CHANGED = 18;

    /**
     * The upper bound of TextPanelEvent ID's.
     */
    public static final int TEXT_PANEL_LAST = 18;

    private int fId;

    /**
     * Create a new TextPanelEvent.
     * @param source the MTextPanel which generated the event
     * @param id the ID for this event.  Must be within
     * [TEXT_PANEL_FIRST, TEXT_PANEL_LAST].
     */
    TextPanelEvent(MTextPanel source, int id) {

        super(source);
        if (id < TEXT_PANEL_FIRST || id > TEXT_PANEL_LAST) {
            throw new IllegalArgumentException("id out of range");
        }
        fId = id;
    }

    /**
     * Return the event ID for this event.  Event ID's are
     * one of the class constants.
     * @return the event ID for this event
     */
    public int getID() {

        return fId;
    }
    
    public String toString() {
        
        String desc = null;
        
        switch(fId) {
            case SELECTION_RANGE_CHANGED:
                desc = "SELECTION_RANGE_CHANGED";
                break;
            case SELECTION_EMPTY_CHANGED:
                desc = "SELECTION_EMPTY_CHANGED";
                break;
            case TEXT_CHANGED:
                desc = "TEXT_CHANGED";
                break;
            case SELECTION_STYLES_CHANGED:
                desc = "SELECTION_STYLES_CHANGED";
                break;
            case UNDO_STATE_CHANGED:
                desc = "UNDO_STATE_CHANGED";
                break;
            case CLIPBOARD_CHANGED:
                desc = "CLIPBOARD_CHANGED";
                break;
            case FORMAT_WIDTH_CHANGED:
                desc = "FORMAT_WIDTH_CHANGED";
                break;
            case KEYREMAP_CHANGED:
                desc = "KEYREMAP_CHANGED";
                break;
        }
        return "[TextPanelEvent:"+desc+"]";
    }
}
