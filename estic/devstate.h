/*****************************************************************************/
/*                                                                           */
/*                                 DEVSTATE.H                                */
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



// State of one device



#ifndef _DEVSTATE_H
#define _DEVSTATE_H



#include "event.h"
#include "datetime.h"

#include "icintcon.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Wait time after a call before requesting the charges
extern int DebugWaitAfterCall;

// How many digits of the phone number should be replaced by an 'X'?
extern int XDigits;



/*****************************************************************************/
/*                            class DevStateInfo                             */
/*****************************************************************************/



class DevStateInfo: public Object, public EventHandler {

    // Columns for the matrix window
    //           1         2         3         4         5         6         7
    // 01234567890123456789012345678901234567890123456789012345678901234567890
    //
    //  Nr. Amt1 Amt2 Int1 Int2 Int3 Ton WTon TFE Ruf Nummer
    enum {
        colNr       =  1,
        colSchleife =  3,
        colAmt1     =  6,
        colAmt2     = 11,
        colInt1     = 16,
        colInt2     = 21,
        colInt3     = 26,
        colTon      = 31,
        colWTon     = 35,
        colTFE      = 40,
        colRuf      = 44,
        colPhone    = 47
    };


    void ClearCallPhone ();
    // Clear the call phone number, reset the stuff in the matrix line


public:
    const unsigned char DevNum;         // Device number
    u32                 State;          // Device state
    String              CallPhone;      // Phone number dialed
    Time                CallStart;      // Start of call
    TimeDiff            CallDuration;   // Duration of call
    u16                 StartCharges;   // Charges at start of call
    u16                 CallCharges;    // Charges for call
    unsigned            HasExt;         // True if device had an external call
    String              MatrixLine;     // The line that's displayed
    Time                ForcedRingEnd;  // End of forced ring


    DevStateInfo (unsigned char Device);
    // Create a DevStateInfo object

    DevStateInfo (const DevStateInfo& X);
    // Copy constructor

    String LogMsg ();
    // Create a message for the logfile from the data

    int GetState (unsigned StateMask);
    void SetState (unsigned NewState);
    void ClrState (unsigned NewState);
    // Get/set/clear bits in State

    void SetSchleife (int State);
    void SetAmt (int State, unsigned Amt);
    void SetInt (int State, unsigned Int);
    void SetTon (int State);
    void SetWTon (int State);
    void SetTFE (int State);
    void SetRuf (int State);
    // Set specific matrix states

    void AddDigit (char Digit);
    // Add a digit to the phone number if the device is in a state where a digit
    // is accepted (dialed)

    void ReplaceDigits (int Count, char C = 'X');
    // Replace the given number of trailing digits of the phone number by
    // the character C.

    virtual void HandleEvent (Event& E);
    // Handle incoming events.
};



// End of DEVSTATE.H

#endif



