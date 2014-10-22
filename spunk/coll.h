/*****************************************************************************/
/*									     */
/*				     COLL.H				     */
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



#ifndef _COLL_H
#define _COLL_H


#include <stdlib.h>
#include <iostream.h>

#include "check.h"
#include "object.h"
#include "strmable.h"
#include "stream.h"



static const coIndexError	= 1;		// Index out of range
static const coOverflowError	= 2;		// Collection overflow



// max number of elements in a collection
#ifdef DOS
static const int MaxCollectionSize = 0x4000;
#else
// real limit is memory, so use any value big enough
static const int MaxCollectionSize = 0x400000L;
#endif



// Pointer to error routine
extern void (*CollectionError) (int);


// Forwards for classes
class Stream;
class Streamable;



/*****************************************************************************/
/*			       class _Collection			     */
/*****************************************************************************/



// This is a base class for the template class Collection that has full
// functionality but uses void pointers instead of typed pointers. The
// implementation of class Collection consists merely of inline functions
// calling the derived member functions and casting when necessary. This will
// hopefully help to decrease program size.


class _Collection : public Streamable {

protected:
    void** Items;		// Pointer to Items
    int Count;			// Item count
    int Limit;			// Upper limit for Count
    int Delta;			// increase when growing

public:
    int ShouldDelete;		// If true, items are owned by the collection


protected:
    virtual void FreeItem (void* Item);
    virtual void PutItem (Stream& S, void* Item) const;
    virtual void* GetItem (Stream& S);

    // Allocate and deallocate item space
    virtual void** AllocItemSpace (int ItemCount);
    virtual void FreeItemSpace (void** Space);


public:
    _Collection (int aLimit, int aDelta, int aShouldDelete = 0);
    _Collection (StreamableInit);
    virtual ~_Collection ();

    // derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;

    // New members
    void* At (int Index);
    const void* At (int Index) const;
    void AtDelete (int Index);
    void AtInsert (int Index, void* Item);
    void AtReplace (int Index, void* Item);
    void Delete (void* Item);
    void DeleteAll ();
    int GetCount () const;
    void* Traverse (int Forward, int (*Func) (void*, void*), void* UserData = NULL);
    int IndexOf (const void* Item);
    void Insert (void* Item);
    int IsEmpty ();
    void Pack ();
    void SetLimit (int aLimit);
};



inline int _Collection::GetCount () const
{
    // This is not a virtual function - calling is direct and not via the
    // virtual table pointer. So we can check safely this against NULL to
    // allow empty collections stored as NULL pointers
    return this ? Count : 0;
}



inline int _Collection::IsEmpty ()
{
    return (GetCount () == 0);
}



/*****************************************************************************/
/*			   template class Collection			     */
/*****************************************************************************/



template <class T>
class Collection : public _Collection {

protected:
    virtual void FreeItem (void* Item);

public:
    Collection (int aLimit, int aDelta, int aSouldDelete = 0);
    Collection (StreamableInit);
    virtual ~Collection ();

    // derived from class Streamable
    virtual void Load (Stream&);
    static Streamable* Build ();

    virtual T* At (int Index);
    // Return a pointer to the item at position Index.

    virtual const T* At (int Index) const;
    // Return a pointer to the item at position Index.

    virtual void AtInsert (int Index, T* Item);
    // Insert an item at the given position.

    virtual void AtReplace (int Index, T* Item);
    // Replace the item at the given position by a new one. If ShouldDelete
    // is true, FreeItem is also called for the replaced item.

    virtual void Delete (T* Item);
    // Delete a specific item. The entry is replaced by a NULL pointer, if
    // ShouldDelete is true, FreeItem is called in addition to that.

    virtual T* Traverse (int Forward, int (*Func) (T*, void*), void* UserData = NULL);
    // Traverse through the collection, calling Func for every item, giving
    // a pointer to the item and a user supplied pointer as parameters. If
    // Func returns a value other than zero, traversing stops.

    virtual int IndexOf (const T* Item);
    // Return the index of the given item in the collection.

    virtual void Insert (T* Item);
    // Insert an item into the collection

