/*****************************************************************************/
/*									     */
/*				    LISTBOX.H				     */
/*									     */
/* (C) 1993-95	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// This file contains some unnecessary global scope overrides (::) to work
// around a gcc bug (2.5.8)



#ifndef __LISTBOX_H
#define __LISTBOX_H



#include "stream.h"
#include "keydef.h"
#include "palette.h"
#include "itemwin.h"
#include "progutil.h"



/*****************************************************************************/
/*				 class ListBox				     */
/*****************************************************************************/



template <class T>
class ListBox: public WindowItem {

protected:
    Collection<T>*	Coll;		// Collection used
    Point		Size;		// Size of the listbox
    i16			First;		// First displayed entry
    i16			Selected;	// Number of selected entry
    u16			NormAttr;	// Attribute used for normal text
    u16			SelAttr;	// Attribute used for selected text
    u16			HighAttr;	// Selected text when inactive


    virtual void Up ();
    virtual void Down ();
    virtual void PgUp ();
    virtual void PgDn ();
    virtual void Home ();
    virtual void End ();
    // Handle a specific key

    void DrawSelected (u16 Attr);
    // Draw the selected entry with the given attribute

    virtual void Print (int Index, int X, int Y, u16 Attr) = 0;
    // Display one of the listbox entries

    virtual unsigned GetAttr (int Index);
    // Return the palette index for the entry with the given index

    ListBox (StreamableInit);
    // Build constructor


public:
    ListBox (const String& aItemText, i16 aID, const Point& aSize,
	     u16 aNormAttr, u16 aSelAttr, u16 aHighAttr,
	     WindowItem* NextItem);
    ListBox (const String& aItemText, i16 aID, const Point& aSize,
	     WindowItem* NextItem);
    virtual ~ListBox ();

    // Derived from class Streamable
    virtual void Store (Stream& S) const;
    virtual void Load (Stream& S);

    virtual u16 StreamableID () const;
    // Make shure, ListBox is not stored into a stream (there is no compile time
    // error without that since the base class (WindowItem) has the needed
    // functionality, but the program will crash on a load.

    // Derived from class WindowItem
    virtual void SetWidth (u16 NewWidth);
    virtual void Draw ();

    virtual void Activate ();
    virtual void Deactivate ();
    virtual void Gray ();
    virtual void Select ();
    virtual void Deselect ();


    // -- New functions

    void SetHeight (u16 NewHeight);
    // Set the height of the box

    int GetCount ();
    // Return the number of entries in Coll or zero if no collection is
    // present

    Collection<T>* GetColl ();
    // Return the collection in use

    void SetColl (Collection<T>* C);
    // Set the collection to use

    void FreeCollection ();
    // Free the given Collection, set the pointer to NULL

    T* GetSelection ();
    // Return the item with index Selected from the collection. If Selected is
    // invalid (no items), return NULL.

    T* At (int Index);
    // Returns an entry from the collection

    void Delete (int Index);
    // Delete an entry from the listbox, then redraw the box.

    void Insert (T* Item);
    // Insert a new entry into the collection and redraw the listbox

    void Replace (int Index, T* Item);
    // Replace a entry in the collection by a new one

    virtual void Reset ();
    // Zero First and Selected

    virtual void SetSelected (int NewSel);
    // Set the number of the selected entry. If the box is the active item,
    // the bar is also redrawn. If the index of the new item is out of bounds,
    // the request is ignored.

    int GetSelected ();
    // Return the number of the selected entry or -1 if the collection is
    // empty or non-existant

    void SelectedToTop ();
    // Scroll the contents of the listbox. After the operation, the highlighted
    // entry is at the top of the box.

    virtual void HandleKey (Key& K);
    // Call one of the key specific handle functions. If the key is handled,
    // K is reset to kbNoKey, otherwise K is left unchanged.

    virtual i16 Choose ();
    // Overrides WindowItem::Choose. Allow browsing in the listbox. Returns
    // the ID if a selection (with enter) is made, return 0 otherwise.

};



