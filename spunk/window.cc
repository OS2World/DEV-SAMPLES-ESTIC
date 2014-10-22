/*****************************************************************************/
/*									     */
/*				   WINDOW.CC				     */
/*									     */
/* (C) 1993-96	Ullrich von Bassewitz					     */
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



#include "eventid.h"
#include "window.h"
#include "winattr.h"
#include "screen.h"
#include "palette.h"
#include "program.h"
#include "streamid.h"



// Register class Window
LINK (Window, ID_Window);
LINK (RootWindow, ID_RootWindow);



/*****************************************************************************/
/*			Explicit template instantiation			     */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class ListNode<Window>;
#endif



/*****************************************************************************/
/*				   Locking				     */
/*****************************************************************************/



// Default locking/unlocking functions are NOPs
static void WinLockFunc ()
{
}



static void WinUnlockFunc ()
{
}



// Function vectors to lock/unlock the windows system
void (*WinLock) ()     = WinLockFunc;
void (*WinUnlock) ()   = WinUnlockFunc;



/*****************************************************************************/
/*			       class RootWindow				     */
/*****************************************************************************/



// The root (background) window
RootWindow* Background;



RootWindow::RootWindow ():
    Window ()
// Create the background
{
    // Assign to the variables ob the parent class. Because an (almost)
    // empty constructor of class window is used, they are not set before.
    Number	= 0;
    Flags	= wfVisible;
    LockCount	= 0;
    OBounds	= Rect (0, 0, TheScreen->GetXSize (), TheScreen->GetYSize ());
    IBounds	= OBounds;
    Cursor	= csOff;
    CursorPos	= Point (0, 0);
    Palette	= paRoot;
    BGAttr	= atTextNormal;
    BGChar	= ' ';


    // Calculate the memory requirements
    VirtualScreenSize = OBounds.Chars ();

    // Allocate that memory
    VirtualScreen = new u16 [VirtualScreenSize];

    // Set the module variable for the background window to "this"
    Background = this;

    // Clear the window
    Clear ();

}



u16 RootWindow::StreamableID () const
{
    return ID_RootWindow;
}



Streamable* RootWindow::Build ()
{
    return new RootWindow (Empty);
}



void RootWindow::ScreenSizeChanged (const Rect& NewSize)
// Called when the screen got another resolution. NewSize is the new
// screen size. This functions calls ScreenSizeChanged of all other
// windows on the screen.
{
    // Lock the window system
    WinLock ();

    // Assign the new screen size
    OBounds = NewSize;
    IBounds = NewSize;

    // Delete the old virtual screen
    delete [] VirtualScreen;

    // Calculate the new memory requirements
    VirtualScreenSize = OBounds.Chars ();

    // Allocate that memory
    VirtualScreen = new u16 [VirtualScreenSize];

    // Clear the window
    Clear ();

    // Unlock the window system
    WinUnlock ();
}



void RootWindow::RedrawScreen ()
// Redraw the complete screen in case it's garbled
{
    UpdateScreen (OBounds);
}



void RootWindow::ChangeScreenSize (const Rect& NewSize)
// Tell all windows about a changed screen size
{
    // Lock the window system
    WinLock ();

    // Tell the windows about the changed size
    ListNode<Window>* N = &WNode;
    do {
	N->Contents () -> ScreenSizeChanged (NewSize);
	N = N->Next ();
    } while (N != &WNode);

    // Unlock the window system
    WinUnlock ();
}



Window* RootWindow::GetTopWindow ()
// Return the uppermost window. If there are no windows, return NULL
{
    Window* PrevWindow = WNode.Prev () -> Contents ();
    return PrevWindow != this ? PrevWindow : (Window*) NULL;
}



Window* RootWindow::GetTopVisibleWindow ()
// Return the uppermost window that is visible. If there are no visible
// windows, return NULL
{
    ListNode<Window>* N = WNode.Prev ();
    while (N != &WNode) {
	Window* Win = N->Contents ();
	if (Win->IsVisible ()) {
	    // Found the first visible window
	    return Win;
	}
	N = N->Next ();
    }

    // Found no visible window
    return NULL;
}



Rect RootWindow::GetDesktop () const
// Get the absolute coords of the desktop area
{
    // Get the complete screen
    Rect Desktop = OBounds;

    // Now adjust for the status line and main menue
    if (App->StatusLine) {
	Desktop.B.Y--;
    }
    if (App->MainMenue) {
	Desktop.A.Y++;
    }

    // Return the result
    return Desktop;
}



void RootWindow::HandleEvent (Event& E)
// Handle an incoming event. Default is to do nothing.
{
    switch (E.What) {

	case evScreenSizeChange:
	    ChangeScreenSize (* (const Rect*) E.Info.O);
	    break;

    }
}



/*****************************************************************************/
/*				 class Window				     */
/*****************************************************************************/



Window::Window (const Rect& Bounds, u16 aState, u16 aPalette,
		u16 aNumber, int aLockCount):
    Number (aNumber),
    Flags (aState),
    Options (0),
    LockCount (aLockCount),
    OBounds (Bounds),
    Cursor (csOff),
    CursorPos (0, 0),
    Palette (aPalette),
    BGChar (' '),
    BGAttr (atTextNormal),
    VirtualScreen (NULL),
    VirtualScreenSize (0),
    WNode (this),
    ANode (this)
{
    // Set inner window
    IBounds = OBounds;
    if (IsFramed ()) {
	IBounds.Grow (-1, -1);
    }
    CHECK (IBounds.Chars () > 0);

    // Calculate the memory requirements
    VirtualScreenSize = Bounds.Chars ();

    // Allocate that memory
    VirtualScreen = new u16 [VirtualScreenSize];

    // Insert window as uppermost window
    WinLock ();
    WNode.InsertBefore (&Background->WNode);
    WinUnlock ();

    // Lock the window, so changes are accumulated
    Lock ();

    // Draw frame if window is framed
    DrawFrame ();

    // Clear the window
    Clear ();

    // Unlock window now
    Unlock ();

}



