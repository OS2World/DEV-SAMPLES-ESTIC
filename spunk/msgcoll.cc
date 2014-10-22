/*****************************************************************************/
/*                                                                           */
/*                                 MSGCOLL.CC                                */
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



#include "machine.h"
#include "coll.h"
#include "stream.h"
#include "msg.h"
#include "msgcoll.h"
#include "streamid.h"



// Register class MsgCollection if there are references in this module
LINK(MsgCollection, ID_MsgCollection);



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<Msg>;
template class SortedCollection<Msg, u16>;
#endif



/*****************************************************************************/
/*                            class MsgCollection                            */
/*****************************************************************************/



u16 MsgCollection::StreamableID () const
{
    return ID_MsgCollection;
}



Streamable* MsgCollection::Build ()
{
    return new MsgCollection (Empty);
}



void* MsgCollection::GetItem (Stream& S)
{
    // Create new Msg
    Msg *Item = new Msg (Empty);

    // Read data
    S >> Item;

    return (void*) Item;
}



void MsgCollection::PutItem (Stream& S, void* Item) const
{
    S << (Msg*) Item;
}



int MsgCollection::Compare (const u16* Key1, const u16* Key2)
{
    if (*Key1 > *Key2) {
        return 1;
    } else if (*Key1 < *Key2) {
        return -1;
    } else {
        return 0;
    }
}



const u16* MsgCollection::KeyOf (const Msg* Item)
{
    return &Item->MsgNum;
}



const Msg& MsgCollection::GetMsg (u16 ID)
{
    int Index;

    // Search for item with given ID
    if (Search (&ID, Index) == 0) {
        // Not found, set Index to -1
        Index = -1;
    }

    // Check if found
    if (Index == -1) {
        String Msg = FormatStr ("GetMsg: Message #%u not found", ID);
        FAIL (Msg.GetStr ());
    }

    // Return Msg
    return *At (Index);
}


