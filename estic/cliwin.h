/*****************************************************************************/
/*									     */
/*				     CLIWIN.H				     */
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



// Calling line identification window



#ifndef _CLIWIN_H
#define _CLIWIN_H



#include "icmsgwin.h"



/*****************************************************************************/
/*				class CLIWindow				     */
/*****************************************************************************/



class CLIWindow: public IstecMsgWindow {

    static unsigned	WindowCount;	// Count of CLIWindows

public:
    CLIWindow (StreamableInit);
    // Build constructor

    CLIWindow ();
    // Construct an CLIWindow

    virtual ~CLIWindow ();
    // Destruct an CLIWindow

    virtual u16 StreamableID () const;
    static Streamable* Build ();
    // Make the window persistent

    virtual void HandleEvent (Event& E);
    // Handle an incoming event

};



// End of CLIWIN.H

#endif

