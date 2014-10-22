/*****************************************************************************/
/*                                                                           */
/*                                  ITEMWIN.H                                */
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



#ifndef _ITEMWIN_H
#define _ITEMWIN_H



#include "keydef.h"
#include "event.h"
#include "window.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Highest value for any item ID used by the application
// Legal values are 1 <= ID <= MaxUserID
const i16 MaxUserID             = 0x7FFD;


// Flags
const u16 ifInactive            = 0x0001;       // Item is inactive
const u16 ifGrayed              = 0x0002;       // Item is grayed
const u16 ifSelected            = 0x0004;       // Item is selected
const u16 ifVertical            = 0x0008;       // Reserved
const u16 ifNoSub               = 0x0010;       // Don't show submenue



/*****************************************************************************/
/*                             class WindowItem                              */
/*****************************************************************************/



class WindowItem : public Streamable {

    friend class ResEditApp;                    // Resource editor is a friend
    friend class ItemWindow;

    friend inline WindowItem* SetAccelKey (WindowItem* Item, Key AccelKey);
    // This is a friend of class WindowItem and a short and convienient way to
    // set the accel key when constructing a WindowItem. Just pass the created
    // item through this function, the accel key is set and the item is returned.


protected:
    ListNode<WindowItem>        INode;          // double linked list of items
    ItemWindow*                 Owner;          // Owner window
    u16                         ItemX, ItemY;   // Window position
    u16                         ItemWidth;      // Entry width

    i16                         ID;             // item ID
    u16                         Flags;          // Flag word

    Key                         HotKey;         //
    i16                         HotPos;         // Position of the hotkey
    Key                         AccelKey;       // Accelerator key

    String                      ItemText;       //
    String                      HelpKey;        // Key for the online help


    virtual void SelectNew (WindowItem* NewItem);
    // Deselect the selected item and set NewItem as newly selected item.
    // Checks if this == NewItem and ignores a request in this case

    virtual WindowItem* SelectNext ();
    // Select the next item in the list. Return the item that is selected
    // after the operation has been performed.

    virtual WindowItem* SelectPrev ();
    // Select the previous item in the list. Return the item that is selected
    // after the operation has been performed.

    void RegisterKey ();
    // If the item is active and has an accel key: Register the key at the
    // current thread.

    void UnregisterKey ();
    // If the item is active and has an accel key: Unregister the key at the
    // current thread.

    WindowItem (StreamableInit);
    // Build constructor


public:
    WindowItem (const String& aItemText, i16 aID, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // New functions
    void SetOwner (ItemWindow* aOwner);
    virtual void SetPos (u16 X, u16 Y);
    void SetPos (Point P);
    virtual void SetWidth (u16 NewWidth);
    void SetHelpKey (const String& NewKey);
    const String& GetHelpKey () const;
    void SetAccelKey (Key NewNewKey);
    void SetItemText (const String& aItemText);

    // Access class data
    ItemWindow* GetOwner () const;
    virtual u16 GetWidth () const;
    virtual u16 MinWidth ();
    u16 XPos () const;
    u16 YPos () const;
    Point Pos () const;
    Key GetHotKey () const;
    Key GetAccelKey () const;
    const String& GetItemText () const;
    virtual i16 GetID ();

    // Draw and clear the item and the item text
    virtual void Draw ();
    virtual void Clear ();
    virtual void DrawItemText ();
    virtual void ClearItemText ();

    // Change the status
    virtual void Activate ();
    virtual void Deactivate ();
    virtual void Gray ();
    virtual void Select ();
    virtual void Deselect ();

    // Check the status
    int IsSelected ();
    int IsActive ();
    int IsGrayed ();
    int HasHelp ();

    // Call the help function with the help key of this item
    virtual void CallHelp ();

    // Choose an entry
    virtual i16 Choose ();

    // Locate items
    virtual WindowItem* ItemWithID (i16 aID);
    virtual WindowItem* ItemWithAccelKey (Key aAccelKey);
    virtual WindowItem* ItemWithHotKey (Key aHotKey);

    ItemWindow* GetRootWindow ();
    // Get the root of all windows in the current chain

};




inline WindowItem::WindowItem (StreamableInit) :
    INode (this),
    ItemText (Empty),
    HelpKey (Empty)
{
}



inline void WindowItem::SetOwner (ItemWindow* aOwner)
{
    Owner = aOwner;
}



inline void WindowItem::SetPos (Point P)
{
    SetPos (P.X, P.Y);
}



inline void WindowItem::SetHelpKey (const String& NewKey)
{
    HelpKey = NewKey;
}



inline const String& WindowItem::GetHelpKey () const
{
    return HelpKey;
}



inline void WindowItem::SetAccelKey (Key NewAccelKey)
{
    AccelKey = NewAccelKey;
}



inline ItemWindow* WindowItem::GetOwner () const
{
    return Owner;
}



inline u16 WindowItem::XPos () const
{
    return ItemX;
}



inline u16 WindowItem::YPos () const
{
    return ItemY;
}



inline Point WindowItem::Pos () const
{
    return Point (ItemX, ItemY);
}



inline Key WindowItem::GetHotKey () const
{
    return HotKey;
}



inline Key WindowItem::GetAccelKey () const
{
    return AccelKey;
}



inline const String& WindowItem::GetItemText () const
{
    return ItemText;
}



inline int WindowItem::IsSelected ()
{
    return (Flags & ifSelected) != 0;
}



inline int WindowItem::IsActive ()
{
    return (Flags & ifInactive) == 0;
}



inline int WindowItem::IsGrayed ()
{
    return (Flags & ifGrayed) != 0;
}



inline int WindowItem::HasHelp ()
{
    return HelpKey.Len () > 0;
}



inline WindowItem* SetAccelKey (WindowItem* Item, Key AccelKey)
// This is a friend of class WindowItem and a short and convienient way to
// set the accel key when constructing a WindowItem. Just pass the created
// item through this function, the accel key is set and the item is returned.
{
    Item->AccelKey = AccelKey;
    return Item;
}



/*****************************************************************************/
/*                             class ItemWindow                              */
/*****************************************************************************/



class ItemWindow : public Window, public EventHandler {

