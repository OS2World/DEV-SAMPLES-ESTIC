/*****************************************************************************/
/*									     */
/*				     IMON.CC				     */
/*									     */
/* (C) 1995-96	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#include "chartype.h"
#include "itemwin.h"
#include "textitem.h"
#include "strparse.h"
#include "progutil.h"
#include "stdmsg.h"
#include "syserror.h"
#include "settings.h"
#include "winmgr.h"

#include "icobjid.h"
#include "icmsg.h"
#include "icevents.h"
#include "imon.h"



// Register the classes
LINK (IMonWindow, ID_IMonWindow);



/*****************************************************************************/
/*			       Message Constants			     */
/*****************************************************************************/



const u16 msIMonWindowTitle	= MSGBASE_ICIMON +  0;
const u16 msIMonHeader1		= MSGBASE_ICIMON +  1;
const u16 msIMonHeader2		= MSGBASE_ICIMON +  2;
const u16 msIncoming		= MSGBASE_ICIMON +  3;
const u16 msOutgoing		= MSGBASE_ICIMON +  4;
const u16 msUsageNone		= MSGBASE_ICIMON +  5;
const u16 msUsageRaw		= MSGBASE_ICIMON +  6;
const u16 msUsageModem		= MSGBASE_ICIMON +  7;
const u16 msUsageNet		= MSGBASE_ICIMON +  8;
const u16 msUsageVoice		= MSGBASE_ICIMON +  9;
const u16 msUsageFax		= MSGBASE_ICIMON + 10;
const u16 msUsageUnknown	= MSGBASE_ICIMON + 11;
const u16 msNoInfoFile		= MSGBASE_ICIMON + 12;
const u16 msReadError		= MSGBASE_ICIMON + 13;



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Name that the window uses to store it's position in the settings file
static const String IMonPosition = "IMonWindow.Position";

// Define the ISDN stuff
static const char InfoFileName [] = "/dev/isdninfo";

#ifdef CLEAN_IMON_SOURCE
#include "/usr/src/isdn/include/isdn.h"
#else
// Define the ISDN stuff
const unsigned ISDN_USAGE_NONE		= 0;
const unsigned ISDN_USAGE_RAW		= 1;
const unsigned ISDN_USAGE_MODEM		= 2;
const unsigned ISDN_USAGE_NET		= 3;
const unsigned ISDN_USAGE_VOICE		= 4;
const unsigned ISDN_USAGE_FAX		= 5;
const unsigned ISDN_USAGE_MASK		= 127;
const unsigned ISDN_USAGE_OUTGOING	= 128;
#endif

// Count of IMonWindows
unsigned IMonWindow::WindowCount = 0;



/*****************************************************************************/
/*				 class IMonWindow			     */
/*****************************************************************************/



IMonWindow::IMonWindow (const Point& Pos):
    ItemWindow (Rect (Pos.X, Pos.Y, Pos.X+4, Pos.Y+3),
		wfFramed | wfCanMove | wfCanResize | wfSaveVisible),
    Status (0),
    F (NULL),
    ZoomSize (OBounds)
// Construct an IMonWindow
{
    // Lock window output
    Lock ();

    // Open the info file
    F = fopen (InfoFileName, "r");
    if (F == NULL) {
	// We had an error opening the file.  Do *not* return but construct
	// the window. The window will show but nothing more will happen.
	// This way it's more compatible with the window manager...
	ErrorMsg (FormatStr (LoadAppMsg (msNoInfoFile).GetStr (), InfoFileName));
	Status = errno;
    }

    // Resize the window in X direction (Work() will resize the Y dir)
    const String& Header1 = LoadAppMsg (msIMonHeader1);
    const String& Header2 = LoadAppMsg (msIMonHeader2);

    Rect NewBounds = OBounds;
    NewBounds.B.X = NewBounds.A.X + Header1.Len () + 2;
    Resize (NewBounds);

    // If there is a stored window position in the settings file, move the
    // window to this position. As the window is invisible, this operation
    // is cheap and is performed in any case.
    MoveAbs (StgGetPoint (IMonPosition, OBounds.A));

    // Set the header
    SetHeader (LoadAppMsg (msIMonWindowTitle));

    // Create and insert the items for both header lines
    TextItem* H1 = new TextItem (Header1, 100, atTextNormal, NULL);
    TextItem* H2 = new TextItem (Header2, 101, atTextNormal, NULL);
    AddItem (H1);
    AddItem (H2);
    H1->SetWidth (Header1.Len ());
    H2->SetWidth (Header2.Len ());
    H1->SetPos (0, 0);
    H2->SetPos (0, 1);

    // Ok, we now have one window more
    WindowCount++;

    // Make a first update of the window. This will insert the needed
    // TextItems into the window an resize it according to the channels
    // available.
    Update ();

    // Unlock the output
    Unlock ();

    // Tell the application that the window count has changed
    PostEvent (evIMonWinChange, WindowCount);
}



