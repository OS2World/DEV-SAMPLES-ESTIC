/*****************************************************************************/
/*                                                                           */
/*                                 SERCOM.CC                                 */
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



// System independent serial communication package for the SPUNK library,
// DOS version (no extender).
// The DOS version of this module is just an OOP class that hides an older
// assembler/C module for serial communication. Because of the history of this
// older module, comments are in german, sorry.



#include <errno.h>
#include <dos.h>

#include "check.h"
#include "delay.h"
#include "sercom.h"



/******************************************************************************/
/*         Function prototypes for the external assembler functions           */
/******************************************************************************/



#ifdef __WATCOMC__
// For Watcom C specify new attributes for the pascal calling convention.
// The routines in the assembler module expect the ds segment register to
// contain the default data segment, so tell the compiler to reload ds
// before calling a function specified as pascal
#pragma aux __pascal    "^"                                     \
                        parm loadds reverse routine []          \
                        value struct float struct caller []     \
                        modify [ax bx cx dx es];
#endif




// Type of a port handle
typedef u16 HCOMPORT;



extern "C" {


HCOMPORT far pascal _ComInstall (HCOMPORT Port);
/* Installiert den COM-Port, �ffnet ihn aber nicht. Zur�ck kommt ein Port-
 * Handle das $FFFF ist wenn ein Fehler aufgetreten ist (Port existiert nicht).
 * Dieses PortHandle mu� bei der sp�teren Kommunikation mit den �brigen Routinen
 * des Pakets angegeben werden.
 */


void far pascal _ComDeinstall (HCOMPORT Port);
/* Deinstalliert einen Port. Falls der Port noch offen ist wird er von der
 * Prozedur zuerst geschlossen.
 */


int far pascal _ComIsInstalled (HCOMPORT Port);
/* Ergibt den Wert 1 wenn der Port installiert ist, ansonsten 0. */


void far pascal _ComOpen (HCOMPORT Port);
/* �ffnet den Port */


void far pascal _ComClose (HCOMPORT Port);
/* Schlie�t den Port */


int far pascal _ComIsOpen (HCOMPORT Port);
/* Ergibt den Wert 1 wenn der Port ge�ffnet ist, ansonsten 0. */


void far pascal _ComDTROn (HCOMPORT Port);
/* Macht die DTR-Leitung aktiv und gibt damit der Gegenstelle das Zeichen, da�
 * der Port aktiv ist.
 */


void far pascal _ComDTROff (HCOMPORT Port);
/* Macht die DTR-Leitung inaktiv und gibt damit der Gegenstelle zu erkennen,
 * da� der Port (bzw. der PC) inaktiv ist.
 */


void far pascal _ComRTSOn (HCOMPORT Port);
// Activate the RTS line. This is allowed only if the connection type of the
// port is *not* 'M'odem.


void far pascal _ComRTSOff (HCOMPORT Port);
// Deactivate the RTS line. This is allowed only if the connection type of the
// port is *not* 'M'odem.


unsigned far pascal _ComRXSize (HCOMPORT Port);
/* Gibt die Gr��e des Empfangspuffers zur�ck */


unsigned far pascal _ComRXCount (HCOMPORT Port);
/* Gibt die Anzahl Zeichen im Empfangspuffer zur�ck */


void far pascal _ComRXClear (HCOMPORT Port);
/* L�scht den kompletten Sendepuffer */


unsigned far pascal _ComTXSize (HCOMPORT Port);
/* Gibt die Gr��e des Sendepuffers zur�ck */


unsigned far pascal _ComTXCount (HCOMPORT Port);
/* Gibt die Anzahl der Zeichen im Sendepuffer zur�ck */


unsigned far pascal _ComTXFree (HCOMPORT Port);
/* Gibt die Anzahl freier Pl�tze im Sendepuffer zur�ck */


void far pascal _ComTXClear (HCOMPORT Port);
/* L�scht den kompletten Sendepuffer */


int far pascal _ComReceive (HCOMPORT Port);
/* Holt ein Zeichen aus dem Empfangspuffer f�r den spezifizierten Port. Ist
 * der Puffer leer, so kommt -1 zur�ck.
 */


int far pascal _ComSend (HCOMPORT Port, unsigned char B);
/* Sendet ein Zeichen (legt es im Sendepuffer ab). Ist der Sendepuffer voll,
 * so wird der entsprechende Fehlerz�hler hochgez�hlt und das Zeichen wird
 * weggeworfen.
 */


void far pascal _ComBreak (HCOMPORT Port, double Duration);
/* Sendet ein Break mit der �bergebenen Dauer (in Sekunden). Die Funktion
 * kehrt erst nach Ablauf dieser Zeit zur�ck.
 */


unsigned far pascal _ComModemStatus (HCOMPORT Port);
/* Gibt den Modemstatus (Status der Kontrolleitungen) f�r den gew�nschten Port
 * zur�ck. Die Bits k�nnen anhand der weiter oben definierten Konstanten aus-
 * gewertet werden.
 */



void far pascal _IntCom1 ();
void far pascal _IntCom2 ();
void far pascal _IntCom3 ();
void far pascal _IntCom4 ();
// Interrupt-Handlers for port 1..4



};



