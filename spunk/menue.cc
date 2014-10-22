/*****************************************************************************/
/*                                                                           */
/*                                   MENUE.CC                                */
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



#ifdef __WATCOMC__
// malloc contains alloca()
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#include "keydef.h"
#include "screen.h"
#include "winattr.h"
#include "menue.h"
#include "menuitem.h"
#include "menuedit.h"
#include "national.h"
#include "streamid.h"
#include "progutil.h"
#include "winsize.h"



// Register the classes
LINK (MenueBar, ID_MenueBar);
LINK (TopMenueBar, ID_TopMenueBar);
LINK (Menue, ID_Menue);
LINK (MenueItem, ID_MenueItem);
LINK (SubMenueItem, ID_SubMenueItem);
LINK (MenueBarItem, ID_MenueBarItem);



/*****************************************************************************/
/*                              class MenueItem                              */
/*****************************************************************************/



MenueItem::MenueItem (const String& aItemText, i16 aID, WindowItem *NextItem) :
    Entry (), WindowItem (aItemText, aID, NextItem)
// Create a new menue item
{
}



void MenueItem::Store (Stream& S) const
// Store the item into a stream
{
    // Store parental data
    WindowItem::Store (S);

    // Store instance data
    S << Entry;
}



void MenueItem::Load (Stream& S)
// Load the item from a stream
{
    // Load parental data
    WindowItem::Load (S);

    // Load instance data
    S >> Entry;
}



u16 MenueItem::StreamableID () const
// Return the object ID
{
    return ID_MenueItem;
}



Streamable* MenueItem::Build ()
// Return an empty object instance
{
    return new MenueItem (Empty);
}



void MenueItem::BuildEntry (const String& S)
// Rebuild Entry from ItemText and S. The new value of Entry after
// calling BuildEntry has the form " " + ItemText + Fill + S + " ",
// where Fill is a string of spaces so that Entry gets the length
// Width. This behaviour can change in derived versions of BuildEntry
{
    int FillWidth;

    // Calculate the length of the fill string and set up such a string
    FillWidth = ItemWidth - 2 - ItemText.Len () - S.Len ();
    CHECK (FillWidth >= 0);
    String Fill (FillWidth);
    Fill.Set (0, FillWidth, ' ');

    // Now build the new entry string
    Entry = ' ' + ItemText + Fill + S + ' ';

    // Adjust memory to memory needed
    Entry.Settle ();
}



void MenueItem::SetWidth (u16 NewWidth)
{
    // Do not accept the new value if it is less than MinWidth
    if (NewWidth < MinWidth ()) {
        return;
    }

    // Remember the new width
    ItemWidth = NewWidth;

    // Change Entry according to the new width
    if (AccelKey == kbNoKey) {
        BuildEntry ("");
    } else {
        BuildEntry (GetKeyName (AccelKey));
    }
}



u16 MenueItem::MinWidth ()
{
    // Minimum width includes text width plus two spaces (left + right) plus
    // space to add the accel key name (two space + length of the key name)
    unsigned Width = ItemText.Len () + 2;
    if (AccelKey != kbNoKey) {
        Width += 2 + GetKeyName (AccelKey).Len ();
    }
    return Width;
}



void MenueItem::CallHelp ()
{
    // Allow calling help only if the item is active
    if (IsActive ()) {
        WindowItem::CallHelp ();
    }
}



void MenueItem::Draw ()
{
    unsigned    TextAttr;       // Attribute for normal text
    unsigned    HotAttr;        // Attribute for hot key

    // Bail out if the entry is empty
    if (Entry.Len () == 0) {
        return;
    }

    // Set up attributes according to the state of the item
    if (IsGrayed ()) {
        if (IsSelected ()) {
            TextAttr = atTextGrayedInvers;
            HotAttr  = atTextGrayedInvers;
        } else {
            TextAttr = atTextGrayed;
            HotAttr  = atTextGrayed;
        }
    } else if (IsSelected ()) {
        TextAttr = atTextInvers;
        HotAttr  = atTextHighInvers;
    } else {
        TextAttr = atTextNormal;
        HotAttr  = atTextHigh;
    }

    // Lock the owner window
    Owner->Lock ();

    // Write out the item text
    Owner->Write (ItemX, ItemY, Entry, TextAttr);

    // If there is a hotkey, show it
    if (HotPos >= 0) {
        Owner->Write (ItemX + HotPos + 1, ItemY, ItemText [HotPos], HotAttr);
    }

    // Unlock the owner window
    Owner->Unlock ();

}



