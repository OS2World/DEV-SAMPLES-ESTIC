/*****************************************************************************/
/*                                                                           */
/*                                  SCREEN.CC                                */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
/*              Zwehrenbuehlstrasse 33                                       */
/*              D-72070 Tuebingen                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <termcap.h>
#include <sys/ioctl.h>

#include "../cont.h"
#include "../winattr.h"
#include "../environ.h"
#include "../progutil.h"
#include "../screen.h"


// Instance of the screen class to handle screen output. Must be initialized
// from outside (RootWindow)
Screen* TheScreen;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



static char TermBuf [2048];     // Buffer for the termcap entry
static char CapBuf [1024];      // Buffer for selected capabilities

// Terminal capabilities
static char* IS = NULL;         // Initialization string
static char* RS = NULL;         // Reset string
static char* KS = NULL;         // Keyboard init
static char* KE = NULL;         // Keyboard exit
static char* CM = NULL;         // Cursor motion
static char* VI = NULL;         // Cursor off
static char* VE = NULL;         // Cursor on
static char* VS = NULL;         // Cursor fat
static char* US = NULL;         // Underline on
static char* UE = NULL;         // Underline off
static char* SO = NULL;         // Standout mode on (reverse in most cases)
static char* SE = NULL;         // Standout mode off
static char* MR = NULL;         // Reverse on
static char* MB = NULL;         // Blink on
static char* MD = NULL;         // Bold on
static char* ME = NULL;         // Clear all attributes
static char* LE = NULL;         // cursor left one position
static char* IC = NULL;         // Insert char
static char* IM = NULL;         // Enter insert mode
static char* EI = NULL;         // Exit insert mode
static int XN   = 0;            // Delayed newline

// Abstract capabilities
static char* BoldOn;            // Use this if bold requested
static char* ReverseOn;         // Use this if reverse requested
static char* BlinkOn;           // Use this if blink requested

// Second last char if the terminal does not have the XN capability
static u16 SecLastChar = 0x0720;        // A space (is updated before first use)

// Current attribute
static unsigned LastAttr = 0xFFFF;

// Function pointer for setting attributes
static void (*SetAttr) (unsigned);

// Output buffer and handle for terminal writes
static int ScreenHandle = 1;
static unsigned char ScrBuf [2048];
static unsigned ScrBufFill = 0;

// Last cursor position. Under Linux, screen output is not possible without
// moving the cursor. So we have to restore the cursor position after the
// output.
static Point CursorPos;



/*****************************************************************************/
/*                             Buffer management                             */
/*****************************************************************************/



static void ScrFlushBuffer ()
// Flush the terminal buffer
{
    unsigned char* Buf = ScrBuf;
    while (ScrBufFill) {
        int BytesWritten = write (ScreenHandle, Buf, ScrBufFill);
        if (BytesWritten > 0) {
            Buf += BytesWritten;
            ScrBufFill -= BytesWritten;
        }
    }
}



static inline void ScrWriteChar (unsigned char C)
{
    if (ScrBufFill == sizeof (ScrBuf)) {
        ScrFlushBuffer ();
    }
    ScrBuf [ScrBufFill++] = C;
}



