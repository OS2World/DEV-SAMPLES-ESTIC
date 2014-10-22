/*****************************************************************************/
/*                                                                           */
/*                                 SERCOM.H                                  */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
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



// System independent serial communication package for the SPUNK library.
// Beware: Some functions are no ops on some architectures, others do not
// return exact values. You may assume a specific behaviour, but your
// programs will not be portable then.



#ifndef _SERCOM_H
#define _SERCOM_H



#include "strmable.h"
#include "str.h"



/*****************************************************************************/
/*                              Data and Types                               */
/*****************************************************************************/



// Constants for accessing the array of error counters
const ceRXOverflow              = 0;    // Receive buffer overflow
const ceTXOverflow              = 1;    // Transmit buffer overflow
const ceOverrun                 = 2;    // UART overrun
const ceBreak                   = 3;    // Break received
const ceFrame                   = 4;    // Framing error
const ceParity                  = 5;    // Parity error



// Bitmask constants for evaluating the result of ModemStatus
const csDeltaCTS                = 0x01; // Delta CTS
const csDeltaDSR                = 0x02; // Delta DSR
const csDeltaRI                 = 0x04; // Falling edge of RI
const csDeltaCarrierDetect      = 0x08; // Delta CD
const csClearToSend             = 0x10; // Clear To Send
const csDataSetReady            = 0x20; // Data Set Ready
const csRingIndicator           = 0x40; // Ring Indicator
const csCarrierDetect           = 0x80; // Carrier Detect
const csCTS                     = csClearToSend;
const csDSR                     = csDataSetReady;
const csRI                      = csRingIndicator;
const csCD                      = csCarrierDetect;
const csDeltaCD                 = csDeltaCarrierDetect;



// Type of an array with error counters
typedef u16 ComErrorCounter [6];



/*****************************************************************************/
/*                               class ComPort                               */
/*****************************************************************************/



// All implementation dependant data is contained in a structure that is
// defined in the CC file. Make a forward declaration here.
struct _ComData;



// ComPort class
class ComPort: public Streamable {

private:
    _ComData*   ComData;        // Pointer to implementation data


    // ComPort parameters
    String      PortName;
    u32         Baudrate;
    char        Databits;
    char        Parity;
    char        Stopbits;
    char        Connection;
    char        XonXoff;
    u16         RXBufSize;
    u16         TXBufSize;
    double      RXTimeout;
    double      TXTimeout;


    void Init (unsigned UARTBase = 0, unsigned IntNum = 0);
    // Initialization procedure, called from the constructors


public:
    ComPort (const String& aPortName,
             u32  aBaudrate    = 9600,
             char aDatabits    =    8,  // 5..8
             char aParity      =  'N',  // <N>one, <O>dd, <E>ven, <M>ark, <S>pace
             char aStopbits    =    1,  // 1, 2
             char aConnection  =  'M',  // <D>irect, <M>odem
             char aXonXoff     =  'D',  // <E>nabled, <D>isabled
             unsigned UARTBase = 0,     // I/O base address, DOS only
             unsigned IntNum   = 0      // Number of interrupt used, DOS only
            );
    // Create a ComPort object, use defaults for timeouts and buffer sizes

    virtual ~ComPort ();
    // Destruct a ComPort object

    void SetBufferSize (u16 aRXBufSize, u16 aTXBufSize);
    // Set the sizes for receive and transmit buffer. This function cannot
    // be called if the port is already open, you have to call it after
    // constructing the object or after a close. The function may be ignored
    // if it is not possible to change buffer sizes.

    unsigned Open ();
    // Open the port, return an error code or 0 on success

    void Close ();
    // Close the port

    int IsOpen () const;
    // Return a value != zero if the port is opened, return 0 otherwise

    void SetRXTimeout (double aRXTimeout);
    // Set the timeout value

    void SetTXTimeout (double aTXTimeout);
    // Set the timeout value

    double GetRXTimeout () const;
    // Get the timeout value

    double GetTXTimeout () const;
    // Get the timeout value

    void DTROn ();
    // Make the DTR line active

    void DTROff ();
    // Make the DTR line inactive

    void RTSOn ();
    // Make the RTS line active. A call to this function is not allowed if the
    // connection type is 'M'odem

    void RTSOff ();
    // Make the RTS line inactive. A call to this function is not allowed if the
    // connection type is 'M'odem

    unsigned RXCount () const;
    // Return the count of chars in the receive buffer, or just true 1 if the
    // exact amount of chars in the buffer cannot be determined and the value
    // is at least one.

    unsigned TXCount () const;
    // Return the count of chars in the transmit buffer. This function must
    // not exist, as it is impossible to determine the return value on some
    // operating systems.

    void RXClear ();
    // Clear the receive buffer. Maybe implemented as a no op function.

    void TXClear ();
    // Clear the transmit buffer. Maybe implemented as a no op function.

    unsigned TXFree () const;
    // Return the amount of free space in the transmit buffer. The function
    // may return the exact free space or just 1, if at least one character
    // can be placed into the send buffer (meaning, the Send function will
    // not block).

    int Receive ();
    // Return a character from the receive buffer. If the buffer is empty,
    // the function waits until a character is available.

    int Send (unsigned char B);
    // Send the character (put it into the transmit buffer). If there is no
    // room in the transmit buffer, the error counter is incremented, the
    // character is discarded and the function returns -1. To avoid this,
    // check TXFree before calling this function. If the character could be
    // placed into the transmit buffer, B is returned.

    int TimedReceive ();
    // Return a character from the receive buffer. If the buffer is empty,
    // the function waits until a character is available or the time given
    // with SetReceiveTimeout is over. If a timeout condition occurred, the
    // function returns -1, otherwise the character received.

    int TimedSend (unsigned char B);
    // Send the character (put it into the transmit buffer). If there is no
    // room in the transmit buffer, the function waits until there is free
    // room in the transmit buffer or the time given with SetSendTimeout is
    // over. If a timeout condition occurred, the error counter is incremented,
    // the character is discarded and the function returns -1. To avoid this,
    // check TXFree before calling this function. If the character could be
    // placed into the transmit buffer, B is returned.

    void TimedReceiveBlock (void* Buffer, u32 Count, u32& ReadCount);
    // Wait until Count characters are read or the timeout is over. The
    // variable ReadCount returns the amount of character actually read.

    void TimedSendBlock  (const void* Buffer, u32 Count, u32& WriteCount);
    // Wait until Count characters have been written  or the timeout is over.
    // The variable WriteCount returns the amount of character actually written.
    // If a timeout condition occurs, TXOverflow is incremented.

    void Break (double Duration);
    // Send a break with the given time in seconds

    ComErrorCounter& GetErrors ();
    // Get a reference to the array of error counters. These counters are
    // incremented but never decremented or zeroed by the object. On some
    // architectures com errors are handled by the operating system, so
    // there is no way to get this information. If this is true, all counters
    // will always be zero.

    unsigned ModemStatus () const;
    // Return the state of the modem status lines

    const String& GetPortName () const;
    // Return the name of the port

};



inline double ComPort::GetRXTimeout () const
// Get the timeout value
{
    return RXTimeout;
}



inline double ComPort::GetTXTimeout () const
// Get the timeout value
{
    return TXTimeout;
}



inline const String& ComPort::GetPortName () const
// Return the name of the port
{
    return PortName;
}



// End of SERCOM.H

#endif


