/*****************************************************************************/
/*                                                                           */
/*                                 SERSTRM.H                                 */
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



#ifndef _SERSTRM_H
#define _SERSTRM_H



#include "sercom.h"
#include "charstrm.h"



/*****************************************************************************/
/*                            class SerialStream                             */
/*****************************************************************************/



class SerialStream: public CharacterStream {

protected:
    ComPort     Port;

public:
    SerialStream (const String& aPortName,
                  u32  aBaudrate    = 9600,
                  char aDatabits    =    8,  // 5..8
                  char aParity      =  'N',  // <N>one, <O>dd, <E>ven, <M>ark, <S>pace
                  char aStopbits    =    1,  // 1, 2
                  char aConnection  =  'M',  // <D>irect, <M>odem
                  char aXonXoff     =  'D',  // <E>nabled, <D>isabled
                  unsigned UARTBase = 0,     // I/O base address, DOS only
                  unsigned IntNum   = 0      // Number of interrupt used, DOS only
                 );
    // Create a SerialStream object, use defaults for timeouts and buffer sizes

    virtual ~SerialStream ();
    // Close the port

    void SetBufferSize (unsigned aRXBufSize, unsigned aTXBufSize);
    // Set the sizes for receive and transmit buffer. This function cannot
    // be called if the port is already open, you have to call it after
    // constructing the object or after a close. The function may be ignored
    // if it is not possible to change buffer sizes.

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

    virtual void Read (void* Buf, size_t Count);
    // Read from the stream

    virtual void Write (const void* Buf, size_t Count);
    // Write to the stream

};



// End of SERSTRM.H

#endif



