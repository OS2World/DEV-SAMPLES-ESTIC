/*****************************************************************************/
/*                                                                           */
/*                                 SERCOM.CC                                 */
/*                                                                           */
/*                      (C) 1996 MU Softwareentwicklung                      */
/*                                                                           */
/*      Ullrich von Bassewitz                          Michael Peschel       */
/*      Wacholderweg 14                                   Ledergasse 3       */
/*      D-70597 Stuttgart                            D-72555 Metzingen       */
/*      uz@ibb.schwaben.com                      mipe@ibb.schwaben.com       */
/*                                                                           */
/*****************************************************************************/



// System independent serial communication package for the SPUNK library,
// NT version.



#include <stdio.h>

#include <windows.h>

#include "check.h"
#include "chartype.h"
#include "delay.h"
#include "sercom.h"



/*****************************************************************************/
/*                            Types and constants                            */
/*****************************************************************************/



// internal used data
struct _ComData {
    HANDLE              hCom;           // Port handle
    DCB                 dcb;            // a copy of the actual dcb
    ComErrorCounter     ErrorCounter;   // Error counters
};



/******************************************************************************/
/*                       Utility and support functions                        */
/******************************************************************************/



static void ComError ()
// called for API Errors
{
    FAIL (FormatStr ("ComPort error #%d", GetLastError ()).GetStr ());
}



static void UpdateErrorCounter (_ComData* ComData, unsigned ErrorWord)
// Error counter update
{
    if (ErrorWord & CE_TXFULL)      ComData->ErrorCounter [ceTXOverflow]++;
    if (ErrorWord & CE_RXOVER)      ComData->ErrorCounter [ceRXOverflow]++;
    if (ErrorWord & CE_OVERRUN)     ComData->ErrorCounter [ceOverrun]++;
    if (ErrorWord & CE_RXPARITY)    ComData->ErrorCounter [ceParity]++;
    if (ErrorWord & CE_FRAME)       ComData->ErrorCounter [ceFrame]++;
    if (ErrorWord & CE_BREAK)       ComData->ErrorCounter [ceBreak]++;
}



static void GetComError (_ComData* ComData)
// gets the error state of the port and updates the error counters
{
    // Read the error information
    DWORD Error;
    COMSTAT Stat;

    if (!ClearCommError (ComData->hCom, &Error, &Stat)) {
        ComError ();
    }

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
    RXBufSize (512),
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

    // Set up ComData
    ComData->hCom      = INVALID_HANDLE_VALUE;    // Port not open

    // reset dcb data
    memset (&(ComData->dcb), 0, sizeof (ComData->dcb));

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

    // get values
    RXBufSize = aRXBufSize;
    TXBufSize = aTXBufSize;
}