/*****************************************************************************/
/*                             struct _ComData                               */
/*****************************************************************************/



// All implementation dependant data is contained in a structure that is
// defined in the CC file.
struct _ComData {

    // Port-Definitione (duplicated)
    u32         Baudrate;
    char        Connection;     // <D>irect, <M>odem
    char        Parity;         // <N>one, <O>dd, <E>ven, <S>pace, <M>ark
    char        Stopbits;       // 1, 2
    char        Databits;       // 5..8
    char        XonXoff;        // <E>nabled, <D>isabled
    char        Fill;           // Make the offset even

    // Circular buffer
    void far*   RXBuf;
    void far*   TXBuf;
    u16         RXBufSize;
    u16         TXBufSize;
    u16         RXStart;
    u16         RXEnd;
    u16         TXStart;
    u16         TXEnd;
    u16         RXCount;
    u16         TXCount;

    // Error counters
    ComErrorCounter ErrorCounter;

    char        Installed;      // Don't use
    char        IntNr;          // Number of the interrupt used
    char        IC1Mask;        // Mask for interrupt controller #1
    char        NotIC1Mask;     // Complement of the above mask
    char        IC2Mask;        // ... the same for IC #2
    char        NotIC2Mask;
    char        FIFOLen;        // Don't use
    char        ParityMask;     // Don't use

    u16         PortAddress;    // UART-Address
    u16         Reserved [6];

    u16         IntHandler;
    void far*   OldVector;

    char        HostOff;
    char        PCOff;
    char        MustSend;
    char        RTS_CTS_Ok;

};



/*****************************************************************************/
/*         Predefined _ComData structs, one for every port on the PC         */
/*****************************************************************************/



