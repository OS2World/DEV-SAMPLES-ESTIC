/*****************************************************************************/
/*                                                                           */
/*                                MENUITEM.H                                 */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
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



#ifndef _MENUITEM_H
#define _MENUITEM_H


#include "menue.h"
#include "datetime.h"
#include "charset.h"



/*****************************************************************************/
/*                              class MenueLine                              */
/*****************************************************************************/



class MenueLine: public MenueItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    // Rebuild Entry when the length has changed. The given String S is
    // ignored.
    virtual void BuildEntry (const String&);

    MenueLine (StreamableInit);
    // Build constructor

public:
    MenueLine (i16 aID, WindowItem* NextItem);

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class MenueItem
    virtual u16 MinWidth ();

};



inline MenueLine::MenueLine (StreamableInit) :
        MenueItem (Empty)
{
}



/*****************************************************************************/
/*                            class EditMenueItem                            */
/*****************************************************************************/



class EditMenueItem: public MenueItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    ItemWindow          *EditWindow;
    i16                 EditItemID;


    EditMenueItem (StreamableInit);
    // Build constructor


public:
    EditMenueItem (const String& aItemText, i16 aID, i16 EditID,
                   ItemWindow* EditWin, WindowItem* NextItem);
    virtual ~EditMenueItem ();

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);

    virtual i16 Choose () = 0;
    // Choose the entry. If an edit window is defined (EditWindow != NULL),
    // the entry with id EditItemID in EditWindow is called. If there was
    // no abort (or EditWindow is not defined), the id of this item is
    // returned.

    void PlaceEditWindow ();
    // Place the edit window below this entry if EditWindow has no position
    // (== has position 0/0).

    virtual WindowItem* ItemWithID (i16 aID);
    // Search for the item with the given ID. The editwindow (if one exists)
    // is also searched.

    void SetEditWindow (ItemWindow *Win, i16 EditID);
    // Set the edit window. Beware: An already existing edit window is not
    // deleted!

    ItemWindow* GetEditWindow () const;
    // Return the EditWindow pointer

};



inline EditMenueItem::EditMenueItem (StreamableInit) :
        MenueItem (Empty)
{
}



inline ItemWindow* EditMenueItem::GetEditWindow () const
// Return the EditWindow pointer
{
    return EditWindow;
}



/*****************************************************************************/
/*                              class LongItem                               */
/*****************************************************************************/



class LongItem: public EditMenueItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    i32         LValue;                 // The value
    u16         Digits;                 // How many digits to display
    i32         LMin, LMax;             // Limits for editing

    LongItem (StreamableInit);
    // Build constructor


public:
    LongItem (const String& aItemText, i16 aID, u16 aDigits, i16 EditID,
              i32 Min, i32 Max, WindowItem* NextItem);
    LongItem (const String& aItemText, i16 aID, u16 aDigits,
              WindowItem* NextItem);


    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class MenueItem
    virtual void SetWidth (u16 NewWidth);
    virtual u16 MinWidth ();

    // Derived from class EditMenueItem

    virtual i16 Choose ();
    // Choose the entry. If an edit window is defined (EditWindow != NULL),
    // the entry with id EditItemID in EditWindow is called. If there was
    // no abort (or EditWindow is not defined), the id of this item is
    // returned.

    // Handling the protected variables
    i32 GetValue () const;
    void SetValue (i32 Val);
    void SetMinMax (i32 Min, i32 Max);
    void GetMinMax (i32& Min, i32& Max) const;
};



inline LongItem::LongItem (StreamableInit) :
        EditMenueItem (Empty)
{
}



inline i32 LongItem::GetValue () const
{
    return LValue;
}



inline void LongItem::GetMinMax (i32& Min, i32& Max) const
{
    Min = LMin;
    Max = LMax;
}



/*****************************************************************************/
/*                               class HexItem                               */
/*****************************************************************************/



class HexItem: public LongItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    HexItem (StreamableInit);
    // Build constructor

public:
    HexItem (const String& aItemText, i16 aID, u16 aDigits, i16 EditID,
             i32 Min, i32 Max, WindowItem* NextItem);
    HexItem (const String& aItemText, i16 aID, u16 aDigits,
             WindowItem* NextItem);


    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class MenueItem

    virtual void SetWidth (u16 NewWidth);

    virtual i16 Choose ();
    // Choose the entry. If an edit window is defined (EditWindow != NULL),
    // the entry with id EditItemID in EditWindow is called. If there was
    // no abort (or EditWindow is not defined), the id of this item is
    // returned.

};



