/*****************************************************************************/
/*                                                                           */
/*                                 STDMSG.H                                  */
/*                                                                           */
/* (C) 1993,94  Ullrich von Bassewitz                                        */
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



#ifndef __STDMSG_H
#define __STDMSG_H


#include "window.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



const unsigned reConfirm        = 0x0001;
const unsigned reAbort          = 0x0002;
const unsigned reIgnore         = 0x0004;
const unsigned reRetry          = 0x0008;
const unsigned reEnd            = 0x0010;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



Window* MsgWindow (const String& Msg, const String& Header, u16 Palette);
// Create a window from the given data and return it. There are a few chars
// in Msg that have a special meaning:
//
//      ~       toggle the attribute between atTextNormal and atTextHigh
//      |       begin a new line
//      ^       begin a new centered line
// Note: The calling function has to free the returned window.

Window* PleaseWaitWindow ();
// Pops up a centered, activated, cyan window with a message like
// "Please wait..." in the current language
// The caller must dispose the window.

unsigned ResponseWindow (const String& Msg, const String& Header, u16 Palette,
                         unsigned ResponseFlags);
// Show a window via MsgWindow with the given Msg, Header, Palette. Ask
// the user for a response. Allowed responses are coded in ResponseFlags.
// The status line is updated to show the valid responses.

void ErrorMsg (const String& Msg);
// Shows an error window. Returns when the window is closed (by vkAbort).

void ErrorMsg (u16 MsgNo, ...);
// Shows an error window. Returns when the window is closed (by vkAbort).

void SysErrorMsg (int Errno);
// Display an appropriate error message for the system error code errno. Errno
// _must_ be a valid error code from ::errno!

void InformationMsg (const String& Msg);
// Shows an information window. Returns when the window is closed (by vkAbort).

void FatalErrorMsg (const String& Msg);
// Shows an error window. Returns when the window is closed (by vkAbort).



// End of STDMSG.H

#endif
