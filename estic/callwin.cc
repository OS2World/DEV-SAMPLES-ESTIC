/*****************************************************************************/
/*									     */
/*				    CALLWIN.CC				     */
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



#include "eventid.h"
#include "event.h"
#include "coll.h"
#include "textitem.h"
#include "settings.h"
#include "progutil.h"

#include "icmsg.h"
#include "icobjid.h"
#include "icevents.h"
#include "devstate.h"
#include "callwin.h"



// Register the class
LINK (CallWindow, ID_CallWindow);



/*****************************************************************************/
/*			       Message constants			     */
/*****************************************************************************/



const u16 msCallWindowTitle		= MSGBASE_ICCWIN + 0;
const u16 msCallWindowHeader1		= MSGBASE_ICCWIN + 1;
const u16 msCallWindowHeader2		= MSGBASE_ICCWIN + 2;



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Count of CallWindows
unsigned CallWindow::WindowCount = 0;



/*****************************************************************************/
/*			       class CallWindow				     */
/*****************************************************************************/



inline CallWindow::CallWindow (StreamableInit):
    IstecMsgWindow (Empty)
// Build constructor
{
    // One window more
    WindowCount++;

    // Tell the program that a new window is active
    PostEvent (evCallWinChange, WindowCount);
}



CallWindow::CallWindow ():
    IstecMsgWindow (msCallWindowTitle, msCallWindowHeader1,
		    msCallWindowHeader2, "CallWindow.Bounds")
// Construct an CallWindow
{
    // Ok, we have the window now
    WindowCount++;

    // Tell the program that a new window is active
    PostEvent (evCallWinChange, WindowCount);
}



CallWindow::~CallWindow ()
// Destruct an CallWindow
{
    // Decrease the window count and invalidate the global pointer
    WindowCount--;

    // Tell the program that a window has been destroyed
    PostEvent (evCallWinChange, WindowCount);
}



u16 CallWindow::StreamableID () const
{
    return ID_CallWindow;
}



Streamable* CallWindow::Build ()
// Make the window persistent
{
    return new CallWindow (Empty);
}



void CallWindow::HandleEvent (Event& E)
// Handle an incoming event
{
    // Call the derived function
    IstecMsgWindow::HandleEvent (E);
    if (E.Handled) {
	return;
    }

    // Switch on the type of the arriving event
    switch (E.What) {

	case evCallComplete:
	    Write (((DevStateInfo*) E.Info.O)->LogMsg ());
	    break;

    }
}