// It seems that Watcom mangles variable names (is this standard?)
extern "C" {



// Descriptor for COM 1
_ComData _ComPort1 = {

    9600,                       // <-- Baudrate   (9600)
    'M',                        // <-- Connection (Modem)
    'N',                        // <-- Parity     (None)
    1,                          // <-- StopBits   (1)
    8,                          // <-- DataBits   (8)
    'D',                        // <-- XonXoff    (Disabled)

    0,                          // Make the offset even

    NULL,                       // RXBuf
    NULL,                       // TXBuf
    0,                          // RXBufSize
    0,                          // TXBufSize
    0,                          // RXStart
    0,                          // RXEnd
    0,                          // TXStart
    0,                          // TXEnd
    0,                          // RXCount
    0,                          // TXCount

    { 0, 0, 0, 0, 0, 0 },       // Error counters

    0,                          // Installed
    0x0C,                       // * IntNr
    0x10,                       // * IC1Mask
    0xEF,                       // * NotIC1Mask
    0x00,                       // * IC2Mask
    0xFF,                       // * NotIC2Mask
    14,                         // FIFOLen
    0,                          // ParityMask

    0x3F8,                      // * PortAddress
    { 0, 0, 0, 0, 0, 0 },       // Reserved

    FP_OFF (_IntCom1),          // Offset Interrupt-Handler
    NULL,                       // OldVector

    0,                          // HostOff
    0,                          // PCOff
    0,                          // MustSend
    0                           // RTS_CTS_Ok

};


// Descriptor for COM 2
_ComData _ComPort2 = {

    9600,                       // <-- Baudrate   (9600)
    'M',                        // <-- Connection (Modem)
    'N',                        // <-- Parity     (None)
    1,                          // <-- StopBits   (1)
    8,                          // <-- DataBits   (8)
    'D',                        // <-- XonXoff    (Disabled)

    0,                          // Make the offset even

    NULL,                       // RXBuf
    NULL,                       // TXBuf
    0,                          // RXBufSize
    0,                          // TXBufSize
    0,                          // RXStart
    0,                          // RXEnd
    0,                          // TXStart
    0,                          // TXEnd
    0,                          // RXCount
    0,                          // TXCount

    { 0, 0, 0, 0, 0, 0 },       // Fehlerz�hler

    0,                          // Installed
    0x0B,                       // * IntNr
    0x08,                       // * IC1Mask
    0xF7,                       // * NotIC1Mask
    0x00,                       // * IC2Mask
    0xFF,                       // * NotIC2Mask
    14,                         // FIFOLen
    0,                          // ParityMask

    0x2F8,                      // * PortAddress
    { 0, 0, 0, 0, 0, 0 },       // Reserved

    FP_OFF (_IntCom2),          // Offset of interrupt handler
    NULL,                       // OldVector

    0,                          // HostOff
    0,                          // PCOff
    0,                          // MustSend
    0                           // RTS_CTS_Ok

};



// Descriptor for COM 3
_ComData _ComPort3 = {

    9600,                       // <-- Baudrate   (9600)
    'M',                        // <-- Connection (Modem)
    'N',                        // <-- Parity     (None)
    1,                          // <-- StopBits   (1)
    8,                          // <-- DataBits   (8)
    'D',                        // <-- XonXoff    (Disabled)

    0,                          // Make the offset even

    NULL,                       // RXBuf
    NULL,                       // TXBuf
    0,                          // RXBufSize
    0,                          // TXBufSize
    0,                          // RXStart
    0,                          // RXEnd
    0,                          // TXStart
    0,                          // TXEnd
    0,                          // RXCount
    0,                          // TXCount

    { 0, 0, 0, 0, 0, 0 },       // Error counters

    0,                          // Installed
    0x0C,                       // * IntNr
    0x10,                       // * IC1Mask
    0xEF,                       // * NotIC1Mask
    0x00,                       // * IC2Mask
    0xFF,                       // * NotIC2Mask
    14,                         // FIFOLen
    0,                          // ParityMask

    0x3E8,                      // * PortAddress
    { 0, 0, 0, 0, 0, 0 },       // Reserved

    FP_OFF (_IntCom3),          // Offset of interrupt handler
    NULL,                       // OldVector

    0,                          // HostOff
    0,                          // PCOff
    0,                          // MustSend
    0                           // RTS_CTS_Ok

};



// Descriptor for COM 4
_ComData _ComPort4 = {

    9600,                       // <-- Baudrate   (9600)
    'M',                        // <-- Connection (Modem)
    'N',                        // <-- Parity     (None)
    1,                          // <-- StopBits   (1)
    8,                          // <-- DataBits   (8)
    'D',                        // <-- XonXoff    (Disabled)

    0,                          // Make the offset even

    NULL,                       // RXBuf
    NULL,                       // TXBuf
    0,                          // RXBufSize
    0,                          // TXBufSize
    0,                          // RXStart
    0,                          // RXEnd
    0,                          // TXStart
    0,                          // TXEnd
    0,                          // RXCount
    0,                          // TXCount

    { 0, 0, 0, 0, 0, 0 },       // Error counters

    0,                          // Installed
    0x0B,                       // * IntNr
    0x08,                       // * IC1Mask
    0xF7,                       // * NotIC1Mask
    0x00,                       // * IC2Mask
    0xFF,                       // * NotIC2Mask
    14,                         // FIFOLen
    0,                          // ParityMask

    0x2E8,                      // * PortAddress
    { 0, 0, 0, 0, 0, 0 },       // Reserved

    FP_OFF (_IntCom4),          // Offset of interrupt handler
    NULL,                       // OldVector

    0,                          // HostOff
    0,                          // PCOff
    0,                          // MustSend
    0                           // RTS_CTS_Ok

};



}       // extern "C"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



