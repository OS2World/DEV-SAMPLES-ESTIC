/*****************************************************************************/
/*                                                                           */
/*                                 SERCOM.CC                                 */
/*                                                                           */
/* (C) 1993,94  Michael Peschel                                              */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



// System independent serial communication package for the SPUNK library,
// OS/2 version.



#include <stdio.h>
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSPROCESS
#include <os2.h>

#include "check.h"
#include "chartype.h"
#include "delay.h"
#include "sercom.h"



/*****************************************************************************/
/*                            Types and constants                            */
/*****************************************************************************/



// additional commands, not implemented in bsedev.h
#define ASYNC_SETEXTBAUDRATE    0x0043
#define ASYNC_SETENHPARAMETER   0x0054
#define ASYNC_GETEXTBAUDRATE    0x0063
#define ASYNC_GETENHPARAMETER   0x0074



// extended mode structures

// for setting baudrates > 57500 baud
typedef struct _EXTBAUDRATE {
        ULONG   BAUDRATE;
        BYTE    FRACTION;
} EXTBAUDRATE;
typedef EXTBAUDRATE *PEXTBAUDRATE;



// checking the actual baudrate and the limits
typedef struct _QEXTBAUDRATE {
        ULONG   BAUDRATE;
        BYTE    FRACTBAUD;
        ULONG   MINBAUDRATE;
        BYTE    MINFRACTBAUD;
        ULONG   MAXBAUDRATE;
        BYTE    MAXFRACTBAUD;
} QEXTBAUDRATE;
typedef QEXTBAUDRATE *PQEXTBAUDRATE;



// checking the enhanced parameters
typedef struct _ENHPARAMETER {
        BYTE    EnhFlags;
        ULONG   Reserverd;
} ENHPARAMETER;
typedef ENHPARAMETER *PENHPARAMETER;



// enhanced parameter masks, ASYNC_GETENHPARAMETER
#define ENHANCEDMODE_SUPPORTED  0x01
#define ENHANCEDMODE_ENABLE     0x02
#define DMA_RX_ENABLE           0x04
#define DMA_RX_DEDICATE         0x08
#define DMA_TX_ENABLE           0x10
#define DMA_TX_DEDICATE         0x20
#define DMA_RX_MODE             0x40
#define DMA_TX_MODE             0x80



// Code for "no error"
#ifndef NO_ERROR
#define NO_ERROR        0
#endif



// internal used data
struct _ComData {

    HFILE               hf;             // Port handle

    u16                 RXCount;        // characters in receive queue
    u16                 TXCount;        // characters in transmit queue
    DCBINFO             dcb;            // a copy of the actual dcb

    u16                 APICode;        // returncode of the last IOCtl-command
    ComErrorCounter     ErrorCounter;   // Error counters
};



/******************************************************************************/
/*                       Utility and support functions                        */
/******************************************************************************/



static void ComError (u32 ErrorCode)
// called for API Errors
{
    FAIL (FormatStr ("ComPort error #%d", ErrorCode).GetStr ());
}



static void ASYNC_IOCtl (_ComData* ComData,
                         u32 Function,
                         void *ParmList, u32 ParmLen,
                         void *DataList, u32 DataLen)
// async device control
{
    ULONG ParmLenInOut = ParmLen;
    ULONG DataLenInOut = DataLen;

    ComData->APICode = DosDevIOCtl (ComData->hf, IOCTL_ASYNC, Function,
                                    ParmList, ParmLen, &ParmLenInOut,
                                    DataList, DataLen, &DataLenInOut);
    if (ComData->APICode != NO_ERROR) {
        ComError (ComData->APICode);
    }
}



static void GENERAL_IOCtl (_ComData* ComData,
                           u32 Function,
                           void *ParmList, u32 ParmLen,
                           void *DataList, u32 DataLen)
// general device control
{
    ULONG ParmLenInOut = ParmLen;
    ULONG DataLenInOut = DataLen;

    ComData->APICode = DosDevIOCtl (ComData->hf, IOCTL_GENERAL, Function,
                                    ParmList, ParmLen, &ParmLenInOut,
                                    DataList, DataLen, &DataLenInOut);
    if (ComData->APICode != NO_ERROR) {
        ComError (ComData->APICode);
    }
}



static void UpdateErrorCounter (_ComData* ComData, unsigned ErrorWord)
// Error counter update
{
    if (ErrorWord & RX_QUE_OVERRUN)       ComData->ErrorCounter [ceRXOverflow]++;
    if (ErrorWord & RX_HARDWARE_OVERRUN)  ComData->ErrorCounter [ceOverrun]++;
    if (ErrorWord & PARITY_ERROR)         ComData->ErrorCounter [ceParity]++;
    if (ErrorWord & FRAMING_ERROR)        ComData->ErrorCounter [ceFrame]++;
}



