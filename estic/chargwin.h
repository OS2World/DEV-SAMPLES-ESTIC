/*****************************************************************************/
/*                                                                           */
/*                                  CHARGWIN.H                               */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
/*              Wacholderweg 14                                              */
/*              D-70597 Stuttgart                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// Charge display window



#ifndef _CHARGWIN_H
#define _CHARGWIN_H



#include "itemwin.h"



/*****************************************************************************/
/*                            class ChargeWindow                             */
/*****************************************************************************/



class ChargeWindow: public ItemWindow {

    static unsigned             WindowCount;
    // Count of ChargeWindows

    Rect                        ZoomSize;
    // Small window size for zooming

    class ChargeListbox*        Box;
    // Pointer to the charge listbox


protected:
    virtual void HandleKey (Key& K);
    // Key dispatcher used in Browse

public:
    ChargeWindow (StreamableInit);
    // Build constructor

    ChargeWindow ();
    // Construct an ChargeWindow

    virtual ~ChargeWindow ();
    // Destruct an ChargeWindow

    virtual void Store (Stream &) const;
    virtual void Load (Stream &);
    virtual u16 StreamableID () const;
    static Streamable* Build ();
    // Make the window persistent

    virtual unsigned MinXSize () const;
    // Return the minimum X size of the window. Override this to limit resizing.

    virtual unsigned MinYSize () const;
    // Return the minumim Y size of the window. Override this to limit resizing.

    virtual void Resize (const Rect& NewBounds);
    // Resize the window to the new bounds (this can also be used to move the
    // window but Move is faster if the window should not be resized).

    virtual void Zoom ();
    // Zoom the window

    void Update ();
    // Update the window if information has changed

    void HandleEvent (Event& E);
    // Handle incoming events. Calls Update() if the application is idle

};



// End of CHARGWIN.H

#endif