template <class T>
ListBox<T>::ListBox (const String& aItemText, i16 aID, const Point& aSize,
		     u16 aNormAttr, u16 aSelAttr, u16 aHighAttr,
		     WindowItem* NextItem):
    WindowItem (aItemText, aID, NextItem),
    Coll (NULL),
    Size (aSize),
    First (0),
    Selected (-1),
    NormAttr (aNormAttr),
    SelAttr (aSelAttr),
    HighAttr (aHighAttr)
{
    // Set the correct item width (work around bugs in gcc)
    unsigned XSize = Size.X;
    if (XSize > ItemWidth) {
	ItemWidth = XSize;
    }
}



template <class T>
ListBox<T>::ListBox (const String& aItemText, i16 aID, const Point& aSize,
		     WindowItem* NextItem) :
    WindowItem (aItemText, aID, NextItem),
    Coll (NULL),
    Size (aSize),
    First (0),
    Selected (-1),
    NormAttr (atTextNormal),
    SelAttr (atTextInvers),
    HighAttr (atTextHigh)
{
    // Set the correct item width (work around bugs in gcc)
    unsigned XSize = Size.X;
    if (XSize > ItemWidth) {
	ItemWidth = XSize;
    }
}



template <class T>
ListBox<T>::~ListBox ()
{
    FreeCollection ();
}



template <class T>
inline ListBox<T>::ListBox (StreamableInit):
    WindowItem (Empty)
{
}



template <class T>
void ListBox<T>::Up ()
{
    SetSelected (Selected - 1);
}



template <class T>
void ListBox<T>::Down ()
{
    SetSelected (Selected + 1);
}



template <class T>
void ListBox<T>::PgUp ()
{
    if (Selected > 0) {
	int NewSel = Selected - Size.Y + 1;
	if (NewSel < 0) {
	    NewSel = 0;
	}
	SetSelected (NewSel);
    }
}



template <class T>
void ListBox<T>::PgDn ()
{
    int Last = GetCount () - 1;
    if (Selected != Last) {
	int NewSel = Selected + Size.Y - 1;
	if (NewSel > Last) {
	    NewSel = Last;
	}
	SetSelected (NewSel);
    }
}



template <class T>
void ListBox<T>::Home ()
{
    SetSelected (0);
}



template <class T>
void ListBox<T>::End ()
{
    SetSelected (GetCount () - 1);
}



template <class T>
void ListBox<T>::DrawSelected (u16 Attr)
// Draw the selected entry with the given attribute
{
    if (Selected == -1) {
	// Nothing to do
	return;
    }
    int YPos = ItemY + (Selected - First);
    if (ItemText.Len () > 0) {
	// Add one line for the item text
	YPos++;
    }

    Print (Selected, ItemX, YPos, Attr);
}



template <class T>
void ListBox<T>::Store (Stream& S) const
{
    WindowItem::Store (S);
    S.Put (Coll);
    S << Size << First << Selected << NormAttr << SelAttr << HighAttr;
}



template <class T>
void ListBox<T>::Load (Stream& S)
{
    WindowItem::Load (S);
    Coll = (Collection<T>*) S.Get ();
    S >> Size >> First >> Selected >> NormAttr >> SelAttr >> HighAttr;
}



template <class T>
u16 ListBox<T>::StreamableID () const
// Make shure, ListBox is not stored into a stream (there is no compile time
// error without that since the base class (WindowItem) has the needed
// functionality, but the program will crash on a load.
{
    ABSTRACT ();
    return 0;
}



template <class T>
void ListBox<T>::SetWidth (u16 NewWidth)
{
    if (NewWidth >= MinWidth ()) {
	ItemWidth = NewWidth;
	Size.X	  = NewWidth;
    }
}



template <class T>
unsigned ListBox<T>::GetAttr (int Index)
// Return the palette index for the given entry
{
    if (Index < 0 || Index >= Coll->GetCount ()) {
	// No valid index
	return NormAttr;
    } else if (Index == Selected) {
	// Selected entry
	if (IsSelected ()) {
	    return SelAttr;
	} else {
	    return HighAttr;
	}
    } else {
	return NormAttr;
    }
}



