/*****************************************************************************/
/*                                                                           */
/*                                 STRBOX.CC                                 */
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



// This is a listbox for a collection of strings



#include "strbox.h"



/*****************************************************************************/
/*                               class StrBox                                */
/*****************************************************************************/



void StringListBox::Print (int Index, int X, int Y, u16 Attr)
// Display one of the listbox entries
{
    String Line (Size.X);

    // Get the entry
    const String* S = Coll->At (Index);
    if (S == NULL) {

        // Entry is empty, clear it
        Line.Set (0, Size.X);

    } else {

        // Build the line
        Line += ' ';
        Line += *S;
        Line.Pad (String::Right, Size.X - 1);
        Line += ' ';

    }

    // Write it to the window
    Owner->Write (X, Y, Line, Attr);
}



StringListBox::StringListBox (const String& aItemText, i16 aID,
                              const Point& aSize, u16 aNormAttr,
                              u16 aSelAttr, u16 aHighAttr,
                              WindowItem* NextItem):
    ListBox<String> (aItemText, aID, aSize, aNormAttr, aSelAttr, aHighAttr, NextItem)
{
}



StringListBox::StringListBox (const String& aItemText, i16 aID,
                              const Point& aSize, WindowItem* NextItem):
    ListBox<String> (aItemText, aID, aSize, NextItem)
{
}



