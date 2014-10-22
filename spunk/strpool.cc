/*****************************************************************************/
/*                                                                           */
/*                                 STRPOOL.CC                                */
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



#include <string.h>

#include "machine.h"
#include "object.h"
#include "streamid.h"
#include "strmable.h"
#include "coll.h"
#include "national.h"
#include "str.h"
#include "mempool.h"
#include "strpool.h"



// Register class StringPool
LINK(StringPool, ID_StringPool);



/*****************************************************************************/
/*                                   Types                                   */
/*****************************************************************************/



struct Entry {
    u32 Str;            // Character offset of the string
    u32 Num;            // String number
};



/*****************************************************************************/
/*                         class StringPoolEntryColl                         */
/*****************************************************************************/



// StringPoolEntryColl will set ShouldDelete to 1 and will therefore delete
// all created Entry structs on deletion.



class StringPoolEntryColl: public SortedCollection<Entry, char> {

private:
    MemPool<char> const& Pool;

protected:
    virtual int Compare (const char* Key1, const char* Key2);
    virtual const char* KeyOf (const Entry* Item);

public:
    StringPoolEntryColl (MemPool<char> const& aPool, int aLimit, int aDelta);

};



StringPoolEntryColl::StringPoolEntryColl (MemPool<char> const& aPool,
                                          int aLimit, int aDelta):
    SortedCollection<Entry, char> (aLimit, aDelta, 1),
    Pool (aPool)
{
}



int StringPoolEntryColl::Compare (const char* Key1, const char* Key2)
{
    return NLSCmpStr (Key1, Key2);
}



const char* StringPoolEntryColl::KeyOf (const Entry* Item)
{
    return Pool.Adr (Item->Str);
}



/*****************************************************************************/
/*                        class StringPoolStringColl                         */
/*****************************************************************************/



// StringPoolStringColl has ShouldDelete set to zero and will not touch the
// Entry structs on deletion.



class StringPoolStringColl: public Collection<Entry> {

protected:
    virtual void FreeItem (void*);
    virtual void* GetItem (Stream&);
    virtual void PutItem (Stream&, void*) const;

public:
    StringPoolStringColl (int aLimit, int aDelta);

};



StringPoolStringColl::StringPoolStringColl (int aLimit, int aDelta):
    Collection<Entry> (aLimit, aDelta, 0)
{
}



void StringPoolStringColl::FreeItem (void*)
{
    // Cannot be called
    FAIL ("Call to StringPoolStringColl::FreeItem not allowed");
}



void* StringPoolStringColl::GetItem (Stream&)
{
    // Cannot be called
    FAIL ("Call to StringPoolStringColl::GetItem not allowed");
    return NULL;
}



void StringPoolStringColl::PutItem (Stream&, void*) const
{
    // Cannot be called
    FAIL ("Call to StringPoolStringColl::PutItem not allowed");
}



/*****************************************************************************/
/*                              class StringPool                             */
/*****************************************************************************/



StringPool::StringPool (u32 BlockSize):
    Pool (BlockSize),
    EColl (new StringPoolEntryColl (Pool, BlockSize / 20, BlockSize / 20)),
    SColl (new StringPoolStringColl (BlockSize / 20, BlockSize / 20)),
    CurIndex (0)
// Construct a StringPool
{
    PRECONDITION (BlockSize > 0);
}



StringPool::StringPool (char* Buffer, u32 Size, u32 BlockSize):
    Pool (Buffer, Size, BlockSize)
// Create a StringPool from the strings in Buffer. Size is the size of Buffer,
// Buffer is assumed to be fully used.
{
    PRECONDITION (Buffer != NULL && Size > 0 && BlockSize > 0);

    // Estimate Limit and Delta for the collections
    u32 Limit = Size / 20;              // Estimated count of strings
    u32 Delta = Limit > 100? Limit / 10 : 10;

    // Create both collections
    EColl = new StringPoolEntryColl (Pool, Limit, Delta);
    SColl = new StringPoolStringColl (Limit, Delta);

    // Traverse the buffer memory, count the strings
    u32 Pos  = 0;
    CurIndex = 0;
    while (Pos < Size) {

        // Create a new Entry for the current string
        Entry* E = new Entry;
        E->Str = Pos;
        E->Num = CurIndex;

        // Insert the Entry into both collections (only one of them is
        // responsible for deleting the entries)
        SColl->Insert (E);
        EColl->Insert (E);

        // Next string
        unsigned Len = strlen (Buffer) + 1;
        Pos += Len;
        Buffer += Len;
        CurIndex++;
    }
}