Window::~Window ()
// Delete a window
{
    if (this != Background) {

	// If not already done, hide (and deactivate) the window
	Hide ();

	// Unlink the window from the window list if it's not the root window
	WinLock ();
	WNode.Unlink ();
	WinUnlock ();

    }

    // Now delete the allocated memory
    delete [] VirtualScreen;
}



u16 Window::StreamableID () const
// Return the object ID
{
    return ID_Window;
}



Streamable* Window::Build ()
// Build constructor
{
    return new Window (Empty);
}


unsigned Window::MinXSize () const
// Return the minimum X size of the window. Override this to limit resizing.
{
    return IsFramed () ? 3 : 2;
}



unsigned Window::MinYSize () const
// Return the minumim Y size of the window. Override this to limit resizing.
{
    return IsFramed () ? 3 : 2;
}



void Window::SetPalette (u16 NewPalette)
// Set a new window palette. NewPalette is the palette index of the new
// palette. The window is redrawn to show the new palette.
{
    if (NewPalette != Palette) {

	// Assign new palette
	Palette = NewPalette;

	// Redraw the frame and the interior
	Lock ();
	DrawFrame ();
	DrawInterior ();
	Unlock ();

    }
}



void Window::ScrollUp ()
// Scroll up the inner window by one line. The last line is cleared by
// this function (BGAttr/BGChar)
{
    Point ISize = IBounds.Size ();
    Point OSize = OBounds.Size ();

    // Calculate a pointer to the first row
    u16* Target = VirtualScreen;
    if (IsFramed ()) {
	Target += OSize.X + 1;
    }

    // Calculate a pointer to the second row
    u16* Source = Target + OSize.X;

    // Move memory
    for (int Y = ISize.Y - 1; Y > 0; Y--) {
	memmove (Target, Source, ISize.X * sizeof (u16));
	Source += OSize.X;
	Target += OSize.X;
    }

    // Clear the last line
    u16 AttrChar = Pal->BuildAttr (Palette, BGAttr, BGChar);
    for (int X = ISize.X; X > 0; X--) {
	*Target++ = AttrChar;
    }

    // Update the window area
    Update (IBounds);
}



void Window::ScrollDown ()
// Scroll down the inner window by one line. The first line is cleared by
// this function (BGAttr/BGChar)
{
    Point ISize = IBounds.Size ();
    Point OSize = OBounds.Size ();

    // Calculate a pointer to the last row
    u16* Target = VirtualScreen + (OSize.Y - 1) * OSize.X;
    if (IsFramed ()) {
	Target -= OSize.X - 1;
    }

    // Calculate a pointer to the second last row
    u16* Source = Target - OSize.X;

    // Move memory
    for (int Y = ISize.Y - 1; Y > 0; Y--) {
	memmove (Target, Source, ISize.X * sizeof (u16));
	Source -= OSize.X;
	Target -= OSize.X;
    }

    // Clear the first line
    u16 AttrChar = Pal->BuildAttr (Palette, BGAttr, BGChar);
    for (int X = ISize.X; X > 0; X--) {
	*Target++ = AttrChar;
    }

    // Update the window area
    Update (IBounds);
}



void Window::Clear ()
// Clear the inner window using the attribute BGAttr and the character
// BGChar
{
    u16* P;
    u16 AttrChar;
    int X, Y;


    Point ISize = IBounds.Size ();
    Point OSize = OBounds.Size ();
    Point Offs	= IBounds.A - OBounds.A;

    P = VirtualScreen + Offs.X + Offs.Y * OSize.X;
    AttrChar = Pal->BuildAttr (Palette, BGAttr, BGChar);

    for (Y = 0; Y < ISize.Y; Y++, P += OSize.X) {
	for (X = 0; X < ISize.X; X++) {
	    P [X] = AttrChar;
	}
    }

    // Reset the cursor position to zero
    CursorPos = Point (0, 0);

    // Update screen if not locked
    Update (OBounds);
}



void Window::DrawInterior ()
// Draw the interior of the window. You have to override this function
// if your window contains something that has to be redrawn. The default
// is to clear the inner window.
{
    // Just clear the inner window here
    Clear ();
}



void Window::PutOnTop ()
// Put the given window on top of all other windows.
{
    // Get pointer to node of root window
    ListNode<Window>* RootNode = &Background->WNode;

    // Check if any work
    if (WNode.Next () == RootNode) {
	// This window is the upper most window
	return;
    }

    // Unlink at current position and relink on top of all other windows
    WinLock ();
    WNode.Unlink ();
    WNode.InsertBefore (RootNode);
    WinUnlock ();

    // Redraw window hierarchy unconditionally
    Background->UpdateScreen (OBounds);
}



void Window::PutBeneath (Window& W)
// Put a given window beneath another window. This second window cannot be
// the root window (the root window must remain the last window in the
// window hierarchy)
{
    // Cannot put window beneath itself or beneath the root window
    PRECONDITION (&W != this && &W != Background);

    // Unlink at current position and link in before second window
    WinLock ();
    WNode.Unlink ();
    WNode.InsertBefore (&W.WNode);
    WinUnlock ();

    // Redraw window hierarchy unconditionally
    Background->UpdateScreen (OBounds);
}



Window* Window::UpperWindow ()
// Return the window in front of the this-window. If there is no upper
// window, return NULL.
{
    Window* Upper = WNode.Next () -> Contents ();
    return Upper != Background ? Upper : (Window*) NULL;
}



Window* Window::LowerWindow ()
// Return the window below the this-window. If there is no lower window,
// return NULL.
{
    Window* Lower = WNode.Prev () -> Contents ();
    return Lower != Background ? Lower : (Window*) NULL;
}



