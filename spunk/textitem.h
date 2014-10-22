/*****************************************************************************/
/*                                                                           */
/*                                 TEXTITEM.H                                */
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
// of text in a given attribute in a window. It is not thought to be active
// or selected, so there are no special attributes for that.



#ifndef _TEXTITEM_H
#define _TEXTITEM_H



#include "itemwin.h"



/*****************************************************************************/
/*                             class WindowItem                              */
/*****************************************************************************/



class TextItem : public WindowItem {

    friend class ResEditApp;    // Resource editor is a friend

protected:
    u16         Attr;           // Text attribute

public:
    TextItem (const String& aItemText, i16 aID, u16 aAttr, WindowItem* NextItem);
    // Construct a TextItem

    TextItem (StreamableInit);
    // Construct an empty TextItem

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class WindowItem
    virtual u16 MinWidth ();
    virtual void Draw ();

    void SetText (const String& aText);
    // Convenience function, identical to SetItemText
};



// End of TEXTITEM.H

#endif