void MenueItem::DrawItemText ()
{
    MenueItem::Draw ();
}



/*****************************************************************************/
/*                            class GenericMenue                             */
/*****************************************************************************/



GenericMenue::GenericMenue (Key aPrevKey, Key aNextKey, u16 aState,
                            WindowItem* ItemList) :
    ItemWindow (Rect (0, 0, 10, 10), aState, ItemList),
    PrevKey (aPrevKey),
    NextKey (aNextKey),
    AltPrevKey (kbNoKey),
    AltNextKey (kbNoKey),
    AbortKey (kbNoKey)
// Create a new generic menue
{
}



void GenericMenue::InitGenericMenue (const Point& Origin,
                                     const String& HeaderString)
// Special internal function that completes the initialization.
// Works together with the protected constructor and is only
// to be called from the constructors of derived classes
{
    // Set the header. This is crucial because the length of the header
    // influences the result of MinWidth used below.
    SetHeader (HeaderString);

    // Calculate the width and height of the menue
    u16 Width  = MinWidth ();
    u16 Height = MinHeight ();
    Point WinSize (Width, Height);
    if (IsFramed ()) {
        WinSize.X += 2;
        WinSize.Y += 2;
    }

    // Set the positions of the items
    SetPos ();
    SetWidth (Width);

    // Choose a selected item
    ValidateSelectedItem ();

    // Calculate the new window size/pos
    Rect Bounds (Origin, WinSize);

    // Now do a resize which will also do a complete draw of the menue
    Resize (Bounds);

    // The parent class has been called with LockCount == 1, so we
    // have to unlock the screen output here
    Unlock ();
}



void GenericMenue::Store (Stream& S) const
{
    // Store parental data
    ItemWindow::Store (S);

    // Store new data
    S << NextKey << PrevKey << AltNextKey << AltPrevKey << AbortKey;

}



void GenericMenue::Load (Stream& S)
{
    // Load parental data
    ItemWindow::Load (S);

    // Load instance data
    S >> NextKey >> PrevKey >> AltNextKey >> AltPrevKey >> AbortKey;
}



void GenericMenue::SelectItem (i16 ItemID)
// Select the entry with the given id
{
    // Deselect the currently selected entry
    if (SelectedItem) {
        SelectedItem->Deselect ();
    }

    // Get pointer to new selected entry
    SelectedItem = ForcedItemWithID (ItemID);

    // Select new entry
    SelectedItem->Select ();
}



void GenericMenue::DeselectItem (i16 ItemID)
// Deselect the entry with the given id
{
    // The item with the given ID must be the selected item
    PRECONDITION (SelectedItem && SelectedItem == ForcedItemWithID (ItemID));

    // Now deselect the item
    SelectedItem->Deselect ();
    SelectedItem = NULL;
}



void GenericMenue::SetAlternateKeys (Key aPrevKey, Key aNextKey)
// Set the keys for the ids -1 and -2
{
    AltPrevKey = aPrevKey;
    AltNextKey = aNextKey;
}



int GenericMenue::TestItem (ListNode<WindowItem>* Node, void* P)
// Helper function for GenericMenue::GetChoice
{
    return Node->Contents () == (WindowItem*) P;
}



int GenericMenue::StoreOneItem (ListNode<WindowItem>* Node, void* P)
// Helper function for GenericMenue::GetChoice. Insert all active items
// into ItemBuf
{
    // Cast the void pointer
    GenericMenue* M = (GenericMenue*) P;

    if (Node->Contents ()->IsActive ()) {
        // Item is active, insert it
        M->ItemBuf [M->ItemBufCount++] = Node->Contents ();
    }

    return 0;
}



void GenericMenue::BuildItemIndex ()
// Helper function for GenericMenue::GetChoice. Build the index from the
// sorted item list
{
    // Clear all counters and indices
    for (int I = 0; I <= MaxY (); I++) {
        ItemDesc [I].Base  = -1;
        ItemDesc [I].Count = 0;
    }

    // Now build the index
    for (unsigned J = 0; J < ItemBufCount; J++) {
        YDesc* YD = &ItemDesc [ItemBuf [J]->YPos ()];
        if (YD->Base == -1) {
            YD->Base = J;               // Write the index
        }
        YD->Count++;                    // One entry more
    }
}



