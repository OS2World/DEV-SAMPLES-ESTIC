/*****************************************************************************/
/*                                                                           */
/*                                 SERCOM.CC                                 */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
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
// Linux version.



#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/serial.h>

#include "../check.h"
#include "../sercom.h"
#include "../filepath.h"



/*****************************************************************************/
/*                            Types and constants                            */
/*****************************************************************************/



// internal used data
struct _ComData {

    int                 Handle;         // Port handle

    unsigned            RXCount;        // characters in receive queue
    unsigned            TXCount;        // characters in transmit queue

    termios             StartupSettings;// Device settings on startup
    termios             CurrentSettings;// Current device settings

    ComErrorCounter     ErrorCounter;   // Error counters
};



/******************************************************************************/
/*                       Utility and support functions                        */
/******************************************************************************/



static void SetReadTimeout (_ComData* ComData, int Min, int Time)
{
    if (ComData->CurrentSettings.c_cc [VMIN] != Min ||
        ComData->CurrentSettings.c_cc [VTIME] != Time) {
        // Need to set the new values
        ComData->CurrentSettings.c_cc [VMIN]  = Min;
        ComData->CurrentSettings.c_cc [VTIME] = Time;
        ZCHECK (tcsetattr (ComData->Handle, TCSANOW, &ComData->CurrentSettings));
    }
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
    RXBufSize (4096),               // Just some value...
    TXBufSize (4096),
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
    // Create an internally used data structure
    ComData = new _ComData;

    // Reset the error counters
    memset (ComData->ErrorCounter, 0, sizeof (ComData->ErrorCounter));

    // setting up ComData
    ComData->Handle    = -1;                // Port not open
    ComData->RXCount   = 0;
    ComData->TXCount   = 0;
}



void ComPort::SetBufferSize (u16 /*aRXBufSize*/, u16 /*aTXBufSize*/)
// Set the sizes for receive and transmit buffer. This function cannot
// be called if the port is already open, you have to call it after
// constructing the object or after a close. The function may be ignored
// if it is not possible to change buffer sizes.
{
    // This function cannot be called if the port is already open
    PRECONDITION (!IsOpen ());

    // Otherwise this function is ignored under Linux as we could not set any
    // buffer sizes (no harm done anyway)
}