    friend class ResEditApp;                    // Resource editor is a friend

private:
    // Data structures for use in Traverse
    struct FindStruc1 {
        i16 ID;
        WindowItem* Item;
    };
    struct FindStruc2 {
        Key K;
        WindowItem* Item;
    };


    static int StoreOneItem (ListNode<WindowItem>*, void*);
    // Helper function to store a WindowItem into the stream S

    static int SetOneItemWidth (ListNode<WindowItem>*, void*);
    // Reset the width of one item

    static int SetOneOwner (ListNode<WindowItem>*, void*);
    // Helper function for ItemWindow::SetItemListOwner. Sets the owner window
    // for all items in the list

    static int SetOnePos (ListNode<WindowItem>*, void*);
    // Helper function for ItemWindow::SetPos, sets the position of one item

    static int DrawOneItem (ListNode<WindowItem>*, void*);
    // Helper function for DrawItems, draw one item

    static int CheckSelectedItem (ListNode<WindowItem>*, void*);
    // Helper function for ValidateSelectedItem, find active item

    static int FindAccelKey (ListNode<WindowItem>*, void*);
    // Helper function for ItemWithAccelKey

    static int FindHotKey (ListNode<WindowItem>*, void*);
    // Helper function for ItemWithHotKey

    static int FindID (ListNode<WindowItem>*, void*);
    // Helper function for ItemWithID

    static int RegisterOneItemKey (ListNode<WindowItem>*, void*);
    // Register the accel key of one item

    static int UnregisterOneItemKey (ListNode<WindowItem>*, void*);
    // Unregister the accel key of one item


protected:
    u16                         ItemCount;      // Number of items in the list
    WindowItem*                 Owner;          // Owner of the window
    WindowItem*                 SelectedItem;   // Pointer to selected item
    WindowItem*                 FirstItem;      // Pointer to item (list)


    void SelectNextItem ();
    // Choose the next item in the item list as selected

    void SelectPrevItem ();
    // Choose the previous item in the item list as selected

    virtual void SetPos ();
    // Sets the positions of all window items. The items are lined up
    // vertically at the left window border. To change this behaviour,
    // overload this function.

    void SetItemListOwner ();
    // Traverse through all items and tell them their owner window

    void DrawItems ();
    // Draw all items

    void ValidateSelectedItem ();
    // Check if the item pointed to by SelectedItem is still active. If
    // not, a new item is choosen.

    WindowItem* Traverse (int (*F) (ListNode<WindowItem>*, void*),
                                     void* DataPtr = NULL) const;
    // Use ListNode::Traverse to traverse through all window items starting
    // at FirstItem. Return value corresponds to ListNode::Traverse

    virtual u32 GetStatusFlags ();
    // Returns the flags that are used to build the status line in Browse

    virtual void HandleKey (Key& K);
    // Key dispatcher used in Browse

