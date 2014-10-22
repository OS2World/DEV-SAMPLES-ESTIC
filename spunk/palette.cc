/*****************************************************************************/
/*                                                                           */
/*                                 PALETTE.CC                                */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
/*              Zwehrenbuehlstrasse 33                                       */
/*              D-72070 Tuebingen                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include <string.h>

#include "palette.h"
#include "screen.h"
#include "streamid.h"



// Register the classes
LINK (Palette, ID_Palette);



// Instance of class Palette
Palette* Pal;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<unsigned char>;
#endif



/*****************************************************************************/
/*                               class Palette                               */
/*****************************************************************************/



void Palette::FreeItem (void* Item)
{
    delete [] (unsigned char*) Item;
}



void* Palette::GetItem (Stream& S)
{
    return (void*) S.ReadStr ();
}



void Palette::PutItem (Stream& S, void* Item) const
{
    S.WriteStr ((char*) Item);
}



Palette::Palette () :
        Collection<unsigned char> (10, 5, 1),
        Debug (0)
{
}



void Palette::Load (Stream& S)
{
    Collection<unsigned char>::Load (S);
    S >> Debug;
}



void Palette::Store (Stream& S) const
{
    Collection<unsigned char>::Store (S);
    S << Debug;
}



u16 Palette::StreamableID () const
{
    return ID_Palette;
}



Streamable *Palette::Build ()
{
    return new Palette (Empty);
}



unsigned Palette::Add (const unsigned char* Mono, const unsigned char* Color)
// Function to add a palette entry, returns the palette number
{
    unsigned PalNum = Count / 2;
    Insert ((unsigned char*) strcpy (new char [strlen ((char*) Mono)+1], (char*) Mono));
    Insert ((unsigned char*) strcpy (new char [strlen ((char*) Color)+1], (char*) Color));
    return PalNum;
}



u16 Palette::BuildAttr (unsigned PalNum, unsigned AttrIndex, unsigned char C)
// Builds an attribute char from the given palette and index and the given char.
{
    unsigned char *P;
    if (TheScreen->IsColor () == 0) {
        // Mono
        P = At (PalNum * 2);
    } else {
        // Color
        P = At (PalNum * 2 + 1);
    }

    if (Debug) {
        // Slows things down, so check only if Debug set
        PRECONDITION (AttrIndex < strlen ((char*) P));
    }
    return   (u16) ((u16 (P [AttrIndex]) << 8) | u16 (C));
}



