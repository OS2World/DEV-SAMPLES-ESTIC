/*****************************************************************************/
/*                                                                           */
/*                                 ICCOM.CC                                  */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
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



#include "check.h"
#include "delay.h"
#include "sercom.h"
#include "circbuf.h"
#include "progutil.h"

#include "icmsg.h"
#include "icevents.h"
#include "icconfig.h"
#include "istecmsg.h"
#include "icdlog.h"
#include "icdiag.h"
#include "iccli.h"
#include "iccti.h"
#include "iccom.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Port address and irq used
unsigned PortBase = 0;
unsigned PortIRQ  = 0;

// Flag for short or long wait after sending a message
int ShortWaitAfterMsg = 1;

// Allow diag mode or not
int AllowDiagMode = 1;

// Version of the config program (for firmware 2.0 and above)
unsigned char ConfigVersionHigh = 2;
unsigned char ConfigVersionLow  = 0;

// Current ISTEC charges
IstecCharges Charges;

// Some flags that are used when we receive config or charge messages
static int ChargeUpdate         = 0;
//static int BaseConfigUpdate     = 0;
//static int DevConfigUpdate      = 0;

// Com port instance
static ComPort* Port = NULL;

// Variables for the read routine
static enum {
    stIdle,                     // No current message
    stInBlock,                  // Currently reading a block
    stGotBlock,                 // Got a complete block
    stInLastBlock,              // Currently reading the last block of a msg
    stFillBytes                 // Reading the fill bytes of the last block
} ICReadStat = stIdle;

// Buffers and counters
static unsigned ICBlockCount = 0;
static unsigned ICReadCount = 0;
static unsigned ICFillCount = 0;
static unsigned ICMsgCount = 0;
static unsigned char ICReadBuf [512];

// The last non diag message received
static IstecMsg* LastIstecMsg = NULL;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class CircularBuffer<IstecMsg*, 64>;
#endif



/*****************************************************************************/
/*                           Com port related code                           */
/*****************************************************************************/



void CloseComPort ()
// Close the com port
{
    if (Port) {
        if (Port->IsOpen ()) {
            // Switch off DTR
            Port->DTROff ();
        }

        // Delete the port object
        delete Port;
        Port = NULL;
    }
}



int OpenComPort (const String& PortName)
// Try to open the com port. If the port is already open, it is closed and
// reopened. The function returns 0 on success and an error code on failure.
{
    // Close the port in case it is open
    CloseComPort ();

    // Ok, now reopen it. Use 9600 8N1 without handshaking
    Port = new ComPort (PortName, 9600, 8, 'N', 1, 'D', 'D', PortBase, PortIRQ);
    int Result = Port->Open ();
    if (Result != 0) {
        // Error, maybe the device does not exist or is already in use
        CloseComPort ();
        return Result;
    }

    // Set the timeout value for sending and receiving
    Port->SetRXTimeout (3.0);
    Port->SetTXTimeout (3.0);

    // Make the RTS line active. This is needed for the new PCB of the istec
    // (beginning from version #3).
    Port->RTSOn ();

    // Success
    return 0;
}



int ComPortAvail ()
// Return 1 if the com port is open, 0 if not
{
    return Port != NULL;
}



/*****************************************************************************/
/*                       Low level ISTEC specific code                       */
/*****************************************************************************/



static void UpdateCharges (const IstecMsg* Msg)
// Copy the charges
{
    // Unpack the new charges into the static area
    Charges.Unpack (&Msg->Data [1]);

    // We did receive a charge update
    ChargeUpdate = 1;

    // Post an appropriate event
    PostEvent (evChargeUpdate);
}



