/*****************************************************************************/
/*                                                                           */
/*                                PROGUTIL.CC                                */
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
// functions do nothing but call some other functions. This functions are
// easier to use and you need only one include file.



#include "kbd.h"
#include "program.h"
#include "progutil.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



char** GetArgVec ()
// Get the program arguments
{
    return App->ArgVec;
}



int GetArgCount ()
// Get the program argument count
{
    return App->ArgCount;
}



void RemoveArg (int Index)
// Remove the argument with the given index
{
    App->RemoveArg (Index);
}



void Idle ()
// Call the applications Idle() method
{
    // No harm done if App does not exist...
    if (App) {
        App->Idle ();
    }
}



const String& GetProgName ()
// Return the program base name
{
    return App->GetProgName ();
}



int HelpAvail ()
// Calls App->HasHelp, see there
{
    return App->HasHelp ();
}



void CallHelp (const String& HelpKey)
// Calls App->CallHelp, see there
{
    App->CallHelp (HelpKey);
}



const Msg& LoadMsg (u16 MsgNum)
// Calls App->LoadMsg, see there
{
    return App->LoadMsg (MsgNum);
}



const Msg& LoadAppMsg (u16 MsgNum)
// Calls App->LoadAppMsg, see there
{
    return App->LoadAppMsg (MsgNum);
}



void FreeMsgBase ()
// Calls App->FreeMsgBase, see there
{
    App->FreeMsgBase ();
}



void FreeAppMsgBase ()
// Calls App->FreeAppMsgbase, see there
{
    App->FreeAppMsgBase ();
}



Streamable* LoadResource (const String& ResName, int MustHave)
// Calls App->LoadResource, see there
{
    return App->LoadResource (ResName, MustHave);
}



void PushStatusLine (const String& NewLine)
// Push the new status line
{
    App->StatusLine->Push (NewLine);
}



void PushStatusLine (u32 StatusFlags)
// Push a standard status line described by StatusFlags
{
    App->StatusLine->Push (StatusFlags);
}



void PopStatusLine ()
// Pops the next saved status line from the stack
{
    App->StatusLine->Pop ();
}



void ReplaceStatusLine (const String& NewLine)
// Replace the current statusline contents by the given string
{
    App->StatusLine->Replace (NewLine);
}



void ReplaceStatusLine (u32 NewFlags)
// Replace the current statusline contents by the given string
{
    App->StatusLine->Replace (NewFlags);
}



String CreateStatusLine (u32 StatusFlags)
// Create a standard status line string
{
    return StatusLine::CreateLine (StatusFlags);
}



String GetKeyName (Key K)
// Return the name of the given key.
{
    return Kbd->GetKeyName (K);
}



String GetKeyName2 (Key K)
// Return the name of the given key with a '~' before and after it.
{
    return '~' + Kbd->GetKeyName (K) + '~';
}



String GetKeyName3 (Key K)
// Return the name of the given key with " ~" before, and "~ " after the key
// name, ready for a use in the status line.
{
    return " ~" + Kbd->GetKeyName (K) + "~ ";
}



Key KbdGet ()
// Get a key from the keyboard. Calls CurThread()->KbdGet
{
    return CurThread () -> KbdGet ();
}



void KbdPut (Key K)
// Put a key back into the keyboard queue. Calls CurThread()->KbdPut (K)
{
    CurThread () -> KbdPut (K);
}



void RegisterKey (Key K)
// Calls CurThread()->RegisterKey()
{
    CurThread () -> RegisterKey (K);
}



void UnregisterKey (Key K)
// Calls CurThread()->UnregisterKey()
{
    CurThread () -> UnregisterKey (K);
}



int KeyIsRegistered (Key K)
// Calls CurThread()->KeyIsRegistered()
{
    return CurThread () -> KeyIsRegistered (K);
}



void PostEvent (Event* E)
// Post an event to the programs event queue and deliver it
{
    App->PostEvent (E);
}



void PostEvent (unsigned What)
// Post an event to the programs event queue and deliver it
{
    App->PostEvent (new Event (What));
}



void PostEvent (unsigned What, unsigned long Info)
// Post an event to the programs event queue and deliver it
{
    App->PostEvent (new Event (What, Info));
}



void PostEvent (unsigned What, void* Info)
// Post an event to the programs event queue and deliver it
{
    App->PostEvent (new Event (What, Info));
}



void PostEvent (unsigned What, Object* Info)
// Post an event to the programs event queue and deliver it
{
    App->PostEvent (new Event (What, Info));
}