static void GetComError (_ComData* ComData)
// gets the error state of the port and updates the error counters
{
    // Read the error information
    u16     Error;
    ASYNC_IOCtl (ComData, ASYNC_GETCOMMERROR, NULL, 0, &Error, sizeof (Error));

    // Update the counters
    UpdateErrorCounter (ComData, Error);
}



/******************************************************************************/
/*                                   Code                                     */
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
    RXBufSize (512),                    // Just some value...
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

    // Reset the error counters
    memset (ComData->ErrorCounter, 0, sizeof (ComData->ErrorCounter));

    // Set up ComData
    ComData->hf        = -1;                // Port not open
    ComData->RXCount   = 0;
    ComData->TXCount   = 0;
}



void ComPort::SetBufferSize (u16 /*aRXBufSize*/, u16 /*aTXBufSize*/)
// Set the sizes for receive and transmit buffer. This function cannot
// be called if the port is already open, you have to call it after
// constructing the object or after a close.
{
    // This function cannot be called if the port is already open
    PRECONDITION (!IsOpen ());

    // Otherwise this function is ignored under OS/2 as we can not set any
    // buffer sizes (no harm done anyway)
}



unsigned ComPort::Open ()
// Open the port, return an error code or 0 on success
{
    HFILE       hf;
    ULONG       ulAction;
    DCBINFO     DCBInfo;
    EXTBAUDRATE Baud;
    LINECONTROL LineCtrl;
    RXQUEUE     Queue;


    // Open the port
    APIRET rc = DosOpen ((PSZ) PortName.GetStr (), &hf, &ulAction,
                         0, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                         OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE,
                         (PEAOP2) NULL);
    if (rc) {
        return rc;
    }

    // Port-Handle setzen
    ComData->hf = hf;

    // actual settings
    ASYNC_IOCtl (ComData, ASYNC_GETDCBINFO, NULL, 0, &DCBInfo, sizeof (DCBInfo));

    // setting timeouts to approx. 30 character times (assuming 1 char = 10 bit)
    DCBInfo.usWriteTimeout = (30 * 100) / (Baudrate / 10);
    DCBInfo.usReadTimeout  = DCBInfo.usWriteTimeout;

    // Flow Control
    if (XonXoff == 'E') {                     // enable XON/XOFF
        DCBInfo.fbFlowReplace = 0xA3;         // 1010 0011
    } else {                                  // disable XON/XOFF
        DCBInfo.fbFlowReplace = 0xA0;         // 1010 0000
    }
    if (Connection == 'D') {                  // direct connection
        DCBInfo.fbFlowReplace &= 0x3F;        // disable RTS handshake
    }

    // Handshake
    if (Connection == 'M') {                  // using RTS/CTS, DTR/DSR handshake
        DCBInfo.fbCtlHndShake = 0x58;         // 01011000
    } else {
        DCBInfo.fbCtlHndShake = 0x00;         // 00000000
    }

    // Set timeouts and buffers. Beware: Enabling extended hardware buffering
    // on a device that does not support this setting results in an error.
    if ((DCBInfo.fbTimeout & 0x18) == 0) {
        // Device does not support extended hardware buffering
        DCBInfo.fbTimeout = 0x02;
    } else {
        // Extended hardware buffering supported, enable it
        DCBInfo.fbTimeout = 0xD2;               // 11010010
    }

    // Set new DCB
    ASYNC_IOCtl (ComData, ASYNC_SETDCBINFO, &DCBInfo, sizeof (DCBInfo), NULL, 0);

    // Get a copy
    ComData->dcb = DCBInfo;

    // set up the baudrate
    Baud.BAUDRATE = Baudrate;
    Baud.FRACTION = 0;

    ASYNC_IOCtl (ComData, ASYNC_SETEXTBAUDRATE, &Baud, sizeof (Baud), NULL, 0);

    // line control
    LineCtrl.bDataBits  = Databits;
    LineCtrl.bStopBits  = Stopbits-1;

    switch (Parity) {
        case 'N' : LineCtrl.bParity = 0; break;
        case 'O' : LineCtrl.bParity = 1; break;
        case 'E' : LineCtrl.bParity = 2; break;
        case 'M' : LineCtrl.bParity = 3; break;
        case 'S' : LineCtrl.bParity = 4; break;
        default  : FAIL ("ComPort::Open: Invalid parity setting");
    }
    ASYNC_IOCtl (ComData, ASYNC_SETLINECTRL, &LineCtrl, sizeof (LineCtrl), NULL, 0);

    // Get the buffer sizes
    ASYNC_IOCtl (ComData, ASYNC_GETINQUECOUNT, NULL, 0, &Queue, sizeof (Queue));
    RXBufSize = Queue.cb;
    ASYNC_IOCtl (ComData, ASYNC_GETOUTQUECOUNT, NULL, 0, &Queue, sizeof (Queue));
    TXBufSize = Queue.cb;

    // Success
    return 0;
}