void Window::SetWindowNumber (u16 aNumber)
// Set a new window number. If this makes the header line change, the
// header is redrawn.
{
    if (Number != aNumber) {

	// If the new number and old number is invisible, no need to redraw
	if ((Number == 0 || Number > 9) && (aNumber == 0 || aNumber > 9)) {

	    // Just store the number
	    Number = aNumber;

	} else {

	    // Store the window number and redraw
	    Number = aNumber;
	    DrawHeader ();

	}
    }
}



inline unsigned char* Window::GetFrame ()
// Get a pointer to the frame char string according to the current
// window state
{
    // Use the active frame if the window is active or resizing, otherwise
    // use the passive frame
    return Flags & (wfActive | wfResizing) ? ActiveFrame : InactiveFrame;
}



unsigned Window::GetFrameAttr ()
// Return the palette index for the frame according to the current
// window state
{
    register unsigned Index;

    // Check out the correct index
    if (Flags & wfResizing) {
	// Resizing frame
	Index = atFrameResizing;
    } else if (Flags & wfActive) {
	// Active frame
	Index = atFrameActive;
    } else {
	// Inactive frame
	Index = atFrameInactive;
    }

    // Return the attribute char
    return Pal->BuildAttr (Palette, Index, ' ');
}



void Window::DrawHeader ()
// Draw the header line of the window frame. The function exits immidiately
// if the window is not framed.
{
    // Return immidiately if the window has no frame
    if (!IsFramed ()) {
	return;
    }

    // No output until Unlock
    Lock ();

    // Get needed frame characters and frame attribute
    unsigned char* F = GetFrame ();
    u16 AttrChar = GetFrameAttr ();

    // Get the attribute char for the horizontal lines
    u16 FrameAttr = Pal->BuildAttr (AttrChar, F [fcHorizontal]);

    // Calculate the header width
    unsigned Width = IBounds.XSize ();
    unsigned HeaderLen = Header.Len ();
    if (HeaderLen > Width) {
	HeaderLen = Width;
    }

    // Count of chars left and right of the header string
    unsigned LeftChars	= (Width - HeaderLen) / 2;
    unsigned RightChars = Width - HeaderLen - LeftChars;

    // Starting X coord
    unsigned X = 1;
    unsigned I;

    // Draw the left part
    while (LeftChars--) {
	WriteChar (X++, 0, FrameAttr);
    }

    // Headerstring part
    I = 0;
    while (I < HeaderLen) {
	WriteChar (X++, 0, Pal->BuildAttr (AttrChar, Header [I]));
	I++;
    }

    // Draw the right part. This is special, because the windows number apears
    // here
    if (Number >= 1 && Number <= 9 && RightChars >= 3) {

	// There is a number and there's room for the number string. Calculate
	// the space left and right of the number string
	LeftChars  = RightChars - 2;
	RightChars = RightChars - 1 - LeftChars;

	// Draw the left part
	while (LeftChars--) {
	    WriteChar (X++, 0, FrameAttr);
	}

	// Draw the number
	WriteChar (X++, 0, Pal->BuildAttr (AttrChar, Number + '0'));

	// Draw the right part
	while (RightChars--) {
	    WriteChar (X++, 0, FrameAttr);
	}

    } else {

	// No number or no room for the string
	while (RightChars--) {
	    WriteChar (X++, 0, FrameAttr);
	}

    }

    // Unlock the window, update the screen
    Unlock ();
}



void Window::DrawFooter ()
// Draw the footer of the window frame. The function exits immidiately if
// the window is not framed.
// The footer is positioned at the lower right of the frame, there is exactly
// one horizontal frame element to the right of the footer text.
{
    // Return immidiately if the window has no frame
    if (!IsFramed ()) {
	return;
    }

    // No output until Unlock
    Lock ();

    // Get needed frame characters and frame attribute
    unsigned char* F = GetFrame ();
    u16 AttrChar = GetFrameAttr ();

    // Get the attribute char for the horizontal lines
    u16 FrameAttr = Pal->BuildAttr (AttrChar, F [fcHorizontal]);

    unsigned Width = IBounds.XSize ();
    if (Width == 0) {
	// No space for the footer
	Unlock ();
	return;
    }
    unsigned FooterLen = Footer.Len ();
    if (FooterLen >= Width) {
	FooterLen = Width - 1;
    }
    unsigned LeftChars = Width - FooterLen - 1;
    unsigned X = 1;
    unsigned Y = OBounds.YSize () - 1;

    // Write the horizontal characters to the left of the footer text
    while (LeftChars--) {
	WriteChar (X++, Y, FrameAttr);
    }

    // Write the footer text
    unsigned I = 0;
    while (I < FooterLen) {
	WriteChar (X++, Y, Pal->BuildAttr (AttrChar, Footer [I]));
	I++;
    }

    // Write the horizontal frame char to the right of the footer
    WriteChar (X, Y, FrameAttr);

    // Unlock the window, allow screen updates
    Unlock ();

}



void Window::DrawFrame ()
// Draw the window frame. This function is called if the state of the
// window changes in a way that is shown in the frame. The function
// exits immediately if the window has no frame.

{
    // Bail out early if the window has no frame
    if (!IsFramed ()) {
	return;
    }

    // Get the window size
    int XMax  = OBounds.XSize () - 1;
    int YMax = OBounds.YSize () - 1;

    // Get needed frame characters and frame attribute
    unsigned char* F = GetFrame ();
    u16 AttrChar = GetFrameAttr ();

    // Now draw top/bottom/left/right frame part, doing a lock/unlock pair
    // before and after each part. This will prevent redrawing the complete
    // window under linux (where terminal ouput may be slow).

    // Top part
    Lock ();
    WriteChar (0,    0,    Pal->BuildAttr (AttrChar, F [fcTopLeft]));
    WriteChar (XMax, 0,    Pal->BuildAttr (AttrChar, F [fcTopRight]));
    DrawHeader ();
    Unlock ();

    // Bottom part
    Lock ();
    WriteChar (0,    YMax, Pal->BuildAttr (AttrChar, F [fcBotLeft]));
    WriteChar (XMax, YMax, Pal->BuildAttr (AttrChar, F [fcBotRight]));
    DrawFooter ();
    Unlock ();

    // Left part
    AttrChar = Pal->BuildAttr (AttrChar, F [fcVertical]);
    Lock ();
    int Y;
    for (Y = 1; Y < YMax; Y++) {
	WriteChar (0,	 Y, AttrChar);
    }
    Unlock ();

    // Right part
    Lock ();
    for (Y = 1; Y < YMax; Y++) {
	WriteChar (XMax, Y, AttrChar);
    }
    Unlock ();

    // Done
}



