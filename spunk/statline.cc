/*****************************************************************************/
/*                                                                           */
/*                                 STATLINE.CC                               */
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



#include "msgid.h"
#include "streamid.h"
#include "keydef.h"
#include "screen.h"
#include "progutil.h"
#include "statline.h"



// Register class StatusLine
LINK (StatusLine, ID_StatusLine);



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



static const u16 msHelp                 = MSGBASE_STATLINE +  0;
static const u16 msAccept               = MSGBASE_STATLINE +  1;
static const u16 msAbort                = MSGBASE_STATLINE +  2;
static const u16 msEnd                  = MSGBASE_STATLINE +  3;
static const u16 msProceed              = MSGBASE_STATLINE +  4;
static const u16 msChange               = MSGBASE_STATLINE +  5;
static const u16 msPrint                = MSGBASE_STATLINE +  6;
static const u16 msGraphics             = MSGBASE_STATLINE +  7;
static const u16 msInsert               = MSGBASE_STATLINE +  8;
static const u16 msDelete               = MSGBASE_STATLINE +  9;
static const u16 msSelect               = MSGBASE_STATLINE + 10;
static const u16 msConfirm              = MSGBASE_STATLINE + 11;
static const u16 msMove                 = MSGBASE_STATLINE + 12;
static const u16 msLogin                = MSGBASE_STATLINE + 13;
static const u16 msLogout               = MSGBASE_STATLINE + 14;
static const u16 msExit                 = MSGBASE_STATLINE + 15;
static const u16 msZoom                 = MSGBASE_STATLINE + 16;
static const u16 msClose                = MSGBASE_STATLINE + 17;
static const u16 msOpen                 = MSGBASE_STATLINE + 18;
static const u16 msResize               = MSGBASE_STATLINE + 19;
static const u16 msSave                 = MSGBASE_STATLINE + 20;
static const u16 msSelectKeys           = MSGBASE_STATLINE + 21;
static const u16 msSelectChooseKeys     = MSGBASE_STATLINE + 22;
static const u16 msPageKeys             = MSGBASE_STATLINE + 23;
static const u16 msMoveKeys             = MSGBASE_STATLINE + 24;
static const u16 msResizeKeys           = MSGBASE_STATLINE + 25;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Stack<String*>;
#endif



/*****************************************************************************/
/*                            class StatusLine                               */
/*****************************************************************************/



StatusLine::StatusLine (const String& FirstLine):
    Window (Rect (1, 1, 2, 2), 0, paGray, 0, 1),
    Lines (50)
{
    // Initialize the instance
    Init (FirstLine);
}



StatusLine::StatusLine (u32 StdFlags) :
    Window (Rect (1, 1, 2, 2), 0, paGray, 0, 1),
    Lines (50)
{
    // Initialize the instance
    Init (CreateLine (StdFlags));
}



void StatusLine::Init (const String& FirstLine)
// Used from the constructors
{
    // Create an empty line
    CurrentLine = new String (FirstLine);

    // Get the size of the screen
    Rect Bounds = Background->OuterBounds ();

    // Resize the window to the bottom of the screen. This will also draw the
    // statusline text
    Bounds.A.Y = Bounds.B.Y - 1;
    Resize (Bounds);

    Show ();
    Unlock ();
}



StatusLine::~StatusLine ()
{
    // Delete the current line
    delete CurrentLine;

    // Pop all remaining lines to avoid deletion problems in the destructor
    // of class Stack
    while (!Lines.IsEmpty ()) {
        delete Lines.Pop ();
    }
}



void StatusLine::Load (Stream& S)
{
    // Load parent data
    Window::Load (S);

    // Load own data
    S >> Lines;
    CurrentLine = (String*) S.Get ();
}



void StatusLine::Store (Stream& S) const
{
    // Store parent data
    Window::Store (S);

    // Store own data
    S << Lines;
    S.Put (CurrentLine);
}



u16 StatusLine::StreamableID () const
{
    return ID_StatusLine;
}



Streamable* StatusLine::Build ()
{
    return new StatusLine (Empty);
}



void StatusLine::DrawInterior ()
// Redraw the interior of the statusline
{
    // Lock the output
    Lock ();

    // Clear the window
    Clear ();

    // If a string exists, show it
    if (CurrentLine) {
        CWrite (0, 0, *CurrentLine);
    }

    // Now allow screen output
    Unlock ();
}