inline HexItem::HexItem (const String& aItemText, i16 aID, u16 aDigits,
                         i16 EditID, i32 Min, i32 Max, WindowItem* NextItem) :
    LongItem (aItemText, aID, aDigits, EditID, Min, Max, NextItem)
{
}



inline HexItem::HexItem (const String& aItemText, i16 aID, u16 aDigits,
                         WindowItem* NextItem) :
        LongItem (aItemText, aID, aDigits, NextItem)
{
}



inline HexItem::HexItem (StreamableInit) :
        LongItem (Empty)
{
}



/*****************************************************************************/
/*                             class StringItem                              */
/*****************************************************************************/



class StringItem: public EditMenueItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    String      SValue;
    CharSet     AllowedChars;


    StringItem (StreamableInit);
    // Build constructor

public:
    StringItem (const String& aItemText, i16 aID, i16 EditID,
                WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class WindowItem
    virtual void SetWidth (u16 NewWidth);
    virtual u16 MinWidth ();

    virtual i16 Choose ();
    // Choose the entry. If an edit window is defined (EditWindow != NULL),
    // the entry with id EditItemID in EditWindow is called. If there was
    // no abort (or EditWindow is not defined), the id of this item is
    // returned.

    void SetValue (const String& NewVal);
    // Set the current value

    const String& GetValue ();
    // Get the current value

    const CharSet& GetAllowedChars () const;
    // Get the set of allowed input chars

    void SetAllowedChars (const CharSet& CS);
    // Set the allowed input chars

    void AllowEmptyInput ();
    void DisallowEmptyInput ();
    // Allow or disallow an empty input line

};



inline const String& StringItem::GetValue ()
{
    return SValue;
}



/*****************************************************************************/
/*                             class ToggleItem                              */
/*****************************************************************************/



class ToggleItem: public StringItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    u16         TValue;                 // Current toggle value (0..n-1)
    u16         TCount;                 // Number of different values
    u16         TLen;                   // Length of one part
    String      TList;                  // List of display values

    ToggleItem (StreamableInit);
    // Build constructor

public:
    ToggleItem (const String& aItemText, i16 aID, const String& ToggleList,
                unsigned ToggleCount, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    void SetValue (u16 NewVal);
    // Set the new value of the toggle item

    u16 GetValue ();
    // Return the current toggle value

    void Toggle ();
    // Toggle the value

    virtual void SetWidth (u16 NewWidth);
    // Set the new entry width

    virtual u16 MinWidth ();
    // return the width needed

    virtual WindowItem* ItemWithID (i16 aID);
    // Return a pointer to this if the given ID is the ID of the object.
    // A ToggleItem uses all IDs from ID to ID + ToggleCount - 1

    virtual i16 GetID ();
    // Return the ID of the entry. The returned ID is the ID of the item plus
    // the current toggle value

    virtual i16 Choose ();
    // Choose this entry. This implementation toggles to the next value and
    // returns the corresponding ID

};



inline ToggleItem::ToggleItem (StreamableInit) :
        StringItem (Empty), TList (Empty)
{
}



inline u16 ToggleItem::GetValue ()
{
    return TValue;
}



/*****************************************************************************/
/*                              class OffOnItem                              */
/*****************************************************************************/



class OffOnItem: public ToggleItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    OffOnItem (StreamableInit);
    // Build constructor

public:
    OffOnItem (const String& aItemText, i16 aID, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Load (Stream& S);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

};



inline OffOnItem::OffOnItem (StreamableInit) :
        ToggleItem (Empty)
{
}



/*****************************************************************************/
/*                              class NoYesItem                              */
/*****************************************************************************/



class NoYesItem: public ToggleItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    NoYesItem (StreamableInit);
    // Build constructor

public:
    NoYesItem (const String& aItemText, i16 aID, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Load (Stream& S);
    virtual u16 StreamableID () const;
    static Streamable * Build ();

};



inline NoYesItem::NoYesItem (StreamableInit) :
        ToggleItem (Empty)
{
}



/*****************************************************************************/
/*                              class FloatItem                              */
/*****************************************************************************/



class FloatItem: public EditMenueItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    double              FValue;
    u16                 LD, TD;         // Leading and trailing digits
    double              FMin;           // Minimum value
    double              FMax;           // Maximum value

    FloatItem (StreamableInit);
    // Build constructor

