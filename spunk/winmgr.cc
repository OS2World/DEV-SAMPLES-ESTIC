/*****************************************************************************/
/*									     */
/*				    WINMGR.CC				     */
/*									     */
/* (C) 1995-96	Ullrich von Bassewitz					     */
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



// This file contains some unnecessary global overrides (::) to work around
// a gcc bug



#include "msgid.h"
#include "streamid.h"
#include "eventid.h"
#include "listbox.h"
#include "stdmsg.h"
#include "menue.h"
#include "menuedit.h"
#include "filesel.h"
#include "progutil.h"
#include "settings.h"
#include "winmgr.h"



// Register the classes
LINK (WinColl, ID_WinColl);
LINK (WindowManager, ID_WindowManager);



/*****************************************************************************/
/*			      Message constants				     */
/*****************************************************************************/



const u16 msTooManyWindows	= MSGBASE_WINMGR +  0;



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Global accessible window manager instance
WindowManager* WinMgr;



/*****************************************************************************/
/*			Explicit template instantiation			     */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<ItemWindow>;
template class SortedCollection<ItemWindow, u16>;
template class ListBox<ItemWindow>;
#endif



/*****************************************************************************/
/*				 class WinColl				     */
/*****************************************************************************/



WinColl::WinColl (unsigned MaxWindows):
    SortedCollection <ItemWindow, u16> (MaxWindows, 0, 1)
{
}



WinColl::WinColl (StreamableInit):
    SortedCollection <ItemWindow, u16> (Empty)
{
}



u16 WinColl::StreamableID () const
// Return the objects stream ID
{
    return ID_WinColl;
}



Streamable* WinColl::Build ()
// Return an empty window
{
    return new WinColl (Empty);
}



int WinColl::Compare (const u16* Key1, const u16* Key2)
{
    if (*Key1 < *Key2) {
	return -1;
    } else if (*Key1 > *Key2) {
	return 1;
    } else {
	return 0;
    }
}



const u16* WinColl::KeyOf (const ItemWindow* Item)
{
    return &Item->GetWindowNumberRef ();
}



int WinColl::FindWindow (u16 Num)
// Return the index of the window with number Num
{
    int Index;
    return Search (&Num, Index) ? Index : -1;
}



int WinColl::FindWindow (const Window* Win)
// Return the index of the window Win
{
    int Index;
    return Search (&Win->GetWindowNumberRef (), Index) ? Index : -1;
}



/*****************************************************************************/
/*			       class WinListBox				     */
/*****************************************************************************/



class WinListBox: public ListBox <ItemWindow> {

protected:
    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    WinListBox (i16 aID, const Point& aSize, WindowItem* NextItem = NULL);

};



WinListBox::WinListBox (i16 aID, const Point& aSize, WindowItem* NextItem):
    ListBox <ItemWindow> ("", aID, aSize, atEditNormal, atEditBar, atEditHigh, NextItem)
{
}



void WinListBox::Print (int Index, int X, int Y, u16 Attr)
{
    // Get the entry
    ItemWindow* Win = Coll->At (Index);

    // Get the filename and pad it
    String Line = FormatStr (" %2d %s", Win->GetWindowNumber (),
					Win->GetHeader ().GetStr ());
    Line.Pad (String::Right, Size.X - 1);
    Line += ' ';

    // Print the name
    Owner->Write (X, Y, Line, Attr);
}



/*****************************************************************************/
/*				 class WinMgr				     */
/*****************************************************************************/



WindowManager::WindowManager (u16 aMaxWindows):
    Coll (new WinColl (aMaxWindows)),
    MaxWindows (aMaxWindows)
// Construct a window manager
{
}



WindowManager::WindowManager (StreamableInit):
    Coll (NULL)
// Build constructor
{
}



WindowManager::~WindowManager ()
// Delete the window manager instance
{
    // Delete the window collection
    delete Coll;
}



void WindowManager::Load (Stream& S)
// Load the object from a stream
{
    // Read the maximum window count from the stream
    S >> MaxWindows ;

    // Create a new window collection
    Coll = new WinColl (MaxWindows);

    // Read the count of windows from the stream
    u16 Count;
    S >> Count;

    // Read the windows and insert them into the collection
    while (Count--) {
	AddWindow ((ItemWindow*) S.Get ());
    }
}