unsigned ComPort::Open ()
// Open the port, return an error code or 0 on success
{
    HANDLE      hCom;
    DCB         dcb;

    // Open the port
    hCom = CreateFile ((PSZ) PortName.GetStr (),
                       GENERIC_READ | GENERIC_WRITE,
                       0,               // exclusive access
                       NULL,            // no security attr.
                       OPEN_EXISTING,   // Com ports must use this
                       0,               // no overlapped I/O
                       NULL);           // must be NULL for com devices

    if (hCom == INVALID_HANDLE_VALUE) {
        ComError ();
    }

    // Port-Handle setzen
    ComData->hCom = hCom;

    // actual settings
    if (!GetCommState (hCom, &dcb)) {
        ComError ();
    }

    // abort on errors
    dcb.fAbortOnError = 1;

    // set up the baudrate
    dcb.BaudRate = Baudrate;

    // set databits
    dcb.ByteSize = Databits;

    // set stopbits
    switch (Stopbits) {
        case 1 : dcb.StopBits = ONESTOPBIT;   break;
        case 2 : dcb.StopBits = TWOSTOPBITS;  break;
        default  : FAIL ("ComPort::Open: Invalid stopbit setting");
    }

    // set parity
    dcb.fParity = 1;           // Enable parity checking
    switch (Parity) {
        case 'N' : dcb.Parity = NOPARITY;    break;
        case 'O' : dcb.Parity = ODDPARITY;   break;
        case 'E' : dcb.Parity = EVENPARITY;  break;
        case 'M' : dcb.Parity = MARKPARITY;  break;
        case 'S' : dcb.Parity = SPACEPARITY; break;
        default  : FAIL ("ComPort::Open: Invalid parity setting");
    }


    // Flow Control
    if (XonXoff == 'E') {                     // enable XON/XOFF
        dcb.fOutX = 1;
        dcb.fInX  = 1;
    } else {                                  // disable XON/XOFF
        dcb.fOutX = 0;
        dcb.fInX  = 0;
    }

    // Handshake
    if (Connection == 'D') {
        // direct connection
        dcb.fOutxCtsFlow    = 0;
        dcb.fOutxDsrFlow    = 0;
        dcb.fDtrControl     = DTR_CONTROL_DISABLE;
        dcb.fDsrSensitivity = 0;
        dcb.fRtsControl     = RTS_CONTROL_DISABLE;

    } else if (Connection == 'M') {
        // using RTS/CTS, DTR/DSR handshake
        dcb.fOutxCtsFlow    = 1;
        dcb.fOutxDsrFlow    = 1;
        dcb.fDtrControl     = DTR_CONTROL_DISABLE;
        dcb.fDsrSensitivity = 1;
        dcb.fRtsControl     = RTS_CONTROL_HANDSHAKE;

    }  else {
        FAIL ("ComPort: invalid connection setting");
    }

    // set new dcb
    if (!SetCommState (hCom, &dcb)) {
        ComError ();
    }

    // store a copy of actual dcb
    ComData->dcb = dcb;

    // read actual timeout settings
    COMMTIMEOUTS Timeouts;
    if (!GetCommTimeouts (ComData->hCom, &Timeouts)) {
        ComError ();
    }

    // Set timeouts
    Timeouts.ReadIntervalTimeout         = RXTimeout * 1000;
    Timeouts.ReadTotalTimeoutMultiplier  = 0;
    Timeouts.ReadTotalTimeoutConstant    = RXTimeout * 1000;
    Timeouts.WriteTotalTimeoutMultiplier = 0;
    Timeouts.WriteTotalTimeoutConstant   = TXTimeout * 1000;

    if (!SetCommTimeouts (ComData->hCom, &Timeouts)) {
        ComError ();
    }

    // Set the buffer sizes
    if (!SetupComm (hCom, RXBufSize, TXBufSize)) {
        ComError ();
    }

    // because it is up to the driver what he does with the buffer values,
    // so read them back to have the real settings.
    // The Microsoft driver always has a TXBufSize of 0 !!
    COMMPROP Properties;
    if (!GetCommProperties (hCom, &Properties)) {
        ComError ();
    }

    // get current buffer settings
    RXBufSize = Properties.dwCurrentRxQueue;
    TXBufSize = Properties.dwCurrentTxQueue;

    // Success
    return 0;
}



void ComPort::Close ()
// Close the port
{
    // Cannot close a port that is not open...
    PRECONDITION (IsOpen ());

    // set DTR and RTS to OFF before closing the port
    DTROff ();
    RTSOff ();

    // closing
    CloseHandle (ComData->hCom);

    // reset handle
    ComData->hCom = INVALID_HANDLE_VALUE;
}



int ComPort::IsOpen () const
// Return a value != zero if the port is opened, return 0 otherwise
{
    return ComData->hCom != INVALID_HANDLE_VALUE;
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

    // read actual settings
    COMMTIMEOUTS Timeouts;
    if (!GetCommTimeouts (ComData->hCom, &Timeouts)) {
        ComError ();
    }

    // Set the new timeout
    Timeouts.ReadIntervalTimeout      = RXTimeout * 1000;
    Timeouts.ReadTotalTimeoutConstant = RXTimeout * 1000;

    if (!SetCommTimeouts (ComData->hCom, &Timeouts)) {
        ComError ();
    }
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

    // read actual settings
    COMMTIMEOUTS Timeouts;
    if (!GetCommTimeouts (ComData->hCom, &Timeouts)) {
        ComError ();
    }

    // Set the new timeout
    Timeouts.WriteTotalTimeoutConstant = TXTimeout * 1000;

    if (!SetCommTimeouts (ComData->hCom, &Timeouts)) {
        ComError ();
    }
}



void ComPort::DTROn ()
// Make the DTR line active
{
    // The port must be open
    PRECONDITION (IsOpen ());

    if (!EscapeCommFunction (ComData->hCom, SETDTR)) {
        ComError ();
    }
}



void ComPort::DTROff ()
// Make the DTR line inactive
{
    // The port must be open
    PRECONDITION (IsOpen ());

    if (!EscapeCommFunction (ComData->hCom, CLRDTR)) {
        ComError ();
    }
}



