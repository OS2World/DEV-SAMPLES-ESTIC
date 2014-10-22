/*****************************************************************************/
/*                                                                           */
/*                                  ITEMWIN.CC                               */
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



#include "itemwin.h"
#include "national.h"
#include "streamid.h"
#include "thread.h"
#include "program.h"
#include "progutil.h"



// Register class WindowItem and ItemWindow
LINK(ItemWindow, ID_ItemWindow);
LINK(WindowItem, ID_WindowItem);



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class ListNode<WindowItem>;
#endif



/*****************************************************************************/
/*                             class WindowItem                              */
/*****************************************************************************/



WindowItem::WindowItem (const String &aItemText, i16 aID, WindowItem *NextItem) :
        INode (this), ItemX (0), ItemY (0), ID (aID), Flags (0),
        AccelKey (kbNoKey), ItemText (aItemText)
{
    // Check the given parameters
    PRECONDITION (aID <= MaxUserID && aID > 0);

    // Check the item text for a hot key
    HotPos = (i16) ItemText.Pos ('@');
    if (HotPos >= 0) {
        // There is a hot key, delete the marker and grab it
        ItemText.Del (HotPos);
        HotKey = (Key) NLSUpCase (ItemText [HotPos]);
    } else {
        // No hotkey
        HotKey = kbNoKey;
    }

    // Init some other stuff
    AccelKey = kbNoKey;
    SetPos (0, 0);
    ItemWidth = MinWidth ();

    // If the next item is given, make up the list
    if (NextItem) {
        INode.InsertBefore (&NextItem->INode);
    }

}



u16 WindowItem::StreamableID () const
{
    return ID_WindowItem;
}



Streamable* WindowItem::Build ()
{
    return new WindowItem (Empty);
}



void WindowItem::Store (Stream& S) const
// Write instance data to the stream
{
    S << ItemX << ItemY << ItemWidth << ID << Flags << HotKey << HotPos
      << AccelKey << ItemText << HelpKey;
}



void WindowItem::Load (Stream &S)
// Load instance data from the stream
{
    S >> ItemX >> ItemY >> ItemWidth >> ID >> Flags >> HotKey >> HotPos
      >> AccelKey >> ItemText >> HelpKey;
}



void WindowItem::RegisterKey ()
// If the item is active and has an accel key: Register the key at the
// current thread.
{
    if (IsActive () && AccelKey != kbNoKey) {
        CurThread () -> RegisterKey (AccelKey);
    }
}



void WindowItem::UnregisterKey ()
// If the item is active and has an accel key: Unregister the key at the
// current thread.
{
    if (IsActive () && AccelKey != kbNoKey) {
        CurThread () -> UnregisterKey (AccelKey);
    }
}



void WindowItem::SetPos (u16 X, u16 Y)
{
    ItemX = X;
    ItemY = Y;
}



void WindowItem::SetWidth (u16 NewWidth)
{
    ItemWidth = NewWidth;
}



ItemWindow* WindowItem::GetRootWindow ()
// Get the root of all windows in the current chain
{
    ItemWindow *Win = Owner;
    while (Win->GetOwner ()) {
        Win = Win->GetOwner () -> GetOwner ();
    }
    CHECK (Win != NULL);

    return Win;
}



void WindowItem::SelectNew (WindowItem* NewItem)
// Deselect the selected item and set NewItem as newly selected item.
// Checks if this == NewItem and ignores a request in this case
{
    // This item must be selected !
    PRECONDITION (IsSelected ());

    // No action if the new item and this item are the same
    if (NewItem != this) {
        Deselect ();            // Deselect this item
        NewItem->Select ();     // Select other item
    }
}




WindowItem* WindowItem::SelectNext ()
// Select the next item in the list. Return the item that is selected
// after the operation has been performed.
{
    // Get a pointer to the node of the item list
    ListNode<WindowItem>* Node = &INode;
    WindowItem* Item;

    // Search the item list forward for the next matching item
    do {
        Node = Node->Next ();
        Item = Node->Contents ();
    } while (!Item->IsActive ());

    // Select the new item, deselect current
    SelectNew (Item);

    // Return the new selected item
    return Item;
}