void Window::SetHeader (const String& NewHeader)
// Set a new window header. The window header is visible only if the
// window has a frame. In this case, it is centered in the top part of
// the frame. This function redraws the top part of the frame.
{
    Header = NewHeader;
    DrawHeader ();
}



void Window::SetFooter (const String& NewFooter)
// Set a new window footer. The window footer is visible only if the
// window has a frame. In this case, it is right justified in the bottom
// part of the frame. This function redraws the bottom part of the frame.
{
    Footer = NewFooter;
    DrawFooter ();
}



void Window::Absolute (Point& P)
// Make a window relative point absolute (relative to the screen). The
// given coordinates are (0/0) based relativ to the inner window
{
    P += IBounds.A;
}



void Window::Relative (Point& P)
// Make an absolute coordinate relative to the inner window
{
    P -= IBounds.A;
}



void Window::Resize (const Rect& NewBounds)
// Resize the window to the new bounds (this can also be used to move the
// window but Move is faster if the window should not be resized).
// The function checks if the new size is applicable. If yes, the window
// is resized and DrawInterior is called to redraw the windows contents.
{
    // Lock the window system
    WinLock ();

    // Lock screen output
    Lock ();

    // Remember the old window bounds
    Rect OldBounds = OBounds;

    // Use the new bounds
    IBounds = OBounds = NewBounds;

    // Recalculate the inner window. Don't use Move, this can fail!
    if (IsFramed ()) {
	IBounds.Grow (-1, -1);
    }

    // Allocate memory for the new virtual screen
    u16 NewVirtualScreenSize = NewBounds.Chars ();
    u16* NewVirtualScreen = new u16 [NewVirtualScreenSize];

    // Free the current virtual screen and use the new one
    delete [] VirtualScreen;
    VirtualScreen = NewVirtualScreen;
    VirtualScreenSize = NewVirtualScreenSize;

    // Draw the frame, then draw the interior
    DrawFrame ();
    DrawInterior ();

    // If the window is visible, update the union of the old and the
    // new window bounds. This will cause the dirty flag to be reset,
    // so the following unlock does nothing but reseting the lock counter
    if (IsVisible ()) {
	Background->UpdateScreen (Union (OldBounds, OBounds));
    }

    // Allow screen output
    Unlock ();

    // Unlock the window system
    WinUnlock ();
}



void Window::MoveAbs (Point dP)
// Move the window to an absolute position
{
    MoveRel (dP - OBounds.A);
}



void Window::MoveRel (Point dP)
// The window is moved by the given dX/dY
{
    // Lock the window system
    WinLock ();

    if (IsVisible ()) {

	// Remember old coordinates
	Rect OldOBounds = OBounds;

	// Move window
	OBounds.Move (dP.X, dP.Y);
	IBounds.Move (dP.X, dP.Y);

	// If the window is moved not more than 4 units in combined
	// X and Y direction, redraw the union of both rectangles
	if ( (abs (dP.X) + abs (dP.Y)) > 4) {

	    // Update old area, this resets a possibly set dirty flag
	    Background->UpdateScreen (OldOBounds);

	    // Update new area
	    Background->UpdateScreen (OBounds);

	} else {

	    // Update combined area
	    Background->UpdateScreen (Union (OldOBounds, OBounds));

	}

    } else {
	OBounds.Move (dP.X, dP.Y);
	IBounds.Move (dP.X, dP.Y);
    }

    // Unlock the window system
    WinUnlock ();
}



void Window::SetState (u16 State)
// Set the state of the wfActive and wfVisible flags according to the
// values contained in NewFlags.
{
    switch (State & (wfActive | wfVisible)) {

	case wfActive | wfVisible:
	    // Both flags should be set. To avoid redrawing the area twice,
	    // just set the wfVisible Flag and call Activate. Activate will
	    // call Show if the wfVisible flag is set, after redrawing the
	    // frame, so the area is only output once
	    Flags |= wfVisible;
	    Activate ();
	    break;

	case wfActive:
	    // This one is simple...
	    Activate ();
	    break;

	case wfVisible:
	    // This one is simple...
	    Show ();
	    break;

	case 0:
	    // Hide the window. This will reset the wfActive flag
	    Hide ();
	    break;

    }
}



void Window::NewOptions (u16 NewOps)
{
    u16 OldOps = Options;
    Options    = NewOps;

    // Check if anything has changed
    if ((NewOps & cfCenterAll) != (OldOps & cfCenterAll)) {
	// Re-center the window
	Rect NewBounds (OBounds);
	NewBounds.Center (Background->OuterBounds (), Options);
	MoveAbs (NewBounds.A);
    }
}



void Window::MoveResizeAfterLoad (const Point& OldRes)
// This function is called after a load when most of the window is
// constructed. It is used to move the window to a new position if this is
// needed. OldRes is the old screen resolution that was active, when the
// window was stored.
// The default is to recenter the window if the apropriate flags are set
// and the screen resolution has changed.
{
    // If the screen size has changed and some of the center options
    // are set, recenter the window.
    if (Background->OuterBounds ().Size () != OldRes && (Options & cfCenterAll) != 0) {
	// Screen size has changed
	Rect NewBounds (OBounds);
	NewBounds.Center (Background->OuterBounds (), Options);
	MoveAbs (NewBounds.A);
    }
}



