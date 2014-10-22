/*****************************************************************************/
/*                                                                           */
/*                                 TEXTITEM.CC                               */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



// This is a special window item having the only purpose to display a row
// of text in a given attribute in a window. It is not nthought to be active
// or selected, so there are no special attributes for that.



#include "streamid.h"
#include "textitem.h"



// Register the classes
LINK (TextItem, ID_TextItem);



/*****************************************************************************/
/*                             class WindowItem                              */
/*****************************************************************************/



TextItem::TextItem (const String& aItemText, i16 aID,
                    u16 aAttr, WindowItem* NextItem):
    Attr (aAttr),
    WindowItem (aItemText, aID, NextItem)
{
    // Make TextItems inactive by default
    Flags |= ifInactive;
}



TextItem::TextItem (StreamableInit):
    WindowItem (Empty)
// Construct an empty TextItem
{
}



void TextItem::Store (Stream& S) const
// Store the item data into a stream
{
    // Store WindowItem data
    WindowItem::Store (S);

    // Store additional data
    S << Attr;
}



void TextItem::Load (Stream& S)
// Load the item data from a stream
{
    // Load WindowItem data
    WindowItem::Load (S);

    // Load additional data
    S >> Attr;
}



u16 TextItem::StreamableID () const
{
    return ID_TextItem;
}



Streamable* TextItem::Build ()
{
    return new TextItem (Empty);
}



u16 TextItem::MinWidth ()
{
    return 1;
}



void TextItem::Draw ()
{
    // Get the item text
    String S = ItemText;

    // Pad it to length
    S.Pad (String::Right, ItemWidth);

    // Lock the owner window
    Owner->Lock ();

    // Write out the item text
    Owner->Write (ItemX, ItemY, S, Attr);

    // Unlock the owner window
    Owner->Unlock ();
}



void TextItem::SetText (const String& aText)
// Convenience function, identical to SetItemText
{
    SetItemText (aText);
}



