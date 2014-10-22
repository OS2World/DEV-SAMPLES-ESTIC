/*****************************************************************************/
/*									     */
/*				   RESOURCE.H				     */
/*									     */
/* (C) 1993-96	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef __RESOURCE_H
#define __RESOURCE_H



#include "machine.h"
#include "object.h"
#include "strmable.h"
#include "stream.h"
#include "rescoll.h"



// Error values for status (valid only after calling the constructor)
const unsigned reOk			= 0;		// Status is ok
const unsigned reStreamError		= 1;		// Stream error
const unsigned reNoResource		= 2;		// No resource file
const unsigned reNoIndex		= 3;		// No index present



/*****************************************************************************/
/*			      class ResourceFile			     */
/*****************************************************************************/




class ResourceFile : public Object {

    friend class ResEditApp;			// Resource editor is a friend

private:
    // Iterator "action" functions
    static int ObjHasPos (ResourceIndex*, void*);
    static int NotMarked (ResourceIndex*, void*);
    static int ClearMark (ResourceIndex*, void*);

protected:
    Stream*		S;			// Stream to store data in
    unsigned		Status;			// Status after init
    int			Dirty;
    u32			BasePos;		// Base position in stream
    u32			IndexPos;		// Index position in the stream
    ResourceCollection* Index;			// Stream index


public:
    ResourceFile (Stream* aStream);
    // Create a new ResourceFile with the given stream. If there is a valid
    // signature at the beginning of the stream, an existing ResourceFile is
    // assumed, otherwise the ResourceFile is created using the given stream.

    virtual ~ResourceFile ();
    // Write out all dirty buffers, destruct the ResourceFile object

    int GetCount () const;
    // Get the count of stored objects

    void Delete (const String& Key);
    // Delete an object with the given key

    void Flush ();
    // Flush the ResourceFile (write the index) if it is dirty

    Streamable* Get (const String& Key);
    // Get a copy of a stored object by key

    Streamable* Get (int I);
    // Get a stored object by index

    ResourceCollection* GetIndex () const;
    // Return the resource index

    int IsDirty () const;
    // Return true if the ResourceFile has unwritten data

    const String& KeyAt (int I);
    // Return the key of the object at the given position

    int FindKey (const String& Key);
    // Try to find the resource with the given name and return the index.
    // Returns -1 if the key was not found.

    void Put (const Streamable* O, const String& Key);
    void Put (const Streamable& O, const String& Key);
    // Store an object into the resource deleting an already existing object
    // with the same name

    Stream* SwitchTo (Stream* aStream, int Pack = 0);
    // Switch to a new stream, eventually packing the resource

    void Pack ();
    // Pack the ResourceFile if needed

    const Stream& GetStream () const;
    // Get a reference to the stream used

    unsigned GetStatus () const;
    // Return the ResourceFile status

};



inline int ResourceFile::GetCount () const
{
    return Index->GetCount ();
}



inline ResourceCollection* ResourceFile::GetIndex () const
{
    return Index;
}



inline void ResourceFile::Put (const Streamable& O, const String& Key)
{
    Put (&O, Key);
}



inline int ResourceFile::IsDirty () const
{
    return Dirty;
}



inline const Stream& ResourceFile::GetStream () const
{
    return *S;
}



inline unsigned ResourceFile::GetStatus () const
{
    return Status;
}



// End of RESOURCE.H

#endif