IMonWindow::IMonWindow (StreamableInit):
    ItemWindow (Empty)
{
    // One window more
    WindowCount++;

    // Tell the application that the window count has changed
    PostEvent (evIMonWinChange, WindowCount);
}



IMonWindow::~IMonWindow ()
// Destruct an IMonWindow
{
    // Close the file
    if (F) {
	fclose (F);
    }

    // Write the current position to the settings file
    StgPutPoint (OBounds.A, IMonPosition);

    // One window less
    WindowCount--;

    // Tell the application that the window count has changed
    PostEvent (evIMonWinChange, WindowCount);
}



void IMonWindow::Store (Stream &S) const
// Store the window in a stream
{
    // Store the data from ItemWindow
    ItemWindow::Store (S);

    // Store additional data
    S << ZoomSize;
}



void IMonWindow::Load (Stream &S)
// Load the window from a stream
{
    // Load the parental data
    ItemWindow::Load (S);

    // Load additional data
    S >> ZoomSize;

    // We have to open the file again
    F = fopen (InfoFileName, "r");
    if (F == NULL) {
	ErrorMsg (FormatStr (LoadAppMsg (msNoInfoFile).GetStr (), InfoFileName));
	Status = errno;
    } else {
	Status = 0;
    }
}



u16 IMonWindow::StreamableID () const
// Return the stream ID
{
    return ID_IMonWindow;
}



Streamable* IMonWindow::Build ()
// Create an empty IMonWindow
{
    return new IMonWindow (Empty);
}



unsigned IMonWindow::MinXSize () const
// Return the minimum X size of the window. Override this to limit resizing.
{
    return 20;
}



unsigned IMonWindow::MinYSize () const
// Return the minumim Y size of the window. Override this to limit resizing.
{
    return 6;		// Two lines at least
}



void IMonWindow::Zoom ()
// Zoom the window
{
    // Get the desktop bounds
    Rect Desktop = Background->GetDesktop ();

    // Check if we must zoom in or out
    if (OBounds != Desktop) {
	// Remember the old size, then zoom out
	ZoomSize = OBounds;
	Resize (Desktop);
    } else {
	// Zoom in
	Resize (ZoomSize);
    }
}



