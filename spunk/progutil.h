/*****************************************************************************/
/*                                                                           */
/*                                PROGUTIL.H                                 */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
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



// This module contains utility functions for message handling. Most of the
// functions do nathing but call some other functions. This functions are
// easier to use and you need only one include file.



#ifndef __PROGUTIL_H
#define __PROGUTIL_H



#include "keydef.h"
#include "statflag.h"
#include "msg.h"
#include "event.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



char** GetArgVec ();
// Get the program arguments

int GetArgCount ();
// Get the program argument count

void RemoveArg (int Index);
// Remove the argument with the given index

void Idle ();
// Call the applications Idle() method

const String& GetProgName ();
// Return the program base name

int HelpAvail ();
// Calls App->HasHelp, see there

void CallHelp (const String& HelpKey);
// Calls App->CallHelp, see there

const Msg& LoadMsg (u16 MsgNum);
// Calls App->LoadMsg, see there

const Msg& LoadAppMsg (u16 MsgNum);
// Calls App->LoadAppMsg, see there

void FreeMsgBase ();
// Calls App->FreeMsgBase, see there

void FreeAppMsgBase ();
// Calls App->FreeAppMsgbase, see there

Streamable* LoadResource (const String& ResName, int MustHave = 1);
// Calls App->LoadResource, see there

void PushStatusLine (const String& NewLine);
// Push the new status line

void PushStatusLine (u32 StatusFlags);
// Push a standard status line described by StatusFlags

void PopStatusLine ();
// Pops the next saved status line from the stack

String GetKeyName (Key K);
// Return the name of the given key.

String GetKeyName2 (Key K);
// Return the name of the given key with a '~' before and after it.

String GetKeyName3 (Key K);
// Return the name of the given key with " ~" before, and "~ " after the key
// name, ready for a use in the status line.

void ReplaceStatusLine (const String& NewLine);
// Replace the current statusline contents by the given string

void ReplaceStatusLine (u32 NewFlags);
// Replace the current statusline contents by the given string

String CreateStatusLine (u32 StatusFlags);
// Create a standard status line string

Key KbdGet ();
// Get a key from the keyboard. Calls CurThread()->KbdGet

void KbdPut (Key K);
// Put a key back into the keyboard queue. Calls CurThread()->KbdPut (K)

void RegisterKey (Key K);
// Calls CurThread()->RegisterKey()

void UnregisterKey (Key K);
// Calls CurThread()->UnregisterKey()

int KeyIsRegistered (Key K);
// Calls CurThread()->KeyIsRegistered()

void PostEvent (Event* E);
// Post an event to the programs event queue and deliver it

void PostEvent (unsigned What);
// Post an event to the programs event queue and deliver it

void PostEvent (unsigned What, unsigned long Info);
// Post an event to the programs event queue and deliver it

void PostEvent (unsigned What, void* Info);
// Post an event to the programs event queue and deliver it

void PostEvent (unsigned What, Object* Info);
// Post an event to the programs event queue and deliver it



// End of PROGUTIL.H

#endif