extern "C" void far pascal _ComError ()
// Error routine that is called on parameter errors from the assembler module
{
    FAIL ("_ComError: Error in _SERCOM assembler module");
}



extern "C" u16 far pascal _ComWait (u16 ms)
// Wait routine that is called from the assembler module. It returns the time
// actually waited
{
    delay (ms);
    return ms;
}



/*****************************************************************************/
/*                               class ComPort                               */
/*****************************************************************************/



ComPort::ComPort (const String& aPortName,
                  u32  aBaudrate,
                  char aDatabits,
                  char aParity,
                  char aStopbits,
                  char aConnection,
                  char aXonXoff,
                  unsigned UARTBase,
                  unsigned IntNum):
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
    Init (UARTBase, IntNum);
}



ComPort::~ComPort ()
// Destruct a ComPort object
{
    // If the port is installed, deinstall it (this will also close the port)
    if (_ComIsInstalled (FP_OFF (ComData))) {
        _ComDeinstall (FP_OFF (ComData));
    }

    // Free the buffers and reset the buffer pointers
    delete [] ComData->RXBuf;
    delete [] ComData->TXBuf;
    ComData->RXBuf = NULL;
    ComData->TXBuf = NULL;
}



void ComPort::Init (unsigned UARTBase, unsigned IntNum)
// Initialization procedure, called from the constructors
{
    static const u16 DefPortAddress [4] = { 0x3F8, 0x2F8, 0x3E8, 0x2E8 };
    static const unsigned char DefIntNr [4] = { 0x0C, 0x0B, 0x0C, 0x0B };
    static _ComData* DefComData [4] = {
        &_ComPort1, &_ComPort2, &_ComPort3, &_ComPort4
    };

    // Assign one of the predefined ComData structures. To do this, the name
    // must contain just a number between 1 and 4 or it must be COMX, where
    // X is a number between 1 and 4.
    PortName.ToUpper ();
    char C;
    if (PortName.Cut (0, 3) == "COM" && PortName.Len () == 4) {
        C = PortName [3];
    } else if (PortName.Len () == 1) {
        C = PortName [0];
    } else {
        FAIL ("ComPort::Init: Invalid port name");
    }

    // Check the last char
    unsigned Port;
    switch (C) {

        case '1':
            Port = 0;
            break;

        case '2':
            Port = 1;
            break;

        case '3':
            Port = 2;
            break;

        case '4':
            Port = 3;
            break;

        default:
            FAIL ("ComPort::Init: Invalid port name");
            break;

    }

    // Assign the ComData struct
    ComData = DefComData [Port];

    // Transfer the parameters into the ComData struct
    ComData->Baudrate   = Baudrate;
    ComData->Connection = Connection;
    ComData->Databits   = Databits;
    ComData->Parity     = Parity;
    ComData->Stopbits   = Stopbits;
    ComData->XonXoff    = XonXoff;
    ComData->RXBuf      = NULL;
    ComData->TXBuf      = NULL;

    // Assign the port address or use the default if zero
    if (UARTBase == 0) {
        UARTBase = DefPortAddress [Port];
    }
    ComData->PortAddress = UARTBase;

    // Assign the interrupt number or use the default if zero.
    if (IntNum == 0) {
        IntNum = DefIntNr [Port];
    }
    ComData->IntNr = IntNum;

    // Calculate the IC masks
    if (IntNum >= 0x08 && IntNum <= 0x0F) {
        // Low interrupt
        ComData->IC1Mask = 0x01 << (IntNum - 8);
        ComData->IC2Mask = 0x00;
    } else if (IntNum >= 0x70 && IntNum <= 0x77) {
        // High interrupt
        ComData->IC1Mask = 0x00;
        ComData->IC2Mask = 0x01 << (IntNum - 0x70);
    } else {
        FAIL ("ComPort::Init: Unsupported interrupt number");
    }
    ComData->NotIC1Mask = ~ComData->IC1Mask;
    ComData->NotIC2Mask = ~ComData->IC2Mask;

    // Allocate the buffers, setting the size variables
    SetBufferSize (RXBufSize, TXBufSize);

    // Install the port
    _ComInstall (FP_OFF (ComData));
}



