/*****************************************************************************/
/*									     */
/*				   SERCOM.CC				     */
/*									     */
/* (C) 1996	Ullrich von Bassewitz					     */
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



// System independent serial communication package for the SPUNK library,
// Netware version.



#include <stdio.h>
#include <errno.h>
#include <nwtypes.h>
#include <aio.h>

#include "check.h"
#include "chartype.h"
#include "delay.h"
#include "sercom.h"



/*****************************************************************************/
/*			      Types and constants			     */
/*****************************************************************************/



// internal used data
struct _ComData {

    int			Handle;		// Port handle
    int			AIOResult;	// Result of last AIO operation
    int			NewBufferSize;	// Buffer size has been changed

    ComErrorCounter	ErrorCounter;	// Error counters
};





/******************************************************************************/
/*			 Utility and support functions			      */
/******************************************************************************/



/******************************************************************************/
/*				     Code				      */
/******************************************************************************/



ComPort::ComPort (const String& aPortName,
		  u32  aBaudrate,
		  char aDatabits,
		  char aParity,
		  char aStopbits,
		  char aConnection,
		  char aXonXoff,
		  unsigned /*UARTBase*/,
		  unsigned /*IntNum*/):
    PortName (aPortName),
    Baudrate (aBaudrate),
    Databits (aDatabits),
    Parity (aParity),
    Stopbits (aStopbits),
    Connection (aConnection),
    XonXoff (aXonXoff),
    RXBufSize (512),			// Just some value...
    TXBufSize (512),
    RXTimeout (5.0),
    TXTimeout (5.0)
// Create a ComPort object, use defaults for timeouts and buffer sizes
{
    Init ();
}



ComPort::~ComPort ()
{
    if (IsOpen()) {
	Close();
    }
    delete ComData;
}



void ComPort::Init (unsigned /*UARTBase*/, unsigned /*IntNum*/)
// Initialization procedure, called from the constructors
{
    // If the name of the port is just a number, expand it by
    // inserting "COM".
    if (PortName.Len () == 1 && IsDigit (PortName [0])) {
	PortName.Ins (0, "COM");
    }

    // Create an internally used data structure
    ComData = new _ComData;

    // Port is currently not open
    ComData->Handle = -1;

    // Reset flag for new buffer size
    ComData->NewBufferSize = 0;

    // Reset the error counters
    memset (ComData->ErrorCounter, 0, sizeof (ComData->ErrorCounter));
}



void ComPort::SetBufferSize (u16 aRXBufSize, u16 aTXBufSize)
// Set the sizes for receive and transmit buffer. This function cannot
// be called if the port is already open, you have to call it after
// constructing the object or after a close.
{
    // This function cannot be called if the port is already open
    PRECONDITION (!IsOpen ());

    // Check the given parameters
    PRECONDITION (aRXBufSize >= 64 && aRXBufSize <= 16384);
    PRECONDITION (aTXBufSize >= 64 && aTXBufSize <= 16384);

    // Remember the new sizes
    RXBufSize = aRXBufSize;
    TXBufSize = aTXBufSize;

    // Set a flag for open
    ComData->NewBufferSize = 1;
}



