/*****************************************************************************/
/*                                                                           */
/*                                STDMENUE.CC                                */
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



#include "listnode.h"
#include "menuitem.h"
#include "stdmenue.h"
#include "splitmsg.h"
#include "progutil.h"
#include "msgid.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



static const u16 msAreYouShure          = MSGBASE_STDMENUE + 0;
static const u16 msReallyQuit           = MSGBASE_STDMENUE + 1;
static const u16 msDiscardChanges       = MSGBASE_STDMENUE + 2;
static const u16 msSaveChanges          = MSGBASE_STDMENUE + 3;
static const u16 msYesNo                = MSGBASE_STDMENUE + 4;
static const u16 msNoYes                = MSGBASE_STDMENUE + 5;



/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/



unsigned SimpleMenue (const String& Header, const String& MenueText)
// Builds a menue from the given strings, shows that menue and returns the
// users choice. The return value is 0 if the menue was aborted.
// Use \n to separate ther entries, use \c to create a new item that is
// centered. Use '@' to mark hotkeys (as usual).
{
    Rect Bounds;
    ListNode<String>* Node;
    ListNode<String>* N;

    // Split up the text for the menue items
    Node = SplitLine (MenueText, Bounds);
    i16 ItemCount = Bounds.B.Y;

    // Add horizontal room for the border, check against the length of the
    // Header line
    Bounds.B.X += 4;
    int Len = Header.Len ();
    if ((Len + 2) > Bounds.B.X) {
        Bounds.B.X = Len + 2;
    }

    // Add vertical room for the border and for the header line (if one
    // exists)
    if (Len > 0) {
        Bounds.B.Y += 4;
    } else {
        Bounds.B.Y += 2;
    }

    // Traverse through the list and build the menue items
    MenueItem* Item = NULL;
    N = Node->Prev ();
    i16 ID = ItemCount;
    do {
        Item = new MenueItem (*(N->Contents ()), ID, Item);
        ID--;
        N = N->Prev ();
    } while (N != Node->Prev ());

    // Delete the (not longer needed) string list
    ReleaseLines (Node);

    // Get the screen size and center the menue
    Rect ScreenSize (Background->OuterBounds ());
    Bounds.Center (ScreenSize, cfCenterAll);

    // Set up the header item. If a header string is given, the list should
    // contain an inactive menue item with the header and a menue line as
    // the first two items
    if (Len > 0) {
        Item = new MenueItem (Header, 10000, new MenueLine (10001, Item));
    }

    // Now create the menue
    Menue* M = new Menue (Bounds.A, "", Item);

    // Set the menue options so that the menue remains centered if the
    // screen resolution changes, but allow moving the menue
    M->SetOption (cfCenterAll);
    M->SetCanMove ();

    // The menue is invisible. Make the header line inactive (remember: the
    // item representing the header is the first item of the list)
    Item->Deactivate ();

    // Push a status line
    PushStatusLine (siAbort);

    // Time for the user to get active
    unsigned Choice = M->GetChoice ();

    // Pop the status line
    PopStatusLine ();

    // Destroy the menue
    delete M;

    // Return the result
    return Choice;
}



unsigned AskYesNo (const String& Header)
// Pop up a menue with the given header and the entries "Yes" and "No",
// "Yes" being the default entry.
// Returns 0 = Abort, 1 = No, 2 = Yes
{
    // We have to remap the return codes here
    switch (SimpleMenue (Header, LoadMsg (msYesNo))) {
        case 0: return 0;
        case 1: return 2;
        case 2: return 1;
    }
    FAIL ("AskYesNo: Unexpected result code");
    return 0;
}



unsigned AskNoYes (const String& Header)
// Pop up a menue with the given header and the entries "No" and "Yes",
// "No" being the default entry.
// Returns 0 = Abort, 1 = No, 2 = Yes
{
    return SimpleMenue (Header, LoadMsg (msNoYes));
}



unsigned AskReallyQuit ()
// Pops up a menue with the question "Really quit" and returns the result
// 0 = Abort, 1 = No, 2 = Yes
{
    return AskYesNo (LoadMsg (msReallyQuit));
}



unsigned AskDiscardChanges ()
// Pops up a menue with the question "Discard changes?" and returns the result
// 0 = Abort, 1 = No, 2 = Yes
{
    return AskNoYes (LoadMsg (msDiscardChanges));
}



unsigned AskSaveChanges ()
// Pops up a menue with the question "Save changes?" and returns the result
// 0 = Abort, 1 = No, 2 = Yes
{
    return AskYesNo (LoadMsg (msSaveChanges));
}



unsigned AskAreYouShure ()
// Pops up a menue with the question "Are you shure?" and returns the result
// 0 = Abort, 1 = No, 2 = Yes
{
    return AskNoYes (LoadMsg (msAreYouShure));
}



unsigned MenueChoice (const String& ResName)
// Loads a menue with the name ResName from the resource file, gets the user
// response and deletes the menue.
{
    Menue* M = (Menue*) LoadResource (ResName);
    unsigned Choice = M->GetChoice ();
    delete M;
    return Choice;
}




