/*****************************************************************************/
/*                                                                           */
/*                                 SERSTRM.CC                                */
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



// A serial character stream using the sercom module.



#include "serstrm.h"



/*****************************************************************************/
/*                            class SerialStream                             */
/*****************************************************************************/



SerialStream::SerialStream (const String& aPortName,
                            u32  aBaudrate,
                            char aDatabits,
                            char aParity,
                            char aStopbits,
                            char aConnection,
                            char aXonXoff,
                            unsigned UARTBase,
                            unsigned IntNum):
    Port (aPortName, aBaudrate, aDatabits, aParity, aStopbits, aConnection,
          aXonXoff, UARTBase, IntNum)
// Create a SerialStream object, use defaults for timeouts and buffer sizes
{
    // Set the buffer sizes to something big
    SetBufferSize (2048, 2048);

    // Open the port
    unsigned Result = Port.Open ();
    if (Result != 0) {
        Error (stInitError, Result);
        return;
    }

    // Activate the DTR line
    Port.DTROn ();
}



SerialStream::~SerialStream ()
// Close the port
{
    // Beware: We may have had an error
    if (Port.IsOpen ()) {

        // Deactivate the DTR line
        Port.DTROff ();

        // Close the port
        Port.Close ();
    }
}



void SerialStream::SetBufferSize (unsigned aRXBufSize, unsigned aTXBufSize)
// Set the sizes for receive and transmit buffer. This function cannot
// be called if the port is already open, you have to call it after
// constructing the object or after a close. The function may be ignored
// if it is not possible to change buffer sizes.
{
    Port.SetBufferSize (aRXBufSize, aTXBufSize);
}



void SerialStream::SetReadTimeout (double T)
// Set the read timeout for the stream.
// A value of zero means no timeout (error if data not immidiately
// available) a value less than zero means "indefinite wait".
// Beware: The implementation of the timeout is device dependent! As an
// example, there may be devices that cannot do timeouts less than one
// second.
{
    // Set the timeout and re-read it
    Port.SetRXTimeout (T);
    ReadTimeout = Port.GetRXTimeout ();
}



void SerialStream::SetWriteTimeout (double T)
// Set the write timeout for the stream.
// A value of zero means no timeout (error if data cannot be written
// immidiately) a value less than zero means "indefinite wait".
// Beware: The implementation of the timeouts is device dependent! As an
// example, there may be devices that cannot do timeouts less than one
// second.
{
    // Set the timeout and re-read it
    Port.SetTXTimeout (T);
    ReadTimeout = Port.GetTXTimeout ();
}



size_t SerialStream::ReadCount () const
// Return the amount of bytes that can be read without waiting. On many
// devices it is impossible to tell the exact number, in these cases the
// function returns a value of 1 if at least one byte can be read without
// blocking and 0 otherwise.
{
    return Port.RXCount ();
}



size_t SerialStream::WriteCount () const
// Return the amount of bytes that can be written without waiting. On many
// devices it is impossible to tell the exact number, in these cases the
// function returns a value of 1 if at least one byte can be written without
// blocking and 0 otherwise.
{
    return Port.TXFree ();
}



void SerialStream::Read (void* Buf, size_t Count)
// Read from the stream
{
    // Ignore zero sized reads
    if (Count == 0) {
        return;
    }

    // Be shure to read only if the status is ok
    if (GetStatus () == stOk) {

        // Try to read the given amount of characters
        u32 ReadCount;
        Port.TimedReceiveBlock (Buf, Count, ReadCount);

        // If we did not get all characters, we have a timeout condition
        if (ReadCount != Count) {
            Error (stReadTimeout, ReadCount);
        }
    }
}



void SerialStream::Write (const void* Buf, size_t Count)
// Write to the stream
{
    // Ignore zero sized writes
    if (Count == 0) {
        return;
    }

    // Be shure to write only if the status of the stream is ok
    if (GetStatus () == stOk) {

        // Try to write the given amount of characters
        u32 WriteCount;
        Port.TimedSendBlock (Buf, Count, WriteCount);

        // If we could not write all characters, we have a timeout condition
        if (WriteCount != Count) {
            Error (stWriteTimeout, WriteCount);
        }
    }
}