void WindowManager::Store (Stream& S) const
// Store the object into a stream
{
    // Write the maximum window count to the stream
    S << MaxWindows;

    // Write the window count into the stream
    u16 Count = WindowCount ();
    S << Count;

    // Write all windows
    for (unsigned I = 0; I < Count; I++) {
	S.Put (Coll->At (I));
    }
}



u16 WindowManager::StreamableID () const
// Return the objects stream ID
{
    return ID_WindowManager;
}



Streamable* WindowManager::Build ()
// Return an empty WindowManager object
{
    return new WindowManager (Empty);
}



unsigned WindowManager::GetFreeWindowNumber ()
// Get the next free window number. If 9 windows are already open, print
// an error message and return 0
{
    // Search for an empty slot
    for (unsigned I = 1; I <= MaxWindows; I++) {
	if (Coll->FindWindow (I) < 0) {
	    return I;
	}
    }

    // No empty slot, there are already 9 windows
    ErrorMsg (::LoadMsg (msTooManyWindows));
    return 0;
}



int WindowManager::CloseWindow (unsigned Index)
// Close the window with the given Index. Return true if the window has been
// closed, return false if the window refuses to get closed
{
    // Parametercheck
    PRECONDITION (Index < (unsigned) Coll->GetCount ());

    // Ask the window if it can be closed
    ItemWindow* Win = Coll->At (Index);
    if (Win->CanClose () == 0) {
	// Nope, window said no
	return 0;
    }

    // Unregister the window key
    UnregisterKey (GetWindowKey (Win));

    // Delete the window
    Coll->AtDelete (Index);

    // If the count of windows is now one less than the maximum, post an
    // apropriate event
    if (Coll->GetCount () == MaxWindows - 1) {
	PostEvent (new Event (evWinMgrLastClose));
    }

    // If the window count is zero, post an apropriate event
    if (Coll->GetCount () == 0) {
	PostEvent (new Event (evWinMgrNoWindows));
    }

    // Success
    return 1;
}



Key WindowManager::GetWindowKey (const ItemWindow* W)
// Return the hotkey for the given window
{
    static const Key KeyTable [9] = {
	kbMeta1, kbMeta2, kbMeta3, kbMeta4, kbMeta5,
	kbMeta6, kbMeta7, kbMeta8, kbMeta9
    };

    // Get the window number
    unsigned Num = W->GetWindowNumber ();

    // Return the key
    if (Num >= 1 && Num <= 9) {
	return KeyTable [Num-1];
    } else {
	return kbNoKey;
    }
}



void WindowManager::DeleteWindow (u16 Num)
// Delete a window
{
    // Try to find the window
    int I = Coll->FindWindow (Num);
    CHECK (I >= 0);

    // Close the window
    CloseWindow (I);
}



ItemWindow* WindowManager::AddWindow (ItemWindow* Win)
// Add a window to the list. The window gets assigned  an unused number.
// If there are too many windows open, NULL is returned but the window
// is _not_ deleted. If all is ok, the pointer to the window just
// inserted is returned.
{
    // Try to honor an already existing window number if possible
    u16 Num = Win->GetWindowNumber ();
    if (Num != 0) {
	// The window already has a number. If this number is unused, don't
	// change it, otherwise assign a new one
	if (Coll->FindWindow (Num) != -1) {
	    // Number is already in use
	    Num = 0;
	}
    }

    // If the window does not have a number or if the number is already in
    // use, assign a new one
    if (Num == 0) {
	// Get a free window number
	Num = GetFreeWindowNumber ();
	if (Num == 0) {
	    // Too many windows already
	    return NULL;
	}
    }

    // Assign the window number
    Win->SetWindowNumber (Num);

    // Insert the window
    Coll->Insert (Win);

    // Show the window
    Win->Show ();

    // If the window count is now 1, post an appropriate event
    if (Coll->GetCount () == 1) {
	PostEvent (new Event (evWinMgrFirstOpen));
    }

    // If the window count has reached the maximum, post that
    if (Coll->GetCount () == MaxWindows) {
	PostEvent (new Event (evWinMgrMaxWindows));
    }

    // Register the window key
    RegisterKey (GetWindowKey (Win));

    // Return the created window
    return Win;
}