template <class T>
void ListBox<T>::Activate ()
{
    WindowItem::Activate ();
    DrawSelected (GetAttr (Selected));
}



template <class T>
void ListBox<T>::Deactivate ()
{
    WindowItem::Deactivate ();
    DrawSelected (GetAttr (Selected));
}



template <class T>
void ListBox<T>::Gray ()
{
    WindowItem::Gray ();
    DrawSelected (GetAttr (Selected));
}



template <class T>
void ListBox<T>::Select ()
{
    WindowItem::Select ();
    DrawSelected (GetAttr (Selected));
}



template <class T>
void ListBox<T>::Deselect ()
{
    WindowItem::Deselect ();
    DrawSelected (GetAttr (Selected));
}



template <class T>
void ListBox<T>::Draw ()
{
    // Draw the item text
    WindowItem::Draw ();

    // Build an empty string to clear lines
    String S (Size.X);
    S.Set (0, Size.X);

    // Draw the box
    int Current = First;
    int StartY	= ItemY;
    int EndY	= StartY + Size.Y - 1;
    int Count	= Coll->GetCount ();
    if (ItemText.Len () > 0) {
	StartY++;
    }

    // Lock the owner window, accumulate output
    Owner->Lock ();

    // Display all lines of the listbox
    for (int Y = StartY; Y <= EndY; Y++, Current++) {
	if (Current < Count) {
	    // Valid entry, call Print
	    Print (Current, ItemX, Y, GetAttr (Current));
	} else {
	    // Entry is not valid, clear the line
	    Owner->Write (ItemX, Y, S, NormAttr);
	}
    }

    // Now unlock the output
    Owner->Unlock ();
}



template <class T>
void ListBox<T>::SetHeight (u16 NewHeight)
// Set the height of the box
{
    if (ItemText.Len () > 0) {
	if (NewHeight >= 2) {
	    Size.Y = NewHeight;
	}
    } else {
	if (NewHeight >= 1) {
	    Size.Y = NewHeight;
	}
    }
}



template <class T>
int ListBox<T>::GetCount ()
// Return the number of entries in Coll or zero if no collection is
// present
{
    return Coll->GetCount ();
}



template <class T>
inline Collection<T>* ListBox<T>::GetColl ()
// Return the collection in use
{
    return Coll;
}



template <class T>
void ListBox<T>::SetColl (Collection<T> *C)
// Set the collection to use
{
    // Remember new collection
    Coll = C;

    // Reset the listbox
    Reset ();
}



template <class T>
void ListBox<T>::FreeCollection ()
// Free the given Collection, set the pointer to NULL
{
    delete Coll;
    Coll = NULL;
    Reset ();
}



template <class T>
T* ListBox<T>::GetSelection ()
// Return the item with index Selected from the collection. If Selected is
// invalid (no items), return NULL.
{
    return Selected == -1? (T*) NULL : Coll->At (Selected);
}



template <class T>
inline T* ListBox<T>::At (int Index)
// Returns an entry from the collection
{
    return Coll->At (Index);
}



template <class T>
void ListBox<T>::Delete (int Index)
// Delete an entry from the listbox, then redraw the box.
{
    if (Index < 0 || Index >= GetCount ()) {
	// Index is not valid
	return;
    }

    // Index is valid, delete it
    Coll->AtDelete (Index);

    // Be carefull: this could be the last entry!
    if (Coll->GetCount () == 0) {
	// That's it, no more entries
	Selected = -1;
    } else {
	if (Index > 0) {
	    Selected--;
	}
    }

    // Redraw the listbox
    Draw ();

}



