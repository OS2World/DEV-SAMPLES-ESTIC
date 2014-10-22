/*****************************************************************************/
/*                                                                           */
/*                                MEMSTRM.CC                                 */
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

#include "machine.h"
#include "check.h"
#include "streamid.h"
#include "memstrm.h"



// Register the class
LINK (MemoryStream, ID_MemoryStream);



/*****************************************************************************/
/*                            class MemoryStream                             */
/*****************************************************************************/



void MemoryStream::Resize (u32 NewSize)
{
    if (NewSize > Limit) {

        // Round up to full blocks
        NewSize = ((NewSize + Block - 1) / Block) * Block;

        // Allocate a new block
        char* P = new char [NewSize];

        // Copy the old data to the new block
        memcpy (P, Memory, Size);

        // Free old memory block, take new one, remember new size
        delete [] Memory;
        Memory = P;
        Limit = NewSize;

    }

}



MemoryStream::MemoryStream (u32 BlockSize) :
     Memory (new char [BlockSize]),
     Block (BlockSize),
     Limit (BlockSize),
     Size (0),
     CurPos (0)
{
}



MemoryStream::~MemoryStream ()
{
    delete [] Memory;
}



void MemoryStream::Load (Stream& S)
{
    // A MemoryStream loaded has the error codes reset and stream position 0
    Reset ();
    CurPos = 0;

    // Now load the data
    S >> Block >> Limit >> Size;
    CHECK (Limit > 0);
    Memory = new char [Limit];
    if (Size) {
        S.Read (Memory, Size);
    }
}



void MemoryStream::Store (Stream& S) const
{
    // Cannot store a stream that's status is not ok
    PRECONDITION (GetStatus () == stOk);

    // Write the data to the stream
    S << Block << Limit << Size;
    if (Size) {
        S.Write (Memory, Size);
    }
}



u16 MemoryStream::StreamableID () const
{
    return ID_MemoryStream;
}



Streamable* MemoryStream::Build ()
{
    return new MemoryStream (Empty);
}



u32 MemoryStream::GetPos ()
{
    return CurPos;
}



u32 MemoryStream::GetSize ()
{
    return Size;
}



void MemoryStream::Truncate ()
{
    Size = CurPos;
}



void MemoryStream::Read (void* Buf, size_t Count)
{
    PRECONDITION (Buf != NULL && Count != 0);

    // Check end of stream condition
    if ((CurPos + Count) > Size) {
        Error (stReadError, Count);
        return;
    }

    // Copy the data
    memcpy (Buf, &Memory [CurPos], Count);

    // Remember new position
    CurPos += Count;
}



void MemoryStream::Seek (unsigned long Pos)
{
    // Make shure, the position is available
    Resize (Pos);

    // Remember the position
    CurPos = Pos;

    // Keep track of size
    if (CurPos > Size) {
        Size = CurPos;
    }

}



void MemoryStream::Write (const void* Buf, size_t Count)
{
    // Make shure, the new data fits
    Resize (CurPos + Count);

    // Copy the data
    memcpy (&Memory [CurPos], Buf, Count);

    // Remember the new position
    CurPos += Count;

    // Keep track of size
    if (CurPos > Size) {
        Size = CurPos;
    }

}






