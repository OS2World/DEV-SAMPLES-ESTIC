/*****************************************************************************/
/*									     */
/*				    WINDOW.H				     */
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



#ifndef _WINDOW_H
#define _WINDOW_H



#include "event.h"
#include "listnode.h"
#include "strmable.h"
#include "stream.h"
#include "rect.h"
#include "str.h"
#include "palette.h"
#include "winflags.h"
#include "winsize.h"



/*****************************************************************************/
/*				   Locking				     */
/*****************************************************************************/



// Nested calling of the following functions from the same thread is explicitly
// allowed! The following functions must _not_ block if the function is called
// twice from one thread.
// Anyway, the locking assumes, one window is handled by only one thread. If
// you try to call window functions of one window by more than one thread, the
// locking will fail.



// Function vectors to lock/unlock the windows system
extern void (*WinLock) ();
extern void (*WinUnlock) ();



/*****************************************************************************/
/*				 class Window				     */
/*****************************************************************************/



class Window : public Streamable {

    friend class ResEditApp;			// Resource editor is a friend

public:
    enum CursorType { csOff, csOn, csFat };

private:
    Rect		DirtyRect;	// Rectangle that is dirty


protected:
    u16			Number;		// window number
    u16			Flags;		// Window state
    u16			Options;	// grow options
    i16			LockCount;
    Rect		OBounds;	// Outer bounds of window
    Rect		IBounds;	// Inner bounds of window

    String		Header;		// Window header
    String		Footer;		// Window footer

    CursorType		Cursor;
    Point		CursorPos;	// Position 0/0 based, inner window

    u16			Palette;	// Number of palette used

    unsigned char	BGChar;		// Char used for background
    unsigned char	BGAttr;		// Number of attribute to use

    u16*		VirtualScreen;	// Pointer to virtual screen
    u16			VirtualScreenSize; // Size of this screen in bytes

    Window*		LastActive;	// Active window chain

    ListNode<Window>	WNode;		// List of all windows
    ListNode<Window>	ANode;		// List of active windows



    // --- "Raw" switching of the active window

    void RawActivate ();
    // Activate the window without manipulating the active window stack

    void RawDeactivate ();
    // Deactivate the window without manipulating the active window stack

    // --- Do something with the window contents

    unsigned char* GetFrame ();
    // Get a pointer to the frame char string according to the current
    // window state

    unsigned GetFrameAttr ();
    // Return the attribute for the frame according to the current
    // window state

    void SetCursor ();
    // If the window is active, set the screen (real) cursor to the position
    // and type of the window cursor

    virtual void DrawHeader ();
    // Draw the header line of the window frame. The function exits immidiately
    // if the window is not framed.

    virtual void DrawFooter ();
    // Draw the footer of the window frame. The function exits immidiately if
    // the window is not framed.
    // The footer is positioned at the lower right of the frame, there is
    // exactly one horizontal frame element to the right of the footer text.

    virtual void DrawFrame ();
    // Draw the window frame. This function is called if the state of the
    // window changes in a way that is shown in the frame. The function
    // exits immediately if the window has no frame.

    // Output
    void WriteChar (int X, int Y, u16 AttrChar);
    void WriteBuf  (int X, int Y, u16* Buf, size_t Count);

    void CollectBuffer (const Rect& Bounds, u16* Buf);
    // Collect screen data into a buffer

    void UpdateScreen (const Rect& Bounds);
    // Update real screen from virtual screen

    void Update (const Rect& Bounds);
    // Update area or mark window as dirty if locked

    virtual void NewOptions (u16 NewOps);
    // Set new option flags

    virtual void MoveResizeAfterLoad (const Point& OldRes);
    // This function is called after a load when most of the window is
    // constructed. It is used to move the window to a new position if this is
    // needed. OldRes is the old screen resolution that was active, when the
    // window was stored.
    // The default is to recenter the window if the apropriate flags are set
    // and the screen resolution has changed.

    Window ();
    // "Do-nothing" constructor for use in derived classes

    Window (StreamableInit);
    // Build constructor

public:
    Window (const Rect& Bounds, u16 aState = wfFramed, u16 aPalette = paGray,
	    u16 Number = 0, int aLockCount = 0);
    // Create a new window

    virtual ~Window ();
    // Delete a window


    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();


    virtual void SetWindowNumber (u16 aNumber);
    // Set a new window number. If this makes the header line change, the
    // header is redrawn.

    u16 GetWindowNumber () const;
    // Return the window number

    const u16& GetWindowNumberRef () const;
    // Return a reference to the window number

    const Rect& OuterBounds () const;
    // Return the outer window rectangle including the frame

    const Rect& InnerBounds () const;
    // Return the inner window rectangle (without the frame)

