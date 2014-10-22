/*****************************************************************************/
/*									     */
/*				   STRCOLL.H				     */
/*									     */
/* (C) 1993,94	Ullrich von Bassewitz					     */
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



#ifndef __STRCOLL_H
#define __STRCOLL_H


#include "strmable.h"
#include "coll.h"
#include "stream.h"
#include "str.h"




/*****************************************************************************/
/*			    class StringCollection			     */
/*****************************************************************************/



class StringCollection : public SortedCollection <String, String> {

public:
    StringCollection (StreamableInit);
    StringCollection (int aLimit, int aDelta);

    // Derived from class Streamable
    static Streamable* Build ();
    virtual u16 StreamableID () const;

protected:
    // Derived from class Collection
    virtual void* GetItem (Stream& S);
    virtual void PutItem (Stream& S, void* Item) const;

    // Derived from class SortedCollection
    virtual int Compare (const String* Key1, const String* Key2);

};



inline StringCollection::StringCollection (StreamableInit) :
    SortedCollection<String, String> (Empty)
{
}



inline StringCollection::StringCollection (int aLimit, int aDelta) :
    SortedCollection<String, String> (aLimit, aDelta)
{
    ShouldDelete = 1;
}



// End of STRCOLL.H

#endif
