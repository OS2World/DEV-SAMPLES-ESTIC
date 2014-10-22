/*****************************************************************************/
/*									     */
/*				   RESOURCE.CC				     */
/*									     */
/* (C) 1993,94	Ullrich von Bassewitz					     */
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



#include "machine.h"
#include "object.h"
#include "strmable.h"
#include "stream.h"
#include "nullstrm.h"
#include "rescoll.h"
#include "resource.h"



/*****************************************************************************/
/*			     struct ResFileHeader			     */
/*****************************************************************************/



// A valid ResourceFile must have this signature
static const u32 ResFileSig = 0x616E6E41;



// Header of a resource file
struct ResFileHeader {
    u32		Sig;			// Signature
    u32		Size;			// Size including header
    u32		IndexPos;		// Position of index

    friend Stream& operator << (Stream& S, const ResFileHeader& Header);
    friend Stream& operator >> (Stream& S, ResFileHeader& Header);
    // Reading and writing from/to a stream

};



Stream& operator << (Stream& S, const ResFileHeader& Header)
{
    S << Header.Sig << Header.Size << Header.IndexPos;
    return S;
}



Stream& operator >> (Stream& S, ResFileHeader& Header)
{
    S >> Header.Sig >> Header.Size >> Header.IndexPos;
    return S;
}



/*****************************************************************************/
/*			      class ResourceFile			     */
/*****************************************************************************/



ResourceFile::ResourceFile (Stream* aStream) :
    S (aStream),
    Status (0),
    Dirty (0),
    BasePos (0),
    IndexPos (0),
    Index (NULL)
{
    // Stream cannot be NULL
    PRECONDITION (S != NULL);

    // Get the current stream position
    BasePos = S->GetPos ();

    // If the current position is at the end of the stream, this is a new
    // resource
    if (BasePos == S->GetSize ()) {

	// Create a new index collection
	Index = new ResourceCollection (40, 20);

	// The index position is after the header
	IndexPos = BasePos + sizeof (ResFileHeader);

	// A new ResourceFile is always dirty, because the index is not
	// written
	Dirty = 1;

    } else {

	// Read the Header of the file
	ResFileHeader Header;
	*S >> Header;

	// Check if the header is correct and the stream status is ok
	if (S->GetStatus () != stOk) {
	    Status = reStreamError;
	    return;
	}
	if (Header.Sig != ResFileSig) {
	    Status = reNoResource;
	    return;
	}

	// Get the position of the index from the header
	IndexPos = Header.IndexPos;

	// Read the index
	S->Seek (IndexPos);
	Index = (ResourceCollection*) S->Get ();

	if (!Index) {
	    Status = reNoIndex;
	    return;
	}
    }

    // Check for any stream errors (maybe a use for an exception)
    if (S->GetStatus () != stOk) {
	Status = reStreamError;
    }
}



ResourceFile::~ResourceFile ()
{
    // Write changes to the stream (ignore errors)
    Flush ();

    // Delete the index
    delete Index;

    // Delete the stream
    delete S;
}



void ResourceFile::Delete (const String& Key)
{
    int I;

    // Search the entry
    if (Index->Search (&Key, I)) {

	// Found, delete it in the index
	Index->AtDelete (I);

	// ResourceFile is dirty now
	Dirty = 1;

    }

}



void ResourceFile::Flush ()
{
    // No work if not dirty
    if (!IsDirty ()) {
	return;
    }

    // Seek to the index position and write the index
    S->Seek (IndexPos);
    S->Put (Index);

    // Create a new header
    ResFileHeader Header;
    Header.Sig		= ResFileSig;
    Header.Size		= S->GetPos () - BasePos;
    Header.IndexPos	= IndexPos;

    // Write the header
    S->Seek (BasePos);
    *S << Header;

    // ResourceFile is clean now
    Dirty = 0;

}