    T* operator [] (int Index);
    const T* operator [] (int Index) const;
    // Synonym for At (Index)

};



template <class T>
inline Collection<T>::Collection (StreamableInit):
    _Collection (Empty)
{
}



template <class T>
inline Collection<T>::Collection (int aLimit, int aDelta, int aShouldDelete):
    _Collection (aLimit, aDelta, aShouldDelete)
{
}



template <class T>
Collection<T>::~Collection ()
{
    // Delete all items here because ~_Collection will not do that (it can't,
    // because _Collection::FreeItem is non functionable).
    DeleteAll ();
}



template <class T>
inline T* Collection<T>::At (int Index)
{
    return (T*) _Collection::At (Index);
}



template <class T>
inline const T* Collection<T>::At (int Index) const
{
    return (T*) _Collection::At (Index);
}



template <class T>
inline void Collection<T>::AtInsert (int Index, T* Item)
{
    _Collection::AtInsert (Index, (void*) Item);
}



template <class T>
inline void Collection<T>::AtReplace (int Index, T* Item)
{
    _Collection::AtReplace (Index, (void*) Item);
}



template <class T>
inline void Collection<T>::Delete (T* Item)
{
    _Collection::Delete ((void*) Item);
}



template <class T>
inline T* Collection<T>::Traverse (int Forward,
				   int (*Func) (T*, void*),
				   void* UserData)
{
    typedef int (*UntypedFunc) (void*, void*);
    return (T*) _Collection::Traverse (Forward, (UntypedFunc) Func, UserData);
}



template <class T>
void Collection<T>::FreeItem (void* Item)
{
    delete (T*) Item;
}



template <class T>
inline int Collection<T>::IndexOf (const T* Item)
{
    return _Collection::IndexOf ((void*) Item);
}



template <class T>
inline void Collection<T>::Insert (T* Item)
{
    _Collection::Insert ((void*) Item);
}



template <class T>
void Collection<T>::Load (Stream& S)
// Load the collection from a stream. This function will *not* call
// _Collection::Load since _Collection::Insert is not virtual.
{
    // The build constructor sets Items to 0. So, if Items is *not* 0 now,
    // someone is calling Load (or operator <<) with existing data. Do the
    // best we can to imitate builtin data types in this case and delete the
    // existing stuff before reloading.
    if (Items) {
	// Delete all items, then delete the item space
	DeleteAll ();
	FreeItemSpace (Items);
    }

    // Read correct size
    u32 aCount, aDelta, aLimit;
    u16 aShouldDelete;
    S >> aCount;
    S >> aDelta;		Delta = aDelta;
    S >> aLimit;		Limit = aLimit;
    S >> aShouldDelete;		ShouldDelete = aShouldDelete;

    // allocate memory
    Items = AllocItemSpace (Limit);

    // Read items. Do not insert them directly but use Insert. This causes
    // nearly no spead penalty on collections and only little on
    // SortedCollections, but does a re-sort if the sort order of a
    // SortedColl has changed (String ordering according to national
    // conventions for example).
    Count = 0;
    while (aCount--) {
	Insert ((T*) GetItem (S));
    }
}



template <class T>
Streamable* Collection<T>::Build ()
{
    return new Collection<T> (Empty);
}



template <class T>
inline T* Collection<T>::operator [] (int Index)
{
    return At (Index);
}



template <class T>
inline const T* Collection<T>::operator [] (int Index) const
{
    return At (Index);
}



/*****************************************************************************/
/*			template class SortedCollection			     */
/*****************************************************************************/



template <class T, class U>
class SortedCollection : public Collection<T> {


public:
    i16		Duplicates;		// Duplicates allowed yes/no

protected:
    virtual int Compare (const U* Key1, const U* Key2);
    // Compare the keys of two items in the collection

    virtual const U* KeyOf (const T* Item);
    // Return the key of a given item

public:
    SortedCollection (int aLimit, int aDelta, int aShouldDelete = 0);
    // Constructor

    SortedCollection (StreamableInit);
    // Build constructor

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    static Streamable* Build ();

