/*****************************************************************************/
/*									     */
/*				    CALLWIN.H				     */
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



// Call window



#ifndef _CALLWIN_H
#define _CALLWIN_H



#include "icmsgwin.h"



/*****************************************************************************/
/*			       class CallWindow				     */
/*****************************************************************************/



class CallWindow: public IstecMsgWindow {

    static unsigned	WindowCount;	// Count of CallWindows

public:
    CallWindow (StreamableInit);
    // Build constructor

    CallWindow ();
    // Construct an CallWindow

    virtual ~CallWindow ();
    // Destruct an CallWindow

    virtual u16 StreamableID () const;
    static Streamable* Build ();
    // Make the window persistent

    virtual void HandleEvent (Event& E);
    // Handle an incoming event

};



// End of CALLWIN.H

#endif