WindowItem* WindowItem::SelectPrev ()
// Select the previous item in the list. Return the item that is selected
// after the operation has been performed.
{
    // Get a pointer to the node of the item list
    ListNode<WindowItem>* Node = &INode;
    WindowItem* Item;

    // Search the item list forward for the next matching item
    do {
        Node = Node->Prev ();
        Item = Node->Contents ();
    } while (!Item->IsActive ());

    // Select the new item, deselect current
    SelectNew (Item);

    // Return the new selected item
    return Item;
}



u16 WindowItem::GetWidth () const
{
    return ItemWidth;
}



u16 WindowItem::MinWidth ()
// Return the minimal needed width of the item
{
    return ItemText.Len ();
}



i16 WindowItem::GetID ()
// Return the item id
{
    return ID;
}



void WindowItem::SetItemText (const String& aItemText)
{
    // Set new string
    ItemText = aItemText;

    // Adjust memory needed for the string
    ItemText.Settle ();

    // Reset width
    SetWidth (GetWidth ());

    // Redraw the item
    Draw ();
}



void WindowItem::Draw ()
{
    unsigned    TextAttr;       // Attribute for normal text
    unsigned    HotAttr;        // Attribute for hot key

    // If the item text is empty, bail out early
    if (ItemText.IsEmpty ()) {
        // Nothing to do
        return;
    }

    // Set up attributes
    if (IsActive ()) {
        HotAttr  = atTextHigh;
        TextAttr = IsSelected () ? atTextSelected : atTextNormal;
    } else {
        if (IsGrayed ()) {
            HotAttr  = atTextGrayed;
            TextAttr = atTextGrayed;
        } else {
            HotAttr  = atTextNormal;
            TextAttr = atTextNormal;
        }
    }

    // Lock the owner window
    Owner->Lock ();

    // Write out the item text
    Owner->Write (ItemX, ItemY, ItemText, TextAttr);

    // If there is a hotkey, show it
    if (HotPos >= 0) {
        Owner->Write (ItemX + HotPos, ItemY, ItemText [HotPos], HotAttr);
    }

    // Unlock the owner window
    Owner->Unlock ();

}



void WindowItem::Clear ()
{
    if (ItemWidth) {
        String S (ItemWidth);
        S.Set (0, ItemWidth, ' ');
        Owner->Write (ItemX, ItemY, S);
    }
}



void WindowItem::DrawItemText ()
{
    WindowItem::Draw ();
}



void WindowItem::ClearItemText ()
{
    unsigned Len = ItemText.Len ();
    if (Len) {
        String S (Len);
        S.Set (0, Len, ' ');
        Owner->Write (ItemX, ItemY, S);
    }
}



void WindowItem::Activate ()
{
    // Clear the gray and inactive attributes
    Flags &= ~(ifGrayed | ifInactive);

    // Redraw the item
    DrawItemText ();
}



void WindowItem::Deactivate ()
{
    // Set new state
    Flags |= ifInactive;
    Flags &= ~ifSelected;

    // Redraw the item
    DrawItemText ();
}



void WindowItem::Gray ()
{
    // Set new state
    Flags |= ifGrayed | ifInactive;
    Flags &= ~ifSelected;

    // Redraw the item
    DrawItemText ();
}



void WindowItem::Select ()
{
    // Set new state
    Flags |= ifSelected;

    // Redraw the item
    DrawItemText ();
}



void WindowItem::Deselect ()
{
    // Set new state
    Flags &= ~ifSelected;

    // Redraw the item
    DrawItemText ();
}



void WindowItem::CallHelp ()
// Call the help function with the help key of this item
{
    if (HasHelp ()) {
        App->CallHelp (HelpKey);
    }
}



i16 WindowItem::Choose ()
// Choose an entry
{
    // Item must be active
    PRECONDITION (IsActive ());

    // Return entry id
    return ID;
}



WindowItem* WindowItem::ItemWithID (i16 aID)
//
{
    return (aID == ID) ? this : (WindowItem*) NULL;
}



