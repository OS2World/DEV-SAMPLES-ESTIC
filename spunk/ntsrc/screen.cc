/*****************************************************************************/
/*                                                                           */
/*                                 SCREEN.CC                                 */
/*                                                                           */
/*                      (C) 1996 MU Softwareentwicklung                      */
/*                                                                           */
/*      Ullrich von Bassewitz                          Michael Peschel       */
/*      Wacholderweg 14                                   Ledergasse 3       */
/*      D-70597 Stuttgart                            D-72555 Metzingen       */
/*      uz@ibb.schwaben.com                      mipe@ibb.schwaben.com       */
/*                                                                           */
/*****************************************************************************/



#include <string.h>
#include <malloc.h>

#include <windows.h>

#include "screen.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



// Instance of the screen class to handle screen output. Must be initialized
// from outside (RootWindow)
Screen* TheScreen;

// the original handle
static HANDLE OriginalScrHandle;

// the screen handle
static HANDLE ScrHandle;

// actual screen dimensions
static unsigned ScreenHeight;
static unsigned ScreenWidth;

// max. window dimensions
static unsigned MaxWidth;
static unsigned MaxHeight;

// min. window dimensions
static const unsigned MinWidth  = 40;
static const unsigned MinHeight = 25;

// Screen is using a color mode
static int ScreenColor = 1;

// actual Mode
static unsigned ScreenMode;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void ScrOpen ()
// open a new screen buffer
{
    // save the original handle
    OriginalScrHandle = GetStdHandle (STD_OUTPUT_HANDLE);

    // check handle
    CHECK (OriginalScrHandle != INVALID_HANDLE_VALUE);

    // get a new handle
    ScrHandle = CreateConsoleScreenBuffer (GENERIC_READ | GENERIC_WRITE,
                                           0,           // not shared
                                           NULL,        // no security attr.
                                           CONSOLE_TEXTMODE_BUFFER,
                                           NULL);
    // check handle
    CHECK (ScrHandle != INVALID_HANDLE_VALUE);

    // activate the new screen buffer
    CHECK (SetConsoleActiveScreenBuffer (ScrHandle));

    // Request the max. current window
    COORD Size = GetLargestConsoleWindowSize (ScrHandle);

    // Set data
    MaxWidth  = Size.X;
    MaxHeight = Size.Y;
}



static void ScrClose ()
// restore the original screen buffer
{
    // activate the new screen buffer
    SetConsoleActiveScreenBuffer (OriginalScrHandle);
}




static void ScrSetMode (u16 Mode)
{
    unsigned XSize, YSize;

    // Request the max. possible window size
    COORD MaxSize = GetLargestConsoleWindowSize (ScrHandle);

    // Set global data
    MaxWidth  = MaxSize.X;
    MaxHeight = MaxSize.Y;

    // Check the new dimensions
    switch (Mode) {

        case vmAsk:
            // request the current window size.
            CONSOLE_SCREEN_BUFFER_INFO Info;
            CHECK (GetConsoleScreenBufferInfo (ScrHandle, &Info) != 0);

            // Set data
            XSize = Info.dwMaximumWindowSize.X;
            YSize = Info.dwMaximumWindowSize.Y;

            // don't change color settings
            break;

        case vmBW40:
            XSize = 40;
            YSize = 25;
            ScreenColor = 0;
            break;

        case vmMono:
        case vmBW80:
            XSize = 80;
            YSize = 25;
            ScreenColor = 0;
            break;

        case vmCO40:
            XSize = 40;
            YSize = 25;
            ScreenColor = 1;
            break;

        case vmCO80:
            XSize = 80;
            YSize = 25;
            ScreenColor = 1;
            break;

        case vmVGA_80x30:
            XSize = 80;
            YSize = 30;
            ScreenColor = 1;
            break;

        case vmVGA_80x34:
            XSize = 80;
            YSize = 34;
            ScreenColor = 1;
            break;

        case vmVGA_80x43:
            XSize = 80;
            YSize = 43;
            ScreenColor = 1;
            break;

        case vmVGA_80x50:
            XSize = 80;
            YSize = 50;
            ScreenColor = 1;
            break;

        case vmVGA_80x60:
            XSize = 80;
            YSize = 60;
            ScreenColor = 1;
            break;

        case vmVGA_94x25:
            XSize = 94;
            YSize = 25;
            ScreenColor = 1;
            break;

        case vmVGA_94x30:
            XSize = 94;
            YSize = 30;
            ScreenColor = 1;
            break;

        case vmVGA_94x34:
            XSize = 94;
            YSize = 34;
            ScreenColor = 1;
            break;

        case vmVGA_94x43:
            XSize = 94;
            YSize = 43;
            ScreenColor = 1;
            break;

        case vmVGA_94x50:
            XSize = 94;
            YSize = 50;
            ScreenColor = 1;
            break;

        case vmVGA_94x60:
            XSize = 94;
            YSize = 60;
            ScreenColor = 1;
            break;

        case vmET4_100x40:
            XSize = 100;
            YSize = 40;
            ScreenColor = 1;
            break;

        default:
            // Ignore modes we do not know
            return;

    }

    // Check if we can resize the screen, bail out if not
    if (XSize > MaxWidth || YSize > MaxHeight) {
        return;
    }

    // set minimum size
    XSize = max (XSize, MinWidth);
    YSize = max (YSize, MinHeight);

    // if the new screen is larger set screen buffer before window-size
    // else window-size before screen buffer
    COORD Size = { XSize, YSize };
    SMALL_RECT Rect = { 0, 0, XSize-1, YSize -1 };

    if ((ScreenWidth * ScreenHeight) <= (XSize * YSize)) {

        SetConsoleScreenBufferSize (ScrHandle, Size);
        SetConsoleWindowInfo (ScrHandle, TRUE, &Rect);

    } else {

        SetConsoleWindowInfo (ScrHandle, TRUE, &Rect);
        SetConsoleScreenBufferSize (ScrHandle, Size);
    }

    // Set the global variables
    ScreenWidth  = XSize;
    ScreenHeight = YSize;
    ScreenMode   = Mode;
}