static void IstecWrite (unsigned char* Msg, unsigned BufSize)
// Recode the binary message into the istec format and send it via the port.
{
    // A static buffer to hold outgoing messages
    static CircularBuffer<IstecMsg*, 64> MsgBuf;

    // Check the parameters
    PRECONDITION (Msg != NULL && BufSize > 0);

    // If the port is unavailable, we have nothing to do
    if (Port == NULL) {
        return;
    }

    // Create a message from the given data
    IstecMsg* IM = new IstecMsg (BufSize, Msg);

    // Insert the message into the output buffer
    MsgBuf.Put (IM, 0);

    // Now have a look at the semaphore variable. If it is already set,
    // someone is currently executing this code and will take care of
    // our message in the buffer.
    static int Running = 0;
    if (Running) {
        // Someone is alredy executing this routine
        return;
    }

    // No one executing, block it
    Running = 1;

    // Send all message in the buffer
    while (!MsgBuf.IsEmpty ()) {

        // Get the next message
        IstecMsg* IM = MsgBuf.Get ();

        // Determine the real message size from the message
        unsigned Size = SendMsgSize (IM->Data, IM->Size);

        // Log the outgoing message
        WriteDebugLog ("Outgoing: " + IM->AsciiData ());

        // Get a pointer to the message data
        unsigned char* Msg = IM->Data;

        // Send the message
        char Buf [4];               // ## should be unsigned
        while (Size) {

            // Set up the length of the paket
            unsigned BytesToSend = Size > 3 ? 3 : Size;
            Size -= BytesToSend;

            // Time to wait after chunk (the times used here are the times the
            // ISTEC software uses: 40ms after a chunk and an additional 60ms
            // [making a total of 100ms] after the last chunk)
            unsigned DelayTime = 40;

            // Set up the header byte
            Buf [0] = BytesToSend;
            if (Size == 0) {
                // Last chunk, mark it
                Buf [0] |= 0x80;

                // If ShortWaitAfterMsg is *not* set, use the long delay to
                // give the istec time to swallow the command.
                if (!ShortWaitAfterMsg) {
                    DelayTime = 100;
                }
            }

            // Copy parameter bytes
            unsigned I = 0;
            while (I < BytesToSend) {
                I++;
                Buf [I] = *Msg++;
            }

            // Fill rest with 0x00
            while (I < sizeof (Buf) - 1) {
                I++;
                Buf [I] = '\0';
            }

            // Send the paket
            for (I = 0; I < sizeof (Buf); I++) {
                if (Port->TimedSend (Buf [I]) == -1) {
                    // Timeout
                    WriteDebugLog ("Error: Timeout on outgoing message!");
                    // Try the next message
                    break;
                }
            }

            if (DelayTime) {
                Delay (DelayTime);
            }

        }

        // Delete the message we've just sent
        delete IM;

    }

    // All messages sent. Reset the flag
    Running = 0;
}