WindowItem* WindowItem::ItemWithHotKey (Key aHotKey)
{
    return (IsActive () && aHotKey == HotKey) ? this : (WindowItem*) NULL;
}



WindowItem* WindowItem::ItemWithAccelKey (Key aAccelKey)
{
    return (IsActive () && aAccelKey == AccelKey) ? this : (WindowItem*) NULL;
}



/*****************************************************************************/
/*                             class ItemWindow                              */
/*****************************************************************************/



ItemWindow::ItemWindow (const Rect& Bounds, u16 aState, WindowItem* ItemList):
    Window (Bounds, aState, paGray, 0, 1),
    Owner (NULL),
    SelectedItem (NULL),
    FirstItem (ItemList)
// Internal constructor, used by GenericMenue
{
    // Set up the item count
    ItemCount = (u16) (FirstItem ? FirstItem->INode.NodeCount () : 0);

    // Set this as the owner of all items in ItemList
    SetItemListOwner ();
}



ItemWindow::ItemWindow (const Rect& Bounds, u16 aState, unsigned aPalette,
            unsigned Number, WindowItem* ItemList):
    Window (Bounds, aState, aPalette, Number, 1),
    Owner (NULL),
    SelectedItem (NULL),
    FirstItem (ItemList)
{
    // Set up the item count
    ItemCount = FirstItem ? FirstItem->INode.NodeCount () : 0;

    // Set this as the owner of all items in ItemList
    SetItemListOwner ();

    // Set the positions of the items
    SetPos ();

    // Choose a selected item
    ValidateSelectedItem ();

    // Draw all items
    DrawItems ();

    // The parent class has been called with LockCount == 1, so we
    // have to unlock the screen output here
    Unlock ();
}



ItemWindow::~ItemWindow ()
// Destructor of class ItemWindow
{
    // Delete the list of items
    if (FirstItem) {

        ListNode<WindowItem>* N = &FirstItem->INode;
        ListNode<WindowItem>* P;

        while (!N->IsEmpty ()) {
            P = N->Next ();
            P->Unlink ();
            delete P->Contents ();
        }
        delete N->Contents ();

    }
}



u16 ItemWindow::StreamableID () const
{
    return ID_ItemWindow;
}



Streamable* ItemWindow::Build ()
{
    return new ItemWindow (Empty);
}



int ItemWindow::StoreOneItem (ListNode<WindowItem>* N, void* S)
// Helper function to store a WindowItem into the stream S
{
    // Get a casted pointer to the stream
    Stream *Str = (Stream *) S;

    // Store the WindowItem
    Str->Put (N->Contents ());

    // keep on traversing...
    return 0;
}



void ItemWindow::Store (Stream& S) const
// Store the object into a stream
{
    // Store parental data
    Window::Store (S);

    // Store the node number of the selected item. Note: If FirstItem is
    // NULL, SelectedItem cannot be != NULL
    i16 Selected;
    if (SelectedItem) {
        CHECK (FirstItem != NULL);
        Selected = (i16) FirstItem->INode.NumberOfNode (&SelectedItem->INode);
    } else {
        Selected = -1;
    }

    // Store the item count and each item
    S << Selected << ItemCount;
    Traverse (StoreOneItem, &S);
}



int ItemWindow::SetOneItemWidth (ListNode<WindowItem>* N, void*)
// Reset the width of one item
{
    // Get a pointer to the item
    WindowItem* W = N->Contents ();

    // Reset the width
    W->SetWidth (W->GetWidth ());

    // keep on traversing...
    return 0;
}



