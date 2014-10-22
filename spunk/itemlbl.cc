/*****************************************************************************/
/*                                                                           */
/*                                  ITEMLBL.CC                               */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



// Class ItemLabel is a special window item that acts as a label for another
// window item.



#include "streamid.h"
#include "itemlbl.h"



// Register class ItemLabel
LINK (ItemLabel, ID_ItemLabel);



/*****************************************************************************/
/*                              class ItemLabel                              */
/*****************************************************************************/


ItemLabel::ItemLabel (const String& aItemText, i16 aID, i16 aCtrlID,
                      WindowItem* NextItem):
    WindowItem (aItemText, aID, NextItem),
    CtrlID (aCtrlID)
{
}



void ItemLabel::Store (Stream& S) const
{
    WindowItem::Store (S);
    S << CtrlID;
}



void ItemLabel::Load (Stream& S)
{
    WindowItem::Load (S);
    S >> CtrlID;
}



u16 ItemLabel::StreamableID () const
{
    return ID_ItemLabel;
}



Streamable* ItemLabel::Build ()
{
    return new ItemLabel (Empty);
}



WindowItem* ItemLabel::GetCtrlItem ()
// Return a pointer to the controlled item or NULL
{
    if (CtrlID != 0) {
        return GetRootWindow () -> ItemWithID (CtrlID);
    } else {
        return NULL;
    }
}



void ItemLabel::Gray ()
{
    // Gray the label
    WindowItem::Gray ();

    // If there is a valid controlled item, gray that one, too
    WindowItem* Item = GetCtrlItem ();
    if (Item) {
        Item->Gray ();
    }
}



void ItemLabel::Select ()
{
    // Select the label
    WindowItem::Select ();

    // If there is a valid controlled item, select that one, too
    WindowItem* Item = GetCtrlItem ();
    if (Item) {
        Item->Select ();
    }
}



void ItemLabel::Deselect ()
{
    // Deselect the label
    WindowItem::Deselect ();

    // If there is a valid controlled item, deselect that one, too
    WindowItem* Item = GetCtrlItem ();
    if (Item) {
        Item->Deselect ();
    }
}



i16 ItemLabel::Choose ()
// Choose an entry
{
    // Item must be active
    PRECONDITION (IsActive ());

    // Use the controlled item
    WindowItem* Item = GetCtrlItem ();
    if (Item) {
        Item->Activate ();
        CtrlChoice = Item->Choose ();
        Item->Deactivate ();
        return CtrlChoice ? ID : 0;
    } else {
        CtrlChoice = 0;
        return ID;
    }
}



