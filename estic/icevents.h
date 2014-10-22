/*****************************************************************************/
/*                                                                           */
/*                                ICEVENTS.H                                 */
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



// ESTIC event codes



#ifndef _ICEVENTS_H
#define _ICEVENTS_H



#include "eventid.h"



/*****************************************************************************/
/*                                Event codes                                */
/*****************************************************************************/



// Charges have changed
const unsigned evChargeUpdate           = evUser + 1;

// A matrix row has changed. Info.P is a pointer to the DevStateInfo entry
const unsigned evMatrixChange           = evUser + 2;

// Log message for call. Info.O is a pointer to a string containing the message
const unsigned evCallLogMsg             = evUser + 3;

// Call complete. Info.O is a pointer to a DevStateInfo object containing the
// call data.
const unsigned evCallComplete           = evUser + 4;

// Ring a phone. The U parameter is the device plus 256 times the duration
// in seconds.
const unsigned evForcedRing             = evUser + 5;

// Incoming call. Info.O points to a CLI object.
const unsigned evIncomingCall           = evUser + 6;

// Log message for incoming call, Info.O points to a string containing the log
// message
const unsigned evIncomingLogMsg         = evUser + 7;

// Changes for some windows. Info.U contains the window count
const unsigned evMatrixWinChange        = evUser + 10;
const unsigned evCallWinChange          = evUser + 11;
const unsigned evChargeWinChange        = evUser + 12;
const unsigned evIMonWinChange          = evUser + 13;
const unsigned evCLIWinChange           = evUser + 14;



// End of ICEVENTS.H

#endif