void ItemWindow::Load (Stream& S)
// Load the window from the stream
{
    i16 Selected;

    // Clear ItemCount and most of the pointers before calling the parental
    // Load function. This is because Window::Load is calling Redraw, which
    // tries to draw not existing items if the data is not initialized.
    ItemCount    = 0;
    Owner        = NULL;
    FirstItem    = NULL;
    SelectedItem = NULL;

    // Now load the window data
    Window::Load (S);

    // Read the number of the selected item and the item count
    S >> Selected >> ItemCount;

    // Read in all items and put them into the linked list
    WindowItem *P;
    for (int I = 0; I < ItemCount; I++) {
        // Load the item from the stream
        P = (WindowItem *) S.Get ();

        // Insert the item into the item list
        if (FirstItem) {
            P->INode.InsertBefore (&FirstItem->INode);
        } else {
            FirstItem = P;
        }
    }

    // Set "this" as the owner of all the items in the list
    SetItemListOwner ();

    // Set the pointer to the selected item
    if (Selected == -1) {
        // No selected item
        SelectedItem = NULL;
    } else {
        CHECK (FirstItem != NULL);
        SelectedItem = FirstItem->INode.NodeWithNumber ((u16) Selected)->Contents ();
    }

    // Before redrawing the items, adjust for a changed size of the window and
    // changed language. Reset the width of all items.
    Traverse (SetOneItemWidth);

    // We have to redraw the items because Window::Redraw has not done this.
    // DrawItems is doing a lock, so we don't have to do this here
    DrawItems ();

}



void ItemWindow::DrawInterior ()
// Redraw the inner part of the window
{
    // Clear the inner window, the redraw the items
    Lock ();
    Clear ();
    DrawItems ();
    Unlock ();
}



void ItemWindow::Activate ()
// Activate the window. This overrides Window::Activate and highlights
// the selected item
{
    // Activate the window
    Window::Activate ();

    // If there is a selected item, highlight it
    if (SelectedItem) {
        SelectedItem->Select ();
    }
}



void ItemWindow::Deactivate ()
// Activate the window. This overrides Window::Activate and deselects
// the selected item
{
    // Deactivate the window
    Window::Deactivate ();

    // If there is a selected item, deselect it
    if (SelectedItem) {
        SelectedItem->Deselect ();
    }
}



Key ItemWindow::Browse ()
// Make the window active and display the window contents in a suitable
// manner. This function should be overridden for special derived
// windows. It returns the key that ended the browse state.
{
    // Remember the state of the window and the cursor form
    u16 OldState = GetState ();
    CursorType Cursor = GetCursor ();

    // new StatusLine
    PushStatusLine (CreateStatusLine (GetStatusFlags ()));

    // Activate the window
    Activate ();

    // Get all keys until abort or a registered key is read
    Key K;
    int Done = 0;
    while (!Done) {

        // Get a key
        K = KbdGet ();

        // Handle the key
        HandleKey (K);

        // Check for some other keys...
        switch (K) {

            case vkAbort:
                Done = 1;
                break;

            default:
                if (KeyIsRegistered (K)) {
                    Done = 1;
                }
                break;
        }

    }

    // Restore the old status line
    PopStatusLine ();

    // Reset window to old state, then deselect the selected item. Do it
    // in this order, because often the menue is hidden after SetState,
    // so the call to Deselect will cause no additional screen output.
    SetState (OldState);
    if (SelectedItem) {
        SelectedItem->Deselect ();
    }

    // Set the old cursor
    SetCursor (Cursor);

    // Return the abort key
    return K;
}



u32 ItemWindow::GetStatusFlags ()
// Returns the flags that are used to build the status line in Browse
{
    u32 Flags = siEnd;
    if (CanMove () || CanResize ()) {
        Flags |= siResize;
    }
    return Flags;
}



void ItemWindow::HandleKey (Key& K)
// Key dispatcher used in Browse
{
    // Handle some known keys
    switch (K) {

        case vkResize:
            MoveResize ();
            K = kbNoKey;
            break;

    }
}



void ItemWindow::AddItem (WindowItem* Item)
// Add the given item to the item list of the window. After that, the
// item is owned by the window and destroyed if the window destructor
// is called. The given WindowItem is unlinked from the list it is
// linked in before it is added to the windows item list.
{
    // unlink the item from the list it is currently in
    Item->INode.Unlink ();

    // Insert the item into the list managed by the window
    if (FirstItem) {
        Item->INode.InsertBefore (&FirstItem->INode);
    } else {
        FirstItem = Item;
    }

    // Set the owner of the item
    Item->SetOwner (this);

    // Track the count of items in the list
    ItemCount++;

    // Validate the selected item
    ValidateSelectedItem ();
}