ItemWindow* WindowManager::ChooseWindow ()
// Choose a new active window
{
    // Name of the position setting
    static const String StgName = "WindowManager.ChooseWindow.Position";

    // Load the window
    Menue* Win = (Menue*) ::LoadResource ("WINMGR.ChooseWindow");

    // If we have a stored window position, use it (moving is cheap now since
    // the window is invisible)
    Point Pos = StgGetPoint (StgName, Win->OuterBounds ().A);
    Win->MoveAbs (Pos);

    // Create a listbox inside the window
    Point Size;
    Size.X = Win->IXSize ();
    Size.Y = Win->IYSize ();
    WinListBox* Box = new WinListBox (1, Size);
    Box->SetColl (Coll);
    Win->AddItem (Box);
    Box->Select ();
    Box->Draw ();
    Win->Activate ();

    // New status line
    PushStatusLine (siAbort | siSelectChooseKeys);

    // Allow choosing an entry
    int Done = 0;
    ItemWindow* HW = NULL;
    while (!Done) {

	// Get keyboard input
	Key K = ::KbdGet ();

	// Let the box look for a useful key
	Box->HandleKey (K);

	// Look what's left
	int Selected;
	switch (K) {

	    case vkResize:
		Win->MoveResize ();
		break;

	    case kbEnter:
		Selected = Box->GetSelected ();
		if (Selected != -1) {
		    HW = Coll->At (Selected);
		    Done = 1;
		}
		break;

	    case vkAbort:
	    case vkClose:
		Done = 1;
		break;

	}

    }

    // Restore the status line
    PopStatusLine ();

    // Set a new collection for the listbox (otherwise the box would try to
    // delete the collection)
    Box->SetColl (NULL);

    // Store the current window position
    StgPutPoint (Win->OuterBounds ().A, StgName);

    // Delete the window
    delete Win;

    // Return the selected window
    return HW;
}



void WindowManager::Browse (ItemWindow* W)
// Allow browsing the windows
{
    // Allow browsing the given window
    while (W) {

	// browse...
	Key K = W->Browse ();

	// Check the abort key
	int I;
	switch (K) {

	    case kbMeta1:
		W = FindWindow (1);
		break;

	    case kbMeta2:
		W = FindWindow (2);
		break;

	    case kbMeta3:
		W = FindWindow (3);
		break;

	    case kbMeta4:
		W = FindWindow (4);
		break;

	    case kbMeta5:
		W = FindWindow (5);
		break;

	    case kbMeta6:
		W = FindWindow (6);
		break;

	    case kbMeta7:
		W = FindWindow (7);
		break;

	    case kbMeta8:
		W = FindWindow (8);
		break;

	    case kbMeta9:
		W = FindWindow (9);
		break;

	    case vkClose:
		I = Coll->FindWindow (W);
		if (CloseWindow (I)) {
		    W = WinMgr->GetTopWindow ();
		}
		break;

	    case vkAbort:
		W = NULL;
		break;

	    default:
		// Key is handled by the main menue, quit browse func
		::KbdPut (K);
		W = NULL;
		break;

	}

    }
}



ItemWindow* WindowManager::FindWindow (unsigned WindowNum)
// Find a window by number
{
    int I = Coll->FindWindow (WindowNum);
    return I >= 0 ? Coll->At (I) : 0;
}



ItemWindow* WindowManager::FindWindowWithKey (Key WindowKey)
// Find a window by hotkey
{
    // Cannot search for a non existing key
    PRECONDITION (WindowKey != kbNoKey);

    // Searching by keys is not supported by Coll, so use a linear search here
    for (int I = 0; I < Coll->GetCount (); I++) {
	ItemWindow* Win = Coll->At (I);
	if (WindowKey == GetWindowKey (Win)) {
	    // Found
	    return Win;
	}
    }

    // Not found
    return NULL;
}



