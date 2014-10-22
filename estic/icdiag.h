/*****************************************************************************/
/*                                                                           */
/*                                 ICDIAG.H                                  */
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



#ifndef _ICDIAG_H
#define _ICDIAG_H



#include "itemwin.h"



/*****************************************************************************/
/*                            class MatrixWindow                             */
/*****************************************************************************/



class MatrixWindow: public ItemWindow {

    class MatrixListBox*        MatrixBox;      // Direct pointer to listbox
    u16                         DevCount;       // Device count
    Rect                        ZoomSize;       // Size for zooming

    static unsigned             WindowCount;    // Count of windows


protected:
    virtual void HandleKey (Key& K);
    // Key dispatcher used in Browse

public:
    MatrixWindow (const Point& Pos, unsigned aDevCount);
    // Create a matrix window

    MatrixWindow (StreamableInit);
    // Create an empty object

    virtual ~MatrixWindow ();
    // Destroy a matrix window

    virtual void Store (Stream&) const;
    // Store the object into a stream

    virtual void Load (Stream&);
    // Load the object from a stream

    virtual u16 StreamableID () const;
    // Return the streamable ID

    static Streamable* Build ();
    // Build an empty object

    virtual unsigned MinXSize () const;
    // Return the minimum X size of the window. Override this to limit resizing.

    virtual unsigned MinYSize () const;
    // Return the minumim Y size of the window. Override this to limit resizing.

    virtual void Resize (const Rect& NewBounds);
    // Resize the window to the new bounds (this can also be used to move the
    // window but Move is faster if the window should not be resized).

    virtual void Zoom ();
    // Zoom the window

    void DrawMatrix ();
    // Redraw the listbox after changes

    virtual void HandleEvent (Event& E);
    // Handle incoming events
};



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



String DiagMsgDesc (const unsigned char* Msg);
// Return a textual description of the given diagnostic message

void HandleDiagMsg (const unsigned char* Msg);
// Handle a diagnostic message from the istec



// End of ICDIAG.H

#endif