static void ScrWriteStr (const char* S)
// Write a string to the terminal
{
    if (S) {
        while (*S) {
            ScrWriteChar (*S++);
        }
    }
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void ScrGoto (unsigned X, unsigned Y)
// Goto a specific cursor location
{
    ScrWriteStr (tgoto (CM, X, Y));
}



static void ScrSetColor (unsigned Attr)
// Set a new color attribute. This should only be called when the screen is
// the console, because all escape sequences are hardcoded.
{
    static char ForegroundColors [8][3] = {
        "30", "34", "32", "36", "31", "35", "33", "37"
    };
    static char BackgroundColors [8][3] = {
        "40", "44", "42", "46", "41", "45", "43", "47"
    };

    // Check if anything changed
    if (Attr == LastAttr) {
        // Nothing to do
        return;
    }

    // Write the start of the color escape sequence
    ScrWriteStr ("\033[");
    unsigned HadValue = 0;

    if ((Attr & 0x0F) != (LastAttr & 0x0F)) {

        // Set intensity
        ScrWriteStr (Attr & 0x08 ? "01;" : "21;");

        // Set foreground color
        ScrWriteStr (ForegroundColors [Attr & 0x07]);
        HadValue = 1;
    }

    if ((Attr & 0xF0) != (LastAttr & 0xF0)) {

        // Add a separator if needed
        if (HadValue) {
            ScrWriteChar (';');
        }

        // Set blink attribute
        ScrWriteStr (Attr & 0x80 ? "05;" : "25;");

        // Set background color
        ScrWriteStr (BackgroundColors [(Attr >> 4) & 0x07]);
    }

    // End the color escape sequence and remember the new attribute
    ScrWriteChar ('m');
    LastAttr = Attr;
}



static void ScrSetAttr (unsigned Attr)
// Set a monochrome attribute.
{
    // Check if anything changed
    if (Attr == LastAttr) {
        // Nothing to do
        return;
    }

    // Reset all attributes
    if (ME) {
        // We have the ME (clear all attributes) cap
        ScrWriteStr (ME);
    } else {
        // OOPS. Check what attributes have been set and try to reset them
        if (LastAttr & 0x70) {
            // LastAttr has been reverse, if ME is not defined, assume that
            // standout mode has been used.
            ScrWriteStr (SE);
        }
        if (LastAttr & 0x80) {
            // LastAttr has the blink attribute set. We can not do anything
            // to reset this...
        }
        if (LastAttr & 0x08) {
            // LastAttr has been bold. Assume that underline mode has been
            // used.
            ScrWriteStr (UE);
        }
    }

    // Set new attributes
    if (Attr & 0x08) {
        // Bold requested.
        ScrWriteStr (BoldOn);
    }
    if (Attr & 0x70) {
        // Reverse mode has been requested.
        ScrWriteStr (ReverseOn);
    }
    if (Attr & 0x80) {
        // Blink mode has been requested
        ScrWriteStr (BlinkOn);
    }

    // Remember the new attribute
    LastAttr = Attr;

}



static inline void ScrWriteAttrChar (u16 C)
// Write one char with attribute to the screen
{
    // Set the new attribute
    SetAttr (C >> 8);

    // Write the character
    ScrWriteChar (C);
}



static void ScrWriteBuf (u16* Buf, u16 X, u16 Y, u16 Count)
// Write the buffer contents to the screen
{
    // Goto the cursor location
    ScrGoto (X, Y);

    // Write the buffer
    while (Count--) {
        // Write the character
        ScrWriteAttrChar (*Buf++);
    }
}



/*****************************************************************************/
/*                               class Screen                                */
/*****************************************************************************/



void Screen::TCInit ()
// Termcap initialization
{
    // Check if we have a color override
    Color = GetEnvBool ("SPUNK_COLOR", Color);

    // Determine the character set and the corresponding translation table
    CP437 = GetEnvBool ("SPUNK_CP437", CP437);
    if (CP437 == 0) {

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
//      } else if (Var.Match ("*KOI-8R*")) {
//          // Currently not used
//          ResName = "SCREEN.KOI-8R-Table";
        } else {
            ResName = "SCREEN.7BIT-ASCII-Table";
        }

        // Load the translation table from the resource
        Container* C = (Container*) LoadResource (ResName);
        TransTable = (unsigned char*) C->RetrieveData ();
        delete C;
    }

    // Check the term environment variable
    char* T;
    if ((T = getenv ("TERM")) == NULL) {
        // OOPS - no way out!
        FAIL ("TERM environment variable not set!");
    }

    // Get the termcap entry for the terminal
    switch (tgetent (TermBuf, T)) {

        case 0:
            FAIL ("No termcap entry for your terminal found!");
            break;

        case -1:
            FAIL ("No /etc/termcap file found!");
            break;

    }

    // Set the starting pointer to the capability buffer
    char* CapPtr = CapBuf;

    // Search for selected terminal capabilities. The cursor motion string
    // is a special case, because it is needed in any case
    CM = tgetstr ("cm", &CapPtr);
    if (CM == NULL) {
        FAIL ("No cursor motion capability found!");
    }

    // Read some other termcap capabilities
    IS = GetIS (tgetstr ("is", &CapPtr));       // Init
    RS = GetRS (tgetstr ("rs", &CapPtr));       // Exit
    KS = tgetstr ("ks", &CapPtr);               // Keyboard init
    KE = tgetstr ("ke", &CapPtr);               // Keyboard exit
    VI = tgetstr ("vi", &CapPtr);               // Cursor off
    VE = tgetstr ("ve", &CapPtr);               // Cursor on
    VS = tgetstr ("vs", &CapPtr);               // Cursor fat
    US = tgetstr ("us", &CapPtr);               // Underline on
    UE = tgetstr ("ue", &CapPtr);               // Underline off
    SO = tgetstr ("so", &CapPtr);               // Standout mode on (reverse in most cases)
    SE = tgetstr ("se", &CapPtr);               // Standout mode off
    MR = tgetstr ("mr", &CapPtr);               // Reverse on
    MB = tgetstr ("mb", &CapPtr);               // Blink on
    MD = tgetstr ("md", &CapPtr);               // Bold on
    ME = tgetstr ("me", &CapPtr);               // Clear all attributes
    LE = tgetstr ("le", &CapPtr);               // Cursor left one position
    IC = tgetstr ("ic", &CapPtr);               // Insert char
    IM = tgetstr ("im", &CapPtr);               // Enter insert mode
    EI = tgetstr ("ei", &CapPtr);               // Exit insert mode
    XN = tgetflag ("xn");                       // Delayed newline

    // If we don't have color, try to determine the abstract attributes
    if (!Color) {
        // Try to set the bold attribute
        if (MD) {
            // We have bold
            BoldOn = MD;
        } else if (SO) {
            // Use standout instead of bold
            BoldOn = SO;
        } else {
            // OOPS! Menues without bold? Unthinkable!
            FAIL ("Terminal capability missing: BOLD (md)");
        }

        // Try to set the reverse attribute
        if (MR) {
            // We have reverse
            ReverseOn = MR;
        } else if (SO) {
            // Use standout instead
            ReverseOn = SO;
        } else {
            // OOPS! Menues without reverse? Unthinkable!
            FAIL ("Terminal capability missing: REVERSE (mr)");
        }

        // Try to set the blink attribute. Don't bother if it's missing
        BlinkOn = MB;

    }

    // Now set the attribute setting function for ScrWriteBuf
    // Beware: ?: does not work when using gcc 2.5.8
    if (Color) {
        SetAttr = ScrSetColor;
    } else {
        SetAttr = ScrSetAttr;
    }

    // Send the terminal and keyboard init strings
    ScrWriteStr (IS);
    ScrWriteStr (KS);

    // Set the mode data for this mode
    SetModeData ();
}