void ItemWindow::DeleteItem (WindowItem* Item)
// Take the item from the item list and delete it. If Item is the
// selected item, a new selected item is choosen. After deleting
// Item, the window is redrawn.
{
    ListNode<WindowItem> *N;

    if (FirstItem) {
        // List is not empty
        N = &FirstItem->INode;

        do {
            if (N->Contents () == Item) {

                // Found the item, clear, then unlink it. Be carefull:
                // Item could be the item, FirstItem is pointing to!
                N->Contents () -> Clear ();
                if (Item == FirstItem) {
                    // Take the next item if possible
                    if (ItemCount > 1) {
                        FirstItem = N->Next () -> Contents ();
                    } else {
                        // This is the last item, the list is empty
                        FirstItem = NULL;
                    }
                }
                N->Unlink ();

                // Keep track of the item count
                ItemCount--;

                // If Item was selected, choose a new selected item
                if (Item == SelectedItem) {
                    SelectedItem = NULL;        // Invalidate item
                    ValidateSelectedItem ();    // Choose a new one
                }

                // Now delete the item and exit
                delete Item;
                return;

            } else {

                // Not this item, try the next
                N = N->Next ();

            }

        // until the first item is reached again
        } while (Item != FirstItem);

    }

    // OOPS! There is no such item in the item list.
    FAIL ("ItemWindow::DeleteItem: Item not found");

}



int ItemWindow::SetOneOwner (ListNode<WindowItem>* N, void* P)
// Helper function for ItemWindow::SetItemListOwner. Sets the owner window
// for all items in the list
{
    N->Contents () -> SetOwner ((ItemWindow *) P);
    return 0;
}



void ItemWindow::SetItemListOwner ()
// Traverse through all items and tell them their owner
{
    Traverse (SetOneOwner, (void*) this);
}



int ItemWindow::SetOnePos (ListNode<WindowItem>* N, void* P)
// Helper function for ItemWindow::SetPos, sets the position of one item
{
    // Get a casted pointer to the given Y position
    u16* Pos = (u16*) P;

    // Set the position for one item
    N->Contents () -> SetPos (0, *Pos);

    // Increment the Y position
    (*Pos)++;

    // Keep on traversing
    return 0;
}



void ItemWindow::SetPos ()
// Sets the positions of all window items. The items are lined up
// vertically at the left window border. To change this behaviour,
// overload this function.
{
    // Position of first item is 0/0
    u16 Y = 0;

    Traverse (SetOnePos, &Y);
}



int ItemWindow::DrawOneItem (ListNode<WindowItem>* N, void*)
// Helper function for DrawItems, draw on item
{
    // Draw the given item
    N->Contents () -> Draw ();

    // Keep on traversing
    return 0;
}



void ItemWindow::DrawItems ()
// Draw all items
{
    if (FirstItem) {
        // Use Lock/Unlock to speed up display processing
        Lock ();
        Traverse (DrawOneItem);
        Unlock ();
    }
}



int ItemWindow::CheckSelectedItem (ListNode<WindowItem>* N, void*)
// Helper function for ValidateSelectedItem, find active item
{
    return N->Contents () -> IsActive ();
}



void ItemWindow::ValidateSelectedItem ()
// Check if the item pointed to by SelectedItem is still active. If
// not, a new item is choosen.
{
    if (FirstItem == NULL) {
        // No items
        SelectedItem = NULL;
        return;
    }

    if (SelectedItem == NULL || SelectedItem->IsActive () == 0) {
        // Selected item is invalid, choose a new one
        SelectedItem = Traverse (CheckSelectedItem);
    }

    // If a selected item exists and the window is active, show the new state
    if (SelectedItem && IsActive ()) {
        SelectedItem->Select ();
    }
}



void ItemWindow::SelectNextItem ()
// Choose the next item in the item list as selected
{
    if (SelectedItem) {
        SelectedItem = SelectedItem->SelectNext ();
    }
}



void ItemWindow::SelectPrevItem ()
// Choose the previous item in the item list as selected
{
    if (SelectedItem) {
        SelectedItem = SelectedItem->SelectPrev ();
    }
}