unsigned ComPort::Open ()
// Open the port, return an error code or 0 on success
{
    // Try to aquire the port
    int HardwareType = AIO_COMX_TYPE;
    int BoardNumber  = AIO_BOARD_NUMBER_WILDCARD;
    int PortNumber   = AIO_PORT_NUMBER_WILDCARD;

    ComData->AIOResult = AIOAcquirePort (&HardwareType,
					 &BoardNumber,
					 &PortNumber,
					 &ComData->Handle);

    if (ComData->AIOResult != AIO_SUCCESS) {
	// Could not aquire port.
	ComData->Handle = -1;
	return EACCES;
    }

    // If the buffer sizes have been set, tell the OS to use the new sizes.
    // Otherwise read the current values and store them for later retrieval.
    if (ComData->NewBufferSize) {
	// Set the new sizes, errors are fatal
	CHECK (AIOSetReadBufferSize (ComData->Handle, RXBufSize) == AIO_SUCCESS);
	CHECK (AIOSetWriteBufferSize (ComData->Handle, TXBufSize) == AIO_SUCCESS);
    } else {
	// Get the buffer sizes and store them
	LONG ReadBufSize, WriteBufSize;
	CHECK (AIOGetReadBufferSize (ComData->Handle, &ReadBufSize) == AIO_SUCCESS);
	CHECK (AIOGetWriteBufferSize (ComData->Handle, &WriteBufSize) == AIO_SUCCESS);
	RXBufSize = (u16)ReadBufSize;
	TXBufSize = (u16)WriteBufSize;
    }

    // Set up the port configuration

    // --- Baudrate ---
    BYTE Bitrate;
    switch (Baudrate) {

	case 50:
	    Bitrate = AIO_BAUD_50;
	    break;

	case 75:
	    Bitrate = AIO_BAUD_75;
	    break;

	case 110:
	    Bitrate = AIO_BAUD_110;
	    break;

	case 150:
	    Bitrate = AIO_BAUD_150;
	    break;

	case 300:
	    Bitrate = AIO_BAUD_300;
	    break;

	case 600:
	    Bitrate = AIO_BAUD_600;
	    break;

	case 1200:
	    Bitrate = AIO_BAUD_1200;
	    break;

	case 1800:
	    Bitrate = AIO_BAUD_1800;
	    break;

	case 2000:
	    Bitrate = AIO_BAUD_2000;
	    break;

	case 2400:
	    Bitrate = AIO_BAUD_2400;
	    break;

	case 3600:
	    Bitrate = AIO_BAUD_3600;
	    break;

	case 4800:
	    Bitrate = AIO_BAUD_4800;
	    break;

	case 7200:
	    Bitrate = AIO_BAUD_7200;
	    break;

	case 9600:
	    Bitrate = AIO_BAUD_9600;
	    break;

	case 19200:
	    Bitrate = AIO_BAUD_19200;
	    break;

	case 38400:
	    Bitrate = AIO_BAUD_38400;
	    break;

	case 57600:
	    Bitrate = AIO_BAUD_57600;
	    break;

	case 115200:
	    Bitrate = AIO_BAUD_115200;
	    break;

	default:
	    FAIL ("ComPort::Open: Unsupported value for Baudrate");
    }

    // --- Number of data bits ---
    BYTE DatabitCount;
    switch (Databits) {

	case 5:
	    DatabitCount = AIO_DATA_BITS_5;
	    break;

	case 6:
	    DatabitCount = AIO_DATA_BITS_6;
	    break;

	case 7:
	    DatabitCount = AIO_DATA_BITS_7;
	    break;

	case 8:
	    DatabitCount = AIO_DATA_BITS_8;
	    break;

	default:
	    FAIL ("ComPort::Open: Unsupported value for Databits");
    }

    // --- Number of stop bits ---
    BYTE StopbitCount;
    switch (Stopbits) {

	case 1:
	    StopbitCount = AIO_STOP_BITS_1;
	    break;

	case 2:
	    StopbitCount = AIO_STOP_BITS_2;
	    break;

	default:
	    FAIL ("ComPort::Open: Unsupported value for Stopbits");
    }

    // --- parity mode ---
    BYTE ParityMode;
    switch (Parity) {

	case 'N':
	    ParityMode = AIO_PARITY_NONE;
	    break;

	case 'O':
	    ParityMode = AIO_PARITY_ODD;
	    break;

	case 'E':
	    ParityMode = AIO_PARITY_EVEN;
	    break;

	case 'M':
	    ParityMode = AIO_PARITY_MARK;
	    break;

	case 'S':
	    ParityMode = AIO_PARITY_SPACE;
	    break;

	default:
	    FAIL ("ComPort::Open: Unsupported value for Parity");
    }

    // --- flow control ---
    BYTE FlowControl = 0;
    switch (XonXoff) {

	case 'E':
	    FlowControl |= AIO_SOFTWARE_FLOWCONTROL_ON;
	    break;

	case 'D':
	    FlowControl |= AIO_SOFTWARE_FLOWCONTROL_OFF;
	    break;

	default:
	    FAIL ("ComPort::Open: Unsupported value for XonXoff");
    }
    switch (Connection) {

	case 'D':
	    FlowControl |= AIO_HARDWARE_FLOWCONTROL_OFF;
	    break;

	case 'M':
	    FlowControl |= AIO_HARDWARE_FLOWCONTROL_ON;
	    break;

	default:
	    FAIL ("ComPort::Open: Unsupported value for Connection");
    }

    // Try to configure the port
    ComData->AIOResult = AIOConfigurePort (ComData->Handle,
					   Bitrate,
					   DatabitCount,
					   StopbitCount,
					   ParityMode,
					   FlowControl);

    // Check for errors
    if (ComData->AIOResult != AIO_SUCCESS) {
	// Some data is invalid
	return EINVAL;
    }

    return 0;
}