Screen::~Screen ()
{
    // Move the cursor to position 0/0. This is necessary with some terminals
    ScrGoto (0, 0);

    // Reset the terminal
    ScrWriteStr (VE);           // Cursor enable
    ScrWriteStr (KE);
    ScrWriteStr (RS);           // reset string
    ScrFlushBuffer ();

    // delete the translation table
    delete [] TransTable;
}



u16* Screen::Translate (u16* Target, u16* Source, unsigned Len)
// Translate a complete buffer via the translation table, return the valid
// one of the two given buffers
{
    if (TransTable) {
        unsigned char* T = (unsigned char*) Target;
        unsigned char* S = (unsigned char*) Source;
        while (Len--) {
            *T++ = TransTable [*S++];           // Translate character
            *T++ = *S++;                        // Copy attribute
        }
        return Target;
    } else {
        // No translation, return the original buffer
        return Source;
    }
}



void Screen::SetModeData ()
// Internally called after setting a new mode, sets cursor data etc.
{
    // Get the size of the screen. Try TIOCGWINSZ first, then try the
    // environment variables COLUMNS/LINES, at last ressort, use the
    // termcap file.
    winsize WinSize;
    WinSize.ws_row = 0;
    WinSize.ws_col = 0;
    ioctl (STDOUT_FILENO, TIOCGWINSZ, &WinSize);

    // Get number of cols
    if ((XSize = WinSize.ws_col) == 0) {
        if ((XSize = GetEnvNum ("COLUMNS")) == 0) {
            int X = tgetnum ("co");
            if (X <= 0) {
                FAIL ("Cannot determine number of terminal columns!");
            }
            XSize = X;
        }
    }

    // Get number of rows
    if ((YSize = WinSize.ws_row) == 0) {
        if ((YSize = GetEnvNum ("LINES")) == 0) {
            int Y = tgetnum ("li");
            if (Y <= 0) {
                FAIL ("Cannot determine number of terminal lines!");
            }
            YSize = Y;
        }
    }

    // Reset the fore/background attribute
    LastAttr = 0xFFFF;

    // Switch off the cursor and put it on position 0/0
    SetCursorOff ();
    SetCursorPos (Point (0, 0));
}



void Screen::SetMode (u16 /*Mode*/)
{
    // Set the new mode data in case the size has changed, but otherwise
    // ignore the request
    SetModeData ();
}



void Screen::SetCursorOn ()
{
    ScrWriteStr (VE);
    ScrFlushBuffer ();
}



void Screen::SetCursorOff ()
{
    ScrWriteStr (VI);
    ScrFlushBuffer ();
}



void Screen::SetCursorFat ()
{
    ScrWriteStr (VS);
    ScrFlushBuffer ();
}



