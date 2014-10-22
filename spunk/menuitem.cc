/*****************************************************************************/
/*                                                                           */
/*                                MENUITEM.CC                                */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
/*              Wacholderweg 14                                              */
/*              D-70597 Stuttgart                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include "menuitem.h"
#include "winattr.h"
#include "menuedit.h"
#include "progutil.h"
#include "streamid.h"
#include "strcvt.h"
#include "msgid.h"



// Register the classes
LINK (MenueLine, ID_MenueLine);
LINK (LongItem, ID_LongItem);
LINK (HexItem, ID_HexItem);
LINK (StringItem, ID_StringItem);
LINK (ToggleItem, ID_ToggleItem);
LINK (OffOnItem, ID_OffOnItem);
LINK (NoYesItem, ID_NoYesItem);
LINK (FloatItem, ID_FloatItem);
LINK (TimeItem, ID_TimeItem);
LINK (DateItem, ID_DateItem);
LINK (RStringItem, ID_RStringItem);



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



static const u16 msOffOn                = MSGBASE_MENUITEM + 0;
static const u16 msNoYes                = MSGBASE_MENUITEM + 1;



/*****************************************************************************/
/*                              class MenueLine                              */
/*****************************************************************************/



MenueLine::MenueLine (i16 aID, WindowItem *NextItem) :
        MenueItem ("", aID, NextItem)
{
    // Menuelines are inactive objects
    Flags |= ifInactive;
}



u16 MenueLine::StreamableID () const
{
    return ID_MenueLine;
}



Streamable* MenueLine::Build ()
{
    return new MenueLine (Empty);
}



void MenueLine::BuildEntry (const String&)
// Rebuild Entry when the length has changed. The given String is
// ignored.
{
    // Cut the entry string if it is longer than the new width
    Entry.Trunc (ItemWidth);

    // Construct a new display string
    Entry.Set (0, ItemWidth, InactiveFrame [fcHorizontal]);

    // Optimize memory
    Entry.Settle ();
}



u16 MenueLine::MinWidth ()
{
    // A line has min width zero
    return 0;
}



/*****************************************************************************/
/*                            class EditMenueItem                            */
/*****************************************************************************/



EditMenueItem::EditMenueItem (const String& aItemText, i16 aID, i16 EditID,
                              ItemWindow* EditWin, WindowItem* NextItem) :
        MenueItem (aItemText, aID, NextItem),
        EditWindow (EditWin), EditItemID (EditID)
{
}



EditMenueItem::~EditMenueItem ()
{
    delete EditWindow;
}



void EditMenueItem::Store (Stream &S) const
// Store the object data into a stream
{
    // Store parental data
    MenueItem::Store (S);

    // Store instance data
    S << EditItemID;
    S.Put (EditWindow);
}



void EditMenueItem::Load (Stream &S)
// Load the object data from a stream
{
    // Load parental data
    MenueItem::Load (S);

    // Load instance data
    S >> EditItemID;
    EditWindow = (ItemWindow *) S.Get ();
}



void EditMenueItem::PlaceEditWindow ()
// Place the edit window below this entry if EditWindow has no position
// (== has position 0/0).
{
    if (EditWindow) {

        // Get the coords of the edit window
        Rect Bounds (EditWindow->OuterBounds ());

        if (Bounds.A == Point (0, 0)) {

            // The edit window has no position. Get the coords of the
            // item and make them absolute
            Point P (ItemX, ItemY);
            Owner->Absolute (P);

            // Now place the edit window below the entry
            P.Y++;
            EditWindow->MoveRel (P);
        }
    }
}



WindowItem* EditMenueItem::ItemWithID (i16 aID)
// Search for the item with the given ID. The editwindow (if one exists)
// is also searched.
{
    if (ID == aID) {
        return this;
    } else {
        return EditWindow ? EditWindow->ItemWithID (aID) : (WindowItem*) NULL;
    }
}



void EditMenueItem::SetEditWindow (ItemWindow *Win, i16 EditID)
// Set the edit window. Beware: An already existing edit window is not
// deleted!
{
    // Check the given parameters
    PRECONDITION (Win == NULL || EditID != 0);

    // Store them
    EditWindow = Win;
    EditItemID = EditID;
}



/*****************************************************************************/
/*                              class LongItem                               */
/*****************************************************************************/



