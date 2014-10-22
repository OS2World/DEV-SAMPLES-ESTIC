/*****************************************************************************/
/*                                                                           */
/*                                CHARSTRM.CC                                */
/*                                                                           */
/* (C) 1996     Ullrich von Bassewitz                                        */
/*              Wacholderweg 14                                              */
/*              D-70597 Stuttgart                                            */
/* EMail:       uz@musoftware.com                                            */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// This module defines a special family of streams. A CharacterStream is
// different from a Stream in that it is not possible to do things like
// Seek and Truncate. Instead, they have timeouts for read and write.
// Usually a CharacterStream works on something like a serial device, a
// network connection and such.



#include "charstrm.h"



/*****************************************************************************/
/*                           class CharacterStream                           */
/*****************************************************************************/



u32 CharacterStream::GetPos ()
// Get the current value of the stream pointer
{
    FAIL ("CharacterStream::Truncate: Operation not allowed on this Stream type");
    return 0;
}



u32 CharacterStream::GetSize ()
// Return the size of the stream in bytes
{
    FAIL ("CharacterStream::Truncate: Operation not allowed on this Stream type");
    return 0;
}



void CharacterStream::Seek (unsigned long /* Pos */)
// Set the stream pointer to the specified position
{
    FAIL ("CharacterStream::Truncate: Operation not allowed on this Stream type");
}



void CharacterStream::SeekToEnd ()
// Set the stream pointer to the end of the stream
{
    FAIL ("CharacterStream::Truncate: Operation not allowed on this Stream type");
}



void CharacterStream::Truncate ()
// Truncate the stream at the current position
{
    FAIL ("CharacterStream::Truncate: Operation not allowed on this Stream type");
}



void CharacterStream::SetReadTimeout (double T)
// Set the read timeout for the stream.
// A value of zero means no timeout (error if data not immidiately
// available) a value less than zero means "indefinite wait".
// Beware: The implementation of the timeout is device dependent! As an
// example, there may be devices that cannot do timeouts less than one
// second.
{
    ReadTimeout = T;
}



void CharacterStream::SetWriteTimeout (double T)
// Set the write timeout for the stream.
// A value of zero means no timeout (error if data cannot be written
// immidiately) a value less than zero means "indefinite wait".
// Beware: The implementation of the timeouts is device dependent! As an
// example, there may be devices that cannot do timeouts less than one
// second.
{
    WriteTimeout = T;
}



size_t CharacterStream::ReadCount () const
// Return the amount of bytes that can be read without waiting. On many
// devices it is impossible to tell the exact number, in these cases the
// function returns a value of 1 if at least one byte can be read without
// blocking and 0 otherwise.
{
    ABSTRACT ();
    return 0;
}



size_t CharacterStream::WriteCount () const
// Return the amount of bytes that can be written without waiting. On many
// devices it is impossible to tell the exact number, in these cases the
// function returns a value of 1 if at least one byte can be written without
// blocking and 0 otherwise.
{
    ABSTRACT ();
    return 0;
}