void Window::Hide ()
// Hide the window. The window is no longer visible after calling this
// function but any window output is still allowed.
{
    if (IsVisible ()) {

	// Do not lock the window but reset the wfVisible flag before calling
	// (eventually) Deactivate. If the wfVisible flag is reset, there
	// will be no screen I/O anyway.
	Flags &= ~wfVisible;
	if (IsActive ()) {
	    Deactivate ();
	}
	Background->UpdateScreen (OBounds);
    }
}



void Window::Show ()
// Show a previously hidden window. This does not put the window in front
// of other windows, so the window may still be covered.
{
    if (!IsVisible ()) {
	Flags |= wfVisible;
	Background->UpdateScreen (OBounds);
    }
}



void Window::StartResizing ()
// Set the wfResizing flag and redraw the frame to show the new window
// state
{
    // Set the flag bit
    Flags |= wfResizing;

    // Redraw the frame if there is one
    DrawFrame ();
}



void Window::EndResizing ()
// Reset the wfResizing flag and redraw the frame to show the new window
// state
{
    // Reset the flag bit
    Flags &= ~wfResizing;

    // Redraw the frame if there is one
    DrawFrame ();
}



void Window::RawActivate ()
// Activate the window without manipulating the active window stack
{
    // If the window is already active, something is wrong
    PRECONDITION (!IsActive ());

    // Lock the window, no screen output
    Lock ();

    // Remember that the window is active and draw the new frame
    Flags |= wfActive;
    DrawFrame ();

    // Lock the window system
    WinLock ();

    // Put window on top. Do not use PutOnTop for this, because PutOnTop
    // will unconditionally redraw the area covered by this window. This
    // is unneccessary and (under Linux) maybe slow.
    // Get pointer to node of root window
    ListNode<Window>* RootNode = &Background->WNode;

    // Check if any work
    if (WNode.Next () != RootNode) {

	// The window is not on top. Unlink it at the current position and
	// relink on top of all other windows
	WNode.Unlink ();
	WNode.InsertBefore (RootNode);

	// Because the window eventually has been covered, we have to redraw
	// the covered area now. Combine this with showing the window by
	// setting the wfVisible flag before updating the window area
	Flags |= wfVisible;
	Background->UpdateScreen (OBounds);

    } else {

	// The window has already been on top. If the window was invisible,
	// show it now, otherwise do nothing
	if (!IsVisible ()) {
	    Show ();
	}

    }

    // Set the cursor form and position
    SetCursor ();

    // Unlock the window system
    WinUnlock ();

    // Unlock the window
    Unlock ();
}



void Window::Activate ()
// Set the window active flag, redraw the window frame to show the new
// state, put the window in front of all other windows, then show it if
// it has been hidden. This function deactivates an eventually active
// window.
{
    // Lock the window system
    WinLock ();

    // Activate the window
    if (!IsActive ()) {

	// Window is not active. Get a pointer to the current active
	// window in the thread. Make that window inactive but remember
	// the pointer.
	Thread* T = CurThread ();
	Window* OldActive = T->ANode.IsEmpty () ? (Window*) NULL : T->ANode.Prev () -> Contents ();

	// If there is a previous active window, lock it before making it
	// inactive. Maybe the window is completely covered by the new
	// active window, so redrawing the inactive frame will never get it
	// to the screen.
	if (OldActive) {
	    // Some other active window, make it inactive
	    OldActive->Lock ();
	    OldActive->RawDeactivate ();
	}

	// Be careful here: The window that should become active could be
	// already in the list of active windows. So unlink it before doing
	// the real work
	ANode.Unlink ();

	// Make the current window active and set the pointer
	// to the current active window to "this".
	ANode.InsertBefore (&T->ANode);
	RawActivate ();

	// If there has been an old active window, unlock it now
	if (OldActive) {
	    OldActive->Unlock ();
	}
    }

    // Unlock the window system
    WinUnlock ();
}



void Window::RawDeactivate ()
// Deactivate the window without manipulating the active window stack
{
    // Window must be active or something is sincerely wrong
    PRECONDITION (IsActive ());

    // Reset the flags and draw a new frame according to the new state
    Flags &= ~wfActive;
    DrawFrame ();
}



void Window::Deactivate ()
// Deactivates the window. This includes redrawing the frame and
// re-activating a previuos active window.
{
    // Lock the window system
    WinLock ();

    if (IsActive ()) {

	// The window is active. Deactivate the window. Get the pointer to
	// the last active window and activate this one. Store the pointer
	// to the last active window as currently active.
	Thread* T = CurThread ();
	ANode.Unlink ();
	RawDeactivate ();
	if (!T->ANode.IsEmpty ()) {
	    T->ANode.Prev () -> Contents () -> RawActivate ();
	}
    }

    // Unlock the window system
    WinUnlock ();
}



void Window::Unlock ()
// Unlock the window. If there have been window changes while the window
// was locked, the physical screen is updated in the area that contains
// the changes. Lock/Unlock may be nested.
{
    if (--LockCount == 0) {
	if (IsDirty () && IsVisible ()) {
	    UpdateScreen (DirtyRect);
	}
    }
}



void Window::WriteChar (int X, int Y, u16 AttrChar)
// Write a character to virtual and real screen. The coordinates are zero
// based relative to the outer window.
{
    // Calculate and check offset
    int Offset = OBounds.XSize () * Y + X;
    PRECONDITION (Offset < VirtualScreenSize);

    // Write char to virtual screen
    *(VirtualScreen + Offset) = AttrChar;

    // Update screen if not locked
    Update (Rect (OBounds.A.X + X, OBounds.A.Y + Y,
		  OBounds.A.X + X + 1, OBounds.A.Y + Y + 1));
}



