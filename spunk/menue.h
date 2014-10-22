/*****************************************************************************/
/*									     */
/*				     MENUE.H				     */
/*									     */
/* (C) 1993-96	Ullrich von Bassewitz					     */
/*		Wacholderweg 14						     */
/*		D-70597 Stuttgart					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef _MENUE_H
#define _MENUE_H



#include "itemwin.h"
#include "datetime.h"



/*****************************************************************************/
/*				class MenueItem				     */
/*****************************************************************************/



class MenueItem: public WindowItem {

    friend class ResEditApp;			// Resource editor is a friend

protected:
    String		Entry;		// Actual view of the item

    // Rebuild Entry from ItemText and S. The new value of Entry after
    // calling BuildEntry has the form " " + ItemText + Fill + S + " ",
    // where Fill is a string of spaces so that Entry gets the length
    // Width. This behaviour can change in derived versions of BuildEntry
    virtual void BuildEntry (const String &S);

    MenueItem (StreamableInit);
    // Build constructor

public:
    MenueItem (const String &aItemText, i16 aID, WindowItem *NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class WindowItem
    virtual void SetWidth (u16 NewWidth);
    virtual u16 MinWidth ();
    virtual void CallHelp ();
    virtual void Draw ();
    virtual void DrawItemText ();
};



inline MenueItem::MenueItem (StreamableInit) :
	WindowItem (Empty),
	Entry (Empty)
{
}



/*****************************************************************************/
/*			      class GenericMenue			     */
/*****************************************************************************/



class GenericMenue : public ItemWindow {

    friend class ResEditApp;		// Resource editor is a friend

    struct YDesc {
	int Base;
	int Count;
    };

private:
    Key		    PrevKey;		// Key to reach the previous entry
    Key		    NextKey;		// Key to reach the next entry
    Key		    AltPrevKey;
    Key		    AltNextKey;
    Key		    AbortKey;		// Key that caused the abort in GetChoice
    WindowItem**    ItemBuf;		// Variables used internal in GetChoice
    unsigned	    ItemBufCount;
    YDesc*	    ItemDesc;


    static int TestItem (ListNode<WindowItem>*, void*);
    // Helper function for GetChoice

    static int StoreOneItem (ListNode<WindowItem>* Node, void* P);
    // Helper function for GenericMenue::GetChoice. Insert all active items
    // into ItemBuf

#if defined (OS2) && defined (__BORLANDC__)
    // The following function is used as a parameter to qsort, but since the
    // Borland C library is compiled using the _stdcall calling convention
    // and on the other side, static member functions have _cdecl calling
    // conventions by default, we have to redefine this one. Yuck!
    // Maybe it would be better to drop support for Borland-C...
    static int _stdcall CompareItems (const void* I1, const void* I2);
#else
    static int CompareItems (const void* I1, const void* I2);
    // Helper function for GenericMenue::GetChoice. Compare two items by position
    // when sorting the item list
#endif

    void BuildItemIndex ();
    // Helper function for GenericMenue::GetChoice. Build the index from the
    // sorted item list

    unsigned FindItemInItemBuf (WindowItem* P);
    // Helper function for GenericMenue::GetChoice. Find an item in the item
    // buffer

    void DynamicLeft ();
    // Handle dynamic cursor movement to the left.

    void DynamicRight ();
    // Handle dynamic cursor movement to the right.


protected:
    void SelectItem (i16 ItemID);
    // Select the entry with the given id

    void DeselectItem (i16 ItemID);
    // Deselect the entry with the given id

    void InitGenericMenue (const Point& Origin, const String& HeaderString);
    // Special internal function that completes the initialization.
    // Works together with the protected constructor and is only
    // to be called from the constructors of derived classes

    GenericMenue (Key aPrevKey, Key aNextKey, u16 aState = wfFramed,
		 WindowItem* ItemList = NULL);
    // Internal used constructor. Can only be called by constructors
    // of derived types. See InitGenericMenue.

    GenericMenue (StreamableInit);
    // Build constructor


public:
    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);

    i16 GetChoice ();
    //

    virtual Key Browse ();
    // Overrides ItemWindow::Browse. Uses GetChoice to display the menue but
    // ignores the menue choice.

    Key GetAbortKey ();
    // Function to return the key that caused an abort in GetChoice

    i16 GetSelectedItem ();
    // Return the id of the currently selected item or return zero if
    // no item is currently selected

    virtual u16 MinWidth () = 0;
    // Return the minimum width of the menue

    virtual u16 MinHeight () = 0;
    // Return the minimum height of the menue

    virtual void SetWidth (u16 NewWidth) = 0;
    // Set a new width for the menue

    void SetAlternateKeys (Key aPrevKey, Key aNextKey);
    // Set the keys for the ids -1 and -2

    // Support functions for using the items in the menue.
    // All functions determine the type of the item involved and act
    // according to the rules of the item. This means that you can use
    // SetStringValue to set the value of a StringItem, an EditLine or
    // a TextEdit. If the type of the item is not known, the program is
    // aborted with an error message.
    // All functions will also abort, if the given ID does not exist.

    double GetFloatValue (i16 aID);
    // Get the float value of an item.

    i32 GetLongValue (i16 aID);
    // Return the long value of an item.

