/*****************************************************************************/
/*									     */
/*				     COLL.H				     */
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



#include <stdlib.h>
#include <string.h>
#include <iostream.h>

#include "machine.h"
#include "check.h"
#include "object.h"
#include "stream.h"
#include "coll.h"




// Pointer to error routine
void (*CollectionError) (int) = NULL;



/*****************************************************************************/
/*			       class _Collection			     */
/*****************************************************************************/



// This is a base class for the template class Collection that has full
// functionality but uses void pointers instead of typed pointers. The
// implementation of class Collection consists merely of inline functions
// calling the derived member functions and casting when necessary. This will
// hopefully help to decrease program size.



_Collection::_Collection (StreamableInit):
    Items (0)
{
}



_Collection::_Collection (int aLimit, int aDelta, int aShouldDelete):
    ShouldDelete (aShouldDelete)
{
    // check parameters
    if (aLimit < 0 || aDelta < 0 || (aLimit + aDelta) == 0 ||
	aLimit > MaxCollectionSize || aDelta > MaxCollectionSize) {
	FAIL ("_Collection::_Collection: Overflow error");
	return;
    }

    // get values - one of <aLimit, aDelta> is != 0
    Limit = aLimit ? aLimit : aDelta;
    Delta = aDelta;
    Count = 0;

    // allocate memory
    Items = AllocItemSpace (Limit);
}



_Collection::~_Collection ()
{
    // Release allocated memory but do not try to call DeleteAll (this calls
    // FreeItem, but _Collection::FreeItem will call ABSTRACT)
    FreeItemSpace (Items);
}



void* _Collection::At (int Index)
{
    // Check range
    if (Index < 0 || Index >= Count) {
	FAIL ("_Collection::At: Index out of bounds");
	return NULL;
    }

    return Items [Index];
}



const void* _Collection::At (int Index) const
{
    // Check range
    if (Index < 0 || Index >= Count) {
	FAIL ("_Collection::At: Index out of bounds");
	return NULL;
    }

    return Items [Index];
}



void _Collection::AtDelete (int Index)
{
    // Check range
    if (Index < 0 || Index >= Count) {
	FAIL ("_Collection::AtDelete: Index out of bounds");
	return;
    }

    // Free entry if necessary
    if (ShouldDelete) {
	FreeItem (Items [Index]);
    }

    // Delete entry
    Count--;
    memmove (Items + Index, Items + Index + 1, (Count - Index) * sizeof (void*));
}



void _Collection::AtInsert (int Index, void* Item)
{
    // Check range, Index may be one more than the index of the last element
    if (Index < 0 || Index > Count) {
	FAIL ("_Collection::AtInsert: Index out of bounds");
	return;
    }

    // Check if the collection must grow
    if (Count == Limit) {
	// Increase size, check if possible
	if (Delta == 0 || (Limit + Delta) > MaxCollectionSize) {
	    // Cannot grow
	    FAIL ("_Collection::AtInsert: Overflow error");
	    return;
	}

	// Change the size
	SetLimit (Limit + Delta);
    }

    // We can insert the element. If the item is not inserted at the end
    // of the collection, we must create a "hole"
    if (Count != Index) {
	memmove (Items + Index + 1, Items + Index, (Count - Index) * sizeof (void*));
    }
    Count++;

    // store the new item
    Items [Index] = Item;
}



void _Collection::AtReplace (int Index, void* Item)
{
    // check range
    if (Index < 0 || Index >= Count) {
	FAIL ("_Collection::AtReplace: Index out of bounds");
	return;
    }

    // Free item if necessary
    if (ShouldDelete) {
	FreeItem (Items [Index]);
    }

    // Store item
    Items [Index] = Item;
}



void _Collection::Delete (void* Item)
{
    int Index = IndexOf (Item);

    if (Index != -1) {
	if (ShouldDelete) {
	    FreeItem (Items [Index]);
	}
	Items [Index] = NULL;
    }
}



void _Collection::DeleteAll ()
{
    if (ShouldDelete) {
	while (Count) {
	    FreeItem (Items [--Count]);
	}
    } else {
	Count = 0;
    }
}



void* _Collection::Traverse (int Forward, int (*Func) (void*, void*),
			      void* UserData)
{
    int I;
    void** P;

    if (Forward) {
	for (I = 0, P = Items; I < Count; I++, P++) {
	    if (Func (*P, UserData) != 0) {
		// stop and return current
		return *P;
	    }
	}
    } else {
	for (I = Count-1, P = Items + I; I >= 0; I--, P--) {
	    if (Func (*P, UserData) != 0) {
		// stop and return current
		return *P;
	    }
	}
    }
    // Not found
    return NULL;

}



void _Collection::FreeItem (void*)
{
    ABSTRACT ();
}



void* _Collection::GetItem (Stream& S)
{
    // Assume it's some sort of object
    return S.Get ();
}



void** _Collection::AllocItemSpace (int ItemCount)
// Allocate item space
{
    return new void* [ItemCount];
}



void _Collection::FreeItemSpace (void** Space)
// Deallocate item space
{
    delete [] Space;
}



int _Collection::IndexOf (const void* Item)
{
    int C;
    void** I;

    // generic collection "features" a linear search
    for (C = 0, I = Items; C < Count; C++, I++) {
	if (*I == Item) {
	    return C;
	}
    }

    // Not found
    return -1;
}



void _Collection::Insert (void* Item)
{
    // Default is to add the item at the end
    AtInsert (Count, Item);
}



void _Collection::Pack ()
{
    int I, J;

    for (I = 0; I < Count; I++) {

	// Try to find a NULL pointer
	if (Items [I] == NULL) {

	    // Count the number of NULL pointers in a row
	    J = I+1;
	    while (J < Count && Items [J] == NULL) {
		J++;
	    }

	    // Delete those NULL pointers
	    memmove (Items + I, Items + J, (Count - J) * sizeof (void*));

	    // Update Count
	    Count -= J - I;

	}

    }

}



void _Collection::PutItem (Stream& S, void* Item) const
{
    // Assume it's a streamable object
    S.Put ((Streamable*) Item);
}



void _Collection::SetLimit (int aLimit)
{
    // Limit the new limit
    if (aLimit < Count) {
	aLimit = Count;
    } else if (aLimit > MaxCollectionSize) {
	aLimit = MaxCollectionSize;
    }

    // allocate memory
    void** P = AllocItemSpace (aLimit);

    // Copy old data into new array
    memcpy (P, Items, Count * sizeof (void*));

    // Release old memory
    FreeItemSpace (Items);

    // Remember new array and limit
    Items = P;
    Limit = aLimit;
}



void _Collection::Load (Stream& S)
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
	Insert (GetItem (S));
    }

}



void _Collection::Store (Stream& S) const
{
    // Write correct size
    u32 aCount = (u32) Count;
    u32 aDelta = (u32) Delta;
    u32 aLimit = (u32) Limit;
    u16 aShouldDelete = (u16) ShouldDelete;

    // Write data
    S << aCount << aDelta << aLimit << aShouldDelete;

    // Write items
    for (int I = 0; I < Count; I++) {
	PutItem (S, Items [I]);
    }

}