void ComPort::Close ()
// Close the port
{
    // Cannot close a port that is not open...
    PRECONDITION (IsOpen ());

    // Close the port
    ComData->AIOResult = AIOReleasePort (ComData->Handle);

    // Invalidate the handle
    ComData->Handle = -1;
}



int ComPort::IsOpen () const
// Return a value != zero if the port is opened, return 0 otherwise
{
    return ComData->Handle != -1;
}



void ComPort::SetRXTimeout (double aRXTimeout)
// Set the timeout value
{
    // Check the parameter
    PRECONDITION (aRXTimeout >= 0.0);

    // Make shure the timeout is not too small
    if (aRXTimeout < 0.01) {
	aRXTimeout = 0.01;
    }

    // Remember the timeout
    RXTimeout = aRXTimeout;

    // Set the timeout
    // ##
}



void ComPort::SetTXTimeout (double aTXTimeout)
// Set the timeout value
{
    // Check the parameter
    PRECONDITION (aTXTimeout >= 0.0);

    // Make shure the timeout is not too small
    if (aTXTimeout < 0.01) {
	aTXTimeout = 0.01;
    }

    // Remember the timeout
    TXTimeout = aTXTimeout;

    // Set the timeout
    // ##
}



void ComPort::DTROn ()
// Make the DTR line active
{
    // The port must be open
    PRECONDITION (IsOpen ());

}



void ComPort::DTROff ()
// Make the DTR line inactive
{
    // The port must be open
    PRECONDITION (IsOpen ());

}



void ComPort::RTSOn ()
// Make the RTS line active
{
    // The port must be open and a modem connection is not allowed
    PRECONDITION (IsOpen () && Connection != 'M');

}



void ComPort::RTSOff ()
// Make the RTS line inactive
{
    // The port must be open and a modem connection is not allowed
    PRECONDITION (IsOpen () && Connection != 'M');

}



unsigned ComPort::RXCount () const
// Return the count of chars in the receive buffer
{
    LONG Count;
    WORD State;
    CHECK (AIOReadStatus (ComData->Handle, &Count, &State) == AIO_SUCCESS);
    return Count;
}



unsigned ComPort::TXCount () const
// Return the count of chars in the transmit buffer
{
    LONG Count;
    WORD State;
    CHECK (AIOWriteStatus (ComData->Handle, &Count, &State) == AIO_SUCCESS);
    return Count;
}



void ComPort::RXClear ()
// Clear the receive buffer
{
    CHECK (AIOFlushBuffers (ComData->Handle, AIO_FLUSH_READ_BUFFER) == AIO_SUCCESS);
}



void ComPort::TXClear ()
// Clear the transmit buffer
{
    CHECK (AIOFlushBuffers (ComData->Handle, AIO_FLUSH_READ_BUFFER) == AIO_SUCCESS);
}



unsigned ComPort::TXFree ()
// Return the amount of free space in the transmit buffer
{
    return TXBufSize - TXCount ();
}