LongItem::LongItem (const String& aItemText, i16 aID, u16 aDigits, i16 EditID,
                    i32 Min, i32 Max, WindowItem* NextItem) :
    EditMenueItem (aItemText, aID, EditID, NULL, NextItem),
    LValue (Min),
    Digits (aDigits),
    LMin (Min),
    LMax (Max)
{
}



LongItem::LongItem (const String& aItemText, i16 aID, u16 aDigits,
                    WindowItem* NextItem) :
    EditMenueItem (aItemText, aID, 0, NULL, NextItem),
    LValue (0),
    Digits (aDigits),
    LMin (0x80000000),
    LMax (0x7FFFFFFF)
{
}



void LongItem::Load (Stream &S)
// Load the object data from a stream
{
    // Load parental data
    EditMenueItem::Load (S);

    // Load instance data
    S >> LValue >> Digits >> LMin >> LMax;
}



void LongItem::Store (Stream &S) const
// Store the object data into a stream
{
    // Store parental data
    EditMenueItem::Store (S);

    // Store instance data
    S << LValue << Digits << LMin << LMax;
}



u16 LongItem::StreamableID () const
{
    return ID_LongItem;
}



Streamable* LongItem::Build ()
{
    return new LongItem (Empty);
}



void LongItem::SetWidth (u16 NewWidth)
{
    // Check against the minimum width needed
    if (NewWidth < MinWidth ()) {
        // Ignore the request
        return;
    }

    // Store the new width
    ItemWidth = NewWidth;

    // Convert the number to string
    String S (I32Str (LValue));
    S.ForceLen (Digits, String::Left);
    S.Ins (0, ' ');
    BuildEntry (S);
}



u16 LongItem::MinWidth ()
{
    u16 Len = ItemText.Len ();
    u16 Width = Len + Digits + 2;       // One space to the left and right
    if (Len) {
        Width += 2;                     // Space between text and number
    }
    return Width;
}



i16 LongItem::Choose ()
// Choose the entry. If an edit window is defined (EditWindow != NULL),
// the entry with id EditItemID in EditWindow is called. If there was
// no abort (or EditWindow is not defined), the id of this item is
// returned.
{
    LongEdit *P;
    int Abort;
    i16 Result;

    // If the edit id is zero, return the own id
    if (EditItemID == 0) {
        return ID;
    }

    // Check if a predefined window exists
    if (EditWindow) {

        // Move the window to the correct position
        PlaceEditWindow ();

        // Allow editing the value
        P = (LongEdit *) EditWindow->ForcedItemWithID (EditItemID);
        P->SetValue (LValue);
        P->Edit (Abort);

        // if no abort, set the value and return the id
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

    } else {

        // No item defined, we have to create one
        P = new LongEdit ("", EditItemID, Digits, NULL);

        // Insert the item into the owner window
        Owner->AddItem (P);

        // Set the position of the newly created item
        P->SetPos (ItemX + ItemWidth - P->GetWidth (), ItemY);

        // Deselect "this"
        Deselect ();

        // Transfer the values from "this" to the new item. Calling
        // SetValue will also draw the edit item
        P->SetMinMax (LMin, LMax);
        P->SetValue (LValue);

        // allow editing
        P->Edit (Abort);

        // Get the edited value if the editing was not aborted
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

        // Delete the edit item from the owners list
        Owner->DeleteItem (P);

        // Select "this". This redraws the item and clears the edit field
        Select ();

    }

    // Return the result
    return Result;

}



void LongItem::SetValue (i32 Val)
{
    // Store the new value
    LValue = Val;

    // Build the new display text. As the number of digits is fixed,
    // the length cannot change.
    // Use SetWidth with the current width to do that
    SetWidth (ItemWidth);

    // Now redraw the item
    Draw ();
}



void LongItem::SetMinMax (i32 Min, i32 Max)
{
    // Check parameters
    PRECONDITION (Min <= Max);

    LMin = Min;
    LMax = Max;
}



/*****************************************************************************/
/*                               class HexItem                               */
/*****************************************************************************/



u16 HexItem::StreamableID () const
{
    return ID_LongItem;
}



Streamable* HexItem::Build ()
{
    return new HexItem (Empty);
}