void WindowManager::CloseAll ()
// Close all windows
{
    unsigned I = Coll->GetCount ();
    while (I--) {
	CloseWindow (I);
    }
}



void WindowManager::VerticalTile (Rect Bounds, unsigned Start, unsigned End)
// Tile the windows with indices Start to end vertically in the given
// rectangle Bounds
{
    // Calculate the window count
    unsigned Count = End - Start + 1;

    // Calculate the height of one window
    unsigned RemainingHeight = Bounds.YSize ();


    // Get the rectangle for the first window
    Rect WinBounds = Bounds;
    WinBounds.A.Y = 1;

    // Loop over all windows but the last
    for (unsigned I = Start; I < End; I++) {

	// Recalculate the height
	unsigned Height = RemainingHeight / Count;
	RemainingHeight -= Height;
	Count--;

	// Resize the rectangle
	WinBounds.B.Y = WinBounds.A.Y + Height;

	// Resize the window, the move the rectangle
	Coll->At (I)->Resize (WinBounds);

	// Set the next rectangle start point
	WinBounds.A.Y = WinBounds.B.Y;

    }

    // When drawing the last window, adjust for the accumulated error
    WinBounds.B.Y = Bounds.B.Y;
    Coll->At (End)->Resize (WinBounds);
}



void WindowManager::Tile ()
// Tile all windows on the screen
{
    // Get the window count
    unsigned WindowCount = Coll->GetCount ();

    // Nothing to do if we have no windows
    if (WindowCount == 0) {
	return;
    }

    // Get the available background area
    Rect Screen = Background->OuterBounds ();
    Screen.Grow (0, -1);

    // Set up for the window rows
    if (WindowCount <= 3) {

	// This one is easy...
	VerticalTile (Screen, 0, WindowCount-1);

    } else if (WindowCount <= 8) {

	// Two rows of windows
	Rect Bounds = Screen;
	Bounds.B.X /= 2;
	unsigned FirstRowCount = WindowCount / 2;
	VerticalTile (Bounds, 0, FirstRowCount-1);

	Bounds.A.X = Bounds.B.X;
	Bounds.B.X = Screen.B.X;
	VerticalTile (Bounds, FirstRowCount, WindowCount-1);

    } else {

	// Three rows
	Rect Bounds = Screen;
	unsigned Width = Bounds.XSize () / 3;
	Bounds.B.X = Width;
	unsigned FirstRowCount = WindowCount / 3;
	VerticalTile (Bounds, 0, FirstRowCount-1);

	Bounds.Move (Width, 0);
	VerticalTile (Bounds, FirstRowCount, 2*FirstRowCount-1);

	Bounds.Move (Width, 0);
	Bounds.B.X = Screen.B.X;		// Correct the errors
	VerticalTile (Bounds, 2*FirstRowCount, WindowCount-1);

    }
}



void WindowManager::Cascade ()
// Build a window cascade on the screen
{
    // Get the desktop background
    Rect Bounds = Background->GetDesktop ();

    // Now loop over all windows
    for (int I = 0; I < Coll->GetCount (); I++) {
	ItemWindow* Win = Coll->At (I);
	Win->Resize (Bounds);
	Win->PutOnTop ();
	Bounds.A.X++;
	Bounds.A.Y++;
    }
}



ItemWindow* WindowManager::GetTopWindow ()
// Return the uppermost ItemWindow or NULL (if there are no windows)
{
    // Return the topmost window
    Window* Win = Background->GetTopWindow ();

    // Now loop through the windows list and check if the given window is
    // in the list
    while (Win != NULL && Coll->FindWindow (Win) == -1) {
	// Get the next lower window
	Win = Win->LowerWindow ();
    }

    // Return the result
    return (ItemWindow*) Win;
}



int WindowManager::CanClose ()
// Return true if all windows answer yes to CanClose, false otherwise
{
    for (int I = 0; I < Coll->GetCount (); I++) {
	    if (Coll->At (I)->CanClose () == 0) {
	    return 0;
	}
    }
    return 1;
}