    virtual int IndexOf (const T* Item);
    virtual void Insert (T* Item);
    virtual int Search (const U* Key, int& Index);

    T* Find (const U* Key);
    // Find the item with the given key. If duplicates are allowed, this
    // function will return the first item with this key (and is therefor
    // almost useless - use search instead). If no matching key is found,
    // the function returns NULL.

};



template <class T, class U>
inline SortedCollection<T, U>::SortedCollection (StreamableInit X) :
	Collection<T> (X)
{
}



template <class T, class U>
inline SortedCollection<T, U>::SortedCollection (int aLimit,
						 int aDelta,
						 int aShouldDelete) :
	Collection<T> (aLimit, aDelta, aShouldDelete),
	Duplicates (0)
{
}



template <class T, class U>
void SortedCollection<T, U>::Load (Stream& S)
{
    Collection<T>::Load (S);
    S >> Duplicates;
}



template <class T, class U>
void SortedCollection<T, U>::Store (Stream& S) const
{
    Collection<T>::Store (S);
    S << Duplicates;
}



template <class T, class U>
Streamable* SortedCollection<T, U>::Build ()
{
    return new SortedCollection<T, U> (Empty);
}



template <class T, class U>
int SortedCollection<T, U>::Compare (const U*, const U*)
{
    ABSTRACT ();
    return 0;
}



template <class T, class U>
int SortedCollection<T, U>::IndexOf (const T* Item)
{
    int Index;
    const U* Key = KeyOf (Item);
    T* Item2;

    // Search entry
    if (Search (Key, Index) == 0) {
	// Item not found
	return -1;
    }

    // If not duplicates allowed, we've found the item.
    if (!Duplicates) {
	return Index;
    }

    // Duplicates allowed. Do a linear search.
    // (Hint: Search returns the first of all entrys with the same key)
    Item2 = (T*) Items [Index];
    do {
	if (Item2 == Item) {
	    // That's it !
	    return Index;
	}
	// Get next
	Item2 = (T*) Items [++Index];
    } while (Index < Count && Compare (Key, KeyOf (Item2)) == 0);

    // Item not found
    return -1;
}



template <class T, class U>
void SortedCollection<T, U>::Insert (T* Item)
// Inserts an item into a sorted collection. If Duplicates is != 0, the
// item is placed in front of all other items with the same key.
{
    int Index;

    // Search for the element
    if (Search (KeyOf (Item), Index) != 0) {
	// An Item with this key exists. Are duplicates allowed?
	if (!Duplicates) {
	    // No duplicates, nothing to do
	    return;
	}
    }

    // Index points to the correct position, insert item
    AtInsert (Index, Item);
}



template <class T, class U>
int SortedCollection<T, U>::Search (const U* Key, int& Index)
{
    // do a binary search
    int First = 0;
    int Last = Count - 1;
    int Current;
    int Result;
    int S = 0;

    while (First <= Last) {

	// Set current to mid of range
	Current = (Last + First) / 2;

	// Do a compare
	Result = Compare (KeyOf ((T*) Items [Current]), Key);
	if (Result < 0) {
	    First = Current + 1;
	} else {
	    Last = Current - 1;
	    if (Result == 0) {
		// Found. If no Duplicates are allowed, end the search.
		// Otherwise repeat the procedure until the first of all
		// items with the same key is found.
		S = 1;	// function result
		if (!Duplicates) {
		    // Set condition to terminate loop
		    First = Current;
		}
	    }
	}

    }

    Index = First;
    return S;
}



template <class T, class U>
const U* SortedCollection<T, U>::KeyOf (const T* Item)
{
    // Return the item itself
    return (U*) Item;
}



template <class T, class U>
T* SortedCollection<T, U>::Find (const U* Key)
// Find the item with the given key. If duplicates are allowed, this
// function will return the first item with this key (and is therefor
// almost useless - use search instead). If no matching key is found,
// the function returns NULL.
{
    int I;
    if (Search (Key, I) != 0) {
	// We found the key, I is the index
	return At (I);
    } else {
	// We did not find the key
	return 0;
    }
}



// End of COLL.H

#endif