    ItemWindow (const Rect &Bounds, u16 aState, WindowItem* ItemList);
    // Internal used constructor, leaves with LockCount == 1


public:
    ItemWindow (StreamableInit);
    ItemWindow (const Rect &Bounds, u16 aState = wfFramed,
                unsigned aPalette = paGray,
                unsigned Number = 0, WindowItem* ItemList = NULL);
    virtual ~ItemWindow ();

    // -- Derived from class Streamable

    virtual void Store (Stream &) const;
    virtual void Load (Stream &);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // -- Derived from class Window

    virtual void DrawInterior ();
    // Draw the window interior. This overrides Window::DrawInterior and
    // additionally draws the items

    virtual void Activate ();
    // Activate the window. This overrides Window::Activate and highlights
    // the selected item

    virtual void Deactivate ();
    // Activate the window. This overrides Window::Activate and deselects
    // the selected item

    // -- New functions

    virtual Key Browse ();
    // Make the window active and display the window contents in a suitable
    // manner. This function should be overridden for special derived
    // windows. It returns the key that ended the browse state.

    WindowItem* GetOwner ();
    // Retrieve the pointer to the owner window

    void SetOwner (WindowItem* aOwner);
    // Set the pointer to the owner window

    void SelectNewItem (WindowItem* NewItem);
    // Select the new item, update SelectedItem

    void SelectNewItem (i16 NewID);
    // Select the new item, update SelectedItem

    void PlaceNear (const Point& Pos);
    // Place the window near the given absolute position. If there is not enough
    // room to place the window below the given item, the window is placed above.

    void PlaceNear (WindowItem* Item);
    // Place the window below the given Item. Item must not be an item
    // owned by this window. If there is not enough room to place the
    // window below the given item, the window is placed above.

    virtual WindowItem* ItemWithAccelKey (Key aAccelKey);
    // Try to find an item with the given accelerator key in the tree
    // below this window. If one is found, a pointer to the item is
    // returned, otherwise the function returns NULL.

    virtual WindowItem* ItemWithHotKey (Key aHotKey);
    // Try to find an item with the given hot key in the tree
    // below this window. If one is found, a pointer to the item is
    // returned, otherwise the function returns NULL.

    virtual WindowItem* ItemWithID (i16 aID);
    // Try to find an item with the given ID in the tree below this
    // window. If one is found, a pointer to the item is returned,
    // otherwise the function returns NULL.

    virtual WindowItem* ForcedItemWithID (i16 aID);
    // Acts like ItemWithID but treats the case different, when no
    // matching ID is found: This is considered as a fatal error.

    void SetAccelKey (i16 aID, Key aAccelKey);
    // Sets the accelerator key for the item with the given ID. If no
    // item with this ID is found, this is considered as a fatal error.

    Key GetHotKey (i16 aID);
    // Return the hot key of the item with the given ID. If no such item
    // is found, this is considered as a fatal error.

    Key GetAccelKey (i16 aID);
    // Return the accelerator key of the item with the given ID. If no
    // such item is found, this is considered as a fatal error.

    void SetHelpKey (i16 aID, const String& aHelpKey);
    // Set the key for the help function for the item with the given ID.
    // If no item with that ID is found, this is handled as a fatal error.

    void AddItem (WindowItem* Item);
    // Add the given item to the item list of the window. After that, the
    // item is owned by the window and destroyed if the window destructor
    // is called. The given WindowItem is unlinked from the list it is
    // linked in before it is added to the windows item list.

    void DeleteItem (WindowItem* Item);
    // Take the item from the item list and delete it. If Item is the
    // selected item, a new selected item is choosen. After deleting
    // Item, the window is redrawn.

    void DrawItem (i16 aID);
    // Redraw the item with the given ID.

    void ActivateItem (i16 aID);
    // Activate the item with the given ID

    void DeactivateItem (i16 aID);
    // Deactivate the item with the given ID

    void GrayItem (i16 aID);
    // Gray the item with the given ID

    void RegisterItemKeys ();
    // Register the accel keys of all active items

    void UnregisterItemKeys ();
    // Unregister the accel keys of all active items

    virtual int CanClose ();
    // Return true if the window is allowed to close. This function is a hook
    // for derived windows, it returns always true.

    virtual void Zoom ();
    // Interface function for derived classes. This function is a no op and
    // must be overloaded

};



inline ItemWindow::ItemWindow (StreamableInit X) :
        Window (X)
{
}



inline WindowItem* ItemWindow::GetOwner ()
{
    return Owner;
}



inline void ItemWindow::SetOwner (WindowItem* aOwner)
{
    Owner = aOwner;
}



// End of ITEMWIN.H

#endif