void ItemWindow::SelectNewItem (WindowItem* NewItem)
// Select the new item, update SelectedItem
{
    if (SelectedItem) {
        SelectedItem->SelectNew (NewItem);
    } else {
        NewItem->Select ();
    }
    SelectedItem = NewItem;
}



void ItemWindow::SelectNewItem (i16 NewID)
// Select the new item, update SelectedItem
{
    SelectNewItem (ForcedItemWithID (NewID));
}



WindowItem* ItemWindow::Traverse (int (*F) (ListNode<WindowItem>*, void*),
                                   void* Data) const
// Use ListNode::Traverse to traverse through all window items starting
// at FirstItem. Return value corresponds to ListNode::Traverse
{
    ListNode<WindowItem> *Node;

    if (FirstItem) {
        Node = FirstItem->INode.Traverse (1, F, Data);
    } else {
        Node = NULL;
    }

    // if Node is not NULL, return the item that Node contains
    return Node ? Node->Contents () : (WindowItem*) NULL;
}



void ItemWindow::PlaceNear (const Point& Pos)
// Place the window near the given absolute position. If there is not enough
// room to place the window below the given item, the window is placed above.
{
    // Get the coords of the window and the screen
    Rect WinBounds (OuterBounds ());
    Rect ScreenBounds (Background->OuterBounds ());

    // Now move the window bounds so that the window is positioned
    // below the given item.
    WinBounds.Move (-WinBounds.A.X, -WinBounds.A.Y);            // Move to 0/0
    WinBounds.Move (Pos.X, Pos.Y+1);

    // Check if the Y position is inside the screen area
    if (WinBounds.B.Y > ScreenBounds.B.Y) {
        // This did not work. Parts of the window are outside the
        // screen area. Move the window to a position above the item
        WinBounds.Move (0, -WinBounds.A.Y);        // Move to X/0
        WinBounds.Move (0, Pos.Y-WinBounds.YSize ());
    }

    // Check if the X position is correct. If the left border of the window
    // is outside the screen, correct that
    if (WinBounds.B.X > ScreenBounds.B.X) {

        WinBounds.Move (ScreenBounds.B.X-WinBounds.B.X, 0);

    }

    // Assume that the position is valid and move the window
    MoveAbs (WinBounds.A);
}



void ItemWindow::PlaceNear (WindowItem* Item)
// Place the window below the given Item. Item must not be an item
// owned by this window. If there is not enough room to place the
// window below the given item, the window is placed above.
{
    // Get the item position in absolute coords
    Point ItemPos (Item->ItemX, Item->ItemY);
    Item->Owner->Absolute (ItemPos);

    // Place the window near this position
    PlaceNear (ItemPos);
}



int ItemWindow::FindAccelKey (ListNode<WindowItem>* N, void* I)
// Helper function for ItemWithAccelKey
{
    // Cast the pointer
    FindStruc2 *P = (FindStruc2 *) I;

    P->Item = N->Contents () -> ItemWithAccelKey (P->K);
    return (P->Item != NULL);
}



WindowItem* ItemWindow::ItemWithAccelKey (Key aAccelKey)
// Try to find an item with the given accelerator key in the tree
// below this window. If one is found, a pointer to the item is
// returned, otherwise the function returns NULL.
{
    FindStruc2 F;

    // Search the given ID
    F.Item = NULL;
    F.K    = aAccelKey;
    Traverse (FindAccelKey, &F);
    return F.Item;
}



int ItemWindow::FindHotKey (ListNode<WindowItem>* N, void* P)
// Helper function for ItemWithHotKey
{
    return (N->Contents () -> ItemWithHotKey (*(Key*)P) != NULL);
}



WindowItem* ItemWindow::ItemWithHotKey (Key aHotKey)
// Try to find an item with the given hot key in the tree
// below this window. If one is found, a pointer to the item is
// returned, otherwise the function returns NULL.
{
    // Searching for a null hotkey does not make sense
    PRECONDITION (aHotKey != kbNoKey);
    return Traverse (FindHotKey, &aHotKey);
}