int ComPort::Receive ()
// Return a character from the receive buffer. If the buffer is empty,
// the function waits until a character is available.
{
    char B;
    LONG BytesRead;

    do {
	CHECK (AIOReadData (ComData->Handle, &B, sizeof (B), &BytesRead) == AIO_SUCCESS);
    } while (BytesRead == 0);

    // return character
    return (int)(unsigned char)B;
}



int ComPort::Send (unsigned char B)
// Send the character (put it into the transmit buffer). If there is no
// room in the transmit buffer, the error counter is incremented, the
// character is discarded and the function returns -1. To avoid this,
// check TXFree before calling this function. If the character could be
// placed into the transmit buffer, B is returned.
{
    if (TXFree () == 0) {
	ComData->ErrorCounter [ceTXOverflow]++;
	return -1;
    } else {
	char C = (char) B;
	LONG BytesWritten;
	CHECK (AIOWriteData (ComData->Handle, &C, sizeof (C), &BytesWritten) == AIO_SUCCESS);
	return B;
    }
}



int ComPort::TimedReceive ()
// Return a character from the receive buffer. If the buffer is empty,
// the function waits until a character is available or the time given
// with SetReceiveTimeout is over. If a timeout condition occurred, the
// function returns -1, otherwise the character received.
{
    return 0;
}



int ComPort::TimedSend (unsigned char B)
// Send the character (put it into the transmit buffer). If there is no
// room in the transmit buffer, the function waits until there is free
// room in the transmit buffer or the time given with SetSendTimeout is
// over. If a timeout condition occurred, the error counter is incremented,
// the character is discarded and the function returns -1. To avoid this,
// check TXFree before calling this function. If the character could be
// placed into the transmit buffer, B is returned.
{
    return 0;
}



void ComPort::TimedReceiveBlock (void* Buffer, u32 Count, u32& ReadCount)
// Wait until Count characters are read or the timeout is over. The
// variable ReadCount returns the amount of character actually read.
{
    // Check params
    PRECONDITION (Count > 0);
}



void ComPort::TimedSendBlock  (const void* Buffer, u32 Count, u32& WriteCount)
// Wait until Count characters have been written  or the timeout is over.
// The variable WriteCount returns the amount of character actually written.
// If a timeout condition occurs, TXOverflow is incremented.
{
    // Check params
    PRECONDITION (Count > 0);
}



void ComPort::Break (double Duration)
// Send a break with the given time in seconds
{
    // BREAK ON
    CHECK (AIOSetExternalControl (ComData->Handle, AIO_BREAK_CONTROL, AIO_SET_BREAK_ON) == AIO_SUCCESS);

    // Wait...
    Delay (Duration * 1000);

    // BREAK OFF
    CHECK (AIOSetExternalControl (ComData->Handle, AIO_BREAK_CONTROL, AIO_SET_BREAK_OFF) == AIO_SUCCESS);
}



ComErrorCounter& ComPort::GetErrors ()
// Get a reference to the array of error counters. These counters are
// incremented but never decremented or zeroed by the object.
{
    return ComData->ErrorCounter;
}



unsigned ComPort::ModemStatus () const
// Return the state of the modem status lines
{
    LONG Status, Delta;
    CHECK (AIOGetExternalStatus (ComData->Handle, &Status, &Delta) == AIO_SUCCESS);

    // Convert to sercom format
    unsigned S = 0;
    if (Status & AIO_EXTSTA_CTS)	S |= csCTS;
    if (Status & AIO_EXTSTA_DSR)	S |= csDSR;
    if (Status & AIO_EXTSTA_RI)		S |= csRI;
    if (Status & AIO_EXTSTA_DCD)	S |= csCD;
    if (Delta  & AIO_EXTSTA_CTS)	S |= csDeltaCTS;
    if (Delta  & AIO_EXTSTA_DSR)	S |= csDeltaDSR;
    if (Delta  & AIO_EXTSTA_RI)		S |= csDeltaRI;
    if (Delta  & AIO_EXTSTA_DCD)	S |= csDeltaCD;

    // Return the result
    return S;
}



