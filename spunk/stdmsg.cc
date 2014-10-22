/*****************************************************************************/
/*                                                                           */
/*                                 STDMSG.H                                  */
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



#include <stdarg.h>

#include "msgid.h"
#include "keydef.h"
#include "splitmsg.h"
#include "syserror.h"
#include "progutil.h"
#include "stdmsg.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



static const u16 msErrorHdr             = MSGBASE_STDMSG +  0;
static const u16 msInformationHdr       = MSGBASE_STDMSG +  1;
static const u16 msFatalErrorHdr        = MSGBASE_STDMSG +  2;
static const u16 msConfirm              = MSGBASE_STDMSG + 10;
static const u16 msAbort                = MSGBASE_STDMSG + 11;
static const u16 msIgnore               = MSGBASE_STDMSG + 12;
static const u16 msRetry                = MSGBASE_STDMSG + 13;
static const u16 msEnd                  = MSGBASE_STDMSG + 14;
static const u16 msPleaseWait           = MSGBASE_STDMSG + 15;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class ListNode<String>;
#endif



/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/



Window* MsgWindow (const String& Msg, const String& Header, u16 Palette)
// Create a window from the given data and return it. There are a few chars
// in Msg that have a special meaning:
//
//      ~       toggle the attribute between atTextNormal and atTextHigh
//      |       begin a new line
//      ^       begin a new centered line
// Note: The calling function has to free the returned window.
{
    Rect Bounds;
    ListNode<String>* Node;
    ListNode<String>* N;

    // Get the length of the window header
    int HeaderLen = Header.Len ();

    // Split up the text for the lines
    Node = SplitLine (Msg, Bounds, HeaderLen);

    // Correct the size of the window
    if (Bounds.B.X < HeaderLen) {
        Bounds.B.X = HeaderLen;
    }
    Bounds.B.X += 4;            // + Space + Frame
    Bounds.B.Y += 2;            // + Frame

    // Center the message window inside the screen
    Rect ScreenSize (Background->OuterBounds ());
    Bounds.Center (ScreenSize, cfCenterAll);

    // Open the window but keep it hidden
    Window* Win = new Window (Bounds, wfFramed | wfCanMove, Palette);

    // Set the window options so that the window will remain centered even
    // if the video mode changes
    Win->SetOption (cfCenterAll);

    // Write the Msg into the window
    N = Node;
    u16 Y = 0;
    do {
        Win->CWrite (1, Y, *(N->Contents ()));
        N = N->Next ();
        Y++;
    } while (N != Node);

    // Release the line list
    ReleaseLines (Node);

    // Set the window header and make it active (this will also show
    // the window)
    if (HeaderLen > 0) {
        Win->SetHeader (Header);
    }
    Win->Activate ();

    // Return the created window
    return Win;
}



Window* PleaseWaitWindow ()
// Pops up a centered, activated, cyan window with a message like
// "Please wait..." in the current language
// The caller must dispose the window.
{
    return MsgWindow (LoadMsg (msPleaseWait), "", paCyan);
}



static String KeyMsg (Key K, unsigned MsgNum)
// Return the name of key K surrounded by '~' and with the message with
// number MsgNum added
{
    return GetKeyName3 (K) + LoadMsg (MsgNum);
}



unsigned ResponseWindow (const String& Msg, const String& Header, u16 Palette,
                         unsigned ResponseFlags)
// Show a window via MsgWindow with the given Msg, Header, Palette. Ask
// the user for a response. Allowed responses are coded in ResponseFlags.
// The status line is updated to show the valid responses.
{
    // ResponseFlags cannot be empty
    PRECONDITION (ResponseFlags != 0);

    // Build a statusline according to the flags set in ResponseFlags
    String StatusText;
    if (ResponseFlags & reConfirm) StatusText += KeyMsg (kbEnter, msConfirm);
    if (ResponseFlags & reAbort)   StatusText += KeyMsg (vkAbort, msAbort);
    if (ResponseFlags & reIgnore)  StatusText += KeyMsg (kbMetaI, msIgnore);
    if (ResponseFlags & reRetry)   StatusText += KeyMsg (kbMetaR, msRetry);
    if (ResponseFlags & reEnd)     StatusText += KeyMsg (vkAbort, msEnd);

    // Create the window
    Window* Win = MsgWindow (Msg, Header, Palette);

    // Show the new status line
    PushStatusLine (StatusText);

    // Now get the users response
    unsigned Result = 0;
    while (Result == 0) {

        Key K = KbdGet ();

        switch (K) {

            case kbEnter:
            case vkAccept:
                if (ResponseFlags & reConfirm) {
                    Result = reConfirm;
                } else if (ResponseFlags & reEnd) {
                    Result = reEnd;
                }
                break;

            case vkAbort:
                if (ResponseFlags & reAbort) {
                    Result = reAbort;
                } else if (ResponseFlags & reEnd) {
                    Result = reEnd;
                }
                break;

            case kbMetaI:
                if (ResponseFlags & reIgnore) {
                    Result = reIgnore;
                }
                break;

            case kbMetaR:
                if (ResponseFlags & reRetry) {
                    Result = reRetry;
                }
                break;

            case vkResize:
                Win->MoveResize ();
                break;


        }
    }

    // Delete the window and restore the old statusline
    delete Win;
    PopStatusLine ();

    return Result;
}



void ErrorMsg (const String& Msg)
// Shows an error window. Returns when the window is closed (by vkAbort).
{
    ResponseWindow (Msg,                                // Formatted message
                    LoadMsg (msErrorHdr),               // window header
                    paRed,                              // palette
                    reAbort);                           // valid responses
}



void ErrorMsg (u16 MsgNo, ...)
// Shows an error window. Returns when the window is closed (by vkAbort).
{
    va_list ap;
    va_start (ap, MsgNo);
    ResponseWindow (String (LoadMsg (MsgNo), ap),       // Formatted message
                    LoadMsg (msErrorHdr),               // window header
                    paRed,                              // palette
                    reAbort);                           // valid responses
    va_end (ap);
}



void SysErrorMsg (int ErrorCode)
// Display an appropriate error message for the system error code errno. Errno
// _must_ be a valid error code from ::errno!
{
    ErrorMsg (GetSysErrorMsg (ErrorCode));
}



void InformationMsg (const String& Msg)
// Shows an information window. Returns when the window is closed (by vkAbort).
{
    ResponseWindow (Msg,                                // Formatted message
                    LoadMsg (msInformationHdr),         // window header
                    paGray,                             // palette
                    reEnd);                             // valid responses
}



void FatalErrorMsg (const String& Msg)
// Shows an error window. Returns when the window is closed (by vkAbort).
{
    ResponseWindow (Msg,                                // Formatted message
                    LoadMsg (msFatalErrorHdr),          // window header
                    paError,                            // palette
                    reAbort);                           // valid responses
}