Streamable* ResourceFile::Get (int Idx)
// Get by index
{
    PRECONDITION (Idx >= 0 && Idx < Index->GetCount ());

    // Get a pointer to the descriptor
    ResourceIndex* P = Index->At (Idx);

    // Load the object
    S->Seek (P->Offset + BasePos);
    Streamable* O = S->Get ();

    // Check the stream status
    if (S->GetStatus () != stOk) {
	Status = reStreamError;
	O = NULL;
    }

    // Return the new instance
    return O;
}



Streamable* ResourceFile::Get (const String& Key)
{
    int I;

    // Search for the key
    if (Index->Search (&Key, I) == 0) {

	// Not found, return NULL
	return NULL;

    }

    // Now get the object by index
    return Get (I);

}



const String& ResourceFile::KeyAt (int I)
{
    // Check the given index
    PRECONDITION (I >= 0 && I < Index->GetCount ());

    // Return a reference to the the name (index is ok now)
    return Index->At (I)->Name;
}



int ResourceFile::FindKey (const String& Key)
// Try to find the resource with the given name and return the index.
// Returns -1 if the key was not found.
{
    int I;

    // Search for the key
    if (Index->Search (&Key, I) == 0) {
	// Not found
	return -1;
    }

    // Found, return the index
    return I;
}



void ResourceFile::Put (const Streamable* O, const String& Key)
{
    int I;
    ResourceIndex* P;

    // Check if an entry with this key already exists
    int Found = Index->Search (&Key, I);

    // Test if the size of the object has changed. If the new size is less
    // or equal than the old size, we can store the object at the same
    // position. If the object has grown, it is deleted at the old position
    // and the Found flag is reset, so that the object is stored at the end
    // of the stream.
    if (Found) {

	// Get a pointer to the entry
	P = Index->At (I);

	// Check the new size - the global namespace override handles a bug
	// in gcc 2.5.8 and is not needed by other compilers
	size_t NewSize = ::GetSize (*O);
	if (NewSize <= P->Size) {

	    // The new size is less or equal than the old. Write the object
	    // to the old position
	    S->Seek (P->Offset);
	    S->Put (O);

	    // Remember the new size
	    P->Size = NewSize;

	} else {

	    // The object has grown. Delete the entry from the index and
	    // reset the Found flag
	    Index->AtDelete (I);
	    Found = 0;

	}

    }

    if (!Found) {

	// Allocate a new index entry
	P = new ResourceIndex (Key);

	// Write the object at the end of the stream
	S->Seek (IndexPos);
	S->Put (O);

	// Store size and position of the object
	u32 CurPos = S->GetPos ();
	P->Size    = CurPos - IndexPos;
	P->Offset  = IndexPos - BasePos;

	// New IndexPos is at the end of the stream
	IndexPos = CurPos;

	// Insert the entry in the index
	Index->Insert (P);

    }

    // The resource file has been modified
    Dirty = 1;

}



Stream* ResourceFile::SwitchTo (Stream* aStream, int PackRes)
// Copy the resource file into the stream aStream. If PackRes is not equal
// to zero, try to compress the resource file by not copying unused areas.
// After calling SwitchTo, the ResourceFile uses the new stream aStream,
// the old one is returned (and should be deleted).
{
    // Check parameters
    PRECONDITION (aStream != 0);

    // If the new stream is in an error condition, there is not much we can do.
    // Set the status code and return the given (instead of the old) stream.
    if (aStream->GetStatus () != stOk) {
	Status = reStreamError;
	return aStream;
    }

    // Now flush all unwritten stuff
    Flush ();

    // Remember the old byte position and use the new one
    u32 OldBasePos = BasePos;
    BasePos = aStream->GetPos ();

    // Handle requests according to PackRes
    if (!PackRes) {

	// Calculate how many bytes to copy
	u32 BytesToCopy = IndexPos - OldBasePos;

	// Just copy all data
	S->Seek (OldBasePos);
	aStream->CopyFrom (*S, BytesToCopy);

    } else {

	// Skip the header
	aStream->Seek (BasePos + sizeof (ResFileHeader));

	// Current position is unknown
	u32 CurPos = 0xFFFFFFFF;

	// Copy each and every object
	u32 NewPos;
	ResourceIndex* P;
	int Count = Index->GetCount ();
	for (int I = 0; I < Count; I++) {

	    // Get a pointer to the directory entry
	    P = Index->At (I);

	    // Calculate the position of the object
	    NewPos = P->Offset + OldBasePos;

	    // Seek to the object position if needed
	    if (NewPos != CurPos) {
		S->Seek (NewPos);
	    }

	    // Store the new object offset (in the new stream!)
	    P->Offset = aStream->GetPos () - BasePos;

	    // Copy the object data
	    aStream->CopyFrom (*S, P->Size);

	    // Remember the current position in the old stream
	    CurPos = NewPos + P->Size;

	}

    }

    // Remember the end position
    IndexPos = aStream->GetPos ();

    // Mark the resource file as dirty (the index is still unwritten)
    Dirty = 1;

    // Switch to the new stream and return the old one
    Stream* Res = S;
    S = aStream;
    return Res;
}



