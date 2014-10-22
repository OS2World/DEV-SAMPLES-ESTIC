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
#include <malloc.h>

#define INCL_VIO
#include <os2.h>

#include "screen.h"


// Instance of the screen class to handle screen output. Must be initialized
// from outside (RootWindow)
Screen * TheScreen;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



struct _ModeInfo {
    u16             Mode;
    unsigned char   Type;
    unsigned char   ColorBits;
    u16             Cols;
    u16             Rows;
    u16             HRes;
    u16             VRes;
};

const ModeCount = 11;
static _ModeInfo ModeInfo [ModeCount] = {
    { vmBW40,       5, 4,  40, 25, 320, 200 },
    { vmCO40,       1, 4,  40, 25, 320, 200 },
    { vmBW80,       5, 4,  80, 25, 640, 350 },
    { vmCO80,       1, 4,  80, 25, 720, 400 },
    { vmMono,       0, 0,  80, 25, 720, 400 },
    { vmVGA_80x30,  1, 4,  80, 30, 720, 480 },
    { vmVGA_80x34,  1, 4,  80, 34, 720, 480 },
    { vmVGA_80x43,  1, 4,  80, 43, 640, 350 },
    { vmVGA_80x50,  1, 4,  80, 50, 720, 400 },
    { vmVGA_80x60,  1, 4,  80, 60, 720, 480 },
    { vmET4_100x40, 1, 4, 100, 40, 800, 600 }
};



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static u16 ScrGetMode ()
// return the current video mode
{
    VIOMODEINFO ModeData;

    // Get mode info
    ModeData.cb = 12;
    VioGetMode (&ModeData, 0);

    // Search for the mode in the table
    for (_ModeInfo* M = ModeInfo; M < &ModeInfo [ModeCount]; M++) {
        if (M->Type == ModeData.fbType && M->ColorBits == ModeData.color &&
            M->Cols == ModeData.col    && M->Rows      == ModeData.row) {
            // Found the mode
            return M->Mode;
        }
    }

    // OOPS - unknown mode
    return vmInvalid;

}



static void ScrSetMode (u16 Mode)
{
    VIOMODEINFO ModeData;

    for (int I = 0; I < (sizeof (ModeInfo) / sizeof (ModeInfo [0])); I++) {
        if (Mode == ModeInfo [I].Mode) {
            ModeData.cb     = 12;
            ModeData.fbType = ModeInfo [I].Type;
            ModeData.color  = ModeInfo [I].ColorBits;
            ModeData.col    = ModeInfo [I].Cols;
            ModeData.row    = ModeInfo [I].Rows;
            ModeData.hres   = ModeInfo [I].HRes;
            ModeData.vres   = ModeInfo [I].VRes;
            VioSetMode (&ModeData, 0);
            return;
        }
    }
}



static u16 ScrGetCursor ()
// get the current cursor type
{
    VIOCURSORINFO CursorInfo;

    VioGetCurType (&CursorInfo, 0);
    return ( (((u16) CursorInfo.yStart) << 8) | (((u16) CursorInfo.cEnd) & 0x00FF) );
}



static void ScrSetCursor (u16 Cursor)
// set a cursor
{
    VIOCURSORINFO CursorInfo;

    CursorInfo.yStart = (Cursor >> 8);
    CursorInfo.cEnd   = (Cursor & 0x00FF);
    CursorInfo.cx     = 0;
    CursorInfo.attr   = (Cursor == 0) ? 0xFFFF : 0;

    VioSetCurType (&CursorInfo, 0);
}



/*****************************************************************************/
/*                               class Screen                                */
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
            *T++ = TransTable [*S++];           // Translate character
            *T++ = *S++;                        // Copy attribute
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
    VIOMODEINFO ModeInfo;
    ModeInfo.cb         = sizeof (ModeInfo);
    VioGetMode (&ModeInfo, 0);
    XSize = ModeInfo.col;
    YSize = ModeInfo.row;
    unsigned YCell = ModeInfo.vres / ModeInfo.row;

    // Calculate the cursor settings from the character cell height
    CF_HiddenCursor = 0;                        // is constant under OS/2
    CF_FatCursor    = 0x0100 + YCell - 1;
    CF_NormalCursor = ((YCell - 2) << 8) + (YCell - 1);

    // Check if color mode
    if (CurrentMode == vmBW40 || CurrentMode == vmBW80 || CurrentMode == vmMono) {
        Color = 0;
    } else {
        Color = 1;
    }

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

    VioSetCurPos ((USHORT) Pos.Y, (USHORT) Pos.X, 0);
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
            VioWrtCellStr ((char*) Buf2, 2*XtoDo, Y, R.A.X, 0);
        } else {
            // No translation
            VioWrtCellStr ((char *) Buf, 2 * XtoDo, Y, R.A.X, 0);
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



