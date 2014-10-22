/*****************************************************************************/
/*                                                                           */
/*                                 PALETTE.H                                 */
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



#ifndef __PALETTE_H
#define __PALETTE_H



#include "coll.h"



// Instance of class Palette
extern class Palette* Pal;



/*****************************************************************************/
/*                               Palette stuff                               */
/*****************************************************************************/



// Indices into the palette arrays
static const atFrameInactive    =   0;  // passive frame
static const atFrameActive      =   1;  // active frame
static const atFrameResizing    =   2;  // resizing frame
static const atTextNormal       =   3;  // normal text
static const atTextInvers       =   4;  // inverted text
static const atTextSelected     =   5;  // selected static text
static const atTextHigh         =   6;  // selected text (i.e. hotkeys)
static const atTextHighInvers   =   7;  // inverted selected text
static const atTextGrayed       =   8;  // grey (inactive) text
static const atTextGrayedInvers =   9;  // dito inverted
static const atEditNormal       =  10;  // normal text in an edit window
static const atEditHigh         =  11;  // i.e. left/right arrows
static const atEditBar          =  12;  // scroll bar (use atEditNormal for text)


// Palette numbers
static const u16 paBlue         =  0;   // blue palette
static const u16 paGray         =  1;   // grey palette (standard)
static const u16 paCyan         =  2;   // cyan palette
static const u16 paRed          =  3;   // red palette
static const u16 paBlack        =  4;   // black palette
static const u16 paError        =  5;   // errorwindow palette
static const u16 paRoot         =  6;   // root window palette
static const u16 paHelp         =  7;   // help window palette
static const u16 paFSel         =  8;   // file selector palette



/*****************************************************************************/
/*                               class Palette                               */
/*****************************************************************************/



class Palette: private Collection<unsigned char> {

protected:
    virtual void FreeItem (void* Item);
    virtual void* GetItem (Stream& S);
    virtual void PutItem (Stream& S, void* Item) const;


    Palette (StreamableInit);
    // Build constructor

public:
    i16         Debug;

public:
    Palette ();

    // Derived from class Streamable
    virtual void Load (Stream& S);
    virtual void Store (Stream& S) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Function to add a palette entry, returns the palette number
    unsigned Add (const unsigned char *Mono, const unsigned char *Color);

    // Functions to build palette entries
    u16 BuildAttr (unsigned PalNum, unsigned AttrIndex, unsigned char C);
    static u16 BuildAttr (u16 Attr, unsigned char C);
    static u16 BuildAttr (unsigned char Attr, unsigned char C = '\0');
};



inline Palette::Palette (StreamableInit) :
        Collection<unsigned char> (Empty)
{
}



inline u16 Palette::BuildAttr (u16 Attr, unsigned char C)
// Builds an attribute char with the given attribute char and new char C
{
    return (u16) ((Attr & 0xFF00) | u16 (C));
}



inline u16 Palette::BuildAttr (unsigned char Attr, unsigned char C)
// Builds an attribute char with the given attribute and char.
{
    return (u16) ((((u16) Attr) << 8) | C);
}



// End of PALETTE.H

#endif

