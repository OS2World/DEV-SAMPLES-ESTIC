/*****************************************************************************/
/*									     */
/*				   MEMSTRM.H				     */
/*									     */
/* (C) 1993-95	Ullrich von Bassewitz					     */
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



#ifndef __MEMSTRM_H
#define __MEMSTRM_H



#include <stddef.h>

#include "stream.h"



/*****************************************************************************/
/*			      class MemoryStream			     */
/*****************************************************************************/



class MemoryStream : public Stream {

protected:
    char*	Memory;
    u32		Block;
    u32		Limit;
    u32		Size;
    u32		CurPos;

    void Resize (u32 NewSize);

public:
    MemoryStream (u32 BlockSize = 1024);
    MemoryStream (StreamableInit);
    ~MemoryStream ();

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class Stream
    virtual u32 GetPos ();
    virtual u32 GetSize ();
    virtual void Read (void* Buf, size_t Count);
    virtual void Seek (unsigned long Pos);
    virtual void Truncate ();
    virtual void Write (const void* Buf, size_t Count);

};



inline MemoryStream::MemoryStream (StreamableInit)
{
}



/*****************************************************************************/
/*				   Duplicate				     */
/*****************************************************************************/



// Function to duplicate an object
template <class T>
T* Duplicate (T* S)
{
    MemoryStream MemStr;

    // Write the object into the memory stream
    MemStr.Put (S);

    // Seek to the start of the stream and retrieve a copy
    MemStr.Seek (0);

    return (T*) MemStr.Get ();
}



// End of MEMSTRM.H

#endif