static u16 ScrGetCursor ()
// get the current cursor type
{
    CONSOLE_CURSOR_INFO CursorInfo;

    if (!GetConsoleCursorInfo (ScrHandle, &CursorInfo)) {
        return 0;
    }

    u16 Cursor = CursorInfo.dwSize;
    if (CursorInfo.bVisible) {
        Cursor |= 0x8000;
    }

    return Cursor;
}



static void ScrSetCursor (u16 Cursor)
// set a cursor
{
    CONSOLE_CURSOR_INFO CursorInfo;

    CursorInfo.dwSize   = Cursor & 0x7FFF;
    CursorInfo.bVisible = (Cursor & 0x8000) != 0;

    SetConsoleCursorInfo (ScrHandle, &CursorInfo);
}



/*****************************************************************************/
/*                               class Screen                                */
/*****************************************************************************/



Screen::Screen ():
    CP437 (1),
    TransTable (NULL)
{
    // open the screen
    ScrOpen ();

    // get startup values
    StartupCursor = ScrGetCursor ();

    // set mode to actual settings
    SetMode (vmAsk);

    // set cursor data
    CF_HiddenCursor = 25;
    CF_NormalCursor = 0x8000 + 20;
    CF_FatCursor    = 0x8000 + 99;

    // Switch the cursor off
    SetCursorOff ();
}



Screen::~Screen ()
{
    // Reset old video mode and cursor
    ScrClose ();
}



u16* Screen::Translate (u16* Target, u16* Source, unsigned Len)
// Translate a complete buffer via the translation table
{
    if (TransTable) {
        unsigned char* S = (unsigned char*) Target;
        unsigned char* T = (unsigned char*) Source;
        while (Len--) {
            *T++ = TransTable [*S++];           // Translate character
            *T++ = *S++;                        // Copy attribute
        }
        return Target;
    } else {
        // No translation table, return the source buffer
        return Source;
    }
}



void Screen::SetMode (u16 Mode)
{
    if (Mode == vmInvalid) {
        return;
    }

    // set mode
    ScrSetMode (Mode);

    // Remember mode
    CurrentMode = ScreenMode;

    // Set the new size
    XSize = ScreenWidth;
    YSize = ScreenHeight;

    // Colors may have changed
    Color = ScreenColor;

    // Switch cursor off
    SetCursorOff ();
}



void Screen::SetCursorOn ()
{
    ScrSetCursor (CF_NormalCursor);
}



void Screen::SetCursorOff ()
{
    ScrSetCursor (CF_HiddenCursor);
}



void Screen::SetCursorFat ()
{
    ScrSetCursor (CF_FatCursor);
}



void Screen::SetCursorPos (const Point& Pos)
{
    PRECONDITION (Pos.X < XSize && Pos.Y < YSize);

    // set new cursor position
    COORD NewPos = { Pos.X, Pos.Y };

    SetConsoleCursorPosition (ScrHandle, NewPos);
}



void Screen::DisplayBuffer (const Rect& R, u16* Buf)
{
    int XCount = R.XSize ();
    int YCount = R.YSize ();
    int TotalCount = XCount * YCount;

    // Check if anything to do
    if (XCount == 0 || YCount == 0 || R.A.Y > YSize || R.A.X > XSize) {
        // Done
        return;
    }

    // If we have to translate the output, get buffer memory
    u16* Buf2 = 0;
    if (TransTable) {
        Buf2 = new u16 [TotalCount];
        Buf  = Translate (Buf2, Buf, TotalCount);
    }

    // get buffer
    CHAR_INFO* chiBuf = new CHAR_INFO [TotalCount];

    // copy data into buffer
    CHAR_INFO* WorkBuf = chiBuf;
    for (int i = 0; i < TotalCount; i++, WorkBuf++, Buf++) {
        WorkBuf->Char.AsciiChar = (*Buf) & 0xff;
        WorkBuf->Attributes     = (*Buf >> 8) & 0xff;
    }

    // set data for output
    COORD BufCoord = { 0, 0 };
    COORD BufSize  = { XCount, YCount };
    SMALL_RECT WriteRect = { R.A.X, R.A.Y, R.B.X, R.B.Y };

    // write data to screen
    CHECK (WriteConsoleOutput (ScrHandle, chiBuf, BufSize, BufCoord, &WriteRect) != 0);

    // delete buffers
    delete Buf2;
    delete chiBuf;
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