    i16 MaxX () const;
    i16 MaxY () const;
    // Return the maximum allowed X/Y coordinate. Beware: This is one less
    // than the size of the inner window!

    unsigned IXSize ();
    unsigned IYSize ();
    // Return the size of the inner window

    unsigned OXSize ();
    unsigned OYSize ();
    // Return the size of the outer window

    virtual void Absolute (Point& P);
    // Make a window relative point absolute (relative to the screen). The
    // given coordinates are (0/0) based relativ to the inner window

    virtual void Relative (Point& P);
    // Make an absolute coordinate relative to the inner window

    // --- Handle frame and palette ---

    virtual void SetPalette (u16 NewPalette);
    // Set a new window palette. NewPalette is the palette index of the new
    // palette. The window is redrawn to show the new palette.

    virtual void SetHeader (const String& NewHeader);
    // Set a new window header. The window header is visible only if the
    // window has a frame. In this case, it is centered in the top part of
    // the frame. This function redraws the top part of the frame.

    virtual void SetFooter (const String& NewFooter);
    // Set a new window footer. The window footer is visible only if the
    // window has a frame. In this case, it is right justified in the bottom
    // part of the frame. This function redraws the bottom part of the frame.

    u16 GetPalette () const;
    // Return the current window palette

    const String& GetHeader () const;
    // Return the current window header string

    const String& GetFooter () const;
    // Return the current window footer string

    // --- Do something with the window contents

    virtual void ScrollUp ();
    // Scroll up the inner window by one line. The last line is cleared by
    // this function (BGAttr/BGChar)

    virtual void ScrollDown ();
    // Scroll down the inner window by one line. The first line is cleared by
    // this function (BGAttr/BGChar)

    virtual void Clear ();
    // Clear the inner window using the attribute BGAttr and the character
    // BGChar

    virtual void DrawInterior ();
    // Draw the interior of the window. You have to override this function
    // if your window contains something that has to be redrawn. The default
    // is to clear the inner window.

    // --- Moving & Resizing

    virtual unsigned MinXSize () const;
    // Return the minimum X size of the window. Override this to limit resizing.

    virtual unsigned MinYSize () const;
    // Return the minumim Y size of the window. Override this to limit resizing.

    virtual void Resize (const Rect& NewBounds);
    // Resize the window to the new bounds (this can also be used to move the
    // window but Move is faster if the window should not be resized).
    // This function does no checks! Call CanResize (NewBounds) if you don't
    // know if the new size is ok!

    virtual void MoveAbs (Point dP);
    // Move the window to an absolute position

    virtual void MoveRel (Point dP);
    // The window is moved by the given dX/dY. This function does no checks!
    // Call CanMove (dP) if you don't know if the new position is ok!

    virtual void PutOnTop ();
    // Put the given window on top of all other windows.

    virtual void PutBeneath (Window& W);
    // Put a given window beneath another window. This second window cannot be
    // the root window (the root window must remain the last window in the
    // window hierarchy)

    Window* UpperWindow ();
    // Return the window in front of the this-window. If there is no upper
    // window, return NULL.

    Window* LowerWindow ();
    // Return the window below the this-window. If there is no lower window,
    // return NULL.

    virtual void MoveResize ();
    // Allows interactive moving and resizing the window. This functions
    // calls ::MoveResize (this).

    // --- Set flag bits, take appropriate action

    virtual void Activate ();
    // Set the window active flag, redraw the window frame to show the new
    // state, put the window in front of all other windows, then show it if
    // it has been hidden. This function deactivates an eventually active
    // window.

    virtual void Deactivate ();
    // Deactivates the window. This includes redrawing the frame and
    // re-activating a previuos active window.

    virtual void Hide ();
    // Hide the window. The window is no longer visible after calling this
    // function but any window output is still allowed.

    virtual void Show ();
    // Show a previously hidden window. This does not put the window in front
    // of other windows, so the window may still be covered.

    virtual void StartResizing ();
    // Set the wfResizing flag and redraw the frame to show the new window
    // state

    virtual void EndResizing ();
    // Reset the wfResizing flag and redraw the frame to show the new window
    // state

    void SetSaveVisible ();
    // Set the "wfSaveVisible" flag. This causes future calls to Window::Store
    // to write out the Flags word with the wfVisible bit set. The default
    // is to clear this bit in the copy that goes to disk.

    void SetCanMove ();
    // Set the wfCanMove flag

    void SetCanResize ();
    // Set the wfCanResize flag

    void Lock ();
    // Lock the window. This causes screen changes to accumulate using the
    // wfDirty state and the DirtyRect rectangle. If the window is unlocked,
    // changes are written to the physical screen. Lock/Unlock may be nested.

