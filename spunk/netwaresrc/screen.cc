/*****************************************************************************/
/*									     */
/*				    SCREEN.CC				     */
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



#include <string.h>
#include <malloc.h>

#include <conio.h>

#include "screen.h"


/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Instance of the screen class to handle screen output. Must be initialized
// from outside (RootWindow)
Screen * TheScreen;



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



static u16 ScrGetMode ()
// return the current video mode
{
    if (IsColorMonitor ()) {
	return vmCO80;
    } else {
	return vmMono;
    }
}



static void ScrSetMode (u16 /*Mode*/)
{
    // We cannot set the mode, at least I don't know how
}



static u16 ScrGetCursor ()
// get the current cursor type
{
    BYTE Start, End;
    GetCursorShape (&Start, &End);
    return ( (((u16) Start) << 8) | (((u16) End) & 0x00FF) );
}



static void ScrSetCursor (u16 Cursor)
// set a cursor
{
    SetCursorShape (Cursor >> 8, Cursor & 0x00FF);
}



/*****************************************************************************/
/*				 class Screen				     */
/*****************************************************************************/



Screen::Screen ():
    CP437 (1),
    TransTable (NULL)
{
    // Remember old video mode and cursor form
    StartupMode = ScrGetMode ();
    StartupCursor = ScrGetCursor ();

    // Use current screen mode
    CurrentMode = StartupMode;

    // Set the mode data for this mode
    SetModeData ();

    // Switch the cursor off
    SetCursorOff ();
}



Screen::~Screen ()
{
    // Reset old video mode and cursor
    SetMode (StartupMode);
    ScrSetCursor (StartupCursor);
}



u16* Screen::Translate (u16* Target, u16* Source, unsigned Len)
// Translate a complete buffer via the translation table
{
    if (TransTable) {
	unsigned char* S = (unsigned char*) Target;
	unsigned char* T = (unsigned char*) Source;
	while (Len--) {
	    *T++ = TransTable [*S++];		// Translate character
	    *T++ = *S++;			// Copy attribute
	}
	return Target;
    } else {
	// No translation table, return the source buffer
	return Source;
    }
}



void Screen::SetModeData ()
// Internally called after setting a new mode, sets cursor data etc.
{
    // Get size of screen and the character cell height
    WORD Height, Width;
    GetSizeOfScreen (&Height, &Width);
    XSize = Width;
    YSize = Height;
    unsigned YCell = 16;

    // Calculate the cursor settings from the character cell height
    CF_HiddenCursor = 0;			// is constant under OS/2
    CF_FatCursor    = 0x0100 + YCell - 1;
    CF_NormalCursor = ((YCell - 2) << 8) + (YCell - 1);

    // Check if color mode
#if 0
    Color = IsColorMonitor ();
#endif
    Color = 1;
}



void Screen::SetMode (u16 Mode)
{
    if (Mode == vmInvalid) {
	return;
    }

    // Remember mode
    CurrentMode = Mode;

    // set mode
    ScrSetMode (Mode);

    // Get mode data
    SetModeData ();

    // Switch cursor off
    SetCursorOff ();
}



void Screen::SetCursorOn ()
{
    DisplayInputCursor ();
    ScrSetCursor (CF_NormalCursor);
}



void Screen::SetCursorOff ()
{
//  ScrSetCursor (CF_HiddenCursor);
    HideInputCursor ();
}



void Screen::SetCursorFat ()
{
    DisplayInputCursor ();
    ScrSetCursor (CF_FatCursor);
}



void Screen::SetCursorPos (const Point& Pos)
{
    gotoxy (Pos.X, Pos.Y);
}



void Screen::DisplayBuffer (const Rect& R, u16* Buf)
{
    int XtoDo, YtoDo;
    int XCount = R.XSize ();
    int YCount = R.YSize ();

    // Check if anything to do
    if (XCount == 0 || YCount == 0 || R.A.Y > YSize || R.A.X > XSize) {
	// Done
	return;
    }

    // Calculate the size of the output rectangle
    XtoDo = (R.B.X > XSize) ? XSize - R.A.X : XCount;
    YtoDo = (R.B.Y > YSize) ? YSize - R.A.Y : YCount;

    // If we have to translate the output, get buffer memory
    u16* Buf2;
    if (TransTable) {
	Buf2 = (u16*) alloca (XtoDo * sizeof (u16));
    }

    int Y = R.A.Y;
    while (YtoDo--) {
	// Do a translation if neccessary
	if (TransTable) {
	    // Translate and write to screen
	    Translate (Buf2, Buf, XtoDo);
	    CopyToScreenMemory (1,			// Height
				XtoDo,			// Width
				(BYTE*) Buf2,		// Buffer
				R.A.X,			// X position
				Y);			// Y position
	} else {
	    // No translation
	    CopyToScreenMemory (1,			// Height
				XtoDo,			// Width
				(BYTE*) Buf,		// Buffer
				R.A.X,			// X position
				Y);			// Y position
	}
	Buf += XCount;
	Y++;
    }

}



unsigned Screen::TerminalSpeed ()
// Get some information on the terminal speed. This will return a value
// between 0..10, where 10 is a very fast (direct access) screen and
// 0 is a very slow (300 baud) serial line.
// This may be used to change the amount of screen output, a program
// produces.
{
    // We have always direct access to the screen
    return 10;
}