void HexItem::SetWidth (u16 NewWidth)
{
    // Check against the minimum width needed
    if (NewWidth < MinWidth ()) {
        // Ignore the request
        return;
    }

    // Store the new width
    ItemWidth = NewWidth;

    // Convert the number to string
    String S (U32Str (LValue, 16));
    S.ForceLen (Digits, String::Left);
    S.Ins (0, ' ');
    BuildEntry (S);
}



i16 HexItem::Choose ()
// Choose the entry. If an edit window is defined (EditWindow != NULL),
// the entry with id EditItemID in EditWindow is called. If there was
// no abort (or EditWindow is not defined), the id of this item is
// returned.
{
    HexEdit *P;
    int Abort;
    i16 Result;

    // If the edit id is zero, return the own id
    if (EditItemID == 0) {
        return ID;
    }

    // Check if a predefined window exists
    if (EditWindow) {

        // Move the window to the correct position
        PlaceEditWindow ();

        // Allow editing the value
        P = (HexEdit *) EditWindow->ForcedItemWithID (EditItemID);
        P->SetValue (LValue);
        P->Edit (Abort);

        // if no abort, set the value and return the id
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

    } else {

        // No item defined, we have to create one
        P = new HexEdit ("", EditItemID, Digits, NULL);

        // Insert the item into the owner window
        Owner->AddItem (P);

        // Set the position of the newly created item
        P->SetPos (ItemX + ItemWidth - P->GetWidth (), ItemY);

        // Deselect "this"
        Deselect ();

        // Transfer the values from "this" to the new item. Calling
        // SetValue will also draw the edit item
        P->SetMinMax (LMin, LMax);
        P->SetValue (LValue);

        // allow editing
        P->Edit (Abort);

        // Get the edited value if the editing was not aborted
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

        // Delete the edit item from the owners list
        Owner->DeleteItem (P);

        // Select "this". This redraws the item and clears the edit field
        Select ();

    }

    // Return the result
    return Result;

}



/*****************************************************************************/
/*                             class StringItem                              */
/*****************************************************************************/



StringItem::StringItem (const String& aItemText, i16 aID, i16 EditID,
                        WindowItem* NextItem) :
    EditMenueItem (aItemText, aID, EditID, NULL, NextItem)
{
    // Allow all input chars
    AllowedChars.SetAll ();
}



StringItem::StringItem (StreamableInit) :
    EditMenueItem (Empty),
    SValue (Empty)
{
    // Allow all chars
    AllowedChars.SetAll ();
}



void StringItem::Load (Stream &S)
// Load the object data from a stream
{
    // Load parental data
    EditMenueItem::Load (S);

    // Load instance data
    S >> SValue;
}



void StringItem::Store (Stream &S) const
// Store the object data into a stream
{
    // Store parental data
    EditMenueItem::Store (S);

    // Store instance data
    S << SValue;
}



u16 StringItem::StreamableID () const
{
    return ID_StringItem;
}



Streamable* StringItem::Build ()
{
    return new StringItem (Empty);
}



void StringItem::SetWidth (u16 NewWidth)
{
    // Check against the minimum width needed
    u16 MinW = MinWidth ();
    if (NewWidth < MinW) {
        // Ignore the request
        return;
    }

    // Store the new width
    ItemWidth = NewWidth;

    // Get a copy of the value string, pad it left to the length needed
    String S (SValue);
    S.Pad (String::Left, ItemWidth - MinW);

    // Build the new entry
    S.Ins (0, ' ');
    BuildEntry (S);
}



u16 StringItem::MinWidth ()
{
    u16 Len = ItemText.Len ();
    u16 Width = Len + 2;        // One space to the left and right
    if (Len > 0) {
        Width += 2;             // Add two spaces between text and string
    }
    return Width;
}