    virtual void Unlock ();
    // Unlock the window. If there have been window changes while the window
    // was locked, the physical screen is updated in the area that contains
    // the changes. Lock/Unlock may be nested.

    // --- Check flag bits

    int IsFramed () const;
    // Return 1 if the window is framed, 0 otherwise

    int IsActive () const;
    // Return 1 if the window is active, 0 otherwise

    int IsVisible () const;
    // Return 1 if the window is visible, 0 otherwise

    int SaveVisible () const;
    // Return 1 if the wfSaveVisible flag is set, 0 otherwise

    int IsModal () const;
    // Return 1 if the wfModal flag is set, 0 otherwise. The wfModal flag
    // causes windows to ignore any reserved keys.

    int HasLRLink () const;
    // Return 1 if the wfLRLink flag is set, 0 otherwise.

    int IgnoreAccept () const;
    // Return 1 if the wfIgnoreAccept flag is set, 0 otherwise.

    int IsLocked () const;
    // Return 1 if the window output is locked, 0 otherwise.

    int IsDirty () const;
    // Return 1 if the window has changes that are not shown on the
    // physical screen, 0 otherwise.

    int IsResizing () const;
    // Return 1 if the wfResizing flag is set, 0 otherwise.

    int CanResize () const;
    // Return 1 if the wfCanResize flag is set, 0 otherwise.

    int CanMove () const;
    // Return 1 if the wfCanMove flag is set, 0 otherwise.

    // --- Get/Set the flags that describe the current window look

    u16 GetState ();
    // Return the state of the wfActive and wfVisible flags

    virtual void SetState (u16 NewFlags);
    // Set the state of the wfActive and wfVisible flags according to the
    // values contained in NewFlags.

    // --- Get/Set center options

    u16 GetOptions () const;
    void SetOption (u16 NewOptions);
    void ResetOption (u16 ResetOps);

    // --- Setting/getting cursor position and form

    const Point& GetCursorPos () const;
    void SetCursorPos (const Point& Pos);
    void SetCursorOff ();
    void SetCursorOn ();
    void SetCursorFat ();
    void SetCursor (CursorType C);
    CursorType GetCursor () const;

    // Write to the window (inner window, (0/0) based)
    virtual void Write (int X, int Y, const String& S, int Attr = atTextNormal);
    virtual void Write (int X, int Y, char C, int Attr = atTextNormal);
    virtual void FWrite (const String& S, int Attr = atTextNormal);
    virtual void CWrite (int X, int Y, const String& S);
    virtual void FCWrite (const String& S);

    virtual void ChangeAttr (int X, int Y, unsigned Count, unsigned Attr);
    // Change the attribute of the text starting at X, Y for Count chars

    // The following should be protected but there seems to be a bug in the
    // compiler - so leave it untouched!

    virtual void ScreenSizeChanged (const Rect& NewScreen);
    // Called when the screen got another resolution. NewScreen is the new
    // screen size.

};



inline Window::Window () :
    Options (0), WNode (this), ANode (this)
// "Do-nothing" constructor for use in derived classes
{
}



inline Window::Window (StreamableInit) :
	Header (Empty),
	Footer (Empty),
	WNode (this),
	ANode (this)
{
}




inline i16 Window::MaxX () const
// Return the maximum allowed X coordinate. Beware: This is one less
// than the size of the inner window!
{
    return IBounds.XSize () - 1;
}



inline i16 Window::MaxY () const
// Return the maximum allowed Y coordinate. Beware: This is one less
// than the size of the inner window!
{
    return IBounds.YSize () - 1;
}



inline unsigned Window::IXSize ()
// Return the size of the inner window
{
    return IBounds.XSize ();
}



inline unsigned Window::IYSize ()
// Return the size of the inner window
{
    return IBounds.YSize ();
}



inline unsigned Window::OXSize ()
// Return the size of the outer window
{
    return OBounds.XSize ();
}



inline unsigned Window::OYSize ()
// Return the size of the outer window
{
    return OBounds.YSize ();
}



inline void Window::SetSaveVisible ()
// Set the "wfSaveVisible" flag. This causes future calls to Window::Store
// to write out the Flags word with the wfVisible bit set. The default
// is to clear this bit in the copy that goes to disk.
{
    Flags |= wfSaveVisible;
}



inline void Window::SetCanMove ()
// Set the wfCanMove flag
{
    Flags |= wfCanMove;
}



inline void Window::SetCanResize ()
// Set the wfCanResize flag
{
    Flags |= wfCanResize;
}



inline void Window::Lock ()
// Lock the window. This causes screen changes to accumulate using the
// wfDirty state and the DirtyRect rectangle. If the window is unlocked,
// changes are written to the physical screen. Lock/Unlock may be nested.
{
    LockCount++;
}