unsigned GenericMenue::FindItemInItemBuf (WindowItem* P)
// Helper function for GenericMenue::GetChoice. Find an item in the item
// buffer
{
    // Get the base of all entries with the same Y position
    unsigned I = ItemDesc [P->YPos ()].Base;

    // Search from this position
    while (I < ItemBufCount) {
        if (ItemBuf [I] == P) {
            // Found the item
            return I;
        }
        I++;
    }

    // Item not found - should not happen
    FAIL ("GenericMenue::FindItemInBuf: Item not found");
    return 0;
}



// The following function is used as a parameter to qsort, but since the
// Borland C library is compiled using the _stdcall calling convention
// and on the other side, static member functions have _cdecl calling
// conventions by default, we have to redefine this one. Yuck!
// Maybe it would be better to drop support for Borland-C...

int
#if defined (OS2) && defined (__BORLANDC__)
_stdcall
#endif
GenericMenue::CompareItems (const void* I1, const void* I2)
// Helper function for GenericMenue::GetChoice. Compare two items by position
// when sorting the item list
{
    // Cast the pointers
    WindowItem* W1 = * ((WindowItem**) I1);
    WindowItem* W2 = * ((WindowItem**) I2);

    if (W1->YPos () < W2->YPos ()) {
        return -1;
    } else if (W1->YPos () == W2->YPos ()) {
        if (W1->XPos () < W2->XPos ()) {
            return -1;
        } else if (W1->XPos () == W2->XPos ()) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return 1;
    }
}



void GenericMenue::DynamicLeft ()
// Handle dynamic cursor movement to the left.
{
    // Get the index of the selected item and the base index of all items
    // with the same y position
    unsigned Count = ItemDesc [SelectedItem->YPos ()].Count;
    unsigned Base  = ItemDesc [SelectedItem->YPos ()].Base;
    unsigned I     = FindItemInItemBuf (SelectedItem);

    // If there is more than one item in the same row, select a new item
    if (Count > 1) {
        if (I > Base) {
            // item to the left
            SelectNewItem (ItemBuf [I-1]);
        } else {
            // wrap to the right
            SelectNewItem (ItemBuf [Base + Count - 1]);
        }
    }
}



void GenericMenue::DynamicRight ()
// Handle dynamic cursor movement to the right.
{
    // Get the index of the selected item and the base index of all items
    // with the same y position
    unsigned Count = ItemDesc [SelectedItem->YPos ()].Count;
    unsigned Base  = ItemDesc [SelectedItem->YPos ()].Base;
    unsigned I     = FindItemInItemBuf (SelectedItem);

    // If there is more than one item in the same row, select a new item
    if (Count > 1) {
        if (I < Base + Count - 1) {
            // item to the right
            SelectNewItem (ItemBuf [I+1]);
        } else {
            // wrap to the left
            SelectNewItem (ItemBuf [Base]);
        }
    }
}



