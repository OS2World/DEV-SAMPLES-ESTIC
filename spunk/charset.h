/*****************************************************************************/
/*									     */
/*				   CHARSET.H				     */
/*									     */
/* (C) 1995-96	Ullrich von Bassewitz					     */
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



#ifndef __CHARSET_H
#define __CHARSET_H



#include <string.h>

#include "strmable.h"



// Forwards
class String;



/*****************************************************************************/
/*				 class CharSet				     */
/*****************************************************************************/



class CharSet: public Streamable {

protected:
    unsigned char Buf [32];	// Buffer for 256 bits

public:
    CharSet ();
    // Construct a CharSet object. The set is initially empty

    CharSet (StreamableInit);
    // Construct an empty CharSet object

    CharSet (const CharSet&);
    // Construct a charset from another one

    CharSet (const char* Set);
    // Construct a charset from a string (include all characters that are
    // in the string)

    CharSet (const String& Set);
    // Construct a charset from a string (include all characters that are
    // in the string)

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Traverse through all bits that are set
    int Traverse (int Forward, int (*F) (char, void*), void* = NULL) const;

    // Operators
    CharSet& operator = (const char* Set);
    CharSet& operator = (const String& Set);
    CharSet& operator = (const CharSet& Set);

    inline CharSet& operator += (char C);
    CharSet& operator += (const char* Set);
    CharSet& operator += (const String& Set);
    CharSet& operator += (const CharSet& Set);

    inline CharSet& operator -= (char C);
    CharSet& operator -= (const char* Set);
    CharSet& operator -= (const String& Set);
    CharSet& operator -= (const CharSet& Set);

    inline int operator [] (char C) const;

    // Friend operators
    friend CharSet operator ~ (const CharSet& Set);
    friend CharSet operator + (const CharSet& Set, char C);
    friend CharSet operator + (const CharSet& Set1, const CharSet& Set2);
    friend CharSet operator - (const CharSet& Set, char C);
    friend CharSet operator - (const CharSet& Set1, const CharSet& Set2);

    friend inline int operator == (const CharSet& Set1, const CharSet& Set2);
    friend inline int operator != (const CharSet& Set1, const CharSet& Set2);

    // New member functions

    int IsEmpty ();
    // Return true if the charset is empty, return false otherwise

    CharSet& Clear ();
    // Exclude all possible chars

    CharSet& SetAll ();
    // Include all possible chars

    CharSet& AddRange (char C1, char C2);
    // Adds a complete range of characters to the character set

    CharSet& Reverse ();
    // Reverse the set
};



inline CharSet::CharSet ()
// Construct a CharSet object. The set is initially empty
{
    memset (Buf, 0, sizeof (Buf));
}



inline CharSet::CharSet (StreamableInit)
// construct an empty CharSet object
{
}



inline CharSet::CharSet (const CharSet& Set)
// Construct a charset from another one
{
    memcpy (Buf, Set.Buf, sizeof (Buf));
}



inline CharSet& CharSet::operator += (char C)
{
    register unsigned X = (unsigned)(unsigned char)C;
    Buf [X / 8] |= 0x01 << (X % 8);
    return *this;
}



inline CharSet& CharSet::operator -= (char C)
{
    register unsigned X = (unsigned)(unsigned char)C;
    Buf [X / 8] &= ~(0x01 << (X % 8));
    return *this;
}



inline int CharSet::operator [] (char C) const
{
    register unsigned X = (unsigned)(unsigned char)C;
    return (Buf [X / 8] >> (X % 8)) & 0x0001;
}



inline int operator == (const CharSet& Set1, const CharSet& Set2)
{
    return (memcmp (Set1.Buf, Set2.Buf, sizeof (Set1.Buf)) == 0);
}



inline int operator != (const CharSet& Set1, const CharSet& Set2)
{
    return (memcmp (Set1.Buf, Set2.Buf, sizeof (Set1.Buf)) != 0);
}



inline CharSet& CharSet::Clear ()
// Exclude all possible chars
{
    memset (Buf, 0, sizeof (Buf));
    return *this;
}



inline CharSet& CharSet::SetAll ()
// Include all possible chars
{
    memset (Buf, 0xFF, sizeof (Buf));
    return *this;
}



// End of CHARSET.H

#endif