static IstecMsg* IstecReadChar (unsigned char C)
// Is called when a character is available on the serial line. According
// to the status in ICReadStat, the character is handled. If the received
// message is complete, a copy of the message is returned in a buffer allocated
// with new.
{
    // Check the status
    switch (ICReadStat) {

        case stIdle:
        case stGotBlock:
            // We got a complete block and are waiting for the header of a
            // new one. C contains the byte count of the new block.
            if (C & 0x80) {
                // This one is the last block
                ICReadStat = stInLastBlock;
                ICBlockCount = C & 0x7F;
                ICFillCount = 3 - ICBlockCount;
            } else {
                ICReadStat = stInBlock;
                ICBlockCount = C;
            }
            // If we get a block with an invalid block size, we are in
            // trouble (this may happen if we are connected to the wrong
            // port). Ignore the block and go back into idle state hoping
            // that a timeout will clear things.
            if (ICBlockCount == 0 || ICBlockCount > 3) {
                ICReadStat = stIdle;
            }
            break;

        case stInBlock:
            // We are currently reading a block. ICBlockCount contains the
            // count of outstanding characters for this block. Place the
            // received character into the receive buffer.
            if (ICReadCount < sizeof (ICReadBuf)) {
                ICReadBuf [ICReadCount++] = C;
            }
            ICBlockCount--;
            if (ICBlockCount == 0) {
                // Got that block
                ICReadStat = stGotBlock;
            }
            break;

        case stInLastBlock:
            // We are currently reading the last block. ICBlockCount contains
            // the count of outstanding characters for this block. Place the
            // received character into the receive buffer.
            if (ICReadCount < sizeof (ICReadBuf)) {
                ICReadBuf [ICReadCount++] = C;
            }
            ICBlockCount--;
            if (ICBlockCount == 0) {
                // Got that block. Receive fill bytes or end
                if (ICFillCount > 0) {
                    ICReadStat = stFillBytes;
                } else {
                    // Got a complete message
                    ICReadStat = stIdle;
                    ICMsgCount++;
                }
            }
            break;

        case stFillBytes:
            // We are reading the fill bytes of the last block. Ignore the
            // bytes and wait for the end of the message
            ICFillCount--;
            if (ICFillCount == 0) {
                // Got the fill bytes
                ICReadStat = stIdle;
                ICMsgCount++;
            }
            break;

        default:
            FAIL ("IstecReadChar: Invalid machine state");
            break;

    }

    // Check if we did receive a complete message
    while (ICMsgCount > 0) {

        // Create an istec message from the buffer. Check if there is more than
        // one message in the buffer (this seems to be a bug in the istec
        // firmware)
        // If the proposed length is greater than the amount of bytes read,
        // we can only use what we have. If the proposed length is smaller,
        // assume that we received more than one message in a chunk and
        // use only part of the data.
        unsigned ProposedLen = RecMsgSize (ICReadBuf, ICReadCount);
        IstecMsg* IM;
        if (ProposedLen == ICReadCount) {
            // We received the amount of bytes, we expected.
            IM = new IstecMsg (ICReadCount, ICReadBuf);
            ICReadCount = 0;
            ICMsgCount = 0;
        } else if (ICReadCount > ProposedLen) {
            // We got more bytes than we expected. Assume that there is more
            // than one message in the chunk. Since we try to handle this
            // condition silently, we will not set an error code.
            String Msg = FormatStr ("Error: Message length does not match. "
                                    "Expected %d, got %d bytes: ",
                                    ProposedLen, ICReadCount);
            WriteDebugLog (Msg + AsciiData (ICReadBuf, ICReadCount));

            // Create the message
            IM = new IstecMsg (ProposedLen, ICReadBuf);

            // Delete the bytes we read from the buffer but leave ICMsgCount
            // untouched since there are bytes left
            ICReadCount -= ProposedLen;
            memmove (ICReadBuf, &ICReadBuf [ProposedLen], ICReadCount);

        } else {

            // We got less byte then we expected. This is clearly an error as
            // we have not enough bytes to handle.
            String Msg = FormatStr ("Error: Message length does not match. "
                                    "Expected %d, got %d bytes: ",
                                    ProposedLen, ICReadCount);
            WriteDebugLog (Msg + AsciiData (ICReadBuf, ICReadCount));

            // Create the message
            IM = new IstecMsg (ICReadCount, ICReadBuf);

            // Set the error code
            IM->SetError (ieRecBufUnderflow);

            // Delete the bytes we read from the buffer
            ICReadCount = 0;
            ICMsgCount = 0;
        }

        // Log the received message
        WriteDebugLog ("Incoming: " + IM->AsciiData ());

        // Look for special messages and handle them directly (don't return
        // those messages to the caller)
        if (IM->IsDiagMsg ()) {

            // The message is a diagnostic message
            HandleDiagMsg (IM->Data);
            delete IM;
            IM = NULL;

        } else if (IM->IsCLIMsg ()) {

            // The message is a calling line information
            HandleCLIMsg (IM->Data, IM->Size);
            delete IM;
            IM = NULL;

        } else if (IM->IsChargeInfo ()) {

            // The message is a charge info message
            UpdateCharges (IM);
            delete IM;
            IM = NULL;

        } else {

            // Cannot handle the message - return it
            return IM;

        }

    }

    // No complete message left, return a NULL pointer
    return NULL;

}



static IstecMsg* IstecRead ()
// Read and return a complete message from the istec.
{
    IstecMsg* IM;

    // Check if there is an already received message
    if (LastIstecMsg != NULL) {

        // There is an already received message - grab it
        IM = LastIstecMsg;
        LastIstecMsg = NULL;

    } else {

        // No message, try to receive one
        do {
            // Get a char with timeout
            int C = Port->TimedReceive ();
            if (C == -1) {
                // Timeout
                IM = new IstecMsg (0);
                IM->SetError (ieTimeout);
                return IM;
            }

            // Handle the char, receive a complete message if available
            IM = IstecReadChar (C);

        } while (IM == NULL);

    }

    // Return the message.
    return IM;
}



void IstecPoll ()
// Poll the istec for incoming diag messages. If we get a real message, store
// it in LastIstecMsg (there should be only one outstanding real message at a
// time).
{
    // If we don't have a valid port, ignore the call
    if (Port == NULL) {
        return;
    }

    // Handle all characters in the receive queue
    unsigned Count;
    while ((Count = Port->RXCount ()) > 0) {

        while (Count--) {

            // Get a char and handle it
            int C = Port->TimedReceive ();
            CHECK (C != -1);
            IstecMsg* IM = IstecReadChar (C);

            // If we have a message, save it into the message buffer
            if (IM) {

                // This is not a diagnose message. As we have only room to
                // buffer one message, delete the buffer contents before
                // storing (can happen only if the ISTEC does not work
                // correctly, ESTIC should not crash because of that).
                if (LastIstecMsg) {
                    // OOPS - there is a message already waiting
                    WriteDebugLog ("Error: Overwriting waiting message: " +
                                   AsciiData (LastIstecMsg->Data, LastIstecMsg->Size));
                    delete LastIstecMsg;
                }
                LastIstecMsg = IM;

            }
        }
    }
}



