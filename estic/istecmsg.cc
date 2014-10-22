/*****************************************************************************/
/*                                                                           */
/*                               ISTECMSG.CC                                 */
/*                                                                           */
/* (C) 1996     Ullrich von Bassewitz                                        */
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



// Defines a class that holds an istec message



#include "check.h"

#include "icver.h"
#include "icdlog.h"
#include "icdiag.h"
#include "iccti.h"
#include "istecmsg.h"



/*****************************************************************************/
/*                              class IstecMsg                               */
/*****************************************************************************/



IstecMsg::IstecMsg (unsigned MsgSize, const unsigned char* Buf):
    ErrCode (0),
    Data (0)
// Create a message
{
    NewData (MsgSize, Buf);
}



IstecMsg::~IstecMsg ()
{
    delete [] Data;
}



void IstecMsg::NewData (unsigned MsgSize, const unsigned char* Buf)
// Replace the current data by the new stuff. If Buf is not 0, the
// contents of Buf are copied to the message.
{
    // Delete the old stuff
    delete [] Data;

    // Use the new message size
    Size = MsgSize;

    // Check the parameters
    if (MsgSize > 0) {
        Data = new unsigned char [MsgSize];
        if (Buf) {
            // We got data, copy it
            memmove (Data, Buf, MsgSize);
        }
    } else {
        Data = 0;
    }
}



String IstecMsg::AsciiData () const
{
    return ::AsciiData (Data, Size);
}



int IstecMsg::IsDiagMsg () const
// Return true if the message is a diagnostic message
{
    // Unnecessary override for gcc
    return ::IsDiagMsg (Data, Size);
}



int IstecMsg::IsCLIMsg () const
// Return true if the message is a calling line identification message
{
    // Unnecessary override for gcc
    return ::IsCLIMsg (Data, Size);
}



int IstecMsg::IsCTIMsg () const
// Return true if the message is a CTI message
{
    // Unnecessary override for gcc
    return ::IsCTIMsg (Data, Size);
}



int IstecMsg::IsChargeInfo () const
// Return true if the message is a charge info message
{
    // Unnecessary override for gcc
    return ::IsChargeInfo (Data, Size);
}



unsigned char& IstecMsg::At (unsigned Index)
// Access the Data member bounds checked.
{
    PRECONDITION (Index < Size);
    return Data [Index];
}



const unsigned char& IstecMsg::At (unsigned Index) const
// Access the Data member bounds checked.
{
    PRECONDITION (Index < Size);
    return Data [Index];
}



/*****************************************************************************/
/*                     Other Istec Message related stuff                     */
/*****************************************************************************/



String IstecMsgDesc (const unsigned char* Data)
// Return a string describing the Istec message. This is for debug purposes
// only, the string is hardcoded, not loaded from the resource.
{
    PRECONDITION (Data != 0);

    switch (Data [0]) {

        case 'C':
            return "Calling line identification";

        case 0x02:
            return "Are you there?";

        case 0x05:
            return "Write charge data";

        case 0x06:
            return "Request charge data";

        case 0x07:
            return "Write device configuration";

        case 0x08:
            return "Request device configuration";

        case 0x09:
            return "Write base configuration";

        case 0x0a:
            return "Request base configuration";

        case 0x0c:
            return "End configuration";

        case 0x11:
            return "End configuration (ACK)";

        case 0x12:
            return "Hi! Nice to meet you!";

        case 0x13:
            return "Error reply (NACK)";

        case 0x15:
            return "Charge data";

        case 0x16:
            return "Device configuration data";

        case 0x17:
            return "Base configuration data";

        case 0x18:
            return "Write device configuration acknowledged";

        case CTI_START:
            return "CTI message";

        case 0xdd:
            return "Diagnostic message";

        default:
            return "Unknown message";

    }
}



String AsciiData (const unsigned char* Data, unsigned Size)
// Create an ascii string with the hex representation of the message data
{
    // Check the programmer :-)
    PRECONDITION (Data != NULL && Size > 0);

    String S (Size * 3);

    // Create the string
    for (unsigned I = 0; I < Size; I++) {
        S += FormatStr ("%02X ", Data [I]);
    }

    // Add a short description of the message if VerboseDebugLog is enabled
    if (IsDiagMsg (Data, Size)) {
        S += DiagMsgDesc (Data);
    } else if (IsCTIMsg (Data, Size)) {
        S += CTIMsgDesc (Data, Size);
    } else {
        S += IstecMsgDesc (Data);
    }

    // Return the result
    return S;
}