void Window::WriteBuf (int X, int Y, u16* Buf, size_t Count)
{
    // Check for a wrong Y coord
    PRECONDITION (Y < OBounds.YSize ());

    // Get pointer to first mem location
    u16* Start = VirtualScreen + OBounds.XSize () * Y + X;

    // Don't wrap around the right border
    int Max = OBounds.XSize () - X;
    int C = (int (Count) > Max) ? Max : Count;	      // Cast to int

    if (C > 0) {

	// Now write the stuff to the virtual screen
	memcpy (Start, Buf, C * sizeof (u16));

	// Update screen if not locked
	Update (Rect (OBounds.A.X + X, OBounds.A.Y + Y,
		      OBounds.A.X + X + C, OBounds.A.Y + Y + 1));

    }
}



void Window::CollectBuffer (const Rect& Bounds, u16* Buf)
// "Collects" window data in a buffer. If a screen redraw must be done
// because one of the windows in the hierarchy has changes, the window
// allocates a buffer, fills it with the data from its virtual screen
// and passes the buffer together with the window bounds to each one
// of the windows in front. Each of those windows checks if the
// rectangle overlaps with its own area and updates this area in the
// buffer.
//
//	      +-------------+
//	      |		    |
//	      |      +------+------+	   R1 = Bounds
//	      |   R1 |	    |	   |	   R2 = OBounds
//	      |      |	R3  |	   |	   R3 = R
//	      |      |	    |	   |
//	      +------+------+	   |
//		     |		   |
//		     |	   R2	   |
//		     |		   |
//		     |		   |
//		     |		   |
//		     +-------------+
//
//
{
    // If the window is not visible --> exit immidiately
    if (!IsVisible ()) {
	return;
    }

    // Get intersection of both rectangles
    Rect R = Intersection (Bounds, OBounds);

    // If the intersection is empty, nothing has to be done
    if (R.IsEmpty ()) {
	return;
    }

    // If the intersection contains the dirty rectangle,
    // the dirty flag can be reset, because the whole window will
    // be put in the buffer and the whole DirtyRect rectangle will
    // be updated on the real screen
    if (IsDirty () && R.Contains (DirtyRect)) {
	Flags &= ~wfDirty;
    }

    // Update the buffer
    int R1RowChars = Bounds.XSize ();
    int R2RowChars = OBounds.XSize ();
    int R3RowChars = R.XSize ();

    int R1RowOfs   = R.A.X - Bounds.A.X;
    int R2RowOfs   = R.A.X - OBounds.A.X;

    int R1YOfs	   = R.A.Y - Bounds.A.Y;
    int R2YOfs	   = R.A.Y - OBounds.A.Y;

    u16* Target    = Buf + R1YOfs * R1RowChars + R1RowOfs;
    u16* Source    = VirtualScreen + R2YOfs * R2RowChars + R2RowOfs;

    for (int Height = R.YSize (); Height > 0; Height--) {

	memmove (Target, Source, R3RowChars * sizeof (u16));
	Target += R1RowChars;
	Source += R2RowChars;

    }

}



void Window::UpdateScreen (const Rect& Bounds)
// Updates the real screen using the contents of the virtual screen.
{
    // Intersect the given rectangle with the rectangle of the root window
    // clipping at all four borders. So no illegal coords are used.
    Rect B = Intersection (Background->OBounds, Bounds);

    // If the resulting intersection is empty, the rectangle is completely
    // oudside the screen. Ignore the call.
    if (B.IsEmpty ()) {
	return;
    }

    // Allocate buffer
    u16* Buf = new u16 [B.Chars ()];

    // Lock the window system
    WinLock ();

    // Pass the buffer along to all windows in top of the current window,
    // with each window updating it's portion of the buffer
    // The buffer is passed at first to the current window, thus clearing
    // the dirty bit if the given rectangle is equal to the outer bounds
    // of the window.
    ListNode<Window>* RootNode = &Background->WNode;
    ListNode<Window>* NextNode = &WNode;
    do {
	NextNode->Contents ()->CollectBuffer (B, Buf);
	NextNode = NextNode->Next ();
    } while (NextNode != RootNode);

    // Now we have a complete screen image of the area, show it
    TheScreen->DisplayBuffer (B, Buf);

    // Unlock the window system
    WinUnlock ();

    // Free buffer
    delete [] Buf;

}



void Window::Update (const Rect& Bounds)
// Update area or mark window as dirty if locked
{
    // If the window is invisible, there's nothing to do
    if (IsVisible ()) {
	if (IsLocked ()) {
	    // Output is locked, check if the window is already dirty
	    if (IsDirty ()) {
		// Add the new region to the dirty region
		DirtyRect = Union (DirtyRect, Bounds);
	    } else {
		// Not dirty until now
		DirtyRect = Bounds;
		Flags |= wfDirty;
	    }
	} else {
	    // Output is enabled, do the update
	    UpdateScreen (Bounds);
	}
    }
}



void Window::Write (int X, int Y, const String& S, int Attr)
// Write a string to the inner window. Note: Attr is an index, not a real
// attribute !
{
    const BufSize = 256;
    u16 Buf [256];

    // Check if the given string is empty
    int Len = S.Len ();
    if (Len == 0) {
	return;
    }

    // If the Y coord is outside the inner window, there is nothing to do
    if (Y >= IBounds.YSize ()) {
	return;
    }

    // Get the length of the string and clip to the right border of the
    // inner window
    if ((X + Len) > IBounds.XSize ()) {
	Len = IBounds.XSize () - X;
    }

    // Check against length of buffer
    if (Len > BufSize) {
	Len = BufSize;
    }

    // Get an empty attribute char
    u16 AttrChar = Pal->BuildAttr (Palette, Attr, ' ');

    // Fill the buffer
    for (int I = 0; I < Len; I++) {
	Buf [I] = Pal->BuildAttr (AttrChar, S [I]);
    }

    // Now write the buffer
    if (IsFramed ()) {
	X++;
	Y++;
    }
    WriteBuf (X, Y, Buf, Len);

}