i16 StringItem::Choose ()
// Choose the entry. If an edit window is defined (EditWindow != NULL),
// the entry with id EditItemID in EditWindow is called. If there was
// no abort (or EditWindow is not defined), the id of this item is
// returned.
{
    int Abort;
    i16 Result;

    // If the edit id is zero, return the own id
    if (EditItemID == 0) {
        return ID;
    }

    // Check if a predefined window exists
    if (EditWindow) {

        // Move the window to the correct position
        PlaceEditWindow ();

        // Allow editing the value
        TextEdit* P = (TextEdit*) EditWindow->ForcedItemWithID (EditItemID);
        P->SetValue (SValue);
        P->Edit (Abort);

        // if no abort, set the value and return the id
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

    } else {

        // No item defined, we have to create one
        u16 Len = ItemWidth - MinWidth () - 1;
        TextEdit* P = new TextEdit ("", EditItemID, Len-1, Len, NULL);

        // Set the allowed characters for input
        P->SetAllowedChars (AllowedChars);

        // Insert the item into the owner window
        Owner->AddItem (P);

        // Set the position of the newly created item
        P->SetPos (ItemX + ItemWidth - P->GetWidth (), ItemY);

        // Deselect "this"
        Deselect ();

        // Transfer the values from "this" to the new item. Calling
        // SetValue will also draw the edit item
        P->SetValue (SValue);

        // allow editing
        P->Edit (Abort);

        // Get the edited value if the editing was not aborted
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

        // Delete the edit item from the owners list
        Owner->DeleteItem (P);

        // Select "this". This redraws the item and clears the edit field
        Select ();

    }

    // Return the result
    return Result;

}



void StringItem::SetValue (const String& NewVal)
{
    // Store the new value
    SValue = NewVal;
    SValue.Settle ();

    // Build the new display text. As the number of digits is fixed,
    // the length cannot change.
    // Use SetWidth with the current width to do that
    SetWidth (ItemWidth);

    // Now redraw the item
    Draw ();
}



const CharSet& StringItem::GetAllowedChars () const
// Get the set of allowed input chars
{
    return AllowedChars;
}



void StringItem::SetAllowedChars (const CharSet& CS)
// Set the allowed input chars
{
    AllowedChars = CS;
}



void StringItem::AllowEmptyInput ()
// Allow an empty input
{
    AllowedChars += '\0';
}



void StringItem::DisallowEmptyInput ()
// Disallow an empty input line
{
    AllowedChars -= '\0';
}



/*****************************************************************************/
/*                             class ToggleItem                              */
/*****************************************************************************/



ToggleItem::ToggleItem (const String& aItemText, i16 aID,
                        const String& ToggleList, unsigned ToggleCount,
                        WindowItem* NextItem) :
    StringItem (aItemText, aID, 0, NextItem),
    TValue (0),
    TCount (ToggleCount),
    TList (ToggleList)
{
    // Get the length of the value list
    u16 Len = TList.Len ();

    // Check the parameters
    PRECONDITION (Len != 0 && TCount != 0 && (Len % TCount) == 0);

    // Initialize variables
    ItemWidth = MinWidth ();
    TLen = Len / TCount;
    SValue = TList.Cut (0, TLen);
}



void ToggleItem::Load (Stream& S)
// Load an object from a stream
{
    // Load parental data
    StringItem::Load (S);

    // Load new data
    S >> TValue >> TCount >> TLen >> TList;
}



void ToggleItem::Store (Stream& S) const
// Store an object into a stream
{
    // Store parental data
    StringItem::Store (S);

    // Store new data
    S << TValue << TCount << TLen << TList;
}



u16 ToggleItem::StreamableID () const
{
    return ID_ToggleItem;
}



Streamable* ToggleItem::Build ()
{
    return new ToggleItem (Empty);
}



void ToggleItem::SetValue (u16 NewVal)
// Set the new value of the toggle item
{
    // Check the new value
    PRECONDITION (NewVal < TCount);

    // Remember the new value
    TValue = NewVal;

    // Use the inherited function to set the actual value
    StringItem::SetValue (TList.Cut (TValue * TLen, TLen));
}



void ToggleItem::Toggle ()
// Toggle the value
{
    // Select the next value
    if (++TValue >= TCount) {
        TValue = 0;
    }
    SetValue (TValue);
}



void ToggleItem::SetWidth (u16 NewWidth)
// Set the new entry width
{
    // Check if the new width is acceptable
    u16 WMin = MinWidth ();
    if (NewWidth < WMin) {
        // Ignore the request
        return;
    }

    // Remember the new value
    ItemWidth = NewWidth;

    // Build the new entry
    if (SValue.Len () == 0) {
        BuildEntry (" ");
    } else {
        String S;
        S.Set (0, ItemWidth - WMin);

        // One space between value and itemtext
        if (ItemText.Len () != 0) {
            S += ' ';
        }

        BuildEntry (S + SValue);
    }

}



