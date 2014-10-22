/*****************************************************************************/
/*									     */
/*				     CLIWIN.CC				     */
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
#include "iccli.h"
#include "cliwin.h"



// Register the class
LINK (CLIWindow, ID_CLIWindow);



/*****************************************************************************/
/*			       Message constants			     */
/*****************************************************************************/



const u16 msCLIWindowTitle		= MSGBASE_ICCLIWIN + 0;
const u16 msCLIWindowHeader1		= MSGBASE_ICCLIWIN + 1;
const u16 msCLIWindowHeader2		= MSGBASE_ICCLIWIN + 2;



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Count of CLIWindows
unsigned CLIWindow::WindowCount = 0;



/*****************************************************************************/
/*				class CLIWindow				     */
/*****************************************************************************/



inline CLIWindow::CLIWindow (StreamableInit):
    IstecMsgWindow (Empty)
// Build constructor
{
    // One window more
    WindowCount++;

    // Tell the program that a new window is active
    PostEvent (evCLIWinChange, WindowCount);
}



CLIWindow::CLIWindow ():
    IstecMsgWindow (msCLIWindowTitle, msCLIWindowHeader1,
		    msCLIWindowHeader2, "CLIWindow.Bounds")
// Construct an CLIWindow
{
    // Ok, we have the window now
    WindowCount++;

    // Tell the program that a new window is active
    PostEvent (evCLIWinChange, WindowCount);
}



CLIWindow::~CLIWindow ()
// Destruct an CLIWindow
{
    // Decrease the window count and invalidate the global pointer
    WindowCount--;

    // Tell the program that a window has been destroyed
    PostEvent (evCLIWinChange, WindowCount);
}



u16 CLIWindow::StreamableID () const
{
    return ID_CLIWindow;
}



Streamable* CLIWindow::Build ()
// Make the window persistent
{
    return new CLIWindow (Empty);
}



void CLIWindow::HandleEvent (Event& E)
// Handle an incoming event
{
    // Call the derived function
    IstecMsgWindow::HandleEvent (E);
    if (E.Handled) {
	return;
    }

    // Switch on the type of the arriving event
    switch (E.What) {

	case evIncomingLogMsg:
	    CLI* Info;
	    Info = ((CLI*) E.Info.O);
	    Write (* (String*) E.Info.O);
	    break;

    }
}