template <class T>
void ListBox<T>::Insert (T * Item)
// Insert a new entry into the collection and redraw the listbox
{
    // Insert is possible only if the collection is valid
    PRECONDITION (Coll != NULL);

    // Insert the item into the collection
    Coll->Insert (Item);

    // In any case, we have a selected entry now
    if (Selected == -1) {
	Selected = 0;
    }

    // Redraw the listbox
    Draw ();
}



template <class T>
void ListBox<T>::Replace (int Index, T * Item)
// Replace a entry in the collection by a new one
{
    // Replace is possible only if the collection is valid
    PRECONDITION (Coll != NULL);

    // The entry must be in the valid range
    PRECONDITION (Index >= 0 && Index < GetCount ());

    // Replace the item in the collection
    Coll->AtReplace (Index, Item);

    // If the index is in the visible range, we have to redraw this entry
    if (Index >= First && Index < (First + Size.Y)) {
	int Y = Index - First + ItemY;
	Print (Index, ItemX, Y, GetAttr (Index));
    }
}



template <class T>
void ListBox<T>::Reset ()
// Zero First and Selected
{
    First = 0;
    Selected = GetCount () ? 0 : -1;
}



template <class T>
void ListBox<T>::SetSelected (int NewSel)
// Set the number of the selected entry. If the box is the active item,
// the bar is also redrawn. If the index of the new item is out of bounds,
// the request is ignored.
{
    // Ignore invalid indizes
    int Count = GetCount ();
    if (NewSel >= Count || NewSel < 0) {
	return;
    }

    // Calculate the index of the last displayed entry
    int Last = First + Size.Y - 1;
    if (Last >= Count) {
	Last = Count - 1;
    }

    // Check if the new entry is inside the currently displayed area
    if (NewSel < First) {
	// The new entry is below the first displayed one
	First	 = NewSel;
	Selected = NewSel;
	Draw ();
    } else if (NewSel > Last) {
	// The new entry is above the last displayed one
	Selected = NewSel;
	First	 = NewSel - Size.Y + 1;
	Draw ();
    } else {
	// The new entry is inside the visible area
	DrawSelected (NormAttr);
	Selected = NewSel;
	DrawSelected (GetAttr (Selected));
    }
}



template <class T>
inline int ListBox<T>::GetSelected ()
// Return the number of the selected entry or -1 if the collection is
// empty or non-existant
{
    return Selected;
}



template <class T>
void ListBox<T>::SelectedToTop ()
// Scroll the contents of the listbox. After the operation, the highlighted
// entry is at the top of the box.
{
    if (Selected >= 0 && Selected != First) {
	First = Selected;
	Draw ();
    }
}



template <class T>
void ListBox<T>::HandleKey (Key &K)
// Call one of the key specific handle functions. If the key is handled,
// K is reset to kbNoKey, otherwise K is left unchanged.
{
    switch (K) {

	case vkHelp:
	    CallHelp ();
	    K = kbNoKey;
	    break;

	case vkUp:
	    Up ();
	    K = kbNoKey;
	    break;

	case vkDown:
	    Down ();
	    K = kbNoKey;
	    break;

	case vkPgUp:
	    PgUp ();
	    K = kbNoKey;
	    break;

	case vkPgDn:
	    PgDn ();
	    K = kbNoKey;
	    break;

	case vkHome:
	    Home ();
	    K = kbNoKey;
	    break;

	case vkEnd:
	    End ();
	    K = kbNoKey;
	    break;

    }
}



template <class T>
i16 ListBox<T>::Choose ()
// Overrides WindowItem::Choose. Allow browsing in the listbox. Returns
// the ID if a selection (with enter) is made, return 0 otherwise.
{
    while (1) {

	// Get a key
	Key K = ::KbdGet ();

	// Handle listbox keys
	HandleKey (K);

	// Look if something is left
	switch (K) {

	    case vkAbort:
		return 0;

	    case kbEnter:
	    case vkAccept:
		return ID;

	    default:
		if (K != AccelKey && ::KeyIsRegistered (K)) {
		    ::KbdPut (K);
		    return 0;
		}
		break;

	}
    }
}



// End of LISTBOX.H

#endif