static int IstecReadAck (unsigned char Ack)
// Wait for an ack from the istec
{
    // Assume no errors

    // Wait for the acknowledgement
    IstecMsg* Reply = IstecRead ();
    int RetCode = Reply->GetError ();

    if (RetCode == ieDone) {

        // We received the message successfully, check the ack code
        if (Reply->At (0) != Ack) {
            // OOPS - got wrong answer
            WriteDebugLog (FormatStr ("Error: Got wrong ACK: Expected 0x%02X, got 0x%02X",
                                      Ack, Reply->At (0)));
            RetCode = ieInvalidReply;
        }

    }

    // Delete the message and return the result
    delete Reply;
    return RetCode;
}



/*****************************************************************************/
/*                      High level ISTEC specific code                       */
/*****************************************************************************/



void IstecErrorSync ()
// Try to resync the istec after an error
{
    // First, wait some time. IstecPoll will be called in this time
    Delay (250);

    // Call IstecPoll to fetch all characters that are currently in the
    // incoming buffer.
    IstecPoll ();

    // If we have a waiting message now, throw it away
    if (LastIstecMsg) {
        // Log the facts
        WriteDebugLog ("ErrorSync: Killing waiting message: " +
                       LastIstecMsg->AsciiData ());

        // Delete the message
        delete LastIstecMsg;
        LastIstecMsg = NULL;

    }
}



int IstecReady ()
// Check if the istec answers the "Ready" message.
{
    // Old firmware versions expect one byte here, newer version (2.0 and
    // above) expect three bytes. It seems as if the old versions ignore
    // additional bytes, so I will send three bytes in any case. This must
    // be changed if there are problems.
    unsigned char Msg [3] = { 0x02 };

    // Put the version into bytes 1 and 2
    Msg [1] = ConfigVersionHigh;
    Msg [2] = ConfigVersionLow;

    // Send the command to the istec
    IstecWrite (Msg, sizeof (Msg));

    // Ok, now wait for the reply
    return IstecReadAck (0x12);
}



void IstecRequestCharges ()
// Request the device charges from the istec. This function is different from
// the "Get" functions as it does not wait for a reply. The charge messages
// from the ISTEC are handled by the IstecPoll function in the background.
// If new charges are available, they are passed to the function NewChargeInfo
// of the application object.
{
    // ISTEC Command
    static unsigned char Msg [1] = { 0x06 };

    // Send the command to the istec
    IstecWrite (Msg, sizeof (Msg));
}



int IstecGetCharges ()
// Get the device charges from the istec. This function calls the "Request"
// function and waits until a timeout occurs or we get a reply.
{
    // Reset the flag
    ChargeUpdate = 0;

    // Send the command to the istec
    IstecRequestCharges ();

    // Wait for the new charges
    unsigned I = 0;
    do {
        Delay (200);
    } while (++I < 10 && ChargeUpdate == 0);

    // Check for a timeout
    if (ChargeUpdate == 0) {
        // Timeout, return an error code
        return ieTimeout;
    } else {
        return ieDone;
    }
}



void IstecPutCharges (const IstecCharges& NewCharges)
// Write the given charges to the istec
{
    // First byte is opcode, charges are parameters
    unsigned char Buf [ChargeSize + 1];
    Buf [0] = 0x05;
    NewCharges.Pack (&Buf [1]);

    // Write it out
    if (FirmwareVersion < 2.00) {
        // Old versions write charges for 64 devices
        IstecWrite (Buf, 1 + 64 * sizeof (u16));
    } else {
        // New versions write charges for 8 devices
        IstecWrite (Buf, 1 + 8 * sizeof (u16));
    }

    // Use the new charges
    Charges = NewCharges;

    // Post an appropriate event
    PostEvent (evChargeUpdate);
}



