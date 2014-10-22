/*****************************************************************************/
/*									     */
/*				    RESCOLL.CC				     */
/*									     */
/* (C) 1993-95	Ullrich von Bassewitz					     */
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



#include <string.h>

#include "machine.h"
#include "stream.h"
#include "rescoll.h"
#include "streamid.h"



// Register classes
LINK (ResourceIndex, ID_ResourceIndex);
LINK (ResourceCollection, ID_ResourceCollection);



/*****************************************************************************/
/*			Explicit template instantiation			     */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<ResourceIndex>;
template class SortedCollection<ResourceIndex, String>;
#endif



/*****************************************************************************/
/*			      class ResourceIndex			     */
/*****************************************************************************/



ResourceIndex::ResourceIndex (StreamableInit) :
    Name (Empty)
{
}



void ResourceIndex::Load (Stream& S)
{
    S >> Name >> Offset >> Size;
}



void ResourceIndex::Store (Stream& S) const
{
    S << Name << Offset << Size;
}



u16 ResourceIndex::StreamableID () const
{
    return ID_ResourceIndex;
}



Streamable* ResourceIndex::Build ()
{
    return new ResourceIndex (Empty);
}



/*****************************************************************************/
/*			   class ResourceCollection			     */
/*****************************************************************************/



ResourceCollection::ResourceCollection (StreamableInit X) :
    SortedCollection<ResourceIndex, String> (X)
{
}



u16 ResourceCollection::StreamableID () const
{
    return ID_ResourceCollection;
}



Streamable* ResourceCollection::Build ()
{
    return new ResourceCollection (Empty);
}



void* ResourceCollection::GetItem (Stream &S)
{
    return (void*) S.Get ();
}



void ResourceCollection::PutItem (Stream& S, void* O) const
{
    S.Put ((ResourceIndex*) O);
}



int ResourceCollection::Compare (const String* Key1, const String* Key2)
{
    return ::Compare (*Key1, *Key2);
}



const String* ResourceCollection::KeyOf (const ResourceIndex* Item)
{
    return &Item->Name;
}