int IsDiagMsg (const unsigned char* Data, unsigned Size)
// Return true if the given message is a diagnostic message
{
    PRECONDITION (Data != NULL);
    return Size == 6 && Data [0] == 0xdd;
}



int IsCLIMsg (const unsigned char* Data, unsigned Size)
// Return true if the message is a calling line identification message
{
    // At least "CLImn\x0c\x0d"
    if (Size < 7 || Data [0] != 'C' || Data [1] != 'L' || Data [2] != 'I') {
        return 0;
    } else {
        return 1;
    }
}



int IsCTIMsg (const unsigned char* Data, unsigned Size)
// Return true if the message is a CTI message
{
    // The shortest possible message is CTI_START/CTI_GROUP/CTI_OP/CTI_STOP
    return (Size >= 4 && Data [0] == CTI_START);
}



int IsChargeInfo (const unsigned char* Data, unsigned Size)
// Return true if the message contains the ISTEC charges
{
    if (Data [0] != 0x15) {
        // No charge info
        return 0;
    }
    if (FirmwareVersion < 2.00) {
        return (Size == 129);
    } else {
        return (Size == 17);
    }
}



unsigned SendMsgSize (const unsigned char* Data, unsigned BufSize)
// The function returns the real size of the given message to send. Since
// the messages given to this function are created by the programmer (that
// is me - hi!), a non determinable message size is a programming error.
// Note: BufSize is the size of the message buffer, the real message that
// is sent maybe smaller than this value.
{
    unsigned Version;

    // Check the given parameter
    PRECONDITION (Data != NULL && BufSize > 0);

    switch (Data [0]) {

        case 0x02:
        case 0x06:
        case 0x08:
        case 0x0a:
        case 0x0c:
            return 1;

        case 0x05:
            if (FirmwareVersion < 2.00) {
                return 1 + 64 * sizeof (u16);
            } else {
                return 1 + 8 * sizeof (u16);
            }

        case 0x07:
            if (FirmwareVersion < 1.93) {
                return 18;
            } else if (FirmwareVersion < 1.95) {
                return 22;
            } else if (FirmwareVersion < 2.00) {
                return 23;
            } else {
                return 27;
            }

        case 0x09:
            // This is ugly, but I cannot help: Use the firmware version
            // from the message instead of the saved one, since we may
            // not know about the version when receiving this message.
            // To be shure not to access non-existing message members,
            // check the length first.
            CHECK (BufSize >= 94);
            Version = unsigned (Data [4]) * 100 + unsigned (Data [5]);
            if (Version < 193) {
                return 94;
            } else if (Version < 200) {
                return 107;
            } else {
                return 117;
            }

        case CTI_START:
            // A CTI message - use the real size
            return BufSize;

        case 0xdd:
            return 6;

        default:
            FAIL ("SendMsgSize: Trying to create invalid message!");
            return 0;
    }
}



unsigned RecMsgSize (unsigned char* Data, unsigned Size)
// Calculate the size of a message. First byte is opcode.
{
    // Check the given parameter
    PRECONDITION (Data != NULL && Size > 0);

    switch (Data [0]) {

        case 'C':
            // A CLI message, return the actual size
            return Size;

        case 0x11:
        case 0x12:
            return 1;

        case 0x13:
            WriteDebugLog ("Got error reply: " + AsciiData (Data, Size));
            return 2;

        case 0x15:
            if (FirmwareVersion < 2.00) {
                return 129;
            } else {
                return 17;
            }

        case 0x16:
            if (FirmwareVersion < 1.93) {
                return 18;
            } else if (FirmwareVersion < 1.95) {
                return 22;
            } else if (FirmwareVersion < 2.00) {
                return 23;
            } else {
                return 27;
            }

        case 0x17:
            // This is ugly, but I cannot help: Use the firmware version
            // from the message instead of the saved one, since we may
            // not know about the version when receiving this message.
            // To be shure not to access non-existing message members,
            // check the length first.
            if (Size != 94 && Size != 107 && Size != 117) {
                WriteDebugLog ("Error: Invalid message length: " +
                               AsciiData (Data, Size));
                return Size;
            } else {
                // Get the version
                unsigned Version = unsigned (Data [4]) * 100 + unsigned (Data [5]);
                if (Version < 193) {
                    return 94;
                } else if (Version < 200) {
                    return 107;
                } else {
                    return 117;
                }
            }

        case 0x18:
            return 2;

        case CTI_START:
            // A CTI message - use the real size
            return Size;

        case 0xdd:
            return 6;

        default:
            WriteDebugLog ("Warning: Cannot determine message length: " +
                           AsciiData (Data, Size));
            return Size;
    }
}