StringPool::StringPool (StreamableInit):
    Pool (Empty)
{
}



StringPool::~StringPool ()
// Destruct a StringPool object
{
    // One of the collections has ShouldDelete set to 1 and will delete all
    // the Entry structs
    delete SColl;
    delete EColl;
}



void StringPool::Load (Stream& S)
{
    // Load the pool
    Pool.Load (S);

    // Read the index (the index is equal to the count of strings stored in
    // Pool) and recreate the index
    S >> CurIndex;

    // Create new collections
    EColl = new StringPoolEntryColl (Pool, CurIndex, CurIndex / 10);
    SColl = new StringPoolStringColl (CurIndex, CurIndex / 10);

    // Get a pointer to the first string in the pool.
    char* Str = Pool.Adr (0);

    // Traverse through all strings stored in the pool and construct the index
    u32 Pos = 0;
    for (u32 I = 0; I < CurIndex; I++) {

        // Create a new entry
        Entry* E = new Entry;
        E->Str = Pos;
        E->Num = I;

        // Insert the entry in both collections
        EColl->Insert (E);
        SColl->Insert (E);

        // Point to the next string
        unsigned Len = strlen (Str) + 1;
        Str += Len;
        Pos += Len;
    }
}



void StringPool::Store (Stream& S) const
{
    // Store the pool
    Pool.Store (S);

    // Both index collections are not stored, they are created by Load
    S << CurIndex;
}



Streamable* StringPool::Build ()
{
    return new StringPool (Empty);
}



u16 StringPool::StreamableID () const
{
    return ID_StringPool;
}



void StringPool::Clear ()
// Clear all entries in the pool
{
    // Delete the entries of both collections, then delete the memory pool
    SColl->DeleteAll ();
    EColl->DeleteAll ();
    Pool.Clear ();
    CurIndex = 0;
}



void StringPool::Use (char* Buffer, u32 Size)
// Throw the current data away and use the strings in  the given buffer
// instead. Size is the size of the buffer in characters, buffer must
// be allocated on the heap, it will be deleted using delete[] if
// necessary.
{
    // Drop current data
    Clear ();

    // Tell the memory pool that we have new data
    Pool.Use (Buffer, Size);

    // Estimate Limit and Delta for the collections
    u32 Limit = Size / 20;              // Estimated count of strings
    u32 Delta = Limit > 100? Limit / 10 : 10;

    // Create both collections
    EColl = new StringPoolEntryColl (Pool, Limit, Delta);
    SColl = new StringPoolStringColl (Limit, Delta);

    // Traverse the buffer memory, count the strings
    u32 Pos  = 0;
    CurIndex = 0;
    while (Pos < Size) {

        // Create a new Entry for the current string
        Entry* E = new Entry;
        E->Str = Pos;
        E->Num = CurIndex;

        // Insert the Entry into both collections (only one of them is
        // responsible for deleting the entries)
        SColl->Insert (E);
        EColl->Insert (E);

        // Next string
        unsigned Len = strlen (Buffer) + 1;
        Pos += Len;
        Buffer += Len;
        CurIndex++;
    }
}



const char* StringPool::operator [] (u32 Index) const
{
    return Pool.Adr (SColl->At (Index) -> Str);
}



const char* StringPool::At (u32 Index) const
{
    return Pool.Adr (SColl->At (Index) -> Str);
}



u32 StringPool::Insert (const char* S)
{
    // Try to find the string in the pool
    int Index;
    if (EColl->Search (S, Index)) {

        // Found! Return the string number
        return EColl->At (Index) -> Num;

    } else {

        // Not found - add the string to the pool

        // Calculate the string length
        unsigned Len = strlen (S);

        // Create a new entry and set the index number
        Entry* E = new Entry;
        E->Num = CurIndex++;

        // Allocate memory from the pool
        E->Str = Pool.Alloc (Len + 1);

        // Copy the string to the pool
        memcpy (Pool.Adr (E->Str), S, Len + 1);

        // Insert the pointers into both collections
        EColl->Insert (E);
        SColl->Insert (E);

        // Return the index of the stored string
        return CurIndex - 1;

    }

}



u32 StringPool::Find (const char* S)
// Try to find the string in the pool, FAIL if the string is not found,
// otherwise return the index
{
    // Try to find the string in the pool
    int Index;
    int Result = EColl->Search (S, Index);
    CHECK (Result != 0);

    // Found! Return the string number
    return EColl->At (Index) -> Num;
}



