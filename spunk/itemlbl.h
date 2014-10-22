/*****************************************************************************/
/*                                                                           */
/*                                  ITEMLBL.H                                */
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



#ifndef __ITEMLBL_H
#define __ITEMLBL_H



#include "itemwin.h"



/*****************************************************************************/
/*                              class ItemLabel                              */
/*****************************************************************************/


class ItemLabel: public WindowItem {

    friend class ResEditApp;            // Resource editor is a friend

protected:
    i16                 CtrlID;         // ID of item that is controlled
    i16                 CtrlChoice;     // Return code of Choose


    WindowItem* GetCtrlItem ();
    // Return a pointer to the controlled item or NULL

    ItemLabel (StreamableInit);
    // Build constructor


public:
    ItemLabel (const String& aItemText, i16 aID, i16 aCtrlID, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    void SetCtrlID (i16 aCtrlID);
    // Set the ID of the item that is controlled by the label

    i16 GetCtrlID () const;
    // Get the ID of the item that is controlled by the label

    i16 GetCtrlChoice () const;
    // Choose returns the ID of the label item if the controlled item's
    // "Choose" returned a value other than 0. Use this function to retrieve
    // the return code of the controlled items Choose function.

    // Change the status
    virtual void Gray ();
    virtual void Select ();
    virtual void Deselect ();

    // Choose an entry
    virtual i16 Choose ();

};




inline ItemLabel::ItemLabel (StreamableInit):
    WindowItem (Empty)
{
}



inline void ItemLabel::SetCtrlID (i16 aCtrlID)
// Set the ID of the item that is controlled by the label
{
    CtrlID = aCtrlID;
}



inline i16 ItemLabel::GetCtrlID () const
// Get the ID of the item that is controlled by the label
{
    return CtrlID;
}



inline i16 ItemLabel::GetCtrlChoice () const
// Choose returns the ID of the label item if the controlled item's
// "Choose" returned a value other than 0. Use this function to retrieve
// the return code of the controlled items Choose function.
{
    return CtrlChoice;
}



// End of ITEMLBL.H

#endif