void ComPort::SetBufferSize (u16 aRXBufSize, u16 aTXBufSize)
// Set the sizes for receive and transmit buffer. This function cannot
// be called if the port is already open, you have to call it after
// constructing the object or after a close.
{
    // This function cannot be called if the port is already open
    PRECONDITION (!IsOpen ());

    // Assure reasonable max and min values and remember them
    if (aRXBufSize < 64) {
        aRXBufSize = 64;
    } else if (aRXBufSize > 4096) {
        aRXBufSize = 4096;
    }
    RXBufSize = aRXBufSize;

    if (aTXBufSize < 64) {
        aTXBufSize = 64;
    } else if (aTXBufSize > 4096) {
        aTXBufSize = 4096;
    }
    TXBufSize = aTXBufSize;

    // Free the (possibly already existing) buffers
    delete [] ComData->RXBuf;
    delete [] ComData->TXBuf;

    // Allocate new buffers
    ComData->RXBuf = new char [RXBufSize];
    ComData->TXBuf = new char [TXBufSize];

    // Record the new sizes
    ComData->RXBufSize = RXBufSize;
    ComData->TXBufSize = TXBufSize;
}



unsigned ComPort::Open ()
// Open the port, return an error code or 0 on success
{
    // All possible errors happen on _ComInstall, so check if the port
    // is installed
    if (_ComIsInstalled (FP_OFF (ComData))) {
        // Error free, open the port and exit
        _ComOpen (FP_OFF (ComData));
        return 0;
    } else {
        // Return "NO DEVICE"
        return ENODEV;
    }
}



void ComPort::Close ()
// Close the port
{
    _ComClose (FP_OFF (ComData));
}



int ComPort::IsOpen () const
// Return a value != zero if the port is opened, return 0 otherwise
{
    return _ComIsOpen (FP_OFF (ComData));
}



void ComPort::SetRXTimeout (double aRXTimeout)
// Set the timeout value
{
    RXTimeout = aRXTimeout;
}



void ComPort::SetTXTimeout (double aTXTimeout)
// Set the timeout value
{
    TXTimeout = aTXTimeout;
}



void ComPort::DTROn ()
// Make the DTR line active
{
    _ComDTROn (FP_OFF (ComData));
}



void ComPort::DTROff ()
// Make the DTR line inactive
{
    _ComDTROff (FP_OFF (ComData));
}



void ComPort::RTSOn ()
// Make the RTS line active. A call to this function is not allowed if the
// connection type is 'M'odem
{
    _ComRTSOn (FP_OFF (ComData));
}



void ComPort::RTSOff ()
// Make the RTS line inactive. A call to this function is not allowed if the
// connection type is 'M'odem
{
    _ComRTSOff (FP_OFF (ComData));
}



unsigned ComPort::RXCount () const
// Return the count of chars in the receive buffer, or just true 1 if the
// exact amount of chars in the buffer cannot be determined and the value
// is at least one.
{
    return _ComRXCount (FP_OFF (ComData));
}



unsigned ComPort::TXCount () const
// Return the count of chars in the transmit buffer
{
    return _ComTXCount (FP_OFF (ComData));
}



void ComPort::RXClear ()
// Clear the receive buffer
{
    _ComRXClear (FP_OFF (ComData));
}



void ComPort::TXClear ()
// Clear the transmit buffer
{
    _ComTXClear (FP_OFF (ComData));
}