static int IstecGetDevConfig (IstecConfig& Config)
// Request the device configurations from the istec.
{
    // ISTEC Command
    static unsigned char Msg [1] = { 0x08 };

    // Send the command to the istec
    IstecWrite (Msg, sizeof (Msg));

    // Determine how many device infos come from the istec. Note: this depends
    // on the firmware version!
    unsigned DevCount = FirmwareVersion < 1.93? IstecDevCount : Config.GetDevCount ();

    // Ok, now we get many replys
    for (unsigned I = 0; I < DevCount; I++) {

        IstecMsg* Reply = IstecRead ();
        int Result = Reply->GetError ();

        // If the return code is ok, check the message code
        if (Result == ieDone) {

            // Check the return message code
            if (Reply->At (0) != 0x16) {
                WriteDebugLog ("Error: Got invalid reply on request 0x08: " +
                               Reply->AsciiData ());
                Result = ieInvalidReply;
            } else {
                // Ok, we got the answer from the istec. Copy the data into the config
                // struct
                Config.UnpackDevConfig (*Reply);
            }

        }

        // Delete the message
        delete Reply;

        // If the return code is not ok, bail out
        if (Result != ieDone) {
            return Result;
        }
    }

    // Got it
    return ieDone;
}



static int IstecGetBaseConfig (IstecConfig& Config)
// Request the basic configuration from the istec.
{
    // ISTEC Command
    static unsigned char Msg [1] = { 0x0A };

    // Send the command to the istec
    IstecWrite (Msg, sizeof (Msg));

    // Ok, now wait for the reply
    IstecMsg* Reply = IstecRead ();
    int Result = Reply->GetError ();
    if (Result == ieDone) {

        // Check the return message opcode
        if (Reply->At (0) != 0x17) {
            WriteDebugLog ("Error: Got invalid reply on request 0x0A: " +
                           Reply->AsciiData ());
            Result = ieInvalidReply;
        } else {

            // Ok, we got the answer from the istec. Copy the data into the config
            // struct, update the save firmware version from the istec data and
            // return a success code to the caller
            Config.UnpackBaseConfig (*Reply);
            FirmwareVersion = Config.GetFirmwareVersion();
        }
    }

    // Delete the message, return the result code
    delete Reply;
    return Result;
}



int IstecGetConfig (IstecConfig& Config)
// Get the complete configuration from the istec
{
    int Result;

    // Read the base configuration
    if ((Result = IstecGetBaseConfig (Config)) != ieDone) {
        return Result;
    }

    // Read the device configurations
    if ((Result = IstecGetDevConfig (Config)) != ieDone) {
        return Result;
    }

    // Ok, all done
    return ieDone;
}



static int IstecPutDevConfig (const IstecConfig& Config)
// Write a set of device configuration data to the istec.
{
    unsigned DevCount = Config.GetDevCount ();
    for (unsigned I = 0; I < DevCount; I++) {

        // Set up the command
        unsigned char Buf [DevConfigSize + 1];
        Buf [0] = 0x07;
        Config.PackDevConfig (I, &Buf [1]);

        // Write it out
        IstecWrite (Buf, sizeof (Buf));

        // Wait for the reply
        IstecMsg* Reply = IstecRead ();
        int Result = Reply->GetError ();

        if (Result == ieDone) {
            // Check the reply
            if (Reply->At (0) != 0x18) {
                Result = ieInvalidReply;
            } else if (Reply->At (1) != I) {
                // Reply has the wrong device number
                Result = ieWrongDevice;
            }
        }

        // Delete the message
        delete Reply;

        // Bail out if we had an error
        if (Result != ieDone) {
            return Result;
        }
    }

    // Ok, all done
    return ieDone;
}



static void IstecPutBaseConfig (const IstecConfig& Config)
// Write a base configuration to the istec
{
    unsigned char Buf [BaseConfigSize + 1];

    // First byte is opcode, base config is parameter
    Buf [0] = 0x09;
    Config.PackBaseConfig (&Buf [1]);

    // Write it out
    IstecWrite (Buf, sizeof (Buf));
}



static int IstecMakePermanent ()
// Send the command to the istec to store the current configuration into the
// EEPROM.
{
    // Send the command
    static unsigned char Msg [1] = { 0x0C };
    IstecWrite (Msg, sizeof (Msg));

    // Wait for the acknowledgement
    return IstecReadAck (0x11);
}