void Window::Write (int X, int Y, char C, int Attr)
// Write a char to the inner window. Note: Attr is an index, not a real
// attribute
{
    // Check if the coordinates are inside the inner window
    if (X >= 0 && X < IBounds.XSize () && Y >= 0 && Y < IBounds.YSize ()) {
	// Adjust coordinates if window is framed
	if (IsFramed ()) {
	    X++;
	    Y++;
	}
	WriteChar (X, Y, Pal->BuildAttr (Palette, Attr, C));
    }
}



void Window::FWrite (const String& S, int Attr)
// ## This function is a hack
{
    String O (S.Len ());
    int I;
    char C;

    // Lock output
    Lock ();

    // Examine given string
    I = 0;
    while (I < S.Len ()) {
	C = S [I++];
	switch (C) {

	    case '\x0D':
	    case '\x0A':
		// Display collected buffer
		if (O.Len () > 0) {
		    Write (CursorPos.X, CursorPos.Y, O, Attr);
		    CursorPos.X += O.Len ();
		    O.Clear ();
		}
		if (C == 0x0D) {
		    // CR
		    CursorPos.X = 0;
		} else if (C == 0x0A) {
		    // LF
		    if (++CursorPos.Y >= IBounds.YSize ()) {
			// Need scroll
			ScrollUp ();
			CursorPos.Y--;
		    }
		}
		break;

	    default:
		O += C;
		break;

	}
    }

    // Display collected buffer
    if (O.Len () > 0) {
	Write (CursorPos.X, CursorPos.Y, O, Attr);
	CursorPos.X += O.Len ();
    }

    // Unlock the window, allow screen output
    Unlock ();

}



void Window::CWrite (int X, int Y, const String& S)
// Write a string to the inner window. Note: Attr is an index, not a real
// attribute !
{
    const BufSize = 256;
    u16 Buf [BufSize];

    // If the Y coord is outside the inner window, there is nothing to do
    if (Y >= IBounds.YSize ()) {
	return;
    }

    // We can not do any definite length checking because the string contains
    // meta chars ('~') to switch the attribute. Wait for that until we
    // translated it into the buffer. But if the string is empty, bail out here
    if (S.IsEmpty ()) {
	return;
    }

    // Set up starting attribute
    unsigned char Attr = atTextNormal;
    u16 AttrChar = Pal->BuildAttr (Palette, Attr, ' ');

    // Fill the buffer
    unsigned BufLen = 0;
    int CurX	    = X;
    int XEnd	    = IBounds.XSize ();
    int Len	    = S.Len ();
    const char* P   = S.GetStr ();
    while (Len--) {
	if (*P == '~') {
	    Attr = (Attr == atTextNormal) ? atTextHigh : atTextNormal;
	    AttrChar = Pal->BuildAttr (Palette, Attr, ' ');
	} else {
	    CurX++;
	    if (CurX > XEnd || BufLen >= sizeof (Buf)) {
		// We are at the right border or have a buffer overflow - clip
		break;
	    }
	    Buf [BufLen++] = Pal->BuildAttr (AttrChar, *P);
	}
	P++;
    }

    // Now write the buffer
    if (IsFramed ()) {
	X++;
	Y++;
    }
    WriteBuf (X, Y, Buf, BufLen);

}



void Window::FCWrite (const String& S)
// ## This function is a hack
{
    String O (S.Len ());
    int I;
    char C;
    unsigned char Attr = atTextNormal;

    // Lock output
    Lock ();

    // Examine given string
    I = 0;
    while (I < S.Len ()) {
	C = S [I++];
	switch (C) {

	    case '\x0D':
	    case '\x0A':
		// Display collected buffer
		if (O.Len () > 0) {
		    Write (CursorPos.X, CursorPos.Y, O, Attr);
		    CursorPos.X += O.Len ();
		    O.Clear ();
		}
		if (C == 0x0D) {
		    // CR
		    CursorPos.X = 0;
		} else if (C == 0x0A) {
		    // LF
		    if (++CursorPos.Y >= IBounds.YSize ()) {
			// Need scroll
			ScrollUp ();
			CursorPos.Y--;
		    }
		}
		break;

	    case '~':
		// Output string, switch attributes
		if (O.Len () > 0) {
		    Write (CursorPos.X, CursorPos.Y, O, Attr);
		    CursorPos.X += O.Len ();
		    O.Clear ();
		}
		Attr = (Attr == atTextNormal) ? atTextHigh : atTextNormal;
		break;

	    default:
		O += C;
		break;

	}
    }

    // Display collected buffer
    if (O.Len () > 0) {
	Write (CursorPos.X, CursorPos.Y, O, Attr);
	CursorPos.X += O.Len ();
    }

    // Unlock the window, allow screen output
    Unlock ();

}



void Window::ChangeAttr (int X, int Y, unsigned Count, unsigned Attr)
// Change the attribute of the text starting at X, Y for Count chars
{
    // Adjust for a frame
    if (IsFramed ()) {
	X++;
	Y++;
    }

    // Check for a wrong Y coord
    PRECONDITION (Y < OBounds.YSize ());

    // Get pointer to first mem location
    u16* P = VirtualScreen + OBounds.XSize () * Y + X;

    // Don't wrap around the right border
    int Max = OBounds.XSize () - X;
    int C = (int (Count) > Max) ? Max : Count;	      // Cast to int

    if (C > 0) {

	// Get the attribute char
	u16 A = Pal->BuildAttr (Palette, Attr, '\0');

	// Change the attribute
	Count = C;
	while (Count--) {
	    *P = (*P & 0x00FF) | A;
	    P++;
	}

	// Update screen if not locked
	Update (Rect (OBounds.A.X + X, OBounds.A.Y + Y,
		      OBounds.A.X + X + C, OBounds.A.Y + Y + 1));

    }
}