i16 GenericMenue::GetChoice ()
{
    const AltNextKeyCode = MaxUserID + 1;
    const AltPrevKeyCode = MaxUserID + 2;


    // Remember old window state
    u16 OldState = GetState ();

    // If the left/right cursor movement is activated, build the internal
    // data structures that hold the needed information
    if (HasLRLink ()) {

        // Allocate memory
        ItemBuf  = (WindowItem**) alloca (ItemCount * sizeof (WindowItem*));
        ItemDesc = (YDesc*) alloca (IYSize () * sizeof (YDesc));

        // Store the items in the array
        ItemBufCount = 0;
        Traverse (StoreOneItem, this);

        // Sort the items
        qsort (ItemBuf, ItemBufCount, sizeof (WindowItem*), CompareItems);

        // Build the item index
        BuildItemIndex ();
    }

    // Activate and show window, highlight selected entry. Do the
    // highlighting _before_ activating the menue. Often, menues are
    // submenues, that are hidden before Activate is called. If we
    // highlight the selected item when the menue is still hidden,
    // there is no actual screen I/O, which will speed things up
    ValidateSelectedItem ();
    if (SelectedItem) {
        SelectedItem->Select ();
    }
    Activate ();

    // Reset abort key
    AbortKey = kbNoKey;

    // Letz fetz...
    i16 Choice = 0;
    Key C;
    MenueItem* Item;
    while (!Choice) {

        // Get a key
        C = KbdGet ();

        // Check for cursor movement
        if (C == PrevKey) {
            SelectPrevItem ();
        } else if (C == NextKey) {
            SelectNextItem ();
        } else if (C == AltPrevKey) {
            Choice = AltPrevKeyCode;
        } else if (C == AltNextKey) {
            Choice = AltNextKeyCode;
        } else if (HasLRLink () && C == vkLeft) {
            // Dynamic left link
            if (SelectedItem) {
                DynamicLeft ();
            }
        } else if (HasLRLink () && C == vkRight) {
            // Dynamic right link
            if (SelectedItem) {
                DynamicRight ();
            }
        } else {

            switch (C) {

                case vkHelp:
                    if (SelectedItem) {
                        SelectedItem->CallHelp ();
                    }
                    break;

                case kbEnter:
                    if (SelectedItem) {
                        Choice = SelectedItem->Choose ();
                    }
                    break;

                case vkAbort:
                    Choice = 0;
                    AbortKey = vkAbort;
                    goto ExitPoint;

                case vkResize:
                    // Allow resizing if vkResize is not an accel key inside
                    // the menue
                    Item = (MenueItem*) ItemWithAccelKey (C);
                    if (Item) {
                        // It is an accel key. Check if the menue item with
                        // this accel key is in this menue tree.
                        // If so, select it.
                        if (Traverse (TestItem, Item)) {
                            // It is an item in this menue
                            SelectNewItem (Item);
                        }
                        Choice = Item->Choose ();
                    } else {
                        // There is no item with this accel key. Allow resizing.
                        MoveResize ();
                    }
                    break;

                case vkAccept:
                    if (!IgnoreAccept ()) {
                        Choice = 0;
                        AbortKey = vkAccept;
                        goto ExitPoint;
                    }
                    break;

                default:
                    // Is the key a hotkey ?
                    Item = NULL;
                    if (IsPlainKey (C)) {
                        Item = (MenueItem*) ItemWithHotKey (NLSUpCase ((char) C));
                    }
                    if (Item) {
                        // It is a hotkey, select the entry
                        SelectNewItem (Item);
                        Choice = Item->Choose ();
                    } else {
                        // Maybe it's an accelerator key
                        Item = (MenueItem*) ItemWithAccelKey (C);
                        if (Item) {
                            // It is an accel key. Check if the menue item
                            // with this accel key is in this menue tree.
                            // If so, select it.
                            if (Traverse (TestItem, Item)) {
                                // It is an item in this menue
                                SelectNewItem (Item);
                            }
                            Choice = Item->Choose ();
                        } else {
                            // There is no item in this menue tree with
                            // the accel key. Get the root of the menue
                            // and check for the accel key in the whole
                            // menue tree. If we find the item with this
                            // key, return it back with his negative id.
                            // GetChoice knows how to handle this.
                            if (Owner) {
                                Item = (MenueItem*) Owner->GetRootWindow()->ItemWithAccelKey (C);
                                if (Item) {
                                    // Found an item with this accel key
                                    Choice = -Item->GetID ();
                                }
                            }
                        }
                    }

                    if (Item == NULL) {
                        // No accel key. If the menue is not modal, check if
                        // anyone has registered this key for other use. If
                        // the menue is modal, silently drop the key.
                        if (!IsModal () && KeyIsRegistered (C)) {
                            // Put the key back and end GetChoice
                            KbdPut (C);
                            Choice = 0;
                            AbortKey = C;
                            goto ExitPoint;
                        }
                    }
                    break;

            }   // switch

            // Now check for special choice values
            while (Choice == AltPrevKeyCode || Choice == AltNextKeyCode ||
                   (Choice < 0 && Owner == NULL)) {

                // Check if the return value is negative. This is handled by
                // the root menue
                if (Choice < 0 && Owner == NULL) {
                    // Accel key
                    Choice = -Choice;

                    // Check if the item with the accel key is in the direct
                    // descendant menues
                    Item = (MenueItem *) ForcedItemWithID (Choice);
                    if (Traverse (TestItem, Item) != NULL) {
                        // Direct descendant
                        SelectNewItem (Item);
                    }

                    // Get the choice from the item with the accel key
                    Choice = Item->Choose ();

                } else {

                    if (Choice == AltPrevKeyCode) {
                        SelectPrevItem ();
                    } else if (Choice == AltNextKeyCode) {
                        SelectNextItem ();
                    }
                    Choice = SelectedItem->Choose ();

                }

            }

        }

    }

ExitPoint:
    // Reset window to old state, then deselect the selected item. Do it
    // in this order, because often the menue is hidden after SetState,
    // so the call to Deselect will cause no additional screen output.
    SetState (OldState);
    if (SelectedItem) {
        SelectedItem->Deselect ();
    }

    // Return the result
    return Choice;
}