u16 ToggleItem::MinWidth ()
// return the width needed
{
    u16 Len = ItemText.Len ();
    return Len ? Len + TLen + 4 : Len + TLen + 2;
}



WindowItem * ToggleItem::ItemWithID (i16 aID)
// Return a pointer to this if the given ID is the ID of the object.
// A ToggleItem uses all IDs from ID to ID + ToggleCount - 1
{
    if (aID >= ID && aID < (ID + TCount)) {
        return this;
    } else {
        return NULL;
    }
}



i16 ToggleItem::GetID ()
// Return the ID of the entry. The returned ID is the ID of the item plus
// the current toggle value
{
    return ID + TValue;
}



i16 ToggleItem::Choose ()
// Choose this entry. This implementation toggles to the next value and
// returns the corresponding ID
{
    // No chance if the item is inactive
    if (!IsActive ()) {
        return 0;
    }

    // Select and show the next toggle value
    Toggle ();

    // Now return the ID
    return GetID ();
}



/*****************************************************************************/
/*                              class OffOnItem                              */
/*****************************************************************************/



OffOnItem::OffOnItem (const String& aItemText, i16 aID, WindowItem* NextItem) :
        ToggleItem (aItemText, aID, LoadMsg (msOffOn), 2, NextItem)
{
}



u16 OffOnItem::StreamableID () const
{
    return ID_OffOnItem;
}



void OffOnItem::Load (Stream& S)
{
    // Load data from ToggleItem
    ToggleItem::Load (S);

    // Now override the toggle text (loading this instance in a new language
    // environment should show the new language when loaded)
    TList = LoadMsg (msOffOn);
    TLen = TList.Len () / 2;
    SValue = TList.Cut (TValue * TLen, TLen);
}



Streamable* OffOnItem::Build ()
{
    return new OffOnItem (Empty);
}



/*****************************************************************************/
/*                              class NoYesItem                              */
/*****************************************************************************/



NoYesItem::NoYesItem (const String& aItemText, i16 aID, WindowItem* NextItem) :
    ToggleItem (aItemText, aID, LoadMsg (msNoYes), 2, NextItem)
{
}



u16 NoYesItem::StreamableID () const
{
    return ID_NoYesItem;
}



void NoYesItem::Load (Stream& S)
{
    // Load data from ToggleItem
    ToggleItem::Load (S);

    // Now override the toggle text (loading this instance in a new language
    // environment should show the new language when loaded)
    TList = LoadMsg (msNoYes);
    TLen = TList.Len () / 2;
    SValue = TList.Cut (TValue * TLen, TLen);
}



Streamable* NoYesItem::Build ()
{
    return new NoYesItem (Empty);
}



/*****************************************************************************/
/*                              class FloatItem                              */
/*****************************************************************************/



FloatItem::FloatItem (const String& aItemText, i16 aID, u16 aLD, u16 aTD,
                      u16 EditID, double Min, double Max, WindowItem* NextItem) :
    EditMenueItem (aItemText, aID, EditID, NULL, NextItem),
    FValue (Min),
    LD (aLD),
    TD (aTD),
    FMin (Min),
    FMax (Max)
{
}



FloatItem::FloatItem (const String& aItemText, i16 aID, u16 aLD, u16 aTD,
                      WindowItem* NextItem) :
    EditMenueItem (aItemText, aID, 0, NULL, NextItem),
    FValue (0),
    LD (aLD),
    TD (aTD),
    FMin (-100000),
    FMax (+100000)
{
}



void FloatItem::Load (Stream &S)
// Load the instance from a stream
{
    EditMenueItem::Load (S);
    S >> FValue >> LD >> TD >> FMin >> FMax;
}



void FloatItem::Store (Stream &S) const
// Store the instance into a stream
{
    EditMenueItem::Store (S);
    S << FValue << LD << TD << FMin << FMax;
}



u16 FloatItem::StreamableID () const
{
    return ID_FloatItem;
}



Streamable* FloatItem::Build ()
{
    return new FloatItem (Empty);
}



void FloatItem::SetValue (double Val)
// Set the new value of the float item
{
    // Store the new value
    FValue = Val;

    // Build the new display text. As the number of digits is fixed,
    // the length cannot change.
    // Use SetWidth with the current width to do that
    SetWidth (ItemWidth);

    // Now redraw the item
    Draw ();
}



