/*****************************************************************************/
/*									     */
/*				    BITSET.CC				     */
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



#include <string.h>

#include "check.h"
#include "stream.h"
#include "bitset.h"
#include "streamid.h"



// Register the class
LINK (BitSet, ID_BitSet);



/*****************************************************************************/
/*				 class BitSet				     */
/*****************************************************************************/



BitSet::BitSet (int BitCount, int FirstIndex) :
    Count (BitCount),
    First (FirstIndex),
    Last (FirstIndex + BitCount - 1),
    Bytes ((BitCount + 7) / 8)
{
    // Get memory
    BitBucket = new unsigned char [Bytes];

    // Clear memory
    Clear ();
}



BitSet::BitSet (const BitSet& Set)
{
    // Copy Members
    Count = Set.Count;
    First = Set.First;
    Last  = Set.Last;
    Bytes = Set.Bytes;

    // Get memory
    BitBucket = new unsigned char [Bytes];

    // Copy BitBucket
    memcpy (BitBucket, Set.BitBucket, Bytes);
}



BitSet::~BitSet ()
// Destructor
{
    delete [] BitBucket;
}



u16 BitSet::StreamableID () const
{
    return ID_BitSet;
}



void BitSet::Store (Stream& S) const
{
    S << Count << First << Last << Bytes;
    S.Write (BitBucket, Bytes);
}



void BitSet::Load (Stream& S)
{
    S >> Count >> First >> Last >> Bytes;
    BitBucket = new unsigned char [Bytes];
    S.Read (BitBucket, Bytes);
}



BitSet& BitSet::operator = (const BitSet& Set)
// Assignment operator
{

    if (this != &Set) {		// Beware of Set = Set;

	Count = Set.Count;
	Bytes = Set.Bytes;
	First = Set.First;
	Last  = Set.Last;

	delete BitBucket;
	BitBucket = new unsigned char [Bytes];

	memcpy (BitBucket, Set.BitBucket, Bytes);

    }
    return *this;
}



BitSet& BitSet::operator += (int Bit)
{
    // Range-Check
    PRECONDITION ((Bit >= First) && (Bit <= Last));

    Bit -= First;
    BitBucket [Bit / 8] |= (unsigned char) (0x01 << (Bit % 8));

    return *this;
}



BitSet& BitSet::operator += (const BitSet& Set)
{
    unsigned char* P1 = BitBucket;
    unsigned char* P2 = Set.BitBucket;

    for (unsigned I = 0; I < Bytes; I++, P1++, P2++) {
	*P1 |= *P2;
    }

    return *this;
}



BitSet& BitSet::operator -= (int Bit)
{
    // Range-Check
    PRECONDITION ((Bit >= First) && (Bit <= Last));

    Bit -= First;
    BitBucket [Bit / 8] &= (unsigned char) ~(0x01 << (Bit % 8));

    return *this;
}



BitSet& BitSet::operator -= (const BitSet& Set)
{
    unsigned char* P1 = BitBucket;
    unsigned char* P2 = Set.BitBucket;

    for (unsigned I = 0; I < Bytes; I++, P1++, P2++) {
	*P1 &= ~(*P2);
    }

    return *this;
}



BitSet operator ~ (const BitSet& Set)
{
    BitSet NewSet (Set);
    return NewSet.Reverse ();
}



BitSet operator + (const BitSet& Set, int Bit)
{
    return (BitSet (Set) += Bit);
}



BitSet operator + (const BitSet& Set1, const BitSet& Set2)
{
    return (BitSet (Set1) += Set2);
}



BitSet operator - (const BitSet& Set, int Bit)
{
    return (BitSet (Set) -= Bit);
}



BitSet operator - (const BitSet& Set1, const BitSet& Set2)
{
    return (BitSet (Set1) -= Set2);
}



int BitSet::operator [] (int Bit) const
{
    // check range
    PRECONDITION ((Bit >= First) && (Bit <= Last));

    Bit -= First;
    return (*(BitBucket + (Bit / 8)) >> (Bit % 8)) & 0x0001;
}



int operator == (const BitSet& Set1, const BitSet& Set2)
{
    // Must have same margins
    if ((Set1.Count != Set2.Count) ||
	(Set1.First != Set2.First) ||
	(Set1.Last  != Set2.Last)) {
	return 0;
    }

    // compare BitBuckets
    return (memcmp (Set1.BitBucket, Set2.BitBucket, Set1.Bytes) == 0);
}



int operator != (const BitSet& Set1, const BitSet& Set2)
{
    // Must have same margins
    if ((Set1.Count != Set2.Count) ||
	(Set1.First != Set2.First) ||
	(Set1.Last  != Set2.Last)) {
	return 1;
    }

    // compare BitBuckets
    return (memcmp (Set1.BitBucket, Set2.BitBucket, Set1.Bytes) != 0);
}



int BitSet::IsEmpty ()
// returns 1 if the set is empty, 0 otherwise
{
    unsigned I;
    unsigned char* P;

    for (I = 0, P = BitBucket; I < Bytes; I++, P++) {
	if (*P) {
	    // not empty
	    return 0;
	}
    }

    // is empty
    return 1;
}



BitSet& BitSet::Clear ()
// Resets all bits
{
    memset (BitBucket, 0, Bytes);
    return *this;
}



BitSet& BitSet::SetAll ()
// Set all bits. Note: Don't set the bits inside the range covered
// by bytes * 8 but outside First-Last
{
    // Set all but the last byte
    if (Bytes > 1) {
	memset (BitBucket, 0xFF, Bytes - 1);
    }

    // Last byte (maybe no full byte)
    BitBucket [Bytes-1] = (unsigned char) (0xFF >> (7 - ((Count - 1) % 8)));

    // Return a reference
    return *this;
}



BitSet& BitSet::Reverse ()
// Reverse each and every bit
{
    // Reverse all but the last byte
    for (unsigned I = 0; I < Bytes-1; I++) {
	BitBucket [I] ^= 0xFF;
    }

    // Reverse the last byte (maybe no full byte)
    BitBucket [Bytes-1] ^= (0xFF >> (7 - ((Count - 1) % 8)));

    // Return a reference
    return *this;
}



int BitSet::Traverse (int Forward, int (*F) (int, void*), void* Data) const
// Traverse through all bits that are set
{
    i32 I;
    int Bit;

    if (Forward) {
	for (I = First, Bit = 0; I <= Last; I++, Bit++) {
	    if ((*(BitBucket + (Bit / 8)) >> (Bit % 8)) & 0x0001) {
		if (F (I, Data) != 0) {
		    return I;
		}
	    }
	}
    } else {
	for (I = Last, Bit = Count - 1; I >= First; I--, Bit--) {
	    if ((*(BitBucket + (Bit / 8)) >> (Bit % 8)) & 0x0001) {
		if (F (I, Data) != 0) {
		    return I;
		}
	    }
	}
    }

    // Not found
    return Last + 1;
}