int ItemWindow::FindID (ListNode<WindowItem>* N, void* I)
// Helper function for ItemWithID
{
    // Cast the pointer
    FindStruc1* P = (FindStruc1*) I;

    P->Item = N->Contents () -> ItemWithID (P->ID);
    return (P->Item != NULL);
}



WindowItem* ItemWindow::ItemWithID (i16 aID)
// Try to find an item with the given ID in the tree below this
// window. If one is found, a pointer to the item is returned,
// otherwise the function returns NULL.
{
    FindStruc1 F;

    // Check the given parameter
    PRECONDITION (aID > 0 && aID <= MaxUserID);

    // Search the given ID
    F.Item = NULL;
    F.ID   = aID;
    Traverse (FindID, &F);
    return F.Item;
}



WindowItem* ItemWindow::ForcedItemWithID (i16 aID)
// Acts like ItemWithID but treats the case different, when no
// matching ID is found: This is considered as a fatal error.
{
    // Search for the ID
    WindowItem* Item = ItemWithID (aID);

    // Check if a item was found
    CHECK (Item != NULL);

    // Return the result
    return Item;
}



void ItemWindow::SetAccelKey (i16 aID, Key aAccelKey)
// Sets the accelerator key for the item with the given ID. If no
// item with this ID is found, this is considered as a fatal error.
{
    ForcedItemWithID (aID) -> SetAccelKey (aAccelKey);
}



Key ItemWindow::GetHotKey (i16 aID)
// Return the hot key of the item with the given ID. If no such item
// is found, this is considered as a fatal error.
{
    return ForcedItemWithID (aID) -> GetHotKey ();
}



Key ItemWindow::GetAccelKey (i16 aID)
// Return the accelerator key of the item with the given ID. If no
// such item is found, this is considered as a fatal error.
{
    return ForcedItemWithID (aID) -> GetAccelKey ();
}



void ItemWindow::SetHelpKey (i16 aID, const String &aHelpKey)
// Set the key for the help function for the item with the given ID.
// If no item with that ID is found, this is handled as a fatal error.
{
    ForcedItemWithID (aID) -> SetHelpKey (aHelpKey);
}



void ItemWindow::DrawItem (i16 aID)
// Redraw the item with the given ID.
{
    ForcedItemWithID (aID) -> Draw ();
}



void ItemWindow::ActivateItem (i16 aID)
// Activate the item with the given ID
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Activate the item
    Item->Activate ();

    // If there has been no selected item before, choose this one
    ValidateSelectedItem ();
}



void ItemWindow::DeactivateItem (i16 aID)
// Deactivate the item with the given ID
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Deactivate the item
    Item->Deactivate ();

    // If the inactive item is the selected item, choose another selected item
    ValidateSelectedItem ();
}



void ItemWindow::GrayItem (i16 aID)
// Gray the item with the given ID
{
    // Get a pointer to the item
    WindowItem* Item = ForcedItemWithID (aID);

    // Gray the item
    Item->Gray ();

    // If the grayed item is the selected item, choose another selected item
    ValidateSelectedItem ();
}



int ItemWindow::RegisterOneItemKey (ListNode<WindowItem> *N, void *)
// Register the accel key of one item
{
    N->Contents () -> RegisterKey ();
    return 0;
}



int ItemWindow::UnregisterOneItemKey (ListNode<WindowItem> *N, void *)
// Unregister the accel key of one item
{
    N->Contents () -> UnregisterKey ();
    return 0;
}



void ItemWindow::RegisterItemKeys ()
// Register the accel keys of all active items
{
    Traverse (RegisterOneItemKey);
}



void ItemWindow::UnregisterItemKeys ()
// Unregister the accel keys of all active items
{
    Traverse (UnregisterOneItemKey);
}



int ItemWindow::CanClose ()
// Return true if the window is allowed to close. This function is a hook
// for derived windows, it returns always true.
{
    // Simple ItemWindows are allways allowed to close
    return 1;
}



void ItemWindow::Zoom ()
// Interface function for derived classes. This function is a no op and
// must be overloaded
{
}