void Screen::SetCursorPos (const Point& Pos)
{
    // Check the parameters
    PRECONDITION (Pos.X < XSize && Pos.Y < YSize);

    // Output the terminal sequence, then flush the output buffer
    ScrGoto (Pos.X, Pos.Y);
    ScrFlushBuffer ();

    // Remember the new position
    CursorPos = Pos;
}



void Screen::DisplayBuffer (const Rect& R, u16* Buf)
{
    // Get a copy of the output rectangle
    Rect Bounds = R;

    // Get the size of the unclipped rectangle
    int XCount = Bounds.XSize ();
    int YCount = Bounds.YSize ();

    // Check if there is anything to do
    if (XCount == 0 || YCount == 0 || Bounds.A.Y > YSize || Bounds.A.X > XSize) {
        // Done
        return;
    }

    // Clip the rectangle to the lower right border
    if (Bounds.B.X > XSize) {
        Bounds.B.X = XSize;
    }
    if (Bounds.B.Y > YSize) {
        Bounds.B.Y = YSize;
    }

    // Calculate the size of the output rectangle
    int XtoDo = Bounds.XSize ();
    int YtoDo = Bounds.YSize ();

    // Check if anything to do
    if (XtoDo == 0 || YtoDo == 0) {
        return;
    }

    // Get buffer memory for translation (alloca is cheap enough to do this
    // regardless of real needs)
    u16* Buf2 = (u16*) alloca (XtoDo * sizeof (u16));

    // If we will update the second last char in the last line, remember this
    // char
    if (Bounds.Contains (Point (XSize - 2, YSize - 1))) {
        SecLastChar = *Translate (Buf2, Buf + (YtoDo-1) * XCount + XtoDo-2, 1);
    }

    // Check if we need to use the "xn hack" for some terminals...
    int NeedXNHack = 0;
    if (!XN && Bounds.B.Y == YSize && Bounds.B.X == XSize) {
        // Terminal don't has the xn capability and we are about to write
        // the last char on the screen.
        NeedXNHack = 1;
        YtoDo--;                // Use separate algorithm for last line
    }

    // Write the stuff to the screen
    int Y = Bounds.A.Y;
    while (YtoDo--) {

        // Translate the buffer line
        u16* B = Translate (Buf2, Buf, XtoDo);

        // Write the buffer to the screen
        ScrWriteBuf (B, Bounds.A.X, Y, XtoDo);

        // Next line
        Buf += XCount;
        Y++;
    }

    // Apply the "xn hack" if needed
    if (NeedXNHack) {

        // Buf points to the last line. Translate this last line.
        u16* B = Translate (Buf2, Buf, XtoDo);

        // Draw all characters of this line but the last two
        if (XtoDo > 2) {
            ScrWriteBuf (B, Bounds.A.X, Y, XtoDo-2);
        } else {
            // Code below expects the cursor to point to the second last char
            ScrGoto (XSize - 2, YSize - 1);
        }

        // We can only update the last char if we have the "insert mode" or
        // "insert char" capability
        if (IC || (IM && EI)) {

            // Write out the last char in place of the second last
            ScrWriteAttrChar (B [XtoDo-1]);

            // Now go one character back
            if (LE) {
                ScrWriteStr (LE);
            } else {
                ScrGoto (XSize - 2, YSize - 1);
            }

            // Check out which one of both caps to use
            if (IC) {
                // We have the "insert char" capability
                ScrWriteStr (IC);
                ScrWriteAttrChar (SecLastChar);
            } else {
                // Use "insert mode"
                SetAttr (SecLastChar >> 8);
                ScrWriteStr (IM);
                ScrWriteChar (SecLastChar & 0xFF);
                ScrWriteStr (EI);
            }

        } else {

            // We don't have any insert capabilities, so just update the
            // second last char and ignore the last one. Cursor position is
            // allright...
            ScrWriteAttrChar (SecLastChar);

        }

    }

    // Restore the old cursor position
    ScrGoto (CursorPos.X, CursorPos.Y);

    // Flush the tty buffer
    ScrFlushBuffer ();

}



unsigned Screen::TerminalSpeed ()
// Get some information on the terminal speed. This will return a value
// between 0..10, where 10 is a very fast (direct access) screen and
// 0 is a very slow (300 baud) serial line.
// This may be used to change the amount of screen output, a program
// produces.
{
    if (IsConsole ()) {
        // We have always direct access to the screen
        return 10;
    } else {
        // Assume a serial line (this is wrong in an XTerm window!! ## )
        return 5;
    }
}





