/*****************************************************************************/
/*                                                                           */
/*                                  RESCOLL.H                                */
/*                                                                           */
/* (C) 1993,94  Ullrich von Bassewitz                                        */
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



#ifndef _RESCOLL_H
#define _RESCOLL_H



#include "strmable.h"
#include "stream.h"
#include "str.h"
#include "strcoll.h"



/*****************************************************************************/
/*                            class ResourceIndex                            */
/*****************************************************************************/



class ResourceIndex : public Streamable {

    friend class ResourceCollection;
    friend class ResourceFile;

private:
    String      Name;
    u32         Offset;
    u32         Size;

protected:
    ResourceIndex (StreamableInit X);
    // Build constructor

public:
    ResourceIndex (const String & aName);

    // Derived from class Streamable.
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // New functions
    const String& GetName () const;

};



inline ResourceIndex::ResourceIndex (const String& aName) :
    Name (aName)
{
}



inline const String& ResourceIndex::GetName () const
{
    return Name;
}



/*****************************************************************************/
/*                         class ResourceCollection                          */
/*****************************************************************************/



class ResourceCollection: public SortedCollection<ResourceIndex, String> {

protected:
    // Derived from class Collection
    virtual void* GetItem (Stream& S);
    virtual void PutItem (Stream& S, void* Item) const;

    // Derived from class SortedCollection
    virtual int Compare (const String* Key1, const String* Key2);
    virtual const String* KeyOf (const ResourceIndex* Item);


public:
    ResourceCollection (StreamableInit X);
    ResourceCollection (int aLimit, int aDelta);

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable * Build ();

};



inline ResourceCollection::ResourceCollection (int aLimit, int aDelta) :
    SortedCollection<ResourceIndex, String> (aLimit, aDelta, 1)
{
}



// End of RESCOLL.H

#endif