public:
    FloatItem (const String& aItemText, i16 aID, u16 aLD, u16 aTD, u16 EditID,
               double Min, double Max, WindowItem* NextItem);
    FloatItem (const String& aItemText, i16 aID, u16 aLD, u16 aTD,
               WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    void SetValue (double NewVal);
    // Set the new value of the float item

    double GetValue () const;
    // Return the current value

    void SetMinMax (double Min, double Max);
    // Set the values for FMin/FMax

    void GetMinMax (double& Min, double& Max) const;
    // Get the values for FMin/FMax

    virtual void SetWidth (u16 NewWidth);
    // Set the new entry width

    virtual u16 MinWidth ();
    // return the width needed

    virtual i16 Choose ();
    // Choose the entry. If an edit window is defined (EditWindow != NULL),
    // the entry with id EditItemID in EditWindow is called. If there was
    // no abort (or EditWindow is not defined), the id of this item is
    // returned.

};



inline FloatItem::FloatItem (StreamableInit) :
        EditMenueItem (Empty)
{
}



inline double FloatItem::GetValue () const
{
    return FValue;
}



inline void FloatItem::GetMinMax (double& Min, double& Max) const
// Get the values for FMin/FMax
{
    Min = FMin;
    Max = FMax;
}



/*****************************************************************************/
/*                              class TimeItem                               */
/*****************************************************************************/



class TimeItem: public EditMenueItem {

protected:
    Time        TimeVal;


    TimeItem (StreamableInit);
    // Build constructor


public:
    TimeItem (const String& aItemText, i16 aID, i16 EditID,
              WindowItem* NextItem);
    TimeItem (const String& aItemText, i16 aID, WindowItem* NextItem);


    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class MenueItem
    virtual void SetWidth (u16 NewWidth);
    virtual u16 MinWidth ();

    // Derived from class EditMenueItem

    virtual i16 Choose ();
    // Choose the entry. If an edit window is defined (EditWindow != NULL),
    // the entry with id EditItemID in EditWindow is called. If there was
    // no abort (or EditWindow is not defined), the id of this item is
    // returned.

    // Handling the protected variables
    const Time& GetValue () const;
    void SetValue (const Time& Val);
    void SetValue (unsigned Hours, unsigned Minutes, unsigned Seconds);
    void SetValue (u32 Sec);
};



inline TimeItem::TimeItem (StreamableInit) :
    EditMenueItem (Empty),
    TimeVal (Empty)
{
}



inline const Time& TimeItem::GetValue () const
{
    return TimeVal;
}



/*****************************************************************************/
/*                              class DateItem                               */
/*****************************************************************************/



class DateItem: public EditMenueItem {

protected:
    Time        TimeVal;

    DateItem (StreamableInit);
    // Build constructor


public:
    DateItem (const String & aItemText, i16 aID, i16 EditID,
              WindowItem *NextItem);
    DateItem (const String & aItemText, i16 aID, WindowItem *NextItem);


    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable * Build ();

    // Derived from class MenueItem
    virtual void SetWidth (u16 NewWidth);
    virtual u16 MinWidth ();

    // Derived from class EditMenueItem

    virtual i16 Choose ();
    // Choose the entry. If an edit window is defined (EditWindow != NULL),
    // the entry with id EditItemID in EditWindow is called. If there was
    // no abort (or EditWindow is not defined), the id of this item is
    // returned.

    // Handling the protected variables
    const Time& GetValue () const;
    void SetValue (const Time& Val);
    void SetValue (unsigned Year, unsigned Month, unsigned Day);
};



inline DateItem::DateItem (StreamableInit) :
    EditMenueItem (Empty),
    TimeVal (Empty)
{
}



inline const Time& DateItem::GetValue () const
{
    return TimeVal;
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



class RStringItem: public StringItem {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    u16         InputLength;                    // Length of the input string


    RStringItem (StreamableInit);
    // Build constructor

public:
    RStringItem (const String& aItemText, i16 aID, i16 EditID,
                 u16 aInputLength, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    virtual i16 Choose ();
    // Choose the entry. If an edit window is defined (EditWindow != NULL),
    // the entry with id EditItemID in EditWindow is called. If there was
    // no abort (or EditWindow is not defined), the id of this item is
    // returned.

};



// End of MENUITEM.H

#endif



