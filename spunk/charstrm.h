/*****************************************************************************/
/*                                                                           */
/*                                CHARSTRM.H                                 */
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



#ifndef _CHARSTRM_H
#define _CHARSTRM_H



#include <stdlib.h>

#include "stream.h"



/*****************************************************************************/
/*                           class CharacterStream                           */
/*****************************************************************************/



class CharacterStream: public Stream {

protected:
    double      ReadTimeout, WriteTimeout;
    // The timeouts - see the functions SetReadTimeout/SetWriteTimeout for an
    // explanation.

public:
    virtual u32 GetPos ();
    // Get the current value of the stream pointer

    virtual u32 GetSize ();
    // Return the size of the stream in bytes

    virtual void Seek (unsigned long Pos);
    // Set the stream pointer to the specified position

    virtual void SeekToEnd ();
    // Set the stream pointer to the end of the stream

    virtual void Truncate ();
    // Truncate the stream at the current position

    virtual void SetReadTimeout (double T);
    // Set the read timeout for the stream.
    // A value of zero means no timeout (error if data not immidiately
    // available) a value less than zero means "indefinite wait".
    // Beware: The implementation of the timeout is device dependent! As an
    // example, there may be devices that cannot do timeouts less than one
    // second.

    virtual void SetWriteTimeout (double T);
    // Set the write timeout for the stream.
    // A value of zero means no timeout (error if data cannot be written
    // immidiately) a value less than zero means "indefinite wait".
    // Beware: The implementation of the timeouts is device dependent! As an
    // example, there may be devices that cannot do timeouts less than one
    // second.

    double GetReadTimeout () const;
    // Return the read timeout

    double GetWriteTimeout () const;
    // Return the write timeout

    virtual size_t ReadCount () const;
    // Return the amount of bytes that can be read without waiting. On many
    // devices it is impossible to tell the exact number, in these cases the
    // function returns a value of 1 if at least one byte can be read without
    // blocking and 0 otherwise.

    virtual size_t WriteCount () const;
    // Return the amount of bytes that can be written without waiting. On many
    // devices it is impossible to tell the exact number, in these cases the
    // function returns a value of 1 if at least one byte can be written without
    // blocking and 0 otherwise.

};



inline double CharacterStream::GetReadTimeout () const
// Return the read timeout
{
    return ReadTimeout;
}



inline double CharacterStream::GetWriteTimeout () const
// Return the write timeout
{
    return WriteTimeout;
}



// End of CHARSTRM.H

#endif