unsigned ComPort::Open ()
// Open the port, return an error code or 0 on success
{
    // First, check if the port contains a directory. If not, add /dev
    String Dir, Name;
    FSplit (PortName, Dir, Name);
    if (Dir.IsEmpty ()) {
        PortName.Ins (0, "/dev/");
    }

    // Open the port
    ComData->Handle = open (PortName.GetStr (), O_RDWR | O_NONBLOCK);
    if (ComData->Handle == -1) {
        // Got an error, return the error code
        return errno;
    }

    // Device must be a tty
    if (!isatty (ComData->Handle)) {
        // Error, close and exit
        Close ();
        return ENOTTY;
    }

    // Remember the current settings
    tcgetattr (ComData->Handle, &ComData->StartupSettings);

    // Set up the new settings
    ComData->CurrentSettings = ComData->StartupSettings;

    // Set up input flags
    tcflag_t iflag = IGNBRK | IGNPAR;
    if (Databits == 7) {
        iflag |= ISTRIP;                // Strip bit 7
    }
    if (XonXoff == 'E') {
        iflag |= IXON | IXOFF;          // Enable XON/XOFF protocol
    }
    ComData->CurrentSettings.c_iflag = iflag;

    // Set up output flags
    tcflag_t oflag = 0;
    ComData->CurrentSettings.c_oflag = oflag;

    // Set up control flags
    tcflag_t cflag = CREAD | HUPCL;
    switch (Databits) {
        case 5: cflag |= CS5;   break;
        case 6: cflag |= CS6;   break;
        case 7: cflag |= CS7;   break;
        case 8: cflag |= CS8;   break;
        default: FAIL ("ComPort::Init: Unsupported databits value");
    }
    switch (Parity) {
        case 'N': break;
        case 'E': cflag |= PARENB;              break;
        case 'O': cflag |= PARENB | PARODD;     break;
        default:  FAIL ("ComPort::Init: Unsupported parity setting");
    }
    switch (Stopbits) {
        case 1: break;
        case 2: cflag |= CSTOPB; break;
        default: FAIL ("ComPort::Init: Unsupported stopbits value");
    }
    switch (Connection) {
        case 'M': cflag |= CRTSCTS; break;      // not POSIX!
        case 'D': cflag |= CLOCAL;  break;
        default: FAIL ("ComPort::Init: Unsupported connection setting");
    }
    ComData->CurrentSettings.c_cflag = cflag;

    // Set up local flags
    tcflag_t lflag = 0;
    ComData->CurrentSettings.c_lflag = lflag;

    // Set XON/XOFF to Ctrl-S/Ctrl-Q
    ComData->CurrentSettings.c_cc [VSTART] = 0x11;
    ComData->CurrentSettings.c_cc [VSTOP]  = 0x13;

    // Set the baudrate
    speed_t Baud = 0;
    switch (Baudrate) {
        case     50: Baud = B50;        break;
        case     75: Baud = B75;        break;
        case    110: Baud = B110;       break;
        case    134: Baud = B134;       break;
        case    150: Baud = B150;       break;
        case    200: Baud = B200;       break;
        case    300: Baud = B300;       break;
        case    600: Baud = B600;       break;
        case   1200: Baud = B1200;      break;
        case   2400: Baud = B2400;      break;
        case   4800: Baud = B4800;      break;
        case   9600: Baud = B9600;      break;
        case  19200: Baud = B19200;     break;
        case  38400: Baud = B38400;     break;
        case  57600: Baud = B57600;     break;
        case 115200: Baud = B115200;    break;
        case 230400: Baud = B230400;    break;
        default: FAIL ("ComPort::Init: Unsupported baudrate value");
    }
    ComData->CurrentSettings.c_cflag |= Baud;
    cfsetispeed (&ComData->CurrentSettings, Baud);
    cfsetospeed (&ComData->CurrentSettings, Baud);

    // Set timeouts
    ComData->CurrentSettings.c_cc [VMIN] = 1;
    ComData->CurrentSettings.c_cc [VTIME] = int (RXTimeout / 0.1);

    // Actually set up the device
    if (tcsetattr (ComData->Handle, TCSANOW, &ComData->CurrentSettings) < 0) {
        // Could not set
        int Result = errno;
        Close ();
        return Result;
    }

    // When we set cflags == CLOCAL on a direct connection, the RTS and DTR
    // lines will become active. This is not compatible to the other sercom
    // modules, so change it. Also make DTR low if hardware handshake is
    // enabled.
    int HandshakeLines = TIOCM_DTR;
    if (Connection == 'D') {
        HandshakeLines |= TIOCM_RTS;
    }
    ZCHECK (ioctl (ComData->Handle, TIOCMBIC, &HandshakeLines));

    // Reset nonblocking mode
    int Flags = fcntl (ComData->Handle, F_GETFL, 0);
    if (Flags < 0) {
        // Error
        int Result = errno;
        Close ();
        return Result;
    }
    Flags &= ~O_NONBLOCK;
    if (fcntl (ComData->Handle, F_SETFL, Flags) < 0) {
        // Error
        int Result = errno;
        Close ();
        return Result;
    }

    // Success
    return 0;
}



void ComPort::Close ()
// Close the port
{
    // Cannot close a port that is not open
    PRECONDITION (IsOpen ());

    // Close the device
    close (ComData->Handle);

    // reset handle
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
    if (aRXTimeout > 0 && aRXTimeout < 0.1) {
        aRXTimeout = 0.1;
    }

    // Remember the timeout
    RXTimeout = aRXTimeout;

    // Set the timeout
    SetReadTimeout (ComData, 0, int (RXTimeout / 0.1));
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
}



void ComPort::DTROn ()
// Make the DTR line active
{
    // Port must be open
    PRECONDITION (IsOpen ());

    // Set DTR
    int Status = TIOCM_DTR;
    ZCHECK (ioctl (ComData->Handle, TIOCMBIS, &Status));
}



void ComPort::DTROff ()
// Make the DTR line inactive
{
    // Port must be open
    PRECONDITION (IsOpen ());

    // Clear DTR
    int Status = TIOCM_DTR;
    ZCHECK (ioctl (ComData->Handle, TIOCMBIC, &Status));
}



void ComPort::RTSOn ()
// Make the RTS line active. A call to this function is not allowed if the
// connection type is 'M'odem
{
    // Port must be open and no hardware handshaking enabled
    PRECONDITION (IsOpen () && Connection != 'M');

    // Set RTS
    int Status = TIOCM_RTS;
    ZCHECK (ioctl (ComData->Handle, TIOCMBIS, &Status));
}



void ComPort::RTSOff ()
// Make the RTS line inactive. A call to this function is not allowed if the
// connection type is 'M'odem
{
    // Port must be open and no hardware handshaking enabled
    PRECONDITION (IsOpen () && Connection != 'M');

    // Clear RTS
    int Status = TIOCM_RTS;
    ZCHECK (ioctl (ComData->Handle, TIOCMBIC, &Status));
}



unsigned ComPort::RXCount () const
// Return the count of chars in the receive buffer
{
    // Port must be open
    PRECONDITION (IsOpen ());

    int Count;
    ZCHECK (ioctl (ComData->Handle, TIOCINQ, &Count));
    return Count;
}