void ComPort::Close ()
// Close the port
{
    // Cannot close a port that is not open...
    PRECONDITION (IsOpen ());

    // allow changing RTS. RTS is set to OFF when closing the port
    ComData->dcb.fbFlowReplace &= 0x3F;
    ComData->dcb.fbFlowReplace |= 0x40;
    ASYNC_IOCtl (ComData, ASYNC_SETDCBINFO, &ComData->dcb, sizeof (ComData->dcb), NULL, 0);

    // closing
    DosClose (ComData->hf);

    // reset handle
    ComData->hf = -1;
}



int ComPort::IsOpen () const
// Return a value != zero if the port is opened, return 0 otherwise
{
    return ComData->hf != -1;
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
    ComData->dcb.usReadTimeout = (aRXTimeout * 100) - 1;
    ASYNC_IOCtl (ComData, ASYNC_SETDCBINFO, &ComData->dcb, sizeof (ComData->dcb), NULL, 0);
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
    ComData->dcb.usWriteTimeout = (aTXTimeout * 100) - 1;
    ASYNC_IOCtl (ComData, ASYNC_SETDCBINFO, &ComData->dcb, sizeof (ComData->dcb), NULL, 0);
}



void ComPort::DTROn ()
// Make the DTR line active
{
    MODEMSTATUS ModemState;
    USHORT      ErrorWord;

    // The port must be open
    PRECONDITION (IsOpen ());

    /* setting bitmasks */
    ModemState.fbModemOn  = DTR_ON;
    ModemState.fbModemOff = 0xFF;

    /* calling the driver */
    ASYNC_IOCtl (ComData,
                 ASYNC_SETMODEMCTRL,
                 &ModemState, sizeof (ModemState),
                 &ErrorWord, sizeof (ErrorWord));


    /* get and reset errors */
    if (ErrorWord) {
        GetComError (ComData);
    }
}



void ComPort::DTROff ()
// Make the DTR line inactive
{
    MODEMSTATUS ModemState;
    USHORT      ErrorWord;

    // The port must be open
    PRECONDITION (IsOpen ());

    /* setting masks */
    ModemState.fbModemOn  = 0;
    ModemState.fbModemOff = DTR_OFF;

    /* calling the driver */
    ASYNC_IOCtl (ComData,
                 ASYNC_SETMODEMCTRL,
                 &ModemState, sizeof (ModemState),
                 &ErrorWord, sizeof (ErrorWord));

    /* get and reset errors */
    if (ErrorWord) {
        GetComError (ComData);
    }
}



void ComPort::RTSOn ()
// Make the RTS line active
{
    MODEMSTATUS ModemState;
    USHORT      ErrorWord;

    // The call is not allowed if the connection type is 'M'odem
    PRECONDITION (IsOpen () && Connection != 'M');

    /* setting bitmasks */
    ModemState.fbModemOn  = RTS_ON;
    ModemState.fbModemOff = 0xFF;

    /* calling the driver */
    ASYNC_IOCtl (ComData,
                 ASYNC_SETMODEMCTRL,
                 &ModemState, sizeof (ModemState),
                 &ErrorWord, sizeof (ErrorWord));


    /* get and reset errors */
    if (ErrorWord) {
        GetComError (ComData);
    }
}



void ComPort::RTSOff ()
// Make the RTS line inactive
{
    MODEMSTATUS ModemState;
    USHORT      ErrorWord;

    // The call is not allowed if the connection type is 'M'odem
    PRECONDITION (IsOpen () && Connection != 'M');

    /* setting masks */
    ModemState.fbModemOn  = 0;
    ModemState.fbModemOff = RTS_OFF;

    /* calling the driver */
    ASYNC_IOCtl (ComData,
                 ASYNC_SETMODEMCTRL,
                 &ModemState, sizeof (ModemState),
                 &ErrorWord, sizeof (ErrorWord));

    /* get and reset errors */
    if (ErrorWord) {
        GetComError (ComData);
    }
}



unsigned ComPort::RXCount () const
// Return the count of chars in the receive buffer
{
    RXQUEUE RXQueue;

    ASYNC_IOCtl (ComData, ASYNC_GETINQUECOUNT, NULL, 0, &RXQueue, sizeof (RXQueue));
    return RXQueue.cch;
}



unsigned ComPort::TXCount () const
// Return the count of chars in the transmit buffer
{
    RXQUEUE TXQueue;    // same data format as RXQueue

    ASYNC_IOCtl (ComData, ASYNC_GETOUTQUECOUNT, NULL, 0, &TXQueue, sizeof (TXQueue));
    return (TXQueue.cch);
}



