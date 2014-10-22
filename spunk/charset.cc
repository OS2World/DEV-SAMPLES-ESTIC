/*****************************************************************************/
/*                                                                           */
/*                                 CHARSET.CC                                */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
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



#include "streamid.h"
#include "stream.h"
#include "charset.h"



// Register class CharSet
LINK (CharSet, ID_CharSet);



/*****************************************************************************/
/*                               class CharSet                               */
/*****************************************************************************/



CharSet::CharSet (const char* Set)
// Construct a charset from a string (include all characters that are
// in the string)
{
    // Clear the buffer
    Clear ();

    // Add all characters from the string
    *this += Set;
}



CharSet::CharSet (const String& Set)
// Construct a charset from a string (include all characters that are
// in the string)
{
    // Clear the buffer
    Clear ();

    // Add all characters from the string
    *this += Set;
}



void CharSet::Load (Stream& S)
{
    S.Read (Buf, sizeof (Buf));
}



void CharSet::Store (Stream& S) const
{
    S.Write (Buf, sizeof (Buf));
}



u16 CharSet::StreamableID () const
{
    return ID_CharSet;
}



Streamable* CharSet::Build ()
{
    return new CharSet (Empty);
}



// Traverse through all bits that are set
int CharSet::Traverse (int Forward, int (*F) (char, void*), void* Data) const
{
    unsigned I;

    if (Forward) {
        I = 0;
        do {
            if ((Buf [I / 8] >> (I % 8)) & 0x01) {
                if (F (I, Data) != 0) {
                    return I;
                }
            }
            I++;
        } while (I < 256);
    } else {
        I = 256;
        do {
            I--;
            if ((Buf [I / 8] >> (I % 8)) & 0x01) {
                if (F (I, Data) != 0) {
                    return I;
                }
            }
        } while (I > 0);
    }

    // Not found
    return -1;
}



CharSet& CharSet::operator = (const CharSet& Set)
{
    if (this != &Set) {
        memcpy (Buf, Set.Buf, sizeof (Buf));
    }
    return *this;
}



CharSet& CharSet::operator = (const char* Set)
{
    // Clear the set
    Clear ();

    // Add all characters from the string
    return *this += Set;
}



CharSet& CharSet::operator = (const String& Set)
{
    return *this = Set.GetStr ();
}



CharSet& CharSet::operator += (const CharSet& Set)
{
    for (unsigned I = 0; I < sizeof (Buf); I++) {
        Buf [I] |= Set.Buf [I];
    }
    return *this;
}



CharSet& CharSet::operator += (const char* Set)
{
    // Add all characters from the string
    char C;
    while ((C = *Set++) != '\0') {
        *this += C;
    }
    return *this;
}



CharSet& CharSet::operator += (const String& Set)
{
    return *this += Set.GetStr ();
}



CharSet& CharSet::operator -= (const CharSet& Set)
{
    for (unsigned I = 0; I < sizeof (Buf); I++) {
        Buf [I] &= ~Set.Buf [I];
    }
    return *this;
}



CharSet& CharSet::operator -= (const char* Set)
{
    // Sub all characters from the string
    char C;
    while ((C = *Set++) != '\0') {
        *this -= C;
    }
    return *this;
}



CharSet& CharSet::operator -= (const String& Set)
{
    return *this -= Set.GetStr ();
}



CharSet operator ~ (const CharSet& Set)
{
    CharSet NewSet = Set;
    return NewSet.Reverse ();
}



CharSet operator + (const CharSet& Set, char C)
{
    CharSet NewSet = Set;
    return (NewSet += C);
}



CharSet operator + (const CharSet& Set1, const CharSet& Set2)
{
    CharSet NewSet = Set1;
    return (NewSet += Set2);
}



CharSet operator - (const CharSet& Set, char C)
{
    CharSet NewSet = Set;
    return (NewSet -= C);
}



CharSet operator - (const CharSet& Set1, const CharSet& Set2)
{
    CharSet NewSet = Set1;
    return (NewSet -= Set2);
}



int CharSet::IsEmpty ()
// Return true if the charset is empty, return false otherwise
{
    for (unsigned I = 0; I < sizeof (Buf); I++) {
        if (Buf [I] != 0) {
            // Not empty
            return 0;
        }
    }

    // Empty
    return 1;
}



CharSet& CharSet::AddRange (char C1, char C2)
// Adds a complete range of characters to the character set
{
    unsigned U1 = (unsigned)(unsigned char)C1;
    unsigned U2 = (unsigned)(unsigned char)C2;
    while (U1 <= U2) {
        *this += (char) U1;
        U1++;
    }
    return *this;
}



CharSet& CharSet::Reverse ()
// Reverse the set
{
    for (unsigned I = 0; I < sizeof (Buf); I++) {
        Buf [I] ^= 0xFF;
    }
    return *this;
}