void IMonWindow::Update ()
// Update the window if information has changed
{
    // Bail out if we had an error
    if (F == NULL || Status != 0) {
	return;
    }

    // Since isdn4linux 0.7beta, there will only be information available,
    // if something has changed. Use select() to make shure, we can read
    // from the device.
    // Timeout is zero
    timeval Timeout;
    Timeout.tv_usec = 0;
    Timeout.tv_sec  = 0;

    // Set the file descriptor
    fd_set Desc;
    FD_ZERO (&Desc);
    FD_SET (fileno (F), &Desc);

    // Check input status, return if no data available
    int Res = select (fileno (F) + 1, &Desc, NULL, NULL, &Timeout);
    if (Res == 0) {
	// No info
	return;
    } else if (Res < 0) {
	// Some sort of error. Be shure to set the status before displaying an
	// error message
	Status = errno;
	ErrorMsg (GetSysErrorMsg (errno));
	return;
    }

    // Read 6 lines from the file
    String Lines [li_count];
    for (unsigned I = 0; I < li_count; I++) {
	char Line [512];
	if (fgets (Line, sizeof (Line), F) == NULL) {
	    // Error reading the file, be careful to set the status code before
	    // showing the error message to avoid recursive calls
	    if ((Status = errno) == 0) {
		// OOPS - no error. Use generic error code
		Status = -1;
	    }
	    // Display an error message
	    ErrorMsg (FormatStr (LoadAppMsg (msReadError).GetStr (), InfoFileName));
	    return;
	}

	// Make a string and remove the trailing newline (if present)
	Lines [I] = &Line [7];
	int Len = Lines [I].Len ();
	while (Len >= 0 && IsSpace (Lines [I] [Len-1])) {
	    Len--;
	    Lines [I].Trunc (Len);
	}

    }

    // Lock the output
    Lock ();

    // Set up a parser object for each line
    StringParser SP_ChMap (Lines [li_chmap], StringParser::SkipWS);
    StringParser SP_DrMap (Lines [li_drmap], StringParser::SkipWS);
    StringParser SP_Usage (Lines [li_usage], StringParser::SkipWS);
    StringParser SP_Phone (Lines [li_phone], StringParser::SkipWS);

    // Insert TextItems if some are missing, resize the window, write the
    // lines to the window
    unsigned Y = 2;			// First free Y position
    unsigned ItemID = 1;		// ID of first TextItem
    while (1) {

	// Get the infos
	i32 Driver;
	if (SP_DrMap.GetI32 (Driver) != 0) {
	    // Error
	    break;
	}
	i32 Channel;
	if (SP_ChMap.GetI32 (Channel) != 0) {
	    // Error
	    break;
	}
	u32 Usage;
	if (SP_Usage.GetU32 (Usage) != 0) {
	    // Error
	    break;
	}
	String Phone;
	if (SP_Phone.GetToken (Phone) != 0) {
	    // Error
	    break;
	}

	// This is the last entry if the driver number is -1
	if (Driver < 0) {
	    break;
	}

	// Get the service message
	unsigned MsgNum;
	switch (Usage & ISDN_USAGE_MASK) {

	    case ISDN_USAGE_NONE:
		MsgNum = msUsageNone;
		break;

	    case ISDN_USAGE_RAW:
		MsgNum = msUsageRaw;
		break;

	    case ISDN_USAGE_NET:
		MsgNum = msUsageNet;
		break;

	    case ISDN_USAGE_MODEM:
		MsgNum = msUsageModem;
		break;

	    case ISDN_USAGE_VOICE:
		MsgNum = msUsageVoice;
		break;

	    case ISDN_USAGE_FAX:
		MsgNum = msUsageFax;
		break;

	    default:
		MsgNum = msUsageUnknown;
		break;

	}
	String Service = LoadAppMsg (MsgNum);
	Service.Pad (String::Right, 10);

	// Get the direction message, but beware: If the channel is unused,
	// there is no in/out message
	String InOut;
	if ((Usage & ISDN_USAGE_MASK) != ISDN_USAGE_NONE) {
	    if (Usage & ISDN_USAGE_OUTGOING) {
		InOut = LoadAppMsg (msOutgoing);
	    } else {
		InOut = LoadAppMsg (msIncoming);
	    }
	}
	InOut.Pad (String::Center, 7);

	// Setup the line
	// This is the header:	 " Dr  Ch  Ein/Aus  Dienst      Nummer"
	String Line = FormatStr (" %2d  %2d  %s  %s  %s",
				 Driver,
				 Channel,
				 InOut.GetStr (),
				 Service.GetStr (),
				 Phone.GetStr ());

	// Check if the needed textitem is available. If yes, the window does
	// not need resizing
	TextItem* Item = (TextItem*) ItemWithID (ItemID);
	if (Item == NULL) {

	    // Item does not exist - insert one
	    Item = new TextItem (Line, ItemID, atTextNormal, NULL);
	    AddItem (Item);

	    // Adjust position an width
	    Item->SetPos (0, Y);
	    Item->SetWidth (IXSize ());

	    // Make shure, the window is large enough to show the item
	    if (IYSize () <= Y) {
		// We need to resize the window
		Rect NewBounds = OBounds;
		NewBounds.B.Y = NewBounds.A.Y + Y + 3;
		Resize (NewBounds);
	    }

	} else {

	    // Window item exists, just set the item text
	    Item->SetText (Line);

	}

	// Next line, next item
	Y++;
	ItemID++;
    }

    // Unlock the window. This will cause a window update
    Unlock ();
}



void IMonWindow::HandleEvent (Event& E)
// Handle incoming events. Calls Update() if the application is idle
{
    // Call the derived function
    ItemWindow::HandleEvent (E);
    if (E.Handled) {
	return;
    }

    // Check which event
    switch (E.What) {

	case evSecondChange:
	    // Update the window
	    Update ();
	    break;

    }
}



