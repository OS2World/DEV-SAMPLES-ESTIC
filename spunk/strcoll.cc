/*****************************************************************************/
/*                                                                           */
/*                                 STRCOLL.CC                                */
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



#include "strcoll.h"
#include "streamid.h"



// Register class StringCollection if there are references in this module
LINK(StringCollection, ID_StringCollection);



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<String>;
template class SortedCollection<String, String>;
#endif



/*****************************************************************************/
/*                          class StringCollection                           */
/*****************************************************************************/



Streamable* StringCollection::Build ()
{
    return new StringCollection (Empty);
}



void StringCollection::PutItem (Stream& S, void* Item) const
{
    S.Put ((String*) Item);
}



void* StringCollection::GetItem (Stream& S)
{
    return (void*) S.Get ();
}



u16 StringCollection::StreamableID () const
{
    return ID_StringCollection;
}



int StringCollection::Compare (const String* Key1, const String* Key2)
{
    return ::Compare (*Key1, *Key2);
}