void ComPort::RTSOn ()
// Make the RTS line active
{
    // The port must be open
    PRECONDITION (IsOpen ());

    if (!EscapeCommFunction (ComData->hCom, SETRTS)) {
        ComError ();
    }
}



void ComPort::RTSOff ()
// Make the RTS line inactive
{
    // The port must be open
    PRECONDITION (IsOpen ());

    if (!EscapeCommFunction (ComData->hCom, CLRRTS)) {
        ComError ();
    }
}



unsigned ComPort::RXCount () const
// Return the count of chars in the receive buffer
{
    // Read the error information (aha)
    DWORD Error;
    COMSTAT Stat;

    if (!ClearCommError (ComData->hCom, &Error, &Stat)) {
        ComError ();
    }

    // Update the counters
    UpdateErrorCounter (ComData, Error);

    // retrieve count
    return Stat.cbInQue;
}



unsigned ComPort::TXCount () const
// Return the count of chars in the transmit buffer
{
    // Read the error information (aha)
    DWORD Error;
    COMSTAT Stat;

    if (!ClearCommError (ComData->hCom, &Error, &Stat)) {
        ComError ();
    }

    // Update the counters
    UpdateErrorCounter (ComData, Error);

    // retrieve count
    return Stat.cbOutQue;
}



void ComPort::RXClear ()
// Clear the receive buffer
{
    if (!PurgeComm (ComData->hCom, PURGE_RXCLEAR)) {
        ComError ();
    }
}



void ComPort::TXClear ()
// Clear the transmit buffer
{
    if (!PurgeComm (ComData->hCom, PURGE_TXCLEAR)) {
        ComError ();
    }
}



unsigned ComPort::TXFree () const
// Return the amount of free space in the transmit buffer
{
    if (TXBufSize > 0) {
        return TXBufSize - TXCount ();
    }
    return (TXCount () == 0) ? 1 : 0;
}



int ComPort::Receive ()
// Return a character from the receive buffer. If the buffer is empty,
// the function waits until a character is available.
{
    // read one character
    unsigned char    B;

    DWORD chRead;
    do {
        ReadFile (ComData->hCom, &B, 1, &chRead, NULL);
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

    if (TXFree () == 0) {
        ComData->ErrorCounter [ceTXOverflow]++;
        return -1;
    }

    DWORD chWrite;
    WriteFile (ComData->hCom, &B, 1, &chWrite, NULL);

    // check if the character was really written
    if (chWrite != 1) {
        GetComError (ComData);
        return -1;
    } else {
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
    DWORD    chRead;

    ReadFile (ComData->hCom, &B, 1, &chRead, NULL);

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

    WriteFile (ComData->hCom, &B, 1, &chWrite, NULL);
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
    ReadCount = 0;

    while (ReadCount < Count) {
        DWORD chRead   = 0;
        DWORD chToRead = min ((Count - ReadCount), 128);

        // read
        ReadFile (ComData->hCom, (char*) Buffer + ReadCount, chToRead, &chRead, NULL);

        // inc. ReadCount
        ReadCount += chRead;

        // check if all characters are read
        if (chToRead != chRead) {
            break;
        }
    }

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

    // Read with timeout
    WriteCount = 0;

    while (WriteCount < Count) {
        DWORD chWritten = 0;
        DWORD chToWrite = min ((Count - WriteCount), 128);

        // read
        WriteFile (ComData->hCom, (char*) Buffer + WriteCount, chToWrite, &chWritten, NULL);

        // inc. ReadCount
        WriteCount += chWritten;

        // check if all characters are read
        if (chToWrite != chWritten) {

            // Get errors
            ComData->ErrorCounter [ceTXOverflow]++;
            GetComError (ComData);

            break;
        }

    }

    GetComError (ComData);
}



void ComPort::Break (double Duration)
// Send a break with the given time in seconds
{
    // The port must be open
    PRECONDITION (IsOpen ());

    // BREAK ON
    SetCommBreak (ComData->hCom);

    // Wait...
    Delay (Duration * 1000);

    // BREAK OFF
    ClearCommBreak (ComData->hCom);
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
    DWORD    Inputs;
    if (!GetCommModemStatus (ComData->hCom, &Inputs)) {
        ComError ();
    }

    // convert status bits
    unsigned Status = 0;
    if (Inputs & MS_CTS_ON)    Status |= csCTS;
    if (Inputs & MS_DSR_ON)    Status |= csDSR;
    if (Inputs & MS_RING_ON)   Status |= csRI;
    if (Inputs & MS_RLSD_ON)   Status |= csCD;

    // Return the bitmask
    return Status;
}



