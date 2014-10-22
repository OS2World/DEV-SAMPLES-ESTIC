/*****************************************************************************/
/*									     */
/*				   STRBOX.H				     */
/*									     */
/* (C) 1995	Ullrich von Bassewitz					     */
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



// This is a listbox for a collection of strings



#ifndef __STRBOX_H
#define __STRBOX_H



#include "coll.h"
#include "listbox.h"



/*****************************************************************************/
/*				 class StrBox				     */
/*****************************************************************************/



class StringListBox: public ListBox<String> {

protected:
    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    StringListBox (const String& aItemText, i16 aID, const Point& aSize,
		   u16 aNormAttr, u16 aSelAttr, u16 aHighAttr,
		   WindowItem* NextItem);

    StringListBox (const String& aItemText, i16 aID, const Point& aSize,
		   WindowItem* NextItem);

};



// End of STRBOX.H

#endif

