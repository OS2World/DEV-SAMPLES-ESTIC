/*****************************************************************************/
/*									     */
/*				    BITSET.H				     */
/*									     */
/* (C) 1993,94	Ullrich von Bassewitz					     */
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



#ifndef _BITSET_H
#define _BITSET_H



#include "machine.h"
#include "strmable.h"
#include "stream.h"



/*****************************************************************************/
/*				 class BitSet				     */
/*****************************************************************************/



class BitSet : public Streamable {

private:
    unsigned char* BitBucket;
    i32 Count;
    i32 First;
    i32 Last;
    u32 Bytes;		// Redundant, for speedup

public:
    BitSet (int BitCount, int FirstIndex = 0);
    BitSet (const BitSet& Set);
    BitSet (StreamableInit X);
    ~BitSet ();

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Traverse through all bits that are set
    int Traverse (int Forward, int (*F) (int, void*), void* = NULL) const;

    // New member functions (operators)
    BitSet& operator = (const BitSet& Set);   // Assignment
    BitSet& operator += (int Bit);
    BitSet& operator += (const BitSet& Set);
    BitSet& operator -= (int Bit);
    BitSet& operator -= (const BitSet & Set);
    int operator [] (int Bit) const;

    // Friend operators
    friend BitSet operator ~ (const BitSet& Set);
    friend BitSet operator + (const BitSet& Set, int Bit);
    friend BitSet operator + (const BitSet& Set1, const BitSet& Set2);
    friend BitSet operator - (const BitSet& Set, int Bit);
    friend BitSet operator - (const BitSet& Set1, const BitSet& Set2);

    friend int operator == (const BitSet& Set1, const BitSet& Set2);
    friend int operator != (const BitSet& Set1, const BitSet& Set2);

    // New member functions

    int IsEmpty ();
    // Return true if the bitset is empty (all bits zero), return false
    // otherwise

    BitSet& Clear ();
    // Set all bits to zero

    BitSet& SetAll ();
    // Set all bits to one's

    BitSet& Reverse ();
    // Reverse each and every bit

    i32 Low () const;
    // Return the low border of the bit numbers

    i32 High () const;
    // Return the high border of the bit numbers
};



inline BitSet::BitSet (StreamableInit)
{
}



inline Streamable* BitSet::Build ()
{
    return new BitSet (Empty);
}



inline i32 BitSet::Low () const
// Return the low border of the bit numbers
{
    return First;
}



inline i32 BitSet::High () const
// Return the high border of the bit numbers
{
    return Last;
}



// End of BITSET.H

#endif