    const String& GetStringValue (i16 aID);
    // Return the string value of an item.

    u16 GetToggleValue (i16 aID);
    // Return the toggle value of an item.

    const Time& GetTimeValue (i16 aID);
    // Return the time value of an item.

    const Time& GetDateValue (i16 aID);
    // Return the date value of an item.

    void SetFloatValue (i16 aID, double NewVal);
    // Set the float value of an item.

    void SetLongValue (i16 aID, i32 NewVal);
    // Set the long value of an item.

    void SetStringValue (i16 aID, const String & NewVal);
    // Set the string value of an item.

    void SetToggleValue (i16 aID, u16 NewVal);
    // Set the toggle value of an item.

    void SetTimeValue (i16 aID, const Time& NewVal);
    // Set the time value of an item.

    void SetTimeValue (i16 aID, u32 NewVal);
    // Set the value of an item.

    void SetDateValue (i16 aID, const Time& NewVal);
    // Set the date value of an item.

};



inline GenericMenue::GenericMenue (StreamableInit) :
	ItemWindow (Empty)
{
}



/*****************************************************************************/
/*				class MenueBar				     */
/*****************************************************************************/



class MenueBar: public GenericMenue {

    friend class ResEditApp;			// Resource editor is a friend

private:
    static int SetOnePos (ListNode<WindowItem>*, void*);
    // Helper function for MenueBar::SetPos, sets the position of one item


protected:
    // Derived from class ItemWindow
    virtual void SetPos ();

    MenueBar (StreamableInit);
    // Creates an empty instance


public:
    MenueBar (const Point& Origin, WindowItem* ItemList = NULL);
    //

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class GenericMenue
    virtual u16 MinHeight ();
    virtual u16 MinWidth ();
    virtual void SetWidth (u16 NewWidth);

};



inline MenueBar::MenueBar (StreamableInit):
	GenericMenue (Empty)
{
}


/*****************************************************************************/
/*			       class TopMenueBar			     */
/*****************************************************************************/



class TopMenueBar: public MenueBar {

    friend class ResEditApp;			// Resource editor is a friend

protected:
    virtual void ScreenSizeChanged (const Rect& NewScreen);
    // Called when the screen got another resolution. NewScreen is the new
    // screen size.

    virtual void MoveResizeAfterLoad (const Point& OldRes);
    // This function is called after a load when most of the window is
    // constructed. It is used to move the window to a new position if this is
    // needed. OldRes is the old screen resolution that was active, when the
    // window was stored.
    // It is used to position the menuebar at the top of the screen after a
    // load.

    TopMenueBar (StreamableInit);
    // Creates an empty instance


public:
    TopMenueBar (WindowItem* ItemList = NULL);
    //

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

};



inline TopMenueBar::TopMenueBar (StreamableInit) :
	MenueBar (Empty)
{
}


/*****************************************************************************/
/*				  class Menue				     */
/*****************************************************************************/



class Menue: public GenericMenue {

    friend class ResEditApp;			// Resource editor is a friend

protected:
    // Derived from class ItemWindow
//  virtual void SetPos ();

    Menue (StreamableInit);
    // Create an empty instance

public:
    Menue (const Point& Origin, const String& HeaderString,
	   WindowItem* ItemList = NULL);

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class GenericMenue
    virtual u16 MinHeight ();
    virtual u16 MinWidth ();
    virtual void SetWidth (u16 NewWidth);

};



inline Menue::Menue (StreamableInit) :
	GenericMenue (Empty)
{
}



/*****************************************************************************/
/*			      class SubMenueItem			     */
/*****************************************************************************/



class SubMenueItem: public MenueItem {

    friend class ResEditApp;		// Resource editor is a friend

protected:
    Menue*		SubMenue;	// Pointer to submenue


    SubMenueItem (StreamableInit);
    // Create an empty instance


public:
    SubMenueItem (const String& aItemText, i16 aID, WindowItem* MenueList,
		  WindowItem* NextItem);
    virtual ~SubMenueItem ();

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class WindowItem
    virtual i16 Choose ();
    virtual WindowItem* ItemWithID (i16 aID);
    virtual WindowItem* ItemWithAccelKey (Key aAccelKey);

    // Derived from class MenueItem
    virtual u16 MinWidth ();
    virtual void SetWidth (u16 NewWidth);

    // Additional member functions
    virtual void SetSubMenue (Menue* NewSubMenue);

};



inline SubMenueItem::SubMenueItem (StreamableInit) :
	MenueItem (Empty)
{
}



/*****************************************************************************/
/*			      class MenueBarItem			     */
/*****************************************************************************/



class MenueBarItem: public SubMenueItem {

    friend class ResEditApp;			// Resource editor is a friend

protected:
    MenueBarItem (StreamableInit);
    // Create an empty instance


public:
    MenueBarItem (const String& aItemText, i16 aID, WindowItem* MenueList,
		  WindowItem* NextItem);

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class SubMenueItem
    virtual u16 MinWidth ();
    virtual void SetWidth (u16 NewWidth);
    virtual void SetSubMenue (Menue* NewSubMenue);

};



inline MenueBarItem::MenueBarItem (StreamableInit) :
    SubMenueItem (Empty)
{
}



// End of MENUE.H

#endif