unsigned ComPort::TXFree () const
// Return the amount of free space in the transmit buffer. The function
// may return the exact free space or just 1, if at least one character
// can be placed into the send buffer (meaning, the Send function will
// not block).
{
    return _ComTXFree (FP_OFF (ComData));
}



int ComPort::Receive ()
// Return a character from the receive buffer. If the buffer is empty,
// the function waits until a character is available.
{
    return _ComReceive (FP_OFF (ComData));
}



int ComPort::Send (unsigned char B)
// Send the character (put it into the transmit buffer). If there is no
// room in the transmit buffer, the error counter is incremented, the
// character is discarded and the function returns -1. To avoid this,
// check TXFree before calling this function. If the character could be
// placed into the transmit buffer, B is returned.
{
    return _ComSend (FP_OFF (ComData), B);
}



int ComPort::TimedReceive ()
// Return a character from the receive buffer. If the buffer is empty,
// the function waits until a character is available or the time given
// with SetReceiveTimeout is over. If a timeout condition occurred, the
// function returns -1, otherwise the character received.
{
    // Calculate the time to wait in ms
    u32 Wait = RXTimeout * 1000;

    // Wait until a character is available or timeout
    do {
        if (_ComRXCount (FP_OFF (ComData))) {
            // A character is available, grab it
            return _ComReceive (FP_OFF (ComData));
        }

        // Wait some time.
        u16 ActualWait = Wait > 10 ? 10 : Wait;
        ActualWait = _ComWait (ActualWait);

        // Reduce the time to wait
        if (ActualWait > Wait) {
            Wait = 0;
        } else {
            Wait -= ActualWait;
        }

    } while (Wait > 0);

    // Timeout
    return -1;
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
    // Calculate the time to wait in ms
    u32 Wait = TXTimeout * 1000;

    // Wait until the char can be put in the queue or timeout
    do {
        if (_ComTXFree (FP_OFF (ComData))) {
            // There is a free position in the output queue
            return _ComSend (FP_OFF (ComData), B);
        }

        // Wait some time.
        u16 ActualWait = Wait > 10 ? 10 : Wait;
        ActualWait = _ComWait (ActualWait);

        // Reduce the time to wait
        if (ActualWait > Wait) {
            Wait = 0;
        } else {
            Wait -= ActualWait;
        }

    } while (Wait > 0);

    // Timeout. Do an unconditional Send
    return _ComSend (FP_OFF (ComData), B);
}



void ComPort::TimedReceiveBlock (void* Buffer, u32 Count, u32& ReadCount)
// Wait until Count characters are read or the timeout is over. The
// variable ReadCount returns the amount of character actually read.
{
    // Check the given parameters
    PRECONDITION (Count > 0);

    // Cast the buffer pointer
    unsigned char* Buf = (unsigned char*) Buffer;

    // No characters received until now
    ReadCount = 0;

    // Read all chars in a loop
    while (Count > 0) {
        // Try to receive a char
        int C = TimedReceive ();
        if (C == -1) {
            // Timeout
            return;
        }

        // Store the char
        Buf [ReadCount++] = (unsigned char) C;

        // One less
        Count--;
    }

    // Timeout
    return;
}



void ComPort::TimedSendBlock  (const void* Buffer, u32 Count, u32& WriteCount)
// Wait until Count characters have been written  or the timeout is over.
// The variable WriteCount returns the amount of character actually written.
// If a timeout condition occurs, TXOverflow is incremented.
{
    // Check the given parameters
    PRECONDITION (Count > 0);

    // Cast the buffer pointer
    const unsigned char* Buf = (const unsigned char*) Buffer;

    // No characters sent until now
    WriteCount = 0;

    // Write all chars in a loop
    while (Count > 0) {

        // Try to send the char
        if (TimedSend (Buf [WriteCount]) == -1) {
            // Timeout
            return;
        }

        // One more
        WriteCount++;
        Count--;
    }
}



void ComPort::Break (double Duration)
// Send a break with the given time in seconds
{
    _ComBreak (FP_OFF (ComData), Duration * 1000.0);
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
    return _ComModemStatus (FP_OFF (ComData));
}



