/*****************************************************************************/
/*                                                                           */
/*                                  ICCOM.H                                  */
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



#ifndef _ICCOM_H
#define _ICCOM_H



#include "str.h"

#include "icconfig.h"
#include "icshort.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Port address and irq used
extern unsigned PortBase;
extern unsigned PortIRQ;

// Flag for short or long wait after sending a message
extern int ShortWaitAfterMsg;

// Allow diag mode or not
extern int AllowDiagMode;

// Version of the config program (for firmware 2.0 and above)
extern unsigned char ConfigVersionHigh;
extern unsigned char ConfigVersionLow;

// Current ISTEC charges and a flag that is set to 1 on an update
extern IstecCharges Charges;



/*****************************************************************************/
/*                           Com port related code                           */
/*****************************************************************************/



void CloseComPort ();
// Close the com port

int OpenComPort (const String& PortName);
// Try to open the com port. If the port is already open, it is closed and
// reopened. The function returns 0 on success and an error code on failure.

int ComPortAvail ();
// Return 1 if the com port is open, 0 if not



/*****************************************************************************/
/*                       Low level ISTEC specific code                       */
/*****************************************************************************/



void IstecPoll ();
// Poll the istec for incoming debug messages. If we get a real message, store
// it in LastIstecMsg (there should be only one outstanding real message at a
// time).



/*****************************************************************************/
/*                      High level ISTEC specific code                       */
/*****************************************************************************/



// All of the following functions may return a return code as specified below:
//
//      0 Done
//      1 Receive buffer overlow (ESTIC error - should not happen)
//      2 Receive buffer underflow (ESTIC error - should not happen)
//      3 Wrong device number in reply
//      4 Invalid reply code
//      5 Port is not open
//      6 Timeout
//      7 CTI error
//
const int ieDone                = 0;
const int ieRecBufOverflow      = 1;
const int ieRecBufUnderflow     = 2;
const int ieWrongDevice         = 3;
const int ieInvalidReply        = 4;
const int iePortNotOpen         = 5;
const int ieTimeout             = 6;
const int ieCTIError            = 7;



void IstecErrorSync ();
// Try to resync the istec after an error

int IstecReady ();
// Check if the istec answers the "Ready" message.

void IstecRequestCharges ();
// Request the device charges from the istec. This function is different from
// the "Get" functions as it does not wait for a reply. The charge messages
// from the ISTEC are handled by the IstecPoll function in the background.
// If new charges are available, they are copied to Charges and the
// ChargeUpdate flag is set.

int IstecGetCharges ();
// Get the device charges from the istec. This function calls the "Request"
// function and waits until a timeout occurs or we get a reply.

void IstecPutCharges (const IstecCharges& Charges);
// Write the given charges to the istec and to Charges

int IstecGetConfig (IstecConfig& Config);
// Get the complete configuration from the istec

int IstecPutConfig (const IstecConfig& Config);
// Write the complete configuration to the istec.

int IstecGetShortNumbers (ShortNumberColl& ShortNumbers);
// Read the short numbers from the istec. The function may not be called if
// the firmware version is < 2.00!

int IstecPutShortNumbers (const ShortNumberColl& ShortNumbers);
// Store the short numbers into the istec. The function may not be called if
// the firmware version is < 2.00!

void IstecDiagOn ();
// Switch the istec into diag mode

void IstecDiagOff ();
// Disable diag mode

void IstecRingOn (unsigned char Device);
// Ring a phone. Device is 0..n

void IstecRingOff (unsigned char Device);
// Bell off. Device is 0..n



// End of ICCOM.H

#endif