int ResourceFile::ObjHasPos (ResourceIndex* P, void* Data)
{
    return (P->Offset == * ((u32 *) Data));
}



int ResourceFile::NotMarked (ResourceIndex* P, void*)
{
    return (P->Size & 0x80000000) == 0;
}



int ResourceFile::ClearMark (ResourceIndex* P, void*)
{
    P->Size &= ~0x80000000;
    return 0;
}



void ResourceFile::Pack ()
// Try to compress the resource file. When an object is deleted from the
// reource file, the space is not reused. This means that by repeatedly
// deleting and storing the same object, the resource file will grow. This
// has no negative impacts on performance, but on the use of memory or disk
// space.
//
// Packing is done by SwitchTo. To avoid unecessary copying of the resource
// file, Pack checks if packing is really needed, that is, unused space exists
// inside the resource file.
//
// Algorithm for doing that:
//
//  0. Current position is base position.
//  1. Check if an object is stored at the current position. If no, goto 3,
//     if yes, goto 2.
//  2. Mark the found object by setting bit 31 of the variable Size in the
//     index entry. Note: This limits the size of a single object to 2GB.
//     Add the size of the object (beware: the high bit is a flag!) to the
//     current position and continue with 1.
//  3. Are all objects marked? If yes, goto 4, if no, goto 5.
//  4. Store the current position into IndexPos (new objects will be added
//     at this position). Unmark all objects. Goto 7.
//  5. Unmark all objects.
//  6. Create a new stream and use SwitchTo to copy all data into the new
//     stream. Reverse this process and truncate the current stream at
//     IndexPos. Delete the temporary stream. Goto 7.
//  7. Done.
//
{
    // Set up startiing position
    u32 CurPos = sizeof (ResFileHeader);

    // Now check all objects
    ResourceIndex* ObjWithPos;
    do {
	ObjWithPos = Index->Traverse (1, ObjHasPos, &CurPos);
	if (ObjWithPos != NULL) {
	    // There is an object with this position, update CurPos and
	    // mark the object
	    CurPos += ObjWithPos->Size;
	    ObjWithPos->Size |= 0x80000000;
	}
    } while (ObjWithPos != NULL);

    // The chain of continous objects in the file is broken. Check if all
    // objects are marked.
    if (Index->Traverse (1, NotMarked) == NULL) {

	// All objects are marked, no packing, unmark
	Index->Traverse (1, ClearMark);

	// New Index position is at CurPos + BasePos
	CurPos += BasePos;

	// If the new and old index positions are not equal, the
	// file has to be marked as modified.
	if (CurPos != IndexPos) {
	    Dirty = 1;
	}

	// Now remember the new index position
	IndexPos = CurPos;

    } else {

	// File needs packing, clear marks
	Index->Traverse (1, ClearMark);

	// Pack the file
	u32 OldBasePos = BasePos;
	FileStream* NewStream = new FileStream;
	Stream* OldStream = SwitchTo (NewStream, 1);
	OldStream->Seek (OldBasePos);
	NewStream->Seek (0);
	delete SwitchTo (OldStream);

	// Truncate the stream at the current IndexPos
	S->Seek (IndexPos);
	S->Truncate ();

    }

}