Key GenericMenue::Browse ()
// Overrides ItemWindow::Browse. Uses GetChoice to display the menue but
// ignores the menue choice.
{
    // Call GetChoice but ignore the result
    GetChoice ();

    // Get the abort key
    Key K = GetAbortKey ();

    // If the key is registered, there is a key waiting in the queue that
    // is not needed any longer
    if (KeyIsRegistered (K)) {
        (void) KbdGet ();
    }

    // Return the abort key
    return K;
}



Key GenericMenue::GetAbortKey ()
// Function to return the key that caused an abort in GetChoice
{
    return AbortKey;
}



i16 GenericMenue::GetSelectedItem ()
// Return the id of the currently selected item or return zero if
// no item is currently selected
{
    return SelectedItem ? SelectedItem->GetID () : 0;
}



double GenericMenue::GetFloatValue (i16 aID)
// Get the float value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_FloatItem:
            return ((FloatItem*) Item)->GetValue ();

        case ID_FloatEdit:
            return ((FloatEdit*) Item)->GetValue ();

    }

    // Error, unknown type
    FAIL ("GenericMenue::GetFloatValue: Unknown item type");
    return 0;
}



i32 GenericMenue::GetLongValue (i16 aID)
// Return the long value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_LongItem:
            return ((LongItem*) Item)->GetValue ();

        case ID_HexItem:
            return ((HexItem*) Item)->GetValue ();

        case ID_LongEdit:
            return ((LongEdit*) Item)->GetValue ();

        case ID_HexEdit:
            return ((HexEdit*) Item)->GetValue ();

    }

    // Error, unknown type
    FAIL ("GenericMenue::GetLongValue: Unknown item type");
    return 0;
}



const String& GenericMenue::GetStringValue (i16 aID)
// Return the string value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_StringItem:
            return ((StringItem*) Item)->GetValue ();

        case ID_RStringItem:
            return ((RStringItem*) Item)->GetValue ();

        case ID_EditLine:
            return ((EditLine*) Item)->GetValue ();

        case ID_TextEdit:
            return ((TextEdit*) Item)->GetValue ();

        case ID_PasswordEdit:
            return ((PasswordEdit*) Item)->GetValue ();

        case ID_FileEdit:
            return ((FileEdit*) Item)->GetValue ();

    }

    // Error, unknown type
    FAIL ("GenericMenue::GetStringValue: Unknown item type");
    return *(String*) NULL;
}



u16 GenericMenue::GetToggleValue (i16 aID)
// Return the toggle value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_ToggleItem:
            return ((ToggleItem*) Item)->GetValue ();

        case ID_OffOnItem:
            return ((OffOnItem*) Item)->GetValue ();

        case ID_NoYesItem:
            return ((NoYesItem*) Item)->GetValue ();

    }

    // Error, unknown type
    FAIL ("GenericMenue::GetToggleValue: Unknown item type");
    return 0;
}



const Time& GenericMenue::GetTimeValue (i16 aID)
// Return the time value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_TimeItem:
            return ((TimeItem*) Item)->GetValue ();

        case ID_TimeEdit:
            return ((TimeEdit*) Item)->GetValue ();

    }

    // Error, unknown type
    FAIL ("GenericMenue::GetTimeValue: Unknown item type");
    return *(Time*) NULL;
}



const Time& GenericMenue::GetDateValue (i16 aID)
// Return the date value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_DateItem:
            return ((DateItem*) Item)->GetValue ();

        case ID_DateEdit:
            return ((DateEdit*) Item)->GetValue ();

    }

    // Error, unknown type
    FAIL ("GenericMenue::GetDateValue: Unknown item type");
    return *(Time*) NULL;
}



void GenericMenue::SetFloatValue (i16 aID, double NewVal)
// Set the float value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_FloatItem:
            ((FloatItem*) Item)->SetValue (NewVal);
            break;

        case ID_FloatEdit:
            ((FloatEdit*) Item)->SetValue (NewVal);
            break;

        default:
            // Error, unknown type
            FAIL ("GenericMenue::SetFloatValue: Unknown item type");

    }

}



