/*****************************************************************************/
/*                                                                           */
/*                                 STRPOOL.H                                 */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
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



#ifndef __STRPOOL_H
#define __STRPOOL_H



#include "machine.h"
#include "object.h"
#include "strmable.h"
#include "coll.h"
#include "str.h"
#include "mempool.h"



/*****************************************************************************/
/*                              class StringPool                             */
/*****************************************************************************/



// Forwards
class StringPoolEntryColl;
class StringPoolStringColl;


class StringPool: public Streamable {

private:
    MemPool<char>               Pool;       // The buffer pool
    class StringPoolEntryColl*  EColl;      // cont: Entry, sorted by string
    class StringPoolStringColl* SColl;      // cont: String, sorted by number

    u32                         CurIndex;   // Current index of string

public:
    StringPool (u32 BlockSize = 1024);
    // Construct a StringPool

    StringPool (char* Buffer, u32 Size, u32 BlockSize = 1024);
    // Create a StringPool from the strings in Buffer. Size is the size of
    // Buffer, Buffer is assumed to be fully used.

    StringPool (StreamableInit);
    // Construct an empty and uninitialized StringPool

    virtual ~StringPool ();
    // Destruct a StringPool object

    // derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    static Streamable* Build ();
    virtual u16 StreamableID () const;

    void Clear ();
    // Clear all entries in the pool

    void Use (char* Buffer, u32 Size);
    // Throw the current data away and use the strings in  the given buffer
    // instead. Size is the size of the buffer in characters, buffer must
    // be allocated on the heap, it will be deleted using delete[] if
    // necessary.

    // Insert strings into the pool, return the index
    u32 Insert (const char*);
    u32 Insert (const String&);

    u32 Find (const char* S);
    // Try to find the string in the pool, FAIL if the string is not found,
    // otherwise return the index

    // Retrieve a string from the pool
    const char* operator [] (u32 Index) const;
    const char* At (u32 Index) const;

    // Return a pointer to the pool
    const char* GetPoolBuffer () const;

    // Return the size of the pool in chars
    u32 GetPoolSize () const;

};



inline u32 StringPool::Insert (const String& S)
{
    return Insert (S.GetStr ());
}



inline const char* StringPool::GetPoolBuffer () const
// Return a pointer to the pool
{
    return Pool.Adr (0);
}



inline u32 StringPool::GetPoolSize () const
// Return the size of the pool in chars
{
    return Pool.GetCount ();
}



// End of STRPOOL.H

#endif