void FloatItem::SetMinMax (double Min, double Max)
// Set the values for FMin/FMax
{
    FMin = Min;
    FMax = Max;
}



void FloatItem::SetWidth (u16 NewWidth)
// Set the new entry width
{
    // Check against the minimum width needed
    if (NewWidth < MinWidth ()) {
        // Ignore the request
        return;
    }

    // Store the new width
    ItemWidth = NewWidth;

    // Build the new entry
    String S (FloatStr (FValue, LD, TD));
    S.ForceLen (TD ? LD + TD + 1 : LD, String::Left);
    BuildEntry (S);
}



u16 FloatItem::MinWidth ()
// return the width needed
{
    u16 Len = ItemText.Len ();
    u16 Width = Len + 2 + LD + TD;              // one space left and right
    if (TD > 0) {
        Width++;                                // Decimal point
    }
    if (Len > 0) {
        Width += 2;                             // Space between float and text
    }
    return Width;
}



i16 FloatItem::Choose ()
// Choose the entry. If an edit window is defined (EditWindow != NULL),
// the entry with id EditItemID in EditWindow is called. If there was
// no abort (or EditWindow is not defined), the id of this item is
// returned.
{
    FloatEdit *P;
    int Abort;
    i16 Result;

    // If the edit id is zero, return the own id
    if (EditItemID == 0) {
        return ID;
    }

    // Check if a predefined window exists
    if (EditWindow) {

        // Move the window to the correct position
        PlaceEditWindow ();

        // Allow editing the value
        P = (FloatEdit *) EditWindow->ForcedItemWithID (EditItemID);
        P->SetValue (FValue);
        P->Edit (Abort);

        // if no abort, set the value and return the id
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

    } else {

        // No item defined, we have to create one
        P = new FloatEdit ("", EditItemID, LD, TD, NULL);

        // Insert the item into the owner window
        Owner->AddItem (P);

        // Set the position of the newly created item
        P->SetPos (ItemX + ItemWidth - P->GetWidth (), ItemY);

        // Deselect "this"
        Deselect ();

        // Transfer the values from "this" to the new item. Calling
        // SetValue will also draw the edit item
        P->SetMinMax (FMin, FMax);
        P->SetValue (FValue);

        // allow editing
        P->Edit (Abort);

        // Get the edited value if the editing was not aborted
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

        // Delete the edit item from the owners list
        Owner->DeleteItem (P);

        // Select "this". This redraws the item and clears the edit field
        Select ();

    }

    // Return the result
    return Result;

}



/*****************************************************************************/
/*                              class TimeItem                               */
/*****************************************************************************/



TimeItem::TimeItem (const String& aItemText, i16 aID, i16 EditID,
                    WindowItem* NextItem) :
    EditMenueItem (aItemText, aID, EditID, NULL, NextItem)
{
}



TimeItem::TimeItem (const String& aItemText, i16 aID, WindowItem* NextItem) :
    EditMenueItem (aItemText, aID, 0, NULL, NextItem)
{
}



void TimeItem::Load (Stream &S)
{
    EditMenueItem::Load (S);
    S >> TimeVal;
}



void TimeItem::Store (Stream &S) const
{
    EditMenueItem::Store (S);
    S << TimeVal;
}



u16 TimeItem::StreamableID () const
{
    return ID_TimeItem;
}



Streamable* TimeItem::Build ()
{
    return new TimeItem (Empty);
}



void TimeItem::SetWidth (u16 NewWidth)
{
    // Check against the minimum width needed
    if (NewWidth < MinWidth ()) {
        // Ignore the request
        return;
    }

    // Store the new width
    ItemWidth = NewWidth;

    // Build the new entry
    BuildEntry (TimeVal.TimeStr ());
}



u16 TimeItem::MinWidth ()
{
    u16 Len = ItemText.Len ();
    u16 Width = Len + TimeVal.TimeStr().Len() + 2;
    if (Len) {
        Width += 2;
    }
    return Width;
}