void GenericMenue::SetLongValue (i16 aID, i32 NewVal)
// Set the long value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_LongItem:
            ((LongItem*) Item)->SetValue (NewVal);
            break;

        case ID_HexItem:
            ((HexItem*) Item)->SetValue (NewVal);
            break;

        case ID_LongEdit:
            ((LongEdit*) Item)->SetValue (NewVal);
            break;

        case ID_HexEdit:
            ((HexEdit*) Item)->SetValue (NewVal);
            break;

        default:
            // Error, unknown type
            FAIL ("GenericMenue::SetLongValue: Unknown item type");

    }

}



void GenericMenue::SetStringValue (i16 aID, const String & NewVal)
// Set the string value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_StringItem:
            ((StringItem*) Item)->SetValue (NewVal);
            break;

        case ID_RStringItem:
            ((RStringItem*) Item)->SetValue (NewVal);
            break;

        case ID_EditLine:
            ((EditLine*) Item)->SetValue (NewVal);
            break;

        case ID_TextEdit:
            ((TextEdit*) Item)->SetValue (NewVal);
            break;

        case ID_PasswordEdit:
            ((PasswordEdit*) Item)->SetValue (NewVal);
            break;

        case ID_FileEdit:
            ((FileEdit*) Item)->SetValue (NewVal);
            break;

        default:
            // Error, unknown type
            FAIL ("GenericMenue::SetStringValue: Unknown item type");

    }

}



void GenericMenue::SetToggleValue (i16 aID, u16 NewVal)
// Set the toggle value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_ToggleItem:
            ((ToggleItem*) Item)->SetValue (NewVal);
            break;

        case ID_OffOnItem:
            ((OffOnItem*) Item)->SetValue (NewVal);
            break;

        case ID_NoYesItem:
            ((NoYesItem*) Item)->SetValue (NewVal);
            break;

        default:
            // Error, unknown type
            FAIL ("GenericMenue::SetToggleValue: Unknown item type");

    }

}



void GenericMenue::SetTimeValue (i16 aID, const Time& NewVal)
// Set the time value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_TimeItem:
            ((TimeItem*) Item)->SetValue (NewVal);
            break;

        case ID_TimeEdit:
            ((TimeEdit*) Item)->SetValue (NewVal);
            break;

        default:
            // Error, unknown type
            FAIL ("GenericMenue::SetTimeValue: Unknown item type");

    }

}



void GenericMenue::SetTimeValue (i16 aID, u32 NewVal)
// Set the time value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_TimeItem:
            ((TimeItem*) Item)->SetValue (NewVal);
            break;

        case ID_TimeEdit:
            ((TimeEdit*) Item)->SetValue (NewVal);
            break;

        default:
            // Error, unknown type
            FAIL ("GenericMenue::SetTimeValue: Unknown item type");

    }

}



void GenericMenue::SetDateValue (i16 aID, const Time& NewVal)
// Set the date value of an item.
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_DateItem:
            ((DateItem*) Item)->SetValue (NewVal);
            break;

        case ID_DateEdit:
            ((DateEdit*) Item)->SetValue (NewVal);
            break;

        default:
            // Error, unknown type
            FAIL ("GenericMenue::SetDateValue: Unknown item type");

    }

}



/*****************************************************************************/
/*                              class MenueBar                               */
/*****************************************************************************/



MenueBar::MenueBar (const Point& Origin, WindowItem* ItemList) :
    GenericMenue (vkLeft, vkRight, 0, ItemList)
{
    InitGenericMenue (Origin, "");
}



int MenueBar::SetOnePos (ListNode<WindowItem>* Node, void* P)
// Helper function for MenueBar::SetPos, sets the position of one item
{
    // Get a pointer to the window item
    WindowItem* Item = Node->Contents ();

    // Set the position of the window item
    Item->SetPos (*((u16*) P), 0);

    // Calculate the position of the next item
    * ((u16*) P) += Item->MinWidth ();

    // Keep on traversing
    return 0;
}



void MenueBar::SetPos ()
{
    // First item at position one
    u16 X = 1;

    // Set up all item positions
    Traverse (SetOnePos, &X);
}



u16 MenueBar::StreamableID () const
{
    return ID_MenueBar;
}



Streamable* MenueBar::Build ()
{
    return new MenueBar (Empty);
}



u16 MenueBar::MinHeight ()
{
    return 1;
}



static int AddWidth (ListNode<WindowItem>* Node, void* P)
// Helper function for MenueBar::MinWidth
{
    *((u16*) P) += Node->Contents () -> MinWidth ();
    return 0;
}