int IstecPutConfig (const IstecConfig& Config)
// Write the complete configuration to the istec and make it permanent. To
// make things short, the number of devices is given as an parameter
{
    int Result;

    // Write the base configuration
    IstecPutBaseConfig (Config);

    // Write the device configurations
    if ((Result = IstecPutDevConfig (Config)) != ieDone) {
        return Result;
    }

    // Make the changes permanent
    if ((Result = IstecMakePermanent ()) != ieDone) {
        return Result;
    }

    // Ok, all done
    return ieDone;
}



int IstecGetShortNumbers (ShortNumberColl& ShortNumbers)
// Read the short numbers from the istec. The function may not be called if
// the firmware version is < 2.00!
{
    PRECONDITION (FirmwareVersion >= 2.00);

    unsigned char Buf [5] = {
        CTI_START, CTI_QUERY, CTI_LOAD_NUMBER, 0, CTI_STOP
    };

    for (int I = 0; I < ShortNumbers.GetCount (); I++) {

        // Get a pointer to the info
        ShortNumberInfo* Info = ShortNumbers [I];

        // Insert the memory address into the message
        Buf [3] = Info->GetMemory ();

        // Send the message
        IstecWrite (Buf, sizeof (Buf));

        // Get the reply
        IstecMsg* Reply = IstecRead ();
        int Result = Reply->GetError ();

        if (Result == ieDone) {
            // Check the reply
            if (!Reply->IsCTIMsg ()) {
                Result = ieInvalidReply;
            } else if (Reply->At (1) != CTI_ACK         ||
                       Reply->At (2) != CTI_LOAD_NUMBER) {
                Result = ieCTIError;            // ##
            } else if (Reply->At (3) != Info->GetMemory ()) {
                Result = ieWrongDevice;         // ##
            } else {
                // We got a valid reply, put it into the info struct
                Info->Unpack (*Reply);
            }
        }

        // Delete the message
        delete Reply;

        // Bail out if we had an error
        if (Result != ieDone) {
            return Result;
        }

    }

    // Success
    return ieDone;
}



int IstecPutShortNumbers (const ShortNumberColl& ShortNumbers)
// Store the short numbers into the istec. The function may not be called if
// the firmware version is < 2.00!
{
    PRECONDITION (FirmwareVersion >= 2.00);

    for (int I = 0; I < ShortNumbers.GetCount (); I++) {

        // Get a pointer to the info
        const ShortNumberInfo* Info = ShortNumbers [I];

        // Create a message from the info
        IstecMsg Msg (0);
        Info->Pack (Msg);

        // Send the message
        IstecWrite (Msg.Data, Msg.Size);

        // Get the reply
        IstecMsg* Reply = IstecRead ();
        int Result = Reply->GetError ();

        if (Result == ieDone) {
            // Check the reply
            if (!Reply->IsCTIMsg ()) {
                Result = ieInvalidReply;
            } else if (Reply->At (1) != CTI_ACK           ||
                       Reply->At (2) != CTI_STORE_NUMBER  ||
                       Reply->At (3) != CTI_STOP) {
                Result = ieCTIError;            // ##
            }
        }

        // Delete the message
        delete Reply;

        // Bail out if we had an error
        if (Result != ieDone) {
            return Result;
        }

    }

    // Success
    return ieDone;
}



void IstecDiagOn ()
// Switch the istec into diag mode
{
    static unsigned char Msg [6] = { 0xdd, 0x00, 0x69, 0x5a, 0x96, 0xa5 };
    if (AllowDiagMode) {
        IstecWrite (Msg, sizeof (Msg));
    }
}



void IstecDiagOff ()
// Disable diag mode
{
    static unsigned char Msg [6] = { 0xdd, 0x01, 0x00, 0x00, 0x00, 0x00 };
    if (AllowDiagMode) {
        IstecWrite (Msg, sizeof (Msg));
    }
}



void IstecRingOn (unsigned char Device)
// Ring a phone. Device is 0..n
{
    static unsigned char Msg [6] = { 0xdd, 0x03, 0x00, 0x00, 0x01, 0x00 };
    Msg [2] = Device;
    IstecWrite (Msg, sizeof (Msg));
}



void IstecRingOff (unsigned char Device)
// Bell off. Device is 0..n
{
    static unsigned char Msg [6] = { 0xdd, 0x03, 0x00, 0x00, 0x00, 0x00 };
    Msg [2] = Device;
    IstecWrite (Msg, sizeof (Msg));
}





