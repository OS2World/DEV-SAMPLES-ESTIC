/*****************************************************************************/
/*									     */
/*				    SCREEN.CC				     */
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



#include <string.h>

#include "../cont.h"
#include "../winattr.h"
#include "../environ.h"
#include "../progutil.h"
#include "../screen.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Instance of the screen class to handle screen output. Must be initialized
// from outside (RootWindow)
Screen* TheScreen;

// Screen dimensions
extern unsigned ScreenHeight;
extern unsigned ScreenWidth;

// Screen is using codepage 437
extern int ScreenCP437;

// Screen is using a color mode
extern int ScreenColor;



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



// Because of name clashes with the X Window system, the complete functionality
// of class Screen is implemented outside in module console.cc (maybe
// namespaces will help avoid this problem one day).
// The following functions are all external visible and defined in console.cc.



void ScrSetMode (unsigned Mode);
// Set a screen mode

void ScrInit (unsigned XSize, unsigned YSize);
// Initialize the screen

void ScrExit ();
// Destroy the window

void ScrWriteBuf (unsigned X, unsigned Y, const u16* Buf, unsigned Count);
// Write a buffer line to the screen, clipping right if needed

void ScrFlush ();
// Flush the output command queue

void ScrCursorOn ();
// Set the cursor state to on

void ScrCursorOff ();
// Set the cursor state to off

void ScrCursorFat ();
// Set the cursor state to fat

void ScrSetCursorPos (unsigned X, unsigned Y);
// Set the cursor position



/*****************************************************************************/
/*				 class Screen				     */
/*****************************************************************************/



Screen::Screen ():
    XSize (80),
    YSize (25),
    CurrentMode (vmVGA_80x25),
    Color (1),
    Console (1),
    CP437 (1),
    TransTable (NULL)
{
    // Initialize the screen stuff
    ScrInit (XSize, YSize);

    // The values given to the call above are default values. They may be
    // overridden by the user. So get the actual values now.
    XSize = ScreenWidth;
    YSize = ScreenHeight;

    // Check for the codepage 437/850
    if (ScreenCP437 == 0) {

	// Use a replacement string for the frame chars
	ActiveFrame = InactiveFrame = SimpleFrame;

	// We don't have the codepage 437. Check if we have ISO 8859-1 support
	// and load a matching translation table. Allow for both environment
	// variables, SPUNK_CTYPE and LC_CTYPE and allow for some permutations
	// of the ISO-8859-1 name, since everyone handles it different :-(
	String Var = GetEnvVar ("SPUNK_CTYPE");
	if (Var.IsEmpty ()) {
	    Var = GetEnvVar ("LC_CTYPE");
	}

	// Ok, we now have the value of the environment variable. Match it
	// against a pattern to cover the maximum count of possible cases.
	Var.ToUpper ();
	char* ResName;
	if (Var.Match ("*ISO[-_]8859[-_]1*")) {
	    // We got some iso-8859-1 string or the other
	    ResName = "SCREEN.ISO-8859-1-Table";
//	} else if (Var.Match ("*KOI-8R*")) {
//	    // Currently not used
//	    ResName = "SCREEN.KOI-8R-Table";
	} else {
	    ResName = "SCREEN.7BIT-ASCII-Table";
	}

	// Load the translation table from the resource
	Container* C = (Container*) LoadResource (ResName);
	TransTable = (unsigned char*) C->RetrieveData ();
	delete C;
    }

    // Remember if we are using colors
    Color = ScreenColor;
}



Screen::~Screen ()
{
    // Destroy the window
    ScrExit ();

    // Delete the translation table
    delete [] TransTable;
}



u16* Screen::Translate (u16* Target, u16* Source, unsigned Len)
// Translate a complete buffer via the translation table
{
    if (TransTable) {
	u16* T = Target;
	while (Len--) {
	    *T = (*Source & 0xFF00) | TransTable [(*Source & 0xFF)];
	    Source++;
	    T++;
	}
	return Target;
    } else {
	return Source;
    }
}



void Screen::SetMode (u16 Mode)
{
    // Remember mode
    CurrentMode = Mode;

    // Set the mode
    ScrSetMode (Mode);

    // Set the new size
    XSize = ScreenWidth;
    YSize = ScreenHeight;

    // Colors may have changed
    Color = ScreenColor;
}



void Screen::SetCursorOn ()
{
    ScrCursorOn ();
}



void Screen::SetCursorOff ()
{
    ScrCursorOff ();
}



void Screen::SetCursorFat ()
{
    ScrCursorFat ();
}



void Screen::SetCursorPos (const Point &Pos)
{
    ScrSetCursorPos (unsigned (Pos.X), unsigned (Pos.Y));
}



void Screen::DisplayBuffer (const Rect& R, u16* Buf)
{
    int XtoDo, YtoDo;
    int XCount = R.XSize ();
    int YCount = R.YSize ();

    // Check if there is anything to do
    if (XCount == 0 || YCount == 0 || R.A.Y > YSize || R.A.X > XSize) {
	// Done
	return;
    }

    // Calculate the size of the output rectangle
    XtoDo = (R.B.X > XSize) ? XSize - R.A.X : XCount;
    YtoDo = (R.B.Y > YSize) ? YSize - R.A.Y : YCount;

    // If we have to translate the output, get buffer memory. This operation
    // is cheap, so do it, even if we don't need the buffer to avoid warnings
    u16* Buf2 = (u16*) alloca (XtoDo * sizeof (u16));

    // Write the stuff to the screen
    for (unsigned Y = R.A.Y; Y < R.A.Y + YtoDo; Y++) {

	// Copy the data into the virtual screen
	ScrWriteBuf (R.A.X, Y, Translate (Buf2, Buf, XtoDo), XtoDo);

	// Next line
	Buf += XCount;

    }

    // Flush the output queue
    ScrFlush ();
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