u16 MenueBar::MinWidth ()
{
    // The width of the menuebar is the width of all entrys plus a space
    // to the left and right
    u16 W = 2;

    Traverse (AddWidth, &W);

    return W;
}



static int MenueBar_SetOneWidth (ListNode<WindowItem>* Node, void*)
// Helper function for SetWidth
{
    // Get a pointer to the item
    WindowItem* Item = Node->Contents ();

    // Set the width of the entry to the minimum width of that entry
    Item->SetWidth (Item->MinWidth ());

    // Keep on traversing
    return 0;
}



void MenueBar::SetWidth (u16)
{
    Traverse (MenueBar_SetOneWidth);
}



/*****************************************************************************/
/*                             class TopMenueBar                             */
/*****************************************************************************/



TopMenueBar::TopMenueBar (WindowItem* ItemList) :
        MenueBar (Point (0, 0), ItemList)
{
    // Expand the size of the bar from the left to the right screen border
    Resize (Rect (0, 0, TheScreen->GetXSize (), 1));
}



u16 TopMenueBar::StreamableID () const
{
    return ID_TopMenueBar;
}



Streamable* TopMenueBar::Build ()
{
    return new TopMenueBar (Empty);
}



void TopMenueBar::ScreenSizeChanged (const Rect& NewScreen)
// Called when the screen got another resolution. NewScreen is the new
// screen size.
{
    // Expand the size of the bar from the left to the right screen border
    Resize (Rect (0, 0, NewScreen.XSize (), 1));
}



void TopMenueBar::MoveResizeAfterLoad (const Point& /*OldRes*/)
// This function is called after a load when most of the window is
// constructed. It is used to move the window to a new position if this is
// needed. OldRes is the old screen resolution that was active, when the
// window was stored.
// It is used to position the menuebar at the top of the screen after a
// load.
{
    Rect NewBounds = Background->OuterBounds ();
    NewBounds.B.Y = 1;
    Resize (NewBounds);
}



/*****************************************************************************/
/*                                class Menue                                */
/*****************************************************************************/



Menue::Menue (const Point& Origin, const String& HeaderString, WindowItem* ItemList):
    GenericMenue (vkUp, vkDown, wfFramed, ItemList)
{
    InitGenericMenue (Origin, HeaderString);
}



u16 Menue::StreamableID () const
{
    return ID_Menue;
}



Streamable* Menue::Build ()
{
    return new Menue (Empty);
}



u16 Menue::MinHeight ()
{
    return ItemCount;
}



static int CompareWidth (ListNode<WindowItem>* Node, void* P)
// Helper function for Menue::MinWidth
{
    u16* W = (u16 *) P;
    u16 MinWidth = Node->Contents () -> MinWidth ();

    // Put the greater one into W
    if (*W < MinWidth) {
        *W = MinWidth;
    }

    // Keep on traversing
    return 0;
}



u16 Menue::MinWidth ()
{
    // Preset the minimum width with the length of the header string
    u16 W = Header.Len () + 2;

    // Now search for a wider item
    Traverse (CompareWidth, &W);

    // And return the result
    return W;
}



static int Menue_SetOneWidth (ListNode<WindowItem>* Node, void* P)
// Helper function for Menue::SetWidth
{
    // Set the width to the given width
    Node->Contents () -> SetWidth (* ((u16 *) P));

    // keep on traversing
    return 0;
}



void Menue::SetWidth (u16 NewWidth)
{
    Traverse (Menue_SetOneWidth, &NewWidth);
}



/*****************************************************************************/
/*                            class SubMenueItem                             */
/*****************************************************************************/



SubMenueItem::SubMenueItem (const String& aItemText, i16 aID,
                            WindowItem* MenueList, WindowItem* NextItem) :
    MenueItem (aItemText, aID, NextItem),
    SubMenue (NULL)
{
    if (MenueList != NULL) {

        // Build the submenue at 0/0
        Point Origin (0, 0);
        SubMenue = new Menue (Origin, "", MenueList);

        // Set this item as the owner of the submenue
        SubMenue->SetOwner (this);

        // Set the alternate keys
        SubMenue->SetAlternateKeys (vkUp, vkDown);
    }
}



SubMenueItem::~SubMenueItem ()
{
    // Delete the submenue
    delete SubMenue;
}