void ComPort::RXClear ()
// Clear the receive buffer
{
    BYTE  ParmList     = 0;
    BYTE  DataList     = 0;

    GENERAL_IOCtl (ComData,
                   DEV_FLUSHINPUT,
                   &ParmList, sizeof (ParmList),
                   &DataList, sizeof (DataList));
}



void ComPort::TXClear ()
// Clear the transmit buffer
{

    BYTE  ParmList     = 0;
    BYTE  DataList     = 0;

    GENERAL_IOCtl (ComData,
                   DEV_FLUSHOUTPUT,
                   &ParmList, sizeof (ParmList),
                   &DataList, sizeof (DataList));
}



unsigned ComPort::TXFree () const
// Return the amount of free space in the transmit buffer
{
    return TXBufSize - TXCount ();
}



int ComPort::Receive ()
// Return a character from the receive buffer. If the buffer is empty,
// the function waits until a character is available.
{
    // read one character
    unsigned char    B;
    ULONG chRead;
    do {
        DosRead (ComData->hf, &B, 1, &chRead);
    } while (chRead == 0);

    // get errors
    GetComError (ComData);

    // return character
    return B;
}



int ComPort::Send (unsigned char B)
// Send the character (put it into the transmit buffer). If there is no
// room in the transmit buffer, the error counter is incremented, the
// character is discarded and the function returns -1. To avoid this,
// check TXFree before calling this function. If the character could be
// placed into the transmit buffer, B is returned.
{
    ULONG chWrite;

    if (TXFree () == 0) {
        ComData->ErrorCounter [ceTXOverflow]++;
        return -1;
    } else {
        DosWrite (ComData->hf, &B, 1, &chWrite);
        GetComError (ComData);
        return B;
    }
}



int ComPort::TimedReceive ()
// Return a character from the receive buffer. If the buffer is empty,
// the function waits until a character is available or the time given
// with SetReceiveTimeout is over. If a timeout condition occurred, the
// function returns -1, otherwise the character received.
{
    unsigned char B;
    ULONG   chRead;

    DosRead (ComData->hf, &B, 1, &chRead);

    // get errors
    GetComError (ComData);

    // Return the character read or -1 on timeout
    return chRead == 0 ? -1 : B;
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
    ULONG chWrite;

    DosWrite (ComData->hf, &B, 1, &chWrite);
    GetComError (ComData);
    if (chWrite != 1) {
        ComData->ErrorCounter [ceTXOverflow]++;
        return -1;
    } else {
        return B;
    }
}



void ComPort::TimedReceiveBlock (void* Buffer, u32 Count, u32& ReadCount)
// Wait until Count characters are read or the timeout is over. The
// variable ReadCount returns the amount of character actually read.
{
    // Check params
    PRECONDITION (Count > 0);

    // Read with timeout
    DosRead (ComData->hf, Buffer, Count, PULONG (&ReadCount));

    // Get errors
    GetComError (ComData);
}



void ComPort::TimedSendBlock  (const void* Buffer, u32 Count, u32& WriteCount)
// Wait until Count characters have been written  or the timeout is over.
// The variable WriteCount returns the amount of character actually written.
// If a timeout condition occurs, TXOverflow is incremented.
{
    // Check params
    PRECONDITION (Count > 0);

    DosWrite (ComData->hf, (void*) Buffer, Count, PULONG (&WriteCount));
    if (WriteCount != Count) {
        ComData->ErrorCounter [ceTXOverflow]++;
    }
    GetComError (ComData);
}



void ComPort::Break (double Duration)
// Send a break with the given time in seconds
{
    USHORT  ErrorWord;

    // BREAK ON
    ASYNC_IOCtl (ComData, ASYNC_SETBREAKON, NULL, 0, &ErrorWord, sizeof (ErrorWord));

    // Wait...
    Delay (Duration * 1000);

    // BREAK OFF
    ASYNC_IOCtl (ComData, ASYNC_SETBREAKOFF, NULL, 0, &ErrorWord, sizeof (ErrorWord));
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
    BYTE    Inputs;
    USHORT  Events;

    // Read the inputs
    ASYNC_IOCtl (ComData, ASYNC_GETMODEMINPUT, NULL, 0, &Inputs, sizeof (Inputs));

    // Read the event mask
    ASYNC_IOCtl (ComData, ASYNC_GETCOMMEVENT, NULL, 0, &Events, sizeof (Events));

    // copy RI to bit 6
    if (Events & 0x0100) {
        Events |= 0x0040;
    } else {
        Events &= 0xFFBF;
    }

    // shift to bits 0 - 3
    Events >>= 3;

    // Return the bitmask
    return ((Events & 0x0F) | (Inputs & 0xF0));
}