unsigned ComPort::TXCount () const
// Return the count of chars in the transmit buffer
{
    // Port must be open
    PRECONDITION (IsOpen ());

    int Count;
    ZCHECK (ioctl (ComData->Handle, TIOCOUTQ, &Count));
    return Count;
}



void ComPort::RXClear ()
// Clear the receive buffer
{
    // Port must be open
    PRECONDITION (IsOpen ());

    ZCHECK (tcflush (ComData->Handle, TCIFLUSH));
}



void ComPort::TXClear ()
// Clear the transmit buffer
{
    // Port must be open
    PRECONDITION (IsOpen ());

    ZCHECK (tcflush (ComData->Handle, TCOFLUSH));
}



unsigned ComPort::TXFree () const
// Return the amount of free space in the transmit buffer. The function
// may return the exact free space or just 1, if at least one character
// can be placed into the send buffer (meaning, the Send function will
// not block).
{
    // Port must be open
    PRECONDITION (IsOpen ());

    // Timeout is zero
    timeval Timeout;
    Timeout.tv_usec = 0;
    Timeout.tv_sec  = 0;

    // Set the file descriptor to ComData->Handle
    fd_set Desc;
    FD_ZERO (&Desc);
    FD_SET (ComData->Handle, &Desc);

    // Check output status
    if (select (ComData->Handle+1, NULL, &Desc, NULL, &Timeout) > 0) {
        return 1;
    } else {
        return 0;
    }
}



int ComPort::Receive ()
// Return a character from the receive buffer. If the buffer is empty,
// the function waits until a character is available.
{
    // Port must be open
    PRECONDITION (IsOpen ());

    // Set no timeout mode
    SetReadTimeout (ComData, 0, 0);

    // read one character
    unsigned char B;
    int ReadCount;
    do {
        ReadCount = read (ComData->Handle, &B, 1);
    } while (ReadCount != 1);

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
    // Port must be open
    PRECONDITION (IsOpen ());

    if (TXFree () == 0) {
        ComData->ErrorCounter [ceTXOverflow]++;
        return -1;
    } else {
        int WriteCount;
        do {
            WriteCount = write (ComData->Handle, &B, 1);
        } while (WriteCount != 1);
        return B;
    }
}



int ComPort::TimedReceive ()
// Return a character from the receive buffer. If the buffer is empty,
// the function waits until a character is available or the time given
// with SetReceiveTimeout is over. If a timeout condition occurred, the
// function returns -1, otherwise the character received.
{
    // Port must be open
    PRECONDITION (IsOpen ());

    // Set up the device for timeout read
    SetReadTimeout (ComData, 0, int (RXTimeout / 0.1));

    unsigned char B;
    int ReadCount = read (ComData->Handle, &B, 1);

    // Return the character read or -1 on timeout
    return ReadCount == 1 ? B : -1;
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
    // Port must be open
    PRECONDITION (IsOpen ());

    // Use select to check if write is possible
    timeval Timeout;
    Timeout.tv_usec = u32 (TXTimeout * 1000000) % 1000000;
    Timeout.tv_sec  = long (TXTimeout);

    fd_set Desc;
    FD_ZERO (&Desc);
    FD_SET (ComData->Handle, &Desc);

    if (select (ComData->Handle + 1, NULL, &Desc, NULL, &Timeout)) {
        // Descriptor is ready for writing
        write (ComData->Handle, &B, 1);
        return B;
    } else {
        // Timeout or signal. Return an timeout error code when a signal
        // occurs
        ComData->ErrorCounter [ceTXOverflow]++;
        return -1;
    }
}



void ComPort::Break (double /*Duration*/)
// Send a break with the given time in seconds
{
    // Port must be open
    PRECONDITION (IsOpen ());

    // Ignore Duration as values != zero have undefined behavior
    ZCHECK (tcsendbreak (ComData->Handle, 0));
}



#if 0
// gcc 2.5.8 is not able to compile this (bug using the typedef)
// Leave it out and hope no one will notice...
ComErrorCounter& ComPort::GetErrors ()
// Get a reference to the array of error counters. These counters are
// incremented but never decremented or zeroed by the object.
{
    return ComData->ErrorCounter;
}
#endif



unsigned ComPort::ModemStatus () const
// Return the state of the modem status lines
{
    // Port must be open
    PRECONDITION (IsOpen ());

    // Get the modem lines
    int Lines;
    ZCHECK (ioctl (ComData->Handle, TIOCMGET, &Lines));

    // Convert to sercom format, ignore the "delta" lines
    unsigned Status = 0;
    if (Lines & TIOCM_CTS)      Status |= csCTS;
    if (Lines & TIOCM_DSR)      Status |= csDSR;
    if (Lines & TIOCM_RNG)      Status |= csRI;
    if (Lines & TIOCM_CAR)      Status |= csCD;

    // Return the result
    return Status;
}




