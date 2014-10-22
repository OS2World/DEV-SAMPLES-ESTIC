/*****************************************************************************/
/*                                                                           */
/*                                 MSGCOLL.H                                 */
/*                                                                           */
/* (C) 1993,94  Ullrich von Bassewitz                                        */
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



#ifndef __MSGCOLL_H
#define __MSGCOLL_H



#include "machine.h"
#include "stream.h"
#include "coll.h"
#include "msg.h"




/*****************************************************************************/
/*                            class MsgCollection                            */
/*****************************************************************************/



class MsgCollection : public SortedCollection<Msg, u16> {

public:
    MsgCollection (int aLimit, int aDelta);

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

protected:
    // Build constructor
    MsgCollection (StreamableInit);

    // Derived from class Collection
    virtual void* GetItem (Stream&);
    virtual void PutItem (Stream&, void* Item) const;

    // Derived from class SortedCollection
    virtual int Compare (const u16* Key1, const u16* Key2);
    virtual const u16* KeyOf (const Msg* Item);

public:
    // New member functions
    const Msg& GetMsg (u16 ID);

};



inline MsgCollection::MsgCollection (StreamableInit) :
    SortedCollection<Msg, u16> (Empty)
{
}



inline MsgCollection::MsgCollection (int aLimit, int aDelta) :
    SortedCollection<Msg, u16> (aLimit, aDelta)
{
    ShouldDelete = 1;
}



// End of MSGCOLL.H

#endif

