/*****************************************************************************/
/*									     */
/*				  WINSIZE.CC				     */
/*									     */
/* (C) 1995	Ullrich von Bassewitz					     */
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



#include "msgid.h"
#include "window.h"
#include "progutil.h"



/*****************************************************************************/
/*			       Message constants			     */
/*****************************************************************************/



const u16 msMove			= MSGBASE_WINSIZE +  0;
const u16 msResize			= MSGBASE_WINSIZE +  1;



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



void MoveResize (class Window* Win)
// Allow moving and resizing a window
{
    // If moving/resizing is not allowed, return now
    if (Win->CanMove () == 0 && Win->CanResize () == 0) {
	return;
    }

    // Remember the current cursor and switch off the cursor
    Window::CursorType CT = Win->GetCursor ();
    Win->SetCursorOff ();

    // Tell the window what we are doing
    Win->StartResizing ();

    // New status line
    u32 StatusFlags = siAbort | siEnter;
    if (Win->CanMove ()) {
	StatusFlags |= siMoveKeys;
    }
    if (Win->CanResize ()) {
	StatusFlags |= siResizeKeys;
    }
    if (HelpAvail ()) {
	StatusFlags |= siHelp;
    }
    PushStatusLine (StatusFlags);

    // Calculate the desktop size
    Rect Desktop = Background->GetDesktop ();

    // Remember the window shape
    Rect OldBounds = Win->OuterBounds ();
    Rect NewBounds;

    // Now let's size/move...
    int Done = 0;
    while (!Done) {

	switch (KbdGet ()) {

	    case vkHelp:
//		CallHelp ("MoveResize.Help");
		break;

	    case vkAbort:
		// Abort the operation. If the window size has been changed,
		// use Resize to restore the old size. Otherwise just use Move.
		// This prevents the text in simple text windows to vanish
		// when aborting a move/resize operation.
		if (int (Win->OXSize ()) != int (OldBounds.XSize ()) ||
		    int (Win->OYSize ()) != int (OldBounds.YSize ())) {
		    // Size has changed, resize
		    Win->Resize (OldBounds);
		} else {
		    // Size hasn't changed, move the window
		    Win->MoveAbs (OldBounds.A);
		}
		Done = 1;
		break;

	    case vkAccept:
	    case kbEnter:
		Done = 1;
		break;

	    case vkUp:
		if (Win->CanMove ()) {
		    NewBounds = Win->OuterBounds ();
		    if (NewBounds.A.Y > Desktop.A.Y) {
			Win->MoveRel (Point (0, -1));
		    }
		}
		break;

	    case vkDown:
		if (Win->CanMove ()) {
		    NewBounds = Win->OuterBounds ();
		    if (NewBounds.B.Y < Desktop.B.Y) {
			Win->MoveRel (Point (0, 1));
		    }
		}
		break;

	    case vkLeft:
		if (Win->CanMove ()) {
		    NewBounds = Win->OuterBounds ();
		    if (NewBounds.A.X > Desktop.A.X) {
			Win->MoveRel (Point (-1, 0));
		    }
		}
		break;

	    case vkRight:
		if (Win->CanMove ()) {
		    NewBounds = Win->OuterBounds ();
		    if (NewBounds.B.X < Desktop.B.X) {
			Win->MoveRel (Point (1, 0));
		    }
		}
		break;

	    case vkCtrlUp:
		if (Win->CanResize ()) {
		    if (Win->OYSize () > Win->MinYSize ()) {
			NewBounds = Win->OuterBounds ();
			NewBounds.B.Y--;
			Win->Resize (NewBounds);
		    }
		}
		break;

	    case vkCtrlLeft:
		if (Win->CanResize ()) {
		    if (Win->OXSize () > Win->MinXSize ()) {
			NewBounds = Win->OuterBounds ();
			NewBounds.B.X--;
			Win->Resize (NewBounds);
		    }
		}
		break;

	    case vkCtrlDown:
		if (Win->CanResize ()) {
		    NewBounds = Win->OuterBounds ();
		    if (NewBounds.B.Y < Desktop.B.Y) {
			NewBounds.B.Y++;
			Win->Resize (NewBounds);
		    }
		}
		break;

	    case vkCtrlRight:
		if (Win->CanResize ()) {
		    NewBounds = Win->OuterBounds ();
		    if (NewBounds.B.X < Desktop.B.X) {
			NewBounds.B.X++;
			Win->Resize (NewBounds);
		    }
		}
		break;

	}

    }

    // Restore the status line
    PopStatusLine ();

    // Stop the resizing
    Win->EndResizing ();

    // Set the old cursor type
    Win->SetCursor (CT);

}