i16 TimeItem::Choose ()
// Choose the entry. If an edit window is defined (EditWindow != NULL),
// the entry with id EditItemID in EditWindow is called. If there was
// no abort (or EditWindow is not defined), the id of this item is
// returned.
{
    TimeEdit *P;
    int Abort;
    i16 Result;

    // If the edit id is zero, return the own id
    if (EditItemID == 0) {
        return ID;
    }

    // Check if a predefined window exists
    if (EditWindow) {

        // Move the window to the correct position
        PlaceEditWindow ();

        // Allow editing the value
        P = (TimeEdit *) EditWindow->ForcedItemWithID (EditItemID);
        P->SetValue (TimeVal);
        P->Edit (Abort);

        // if no abort, set the value and return the id
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

    } else {

        // No item defined, we have to create one
        P = new TimeEdit ("", EditItemID, NULL);

        // Insert the item into the owner window
        Owner->AddItem (P);

        // Set the position of the newly created item
        P->SetPos (ItemX + ItemWidth - P->GetWidth (), ItemY);

        // Deselect "this"
        Deselect ();

        // Transfer the values from "this" to the new item. Calling
        // SetValue will also draw the edit item
        P->SetValue (TimeVal);

        // allow editing
        P->Edit (Abort);

        // Get the edited value if the editing was not aborted
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

        // Delete the edit item from the owners list
        Owner->DeleteItem (P);

        // Select "this". This redraws the item and clears the edit field
        Select ();

    }

    // Return the result
    return Result;

}



void TimeItem::SetValue (unsigned Hour, unsigned Minute, unsigned Second)
{
    // Set the new value
    TimeVal.SetTime (Hour, Minute, Second);

    // Build the new display text.
    // Use SetWidth with the current width to do that
    SetWidth (ItemWidth);

    // Now redraw the item
    Draw ();
}



void TimeItem::SetValue (u32 Seconds)
{
    unsigned Sec  = Seconds % 60;
    Seconds /= 60;
    unsigned Min  = Seconds % 60;
    unsigned Hour = Seconds / 60;
    CHECK (Hour < 24);

    // Set the new value
    SetValue (Hour, Min, Sec);
}



void TimeItem::SetValue (const Time& Val)
{
    // Set the new value
    TimeVal = Val;

    // Build the new display text.
    // Use SetWidth with the current width to do that
    SetWidth (ItemWidth);

    // Now redraw the item
    Draw ();
}



/*****************************************************************************/
/*                              class DateItem                               */
/*****************************************************************************/




DateItem::DateItem (const String& aItemText, i16 aID, i16 EditID,
                           WindowItem* NextItem) :
    EditMenueItem (aItemText, aID, EditID, NULL, NextItem)
{
}



DateItem::DateItem (const String& aItemText, i16 aID, WindowItem* NextItem) :
    EditMenueItem (aItemText, aID, 0, NULL, NextItem)
{
}



void DateItem::Load (Stream &S)
{
    EditMenueItem::Load (S);
    S >> TimeVal;
}



void DateItem::Store (Stream &S) const
{
    EditMenueItem::Store (S);
    S << TimeVal;
}



u16 DateItem::StreamableID () const
{
    return ID_DateItem;
}



Streamable* DateItem::Build ()
{
    return new DateItem (Empty);
}



void DateItem::SetWidth (u16 NewWidth)
{
    // Check against the minimum width needed
    if (NewWidth < MinWidth ()) {
        // Ignore the request
        return;
    }

    // Store the new width
    ItemWidth = NewWidth;

    // Build the new entry
    BuildEntry (TimeVal.DateStr ());
}



u16 DateItem::MinWidth ()
{
    u16 Len = ItemText.Len ();
    u16 Width = Len + TimeVal.DateStr().Len() + 2;
    if (Len) {
        Width += 2;
    }
    return Width;
}



i16 DateItem::Choose ()
// Choose the entry. If an edit window is defined (EditWindow != NULL),
// the entry with id EditItemID in EditWindow is called. If there was
// no abort (or EditWindow is not defined), the id of this item is
// returned.
{
    DateEdit *P;
    int Abort;
    i16 Result;

    // If the edit id is zero, return the own id
    if (EditItemID == 0) {
        return ID;
    }

    // Check if a predefined window exists
    if (EditWindow) {

        // Move the window to the correct position
        PlaceEditWindow ();

        // Allow editing the value
        P = (DateEdit *) EditWindow->ForcedItemWithID (EditItemID);
        P->SetValue (TimeVal);
        P->Edit (Abort);

        // if no abort, set the value and return the id
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

    } else {

        // No item defined, we have to create one
        P = new DateEdit ("", EditItemID, NULL);

        // Insert the item into the owner window
        Owner->AddItem (P);

        // Set the position of the newly created item
        P->SetPos (ItemX + ItemWidth - P->GetWidth (), ItemY);

        // Deselect "this"
        Deselect ();

        // Transfer the values from "this" to the new item. Calling
        // SetValue will also draw the edit item
        P->SetValue (TimeVal);

        // allow editing
        P->Edit (Abort);

        // Get the edited value if the editing was not aborted
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

        // Delete the edit item from the owners list
        Owner->DeleteItem (P);

        // Select "this". This redraws the item and clears the edit field
        Select ();

    }

    // Return the result
    return Result;

}