inline int Window::IsFramed () const
// Return 1 if the window is framed, 0 otherwise
{
    return (Flags & wfFramed) != 0;
}



inline int Window::IsActive () const
// Return 1 if the window is active, 0 otherwise
{
    return (Flags & wfActive) != 0;
}



inline int Window::IsVisible () const
// Return 1 if the window is visible, 0 otherwise
{
    return (Flags & wfVisible) != 0;
}



inline int Window::SaveVisible () const
// Return 1 if the wfSaveVisible flag is set, 0 otherwise
{
    return (Flags & wfSaveVisible) != 0;
}



inline int Window::IsModal () const
// Return 1 if the wfModal flag is set, 0 otherwise. The wfModal flag
// causes windows to ignore any reserved keys.
{
    return (Flags & wfModal) != 0;
}



inline int Window::HasLRLink () const
// Return 1 if the wfLRLink flag is set, 0 otherwise.
{
    return (Flags & wfLRLink) != 0;
}



inline int Window::IgnoreAccept () const
// Return 1 if the wfIgnoreAccept flag is set, 0 otherwise.
{
    return (Flags & wfIgnoreAccept) != 0;
}



inline int Window::IsLocked () const
// Return 1 if the window output is locked, 0 otherwise.
{
    return LockCount != 0;
}



inline int Window::IsDirty () const
// Return 1 if the window has changes that are not reflected on the
// physical screen, 0 otherwise.
{
    return (Flags & wfDirty) != 0;
}



inline int Window::IsResizing () const
// Return 1 if the wfResizing flag is set, 0 otherwise.
{
    return (Flags & wfResizing) != 0;
}



inline int Window::CanResize () const
// Return 1 if the wfCanResize flag is set, 0 otherwise.
{
    return (Flags & wfCanResize) != 0;
}



inline int Window::CanMove () const
// Return 1 if the wfCanMove flag is set, 0 otherwise.
{
    return (Flags & wfCanMove) != 0;
}



inline u16 Window::GetState ()
// Return the state of the wfActive and wfVisible flags
{
    return Flags & (wfActive | wfVisible);
}



inline u16 Window::GetOptions () const
{
    return Options;
}



inline void Window::SetOption (u16 SetOps)
{
    NewOptions ((u16) (Options | SetOps));
}



inline void Window::ResetOption (u16 ResetOps)
{
    NewOptions ((u16) (Options & ~ResetOps));
}



inline const Point& Window::GetCursorPos () const
{
    return CursorPos;
}



inline Window::CursorType Window::GetCursor () const
{
    return Cursor;
}



inline u16 Window::GetWindowNumber () const
{
    return Number;
}



inline const u16& Window::GetWindowNumberRef () const
{
    return Number;
}



inline const Rect& Window::OuterBounds () const
{
    return OBounds;
}



inline const Rect& Window::InnerBounds () const
{
    return IBounds;
}



inline u16 Window::GetPalette () const
{
    return Palette;
}



inline const String& Window::GetHeader () const
{
    return Header;
}



inline const String& Window::GetFooter () const
{
    return Footer;
}



inline void Window::MoveResize ()
// Allows interactive moving and resizing the window. This functions
// calls ::MoveResize (this).
{
    ::MoveResize (this);
}



/*****************************************************************************/
/*			       class RootWindow				     */
/*****************************************************************************/



class RootWindow: public Window, public EventHandler {

    friend class ResEditApp;			// Resource editor is a friend


protected:
    RootWindow (StreamableInit);
    // Build constructor


public:
    RootWindow ();
    // Construct the root window

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    Rect GetDesktop () const;
    // Get the absolute coords of the desktop area

    void RedrawScreen ();
    // Redraw the complete screen in case it's garbled

    void ChangeScreenSize (const Rect& NewSize);
    // Tell all windows about a changed screen size

    // The following should be protected but there seems to be a bug in the
    // compiler - so leave it untouched!

    virtual void ScreenSizeChanged (const Rect& NewScreen);
    // Called when the screen got another resolution. NewScreen is the new
    // screen size. This functions calls ScreenSizeChanged of all other
    // windows on the screen.

    Window* GetTopWindow ();
    // Return the uppermost window. If there are no windows, return NULL

    Window* GetTopVisibleWindow ();
    // Return the uppermost window that is visible. If there are no visible
    // windows, return NULL

    virtual void HandleEvent (Event& E);
    // Handle an incoming event. Default is to do nothing.

};



// The background window
extern RootWindow* Background;



inline RootWindow::RootWindow (StreamableInit):
	Window (Empty)
{
}



// End of WINDOW.H

#endif