String StatusLine::CreateLine (u32 StdFlags)
// Create and return a string made from StdFlags
{
    String NewLine;

    // Create the new status line string
    for (u32 I = 1; I != 0; I <<= 1) {

        // Continue if the bit is not set
        if ((StdFlags & I) == 0) {
            continue;
        }

        // Look at the value
        switch (I) {

            case siHelp:
                NewLine += GetKeyName3 (vkHelp) + LoadMsg (msHelp);
                break;

            case siAbort:
                NewLine += GetKeyName3 (vkAbort) + LoadMsg (msAbort);
                break;

            case siEnd:
                NewLine += GetKeyName3 (vkAbort) + LoadMsg (msEnd);
                break;

            case siProceed:
                NewLine += GetKeyName3 (vkAbort) + LoadMsg (msProceed);
                break;

            case siAccept:
                NewLine += GetKeyName3 (vkAccept) + LoadMsg (msAccept);
                break;

            case siEnter:
                NewLine += GetKeyName3 (kbEnter) + LoadMsg (msAccept);
                break;

            case siSelectKeys:
                if (TheScreen->IsConsole ()) {
                    // We cannot display the keys if not at the console
                    NewLine += LoadMsg (msSelectKeys);
                }
                break;

            case siSelectChooseKeys:
                if (TheScreen->IsConsole ()) {
                    // We cannot display the keys if not at the console
                    NewLine += LoadMsg (msSelectChooseKeys);
                }
                break;

            case siInsert:
                NewLine += GetKeyName3 (vkIns) + LoadMsg (msInsert);
                break;

            case siDelete:
                NewLine += GetKeyName3 (vkDel) + LoadMsg (msDelete);
                break;

            case siChange:
                NewLine += GetKeyName3 (kbEnter) + LoadMsg (msChange);
                break;

            case siPrint:
                NewLine += GetKeyName3 (kbCtrlD) + LoadMsg (msPrint);
                break;

            case siGraphics:
                NewLine += GetKeyName3 (kbCtrlG) + LoadMsg (msGraphics);
                break;

            case siExit:
                NewLine += GetKeyName3 (vkQuit) + LoadMsg (msExit);
                break;

            case siConfirm:
                NewLine += GetKeyName3 (kbEnter) + LoadMsg (msConfirm);
                break;

            case siPageKeys:
                if (TheScreen->IsConsole ()) {
                    // We cannot display the keys if not at the console
                    NewLine += LoadMsg (msPageKeys);
                }
                break;

            case siLogin:
                NewLine += GetKeyName3 (kbMetaI) + LoadMsg (msLogin);
                break;

            case siLogout:
                NewLine += GetKeyName3 (kbMetaO) + LoadMsg (msLogout);
                break;

            case siZoom:
                NewLine += GetKeyName3 (vkZoom) + LoadMsg (msZoom);
                break;

            case siOpen:
                NewLine += GetKeyName3 (vkOpen) + LoadMsg (msOpen);
                break;

            case siSave:
                NewLine += GetKeyName3 (vkSave) + LoadMsg (msSave);
                break;

            case siClose:
                NewLine += GetKeyName3 (vkClose) + LoadMsg (msClose);
                break;

            case siResize:
                NewLine += GetKeyName3 (vkResize) + LoadMsg (msResize);
                break;

            case siMoveKeys:
                if (TheScreen->IsConsole ()) {
                    // We cannot display the keys if not at the console
                    NewLine += LoadMsg (msMoveKeys);
                }
                break;

            case siResizeKeys:
                if (TheScreen->IsConsole ()) {
                    // We cannot display the keys if not at the console
                    NewLine += LoadMsg (msResizeKeys);
                }
                break;


        }

    }

    return NewLine;
}



void StatusLine::Push (const String& NewLine)
{
    // Remember the length of the current line
    int OldLen = CurrentLine->Len ("~");

    // Push the current line onto the stack
    Lines.Push (CurrentLine);

    // Get a copy of the new statusline
    CurrentLine = new String (NewLine);

    // If the line is longer than the old one, just write it to the
    // screen. Otherwise get a copy, pad that copy to length and write
    // the copy. What makes it slightly complicated is the fact that
    // there are non-displayed characters in both strings....
    int NewLen = CurrentLine->Len ("~");
    if (NewLen >= OldLen) {
        CWrite (0, 0, *CurrentLine);
    } else {
        String S = *CurrentLine;
        S.Pad (String::Right, OldLen + S.Len () - NewLen);
        CWrite (0, 0, S);           // ^ Adjust for non visible chars
    }
}



void StatusLine::Push (u32 StdFlags)
{
    // Push the current line onto the stack
    Push (CreateLine (StdFlags));
}



void StatusLine::Pop ()
{
    // Remember the length of the current line
    int OldLen = CurrentLine->Len ("~");

    // Delete the current copy of the statusline
    delete CurrentLine;

    // Pop the last one
    CurrentLine = Lines.Pop ();

    // If the line is longer than the old one, just write it to the
    // screen. Otherwise get a copy, pad that copy to length and write
    // the copy. What makes it slightly complicated is the fact that
    // there are non-displayed characters in both strings....
    int NewLen = CurrentLine->Len ("~");
    if (NewLen >= OldLen) {
        CWrite (0, 0, *CurrentLine);
    } else {
        String S = *CurrentLine;
        S.Pad (String::Right, OldLen + S.Len () - NewLen);
        CWrite (0, 0, S);           // ^ Adjust for non visible chars
    }
}



void StatusLine::Replace (const String& NewLine)
{
    // Remember the length of the current line
    int OldLen = CurrentLine->Len ("~");

    // Delete current line
    delete CurrentLine;

    // Get a copy of the new statusline
    CurrentLine = new String (NewLine);

    // If the line is longer than the old one, just write it to the
    // screen. Otherwise get a copy, pad that copy to length and write
    // the copy. What makes it slightly complicated is the fact that
    // there are non-displayed characters in both strings....
    int NewLen = CurrentLine->Len ("~");
    if (NewLen >= OldLen) {
        CWrite (0, 0, *CurrentLine);
    } else {
        String S = *CurrentLine;
        S.Pad (String::Right, OldLen + S.Len () - NewLen);
        CWrite (0, 0, S);           // ^ Adjust for non visible chars
    }
}



void StatusLine::Replace (u32 StdFlags)
{
    // Set the line
    Replace (CreateLine (StdFlags));
}



/*****************************************************************************/
/*                          class BottomStatusLine                           */
/*****************************************************************************/



void BottomStatusLine::ScreenSizeChanged (const Rect& NewScreen)
// Called when the screen got another resolution. NewScreen is the new
// screen size.
{
    // Expand the size of the bar from the left to the right screen border
    // at the bottom of the screen
    Rect NewSize (NewScreen);
    NewSize.A.Y = NewSize.B.Y - 1;
    Resize (NewSize);
}




