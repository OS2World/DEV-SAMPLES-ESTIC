/*****************************************************************************/
/*									     */
/*				  ICMSGWIN.H				     */
/*									     */
/* (C) 1996	Ullrich von Bassewitz					     */
/*		Wacholderweg 14						     */
/*		D-70597 Stuttgart					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// A window that holds a header and an area for messages



#ifndef _ICMSGWIN_H
#define _ICMSGWIN_H



#include "itemwin.h"



/*****************************************************************************/
/*			     class IstecMsgWindow			     */
/*****************************************************************************/



class IstecMsgWindow: public ItemWindow {

    class StringMsgColl* Messages;	// List of messages
    Rect		 ZoomSize;	// Small size when zooming
    String		 SavedSizeName; // Name of saved size in settings file


public:
    IstecMsgWindow (StreamableInit);
    // Build constructor

    IstecMsgWindow (unsigned msWindowTitle, unsigned msWindowHeader1,
		    unsigned msWindowHeader2, const String& aSizeName);
    // Construct an IstecMsgWindow

    virtual ~IstecMsgWindow ();
    // Destruct an IstecMsgWindow

    virtual void Store (Stream &) const;
    virtual void Load (Stream &);
    // Make the window persistent

    virtual void DrawInterior ();
    // Draw the window contents

    virtual unsigned MinXSize () const;
    // Return the minimum X size of the window. Override this to limit resizing.

    virtual unsigned MinYSize () const;
    // Return the minumim Y size of the window. Override this to limit resizing.

    virtual void Zoom ();
    // Zoom the window

    void Write (const String& S);
    // Write a line to the window, advance the cursor

};



inline IstecMsgWindow::IstecMsgWindow (StreamableInit):
    ItemWindow (Empty)
// Build constructor
{
}



// End of ICMSGWIN.H

#endif