void SubMenueItem::Store (Stream& S) const
// Store the object into a stream
{
    // Store parental data
    MenueItem::Store (S);

    // Put the submenue into the stream
    S.Put (SubMenue);
}



void SubMenueItem::Load (Stream& S)
// Load an object from a stream
{
    // Load parental data
    MenueItem::Load (S);

    // Construct a complete submenue from the data in the stream
    SubMenue = (Menue*) S.Get ();
}



u16 SubMenueItem::StreamableID () const
{
    return ID_SubMenueItem;
}



Streamable* SubMenueItem::Build ()
{
    return new SubMenueItem (Empty);
}



i16 SubMenueItem::Choose ()
{
    // Return "Abort" if the entry is not active
    if (!IsActive ()) {
        return 0;
    }

    // If the submenue is non existant, or the ifNoSub flag is set,
    // return the ID of the item itself
    if (SubMenue == NULL || Flags & ifNoSub) {
        return ID;
    }

    // Place the submenue below or above the SubMenueItem
    SubMenue->PlaceNear (this);

    // Get the choice from the submenue. This will activate the window and
    // restore the original (hidden) state afterwards.
    return SubMenue->GetChoice ();
}



WindowItem* SubMenueItem::ItemWithID (i16 aID)
{
    if (aID == ID) {
        // The instance itself
        return this;
    } else {
        // Ask the submenue if one exists
        return SubMenue? SubMenue->ItemWithID (aID) : 0;
    }
}



WindowItem* SubMenueItem::ItemWithAccelKey (Key aAccelKey)
{
    if (!IsActive ()) {
        // Not active, ignore request
        return NULL;
    }

    if (aAccelKey == AccelKey) {
        // The instance itself
        return this;
    } else {
        // Ask the submenue if one exists
        return SubMenue? SubMenue->ItemWithAccelKey (aAccelKey) : 0;
    }
}



u16 SubMenueItem::MinWidth ()
{
    // space + Length + space + rightarrow + space
    return ItemText.Len () + 4;
}



void SubMenueItem::SetWidth (u16 NewWidth)
{
    if (NewWidth < MinWidth ()) {
        // Ignore the request
        return;
    }

    // Remember new value
    ItemWidth = NewWidth;

    // Build the new display string
    static char S [3] = { ' ', RightTriangle, '\0' };
    BuildEntry (String (S));
}



void SubMenueItem::SetSubMenue (Menue* NewSubMenue)
{
    // Delete the old submenue, then set the new one
    delete SubMenue;
    SubMenue = NewSubMenue;

    // Set owner and keys for the new submenue
    if (SubMenue != NULL) {
        // Set this item as the owner of the submenue
        SubMenue->SetOwner (this);

        // Set the alternate keys
        SubMenue->SetAlternateKeys (vkUp, vkDown);
    }
}



/*****************************************************************************/
/*                            class MenueBarItem                             */
/*****************************************************************************/



MenueBarItem::MenueBarItem (const String& aItemText, i16 aID,
                            WindowItem* MenueList,
                            WindowItem* NextItem) :
        SubMenueItem (aItemText, aID, MenueList, NextItem)
{
    // The constructor of SubMenueItem has set the wrong alternate keys,
    // correct this
    if (SubMenue) {
        SubMenue->SetAlternateKeys (vkLeft, vkRight);
    }

    // The MenueBarItem has the Meta-version of the hotkey as a
    // default accelerator key
    AccelKey = GetMetaCode (HotKey);
}



u16 MenueBarItem::StreamableID () const
{
    return ID_MenueBarItem;
}



Streamable* MenueBarItem::Build ()
{
    return new MenueBarItem (Empty);
}



void MenueBarItem::SetWidth (u16 NewWidth)
{
    // Do not accept the new value if it is less than MinWidth
    if (NewWidth < MinWidth ()) {
        return;
    }

    // Remember the new width
    ItemWidth = NewWidth;

    // Change Entry according to the new width
    BuildEntry ("");
}



u16 MenueBarItem::MinWidth ()
{
    // Minimum width includes text width plus two spaces (left + right).
    return ItemText.Len () + 2;
}



void MenueBarItem::SetSubMenue (Menue* NewSubMenue)
{
    // Use the derived function, then reset the alternate keys
    SubMenueItem::SetSubMenue (NewSubMenue);

    if (SubMenue) {
        // Set the alternate keys
        SubMenue->SetAlternateKeys (vkLeft, vkRight);
    }
}