void Window::ScreenSizeChanged (const Rect& NewSize)
// Called when the screen got another resolution. NewSize is the new
// screen size.
{
    // Lock the window system
    WinLock ();

    // Recenter if needed
    if (Options & cfCenterAll) {

	// Move the window
	Rect NewBounds (OBounds);
	NewBounds.Center (NewSize, Options);
	MoveAbs (NewBounds.A);

	// Set the cursor to the new position if the window is active
	SetCursor ();
    }

    // Check if the window is out of screen. If so, move it, so that the upper
    // left corner is still visible.
    Point MoveVec (0, 0);
    if (OBounds.A.X >= NewSize.B.X) {
	MoveVec.X = NewSize.B.X - OBounds.A.X - 1;
    }
    if (OBounds.A.Y >= NewSize.B.Y) {
	MoveVec.Y = NewSize.B.Y - OBounds.A.Y - 1;
    }
    if (MoveVec != Point (0, 0)) {

	// Move the window
	MoveRel (MoveVec);

	// Set the cursor to the new position if the window is active
	SetCursor ();

    }

    // Redraw the complete window in case of a mono/color change
    DrawFrame ();
    DrawInterior ();

    // Unlock the window system
    WinUnlock ();
}



void Window::Store (Stream& S) const
// Store the window data into the given stream
{
    // Set up a modified copy of the flags
    u16 SaveFlags = Flags & wfSaveFlags;
    if (SaveVisible ()) {
	// Save as visible
	SaveFlags |= wfVisible | wfSaveVisible;
    }

    // Store the current screen resolution. This value is used to resize/move
    // the window when loading
    S << Background->OuterBounds ().Size ();

    // Store instance data
    S << Number << SaveFlags << Options << LockCount << OBounds << IBounds
      << Header << Footer << (i16) Cursor << CursorPos << Palette << BGChar
      << BGAttr << u16 (VirtualScreenSize * sizeof (u16));
    S.Write (VirtualScreen, VirtualScreenSize * sizeof (u16));
}



void Window::Load (Stream& S)
{
    i16 LoadCursor;

    // Load the screen resolution that was active, when the window was written
    // into the stream
    Point OldRes;
    S >> OldRes;

    // Load instance data
    S >> Number >> Flags >> Options >> LockCount >> OBounds >> IBounds
      >> Header >> Footer >> LoadCursor >> CursorPos >> Palette >> BGChar
      >> BGAttr >> VirtualScreenSize;
    Cursor = (CursorType) LoadCursor;

    // Compatibility fix: Calculate number of u16's from VirtualScreenSize
    VirtualScreenSize /= sizeof (u16);

    // Get memory for virtual screen
    VirtualScreen = new u16 [VirtualScreenSize];

    // Load virtual screen
    S.Read (VirtualScreen, VirtualScreenSize * sizeof (u16));

    // Remember the state of the wfActive and wfVisible bits in the flags,
    // then clear them. This means that all operations below are done with
    // an invisible window.
    u16 NewFlags = Flags & (wfVisible | wfActive);
    Flags &= ~(wfVisible | wfActive);

    // Lock the window system
    WinLock ();

    // Link into window list
    WNode.InsertBefore (&Background->WNode);

    // Allow the window to resize if needed
    MoveResizeAfterLoad (OldRes);

    // Now set the final window state, showing or activating the window if
    // needed
    SetState (NewFlags);

    // Unlock the window system
    WinUnlock ();
}



void Window::SetCursor ()
// If the window is active, set the screen (real) cursor to the position
// and type of the window cursor
{
    if (IsActive ()) {

	// Lock the window system
	WinLock ();

	// Set the real cursor to the window cursor position
	Point Pos = CursorPos;
	Absolute (Pos);
	TheScreen->SetCursorPos (Pos);

	// Switch the cursor according to the state
	switch (Cursor) {

	    case csOff:
		// No cursor
		TheScreen->SetCursorOff ();
		break;

	    case csOn:
		// Cursor on
		TheScreen->SetCursorOn ();
		break;

	    case csFat:
		// Cursor fat
		TheScreen->SetCursorFat ();
		break;

	    default:
		FAIL ("Window::SetCursor: Invalid cursortype");
		break;
	}

	// Unlock the window system
	WinUnlock ();

    }
}



void Window::SetCursorPos (const Point& Pos)
{
    // Remember cursor position
    CursorPos = Pos;

    // Lock the window system
    WinLock ();

    // If the window is active, use the new position
    if (IsActive ()) {
	Point NewPos = CursorPos;
	Absolute (NewPos);
	TheScreen->SetCursorPos (NewPos);
    }

    // Unlock the window system
    WinUnlock ();
}



void Window::SetCursorOff ()
{
    // Remember the new cursor form
    Cursor = csOff;

    // Lock the window system
    WinLock ();

    // If the window is active, use the new cursor
    if (IsActive ()) {
	TheScreen->SetCursorOff ();
    }

    // Unlock the window system
    WinUnlock ();
}



void Window::SetCursorOn ()
{
    // Remember the new cursor form
    Cursor = csOn;

    // Lock the window system
    WinLock ();

    // If the window is active, use the new cursor
    if (IsActive ()) {
	Point NewPos = CursorPos;
	Absolute (NewPos);
	TheScreen->SetCursorPos (NewPos);
	TheScreen->SetCursorOn ();
    }

    // Unlock the window system
    WinUnlock ();
}



void Window::SetCursorFat ()
{
    // Remember the new cursor form
    Cursor = csFat;

    // Lock the window system
    WinLock ();

    // If the window is active, use the new cursor
    if (IsActive ()) {
	Point NewPos = CursorPos;
	Absolute (NewPos);
	TheScreen->SetCursorPos (NewPos);
	TheScreen->SetCursorFat ();
    }

    // Unlock the window system
    WinUnlock ();
}



void Window::SetCursor (CursorType C)
{
    switch (C) {

	case csOff:
	    SetCursorOff ();
	    break;

	case csOn:
	    SetCursorOn ();
	    break;

	case csFat:
	    SetCursorFat ();
	    break;

    }
}