void DateItem::SetValue (unsigned Year, unsigned Month, unsigned Day)
{
    // Set the new value
    TimeVal.SetDate (Year, Month, Day);

    // Build the new display text.
    // Use SetWidth with the current width to do that
    SetWidth (ItemWidth);

    // Now redraw the item
    Draw ();
}



void DateItem::SetValue (const Time& Val)
{
    // Set the new value
    TimeVal = Val;

    // Build the new display text.
    // Use SetWidth with the current width to do that
    SetWidth (ItemWidth);

    // Now redraw the item
    Draw ();
}



/*****************************************************************************/
/*                             class RStringItem                             */
/*****************************************************************************/



// Class RStringItem has the same functionality as class StringItem with one
// exception: The set of allowed input characters is written out on a Store
// and reloaded when calling Load. This has been a compatibility decision:
// The complete functionality to restrict input has been added to class
// StringItem (without changing the default behavior), but writing out the
// additional class would have made existing resources unusable - so there
// is a new class for that.



RStringItem::RStringItem (StreamableInit):
    StringItem (Empty)
// Build constructor
{
}



RStringItem::RStringItem (const String& aItemText, i16 aID, i16 EditID,
                          u16 aInputLength, WindowItem* NextItem):
    StringItem (aItemText, aID, EditID, NextItem),
    InputLength (aInputLength)
{
}



void RStringItem::Load (Stream& S)
{
    StringItem::Load (S);
    S >> AllowedChars >> InputLength;
}



void RStringItem::Store (Stream& S) const
{
    StringItem::Store (S);
    S << AllowedChars << InputLength;
}



u16 RStringItem::StreamableID () const
{
    return ID_RStringItem;
}



Streamable* RStringItem::Build ()
{
    return new RStringItem (Empty);
}



i16 RStringItem::Choose ()
// Choose the entry. If an edit window is defined (EditWindow != NULL),
// the entry with id EditItemID in EditWindow is called. If there was
// no abort (or EditWindow is not defined), the id of this item is
// returned.
{
    int Abort;
    i16 Result;

    // If the edit id is zero, return the own id
    if (EditItemID == 0) {
        return ID;
    }

    // Check if a predefined window exists
    if (EditWindow) {

        // Move the window to the correct position
        PlaceEditWindow ();

        // Allow editing the value
        TextEdit* P = (TextEdit*) EditWindow->ForcedItemWithID (EditItemID);
        P->SetValue (SValue);
        P->Edit (Abort);

        // if no abort, set the value and return the id
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

    } else {

        // No item defined, we have to create one
        u16 Len = ItemWidth - MinWidth () - 1;
        if (Len > InputLength + 1) {
            Len = InputLength + 1;
        }
        TextEdit* P = new TextEdit ("", EditItemID, InputLength, Len, NULL);

        // Set the allowed characters for input
        P->SetAllowedChars (AllowedChars);

        // Insert the item into the owner window
        Owner->AddItem (P);

        // Set the position of the newly created item
        P->SetPos (ItemX + ItemWidth - P->GetWidth (), ItemY);

        // Deselect "this"
        Deselect ();

        // Transfer the values from "this" to the new item. Calling
        // SetValue will also draw the edit item
        P->SetValue (SValue);

        // allow editing
        P->Edit (Abort);

        // Get the edited value if the editing was not aborted
        if (Abort) {
            Result = 0;
        } else {
            SetValue (P->GetValue ());
            Result = ID;
        }

        // Delete the edit item from the owners list
        Owner->DeleteItem (P);

        // Select "this". This redraws the item and clears the edit field
        Select ();

    }

    // Return the result
    return Result;

}



