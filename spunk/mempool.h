/*****************************************************************************/
/*                                                                           */
/*                                 MEMPOOL.H                                 */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
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



#ifndef __MEMPOOL_H
#define __MEMPOOL_H



#include <string.h>

#include "machine.h"
#include "check.h"
#include "object.h"
#include "strmable.h"
#include "stream.h"



/*****************************************************************************/
/*                               class MemPool                               */
/*****************************************************************************/



template<class T>
class MemPool: public Streamable {

protected:
    T*          Buffer;                 // Pointer to buffer memory
    u32         BlockSize;              // blocksize in sizeof (T) units
    u32         Limit;                  // Current memory limit in sizeof (T) units
    u32         Top;                    // Index of first free memory location

    virtual u32 CheckExpand (u32 NewTop);
    // Check if the buffer is big enough to hold NewTop items, if not, expand
    // the buffer. Top is set to NewTop by this function and the old top is
    // returned.

public:
    MemPool (u32 aBlockSize = 1024);
    // Create an (empty) MemPool

    MemPool (T* aBuffer, u32 Size, u32 aBlockSize = 1024);
    // Create a MemPool from aBuffer. Size is the Size of aBuffer in sizeof (T)
    // units and is assumed to be fully used

    MemPool (StreamableInit);
    // Create an empty and uninitialized MemPool

    virtual ~MemPool ();
    // Destruct a MemPool

    // derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    static Streamable* Build ();

    virtual void Clear ();
    // Throw away all data - the pool is empty after calling Clear

    virtual void Use (T* aBuffer, u32 Size);
    // Throw away all current data, and use the given buffer instead. Size is
    // the count of T objects in this buffer. The buffer must be allocated
    // on free store as it is deleted when the destructor is called!

    virtual u32 Alloc (u32 Count = 1);
    // Allocate memory in sizeof (T) chunks

    virtual const T* Adr (u32 Index) const;
    virtual T* Adr (u32 Index);
    // Get the adress of the object Index

    virtual u32 Pos (const T* P);
    // Return the index for the given address

    u32 GetCount () const;
    // Return the count of T items stored

};



template<class T>
MemPool<T>::MemPool (u32 aBlockSize):
    Buffer (NULL),
    BlockSize (aBlockSize),
    Limit (0),
    Top (0)
{
}



template<class T>
MemPool<T>::MemPool (T* aBuffer, u32 Size, u32 aBlockSize):
    Buffer (aBuffer),
    BlockSize (aBlockSize),
    Limit (Size),
    Top (Size)
// Create a MemPool from aBuffer. Size is the Size of aBuffer in sizeof (T)
// units and is assumed to be fully used
{
}



template<class T>
inline MemPool<T>::MemPool (StreamableInit)
{
}



template<class T>
MemPool<T>::~MemPool ()
{
    delete [] Buffer;
}



template<class T>
void MemPool<T>::Clear ()
// Throw away all data - the pool is empty after calling Clear
{
    // Delete the buffer, reset all counters
    delete [] Buffer;
    Top = Limit = 0;
}



template<class T>
void MemPool<T>::Use (T* aBuffer, u32 Size)
// Throw away all current data, and use the given buffer instead. Size is
// the count of T objects in this buffer. The buffer must be allocated
// on free store as it is deleted when the destructor is called!
{
    // Clear the current data
    Clear ();

    // Use the new buffer instead
    Buffer = aBuffer;
    Top = Limit = Size;
}



template<class T>
u32 MemPool<T>::CheckExpand (u32 NewTop)
// Check if the buffer is big enough to hold NewTop items, if not, expand
// the buffer. Top is set to NewTop by this function and the old top is
// returned.
{
    if (NewTop > Limit) {

        // We need to expand the buffer. Save the old buffer
        T* OldBuffer = Buffer;

        // Calculate the new limit and allocate memory for the new buffer
        Limit = ((NewTop + BlockSize - 1) / BlockSize) * BlockSize;
        Buffer = new T [Limit];

        // Copy the old buffer memory to the new one
        memcpy (Buffer, OldBuffer, Top * sizeof (T));

        // Release the old buffer
        delete [] OldBuffer;

    }

    // Adjust Top and return the old Top
    u32 OldTop = Top;
    Top = NewTop;
    return OldTop;
}



template<class T>
void MemPool<T>::Load (Stream& S)
{
    // Read in the size variables
    S >> BlockSize >> Limit >> Top;

    if (Limit > 0) {

        // Allocate buffer memory
        Buffer = new T [Limit];

        // Read in the data
        S.Read (Buffer, Top * sizeof (T));

    }
}



template<class T>
void MemPool<T>::Store (Stream& S) const
{
    // Write out the size variables
    S << BlockSize << Limit << Top;

    // Write the buffer to the file
    S.Write (Buffer, Top * sizeof (T));
}



template<class T>
Streamable* MemPool<T>::Build ()
// Create an empty MemPool instance
{
    return new MemPool<T> (Empty);
}



template<class T>
u32 MemPool<T>::Alloc (u32 Count)
// Allocate memory in sizeof (T) chunks
{
    // Expand the buffer if necessary and return the index
    return CheckExpand (Top + Count);
}



template<class T>
const T* MemPool<T>::Adr (u32 Index) const
// Get the adress of the object Index
{
    CHECK (Index < Top);
    return Buffer + Index;
}



template<class T>
T* MemPool<T>::Adr (u32 Index)
// Get the adress of the object Index
{
    CHECK (Index < Top);
    return Buffer + Index;
}



template<class T>
u32 MemPool<T>::Pos (const T* P)
// Return the index for the given address
{
    return P - Buffer;
}



template<class T>
inline u32 MemPool<T>::GetCount () const
// Return the count of T items stored
{
    return Top;
}



// End of MEMPOOL.H

#endif

