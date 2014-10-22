/*****************************************************************************/
/*									     */
/*				   DEVSTATE.CC				     */
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



// State of one device
// This module contains some unnecessary globale overrides ("::") to work
// around one of the gcc bugs.



#include "event.h"
#include "coll.h"
#include "delay.h"
#include "strcvt.h"
#include "progutil.h"

#include "devstate.h"
#include "icevents.h"
#include "icac.h"
#include "icalias.h"
#include "iccom.h"
#include "iclog.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Wait time after a call before requesting the charges
int DebugWaitAfterCall	= 700;

// How many digits of the phone number should be replaced by an 'X'?
int XDigits = 0;

// Max length of the phone number
static const unsigned CallPhoneMaxLen = 16;

// Width of the matrix window. ## BEWARE: DUPLICATED IN ICDIAG.CC!!!
static const unsigned MatrixWindowWidth = 64;



/*****************************************************************************/
/*		 Constants for the State field in DevStateInfo		     */
/*****************************************************************************/



const unsigned stSchleife	= 0x0001;
const unsigned stAmt1		= 0x0002;
const unsigned stAmt2		= 0x0004;
const unsigned stAmt		= stAmt1 | stAmt2;
const unsigned stInt1		= 0x0008;
const unsigned stInt2		= 0x0010;
const unsigned stInt3		= 0x0020;
const unsigned stInt		= stInt1 | stInt2 | stInt3;
const unsigned stTon		= 0x0040;
const unsigned stWTon		= 0x0080;
const unsigned stTFE		= 0x0100;
const unsigned stRuf		= 0x0200;
const unsigned stBusy		= 0x0FFF;   // Idle if none of the bits is set

const unsigned stAliasRead	= 0x1000;
const unsigned stForcedRing	= 0x2000;
const unsigned stForcedRingOn	= 0x4000;   // Ring state for forced ring



/*****************************************************************************/
/*				class CallQueue				     */
/*****************************************************************************/



class CallQueue: public Collection<DevStateInfo>, public EventHandler {

public:
    CallQueue ();
    // Create a CallQueue

    DevStateInfo* At (int Index);
    // Return a pointer to the item at position Index.
    // OVERRIDE FOR DEBUGGING

    virtual void HandleEvent (Event& E);
    // Handle incoming events
};



CallQueue::CallQueue ():
    Collection <class DevStateInfo> (10, 10)
{
}



DevStateInfo* CallQueue::At (int Index)
// Return a pointer to the item at position Index.
// OVERRIDE FOR DEBUGGING
{
    // Check range
    if (Index < 0 || Index >= Count) {
	FAIL ("CallQueue::At: Index out of bounds");
	return NULL;
    }

    return Collection<DevStateInfo>::At (Index);
}



void CallQueue::HandleEvent (Event& E)
// Handle incoming events
{
    // Switch on the type of event
    switch (E.What) {

	case evChargeUpdate:
	    if (GetCount () > 0) {

		// There's something in the queue. Get the first entry.
		DevStateInfo* DS = At (0);

		// Update the charges
		DS->CallCharges = ::Charges [DS->DevNum] - DS->StartCharges;

		// Replace the last few digits of the phone number by the
		// character 'X'
		DS->ReplaceDigits (XDigits);

		// Get the log message
		String Msg = DS->LogMsg ();

		// Post the message with the log string
		PostEvent (evCallLogMsg, new String (Msg));

		// Now post the "call complete" event with the object as
		// data. The DevStateInfo object will be deleted when the
		// call is delivered.
		PostEvent (evCallComplete, DS);

		// Delete the entry (since ShouldDelete is zero, this will
		// only delete the pointer, not the object).
		AtDelete (0);

	    }
	    break;

    }
}



static CallQueue Queue;



/*****************************************************************************/
/*			      class DevStateInfo			     */
/*****************************************************************************/



DevStateInfo::DevStateInfo (unsigned char Device):
    DevNum (Device),
    State (0),
    HasExt (0),
    MatrixLine (MatrixWindowWidth)
{
    // Set the width of the matrix line and insert the number
    MatrixLine.Set (0, MatrixWindowWidth, ' ');
    MatrixLine.Replace (colNr, U32Str (Device + 21));

    // Create the rest of the line
    MatrixLine.Replace (colAmt1, '-');
    MatrixLine.Replace (colAmt2, '-');
    MatrixLine.Replace (colInt1, '-');
    MatrixLine.Replace (colInt2, '-');
    MatrixLine.Replace (colInt3, '-');
    MatrixLine.Replace (colTon, '-');
    MatrixLine.Replace (colWTon, '-');
    MatrixLine.Replace (colTFE, '-');
    MatrixLine.Replace (colRuf, '-');
}



DevStateInfo::DevStateInfo (const DevStateInfo& X):
    DevNum (X.DevNum),
    State (X.State),
    CallPhone (X.CallPhone),
    CallStart (X.CallStart),
    CallDuration (X.CallDuration),
    StartCharges (X.StartCharges),
    CallCharges (X.CallCharges),
    HasExt (X.HasExt),
    MatrixLine (X.MatrixLine)
{
}



void DevStateInfo::ClearCallPhone ()
// Clear the call phone number, reset the stuff in the matrix line
{
    // Clear the number
    CallPhone.Clear ();

    // Reset the matrix line
    MatrixLine.Set (colPhone, CallPhoneMaxLen, ' ');
}



String DevStateInfo::LogMsg ()
// Create a message for the logfile from the data
{
    // If AutoReadAliases is set, reread the alias file before trying to
    // resolve call data, but do this only once.
    if (AutoReadAliases != 0 && GetState (stAliasRead) == 0) {
	SetState (stAliasRead);
	ReadAliasFile ();
    }

    // Format of the line:
    //		 1	   2	     3	       4	 5	   6	     7	       8
    // 012345678901234567890123456789012345678901234567890123456789012345678901234567890
    // NR  ALIAS_______  DATE____  START___  DURATION  CALLPHONE_______  UNIT  DM____

    String StartDate = CallStart.DateTimeStr ("%d.%m.%y");
    String StartTime = CallStart.DateTimeStr ("%H:%M:%S");
    unsigned DurSec = CallDuration.GetSec ();
    unsigned DurMin = (DurSec % 3600) / 60;
    unsigned DurHour = DurSec / 3600;
    DurSec %= 60;
    String DevAlias = GetAlias (DevNum+21);
    DevAlias.Trunc (12);

    // Use the alias of the phone number if one is defined, otherwise use
    // the number
    String Phone = GetAlias (CallPhone);
    if (Phone.IsEmpty ()) {
	Phone = CallPhone;
    }
    Phone.Trunc (CallPhoneMaxLen);

    return FormatStr ("%d  %-12s  %s  %s  %02d:%02d:%02d  %-16s  %4d  %6.2f",
		      DevNum+21,
		      DevAlias.GetStr (),
		      StartDate.GetStr (),
		      StartTime.GetStr (),
		      DurHour, DurMin, DurSec,
		      Phone.GetStr (),
		      CallCharges,
		      CallCharges * PricePerUnit);
}



int DevStateInfo::GetState (unsigned StateMask)
{
    return (State & StateMask) != 0;
}



void DevStateInfo::SetState (unsigned NewState)
{
    State |= NewState;
}



void DevStateInfo::ClrState (unsigned NewState)
{
    State &= ~NewState;
}



void DevStateInfo::SetSchleife (int State)
{
    // If we already have the correct settings, ignore the call
    if (State == GetState (stSchleife)) {
	// Ignore it
	return;
    }

    // Now check which change
    if (State) {

	// Mark the device in the matrix
	MatrixLine.Replace (colSchleife, '*');

	// Set the state bit
	SetState (stSchleife);

	// Clear forced ring
	ClrState (stForcedRing | stForcedRingOn);

	// Clear the phone number
	ClearCallPhone ();

    } else {

	// Unmark the device
	MatrixLine.Replace (colSchleife, ' ');

	// Reset the state bit
	ClrState (stSchleife);

	// Clear the phone number if we have no external line. Otherwise
	// simulate the "Amt off" message to circumvent a bug in the istec
	// firmware
	if (GetState (stAmt) == 0) {

	    // Clear the phonre number
	    ClearCallPhone ();

	} else {

	    // Simulate "Amt off"
	    SetAmt (0, GetState (stAmt1)? 1 : 2);

	}
    }
}



void DevStateInfo::SetAmt (int State, unsigned Amt)
{
    // If we already have the correct settings, ignore the call
    if (State == GetState (stAmt)) {
	// Ignore it
	return;
    }

    // Map the line number to a column number in the matrix
    unsigned Column = 0;		// Make gcc happy
    unsigned Mask = 0;			// Make gcc happy
    switch (Amt) {
	case 1:   Column = colAmt1; Mask = stAmt1;	break;
	case 2:   Column = colAmt2; Mask = stAmt2;	break;
	default:  FAIL ("SetAmt: Invalid value for Amt");
    }

    // Check which state change
    if (State) {

	// Update the matrix line
	MatrixLine.Replace (Column, 'x');

	// Remember the line used
	SetState (Mask);

	// This should be an external call, so handle the phone number.
	// This is somewhat difficult to handle since there are different
	// firmware versions, some that get a line if you pick up the phone,
	// others that need a predialed zero. Worse, sometimes the diag message
	// indicating that we have an external line comes late, after dialing
	// two or even three digits. Further more, the new firmware with the
	// "knock" feature disables and then re-enables the external line
	// status each time, a knock signal is generated.
	// So assume the following: If the phone number is not empty when we
	// get the external line signal, AND if the device is active (loop)
	// AND there is no tone signal on this line, THEN delete the first
	// digit of the phone number.
	if (GetState (stSchleife) && GetState (stTon) == 0 && CallPhone.Len () > 0) {
	    // Delete the string in the matrix line
	    MatrixLine.Set (colPhone, CallPhoneMaxLen, ' ');

	    // Delete the first digit
	    CallPhone.Del (0, 1);

	    // Try to beautify the phone number by putting a separator behind
	    // the areacode
	    CallPhone = IstecBeautifyPhone (CallPhone);

	    // Put the new number into the matrix line
	    if (CallPhone.Len () > 0) {
		MatrixLine.Replace (colPhone, CallPhone);
	    }
	}

    } else {

	// Update the matrix line
	MatrixLine.Replace (Column, '-');

	// Clear the used line
	ClrState (stAmt);

	// Unfortunately, beginning with firmware 1.95, the release of the
	// external line does *not* mean that the call is terminated. The
	// Istec generates a off/on sequence each time, the knock signal
	// tone is generated. To verify that the call is really terminated,
	// use the Tone state in addition to the "line off" state.

	// If we had an external call, print the info
	if (GetState (stTon) == 0 && HasExt) {

	    // Get current time
	    Time Current = Now ();

	    // Calculate the durcation of the call
	    CallDuration = Current - CallStart;

	    // Reset flag for external call
	    HasExt = 0;

	    // Create an entry for the queue
	    DevStateInfo* DI = new DevStateInfo (*this);

	    // Wait some time
	    Delay (DebugWaitAfterCall);

	    // Insert the entry into into the call queue
	    Queue.Insert (DI);

	    // Request a charge update from the istec
	    IstecRequestCharges ();

	}

	// If the device is inactive now (Schleife == 0), clear the called
	// phone number. SetSchleife will act in a similiar way, clearing the
	// call phone if there is no external line, so the number is cleared
	// if both, Amt and Schleife are inactive.
	if (GetState (stSchleife) == 0) {
	    // Both, Amt and Schleife are inactive, clear the displayed
	    // phone number
	    ClearCallPhone ();
	}
    }
}



void DevStateInfo::SetInt (int State, unsigned Int)
{
    // Map the line number to a column number in the matrix
    unsigned Column = 0;		// Make gcc happy
    unsigned Mask = 0;			// Make gcc happy
    switch (Int) {
	case 1:   Column = colInt1; Mask = stInt1;	break;
	case 2:   Column = colInt2; Mask = stInt2;	break;
	case 3:   Column = colInt3; Mask = stInt3;	break;
	default:  FAIL ("SetInt: Invalid column number");
    }

    // Ignore the call if we already have the correct state
    if (State == GetState (Mask)) {
	return;
    }

    // Check which state change we have
    if (State) {

	// Show the change
	MatrixLine.Replace (Column, 'x');

	// Set the state bit
	SetState (Mask);

	// Remember which device uses this internal line
	IntCon [Int-1] = DevNum;

    } else {

	// Show the change
	MatrixLine.Replace (Column, '-');

	// Reset the state bit
	ClrState (Mask);

	// If the internal connection goes off, but we have an external one,
	// we have an external call.
	if (GetState (stAmt)) {

	    // Remember the starting time
	    CallStart = Now ();

	    // Remember the charge info on start
	    StartCharges = ::Charges [DevNum];

	    // Remember that we are connected externally
	    HasExt = 1;

	}
    }
}



void DevStateInfo::SetTon (int State)
{
    if (State) {
	MatrixLine.Replace (colTon, 'x');
	SetState (stTon);
    } else {
	MatrixLine.Replace (colTon, '-');
	ClrState (stTon);
    }
}



void DevStateInfo::SetWTon (int State)
{
    if (State) {
	MatrixLine.Replace (colWTon, 'x');
	SetState (stWTon);
    } else {
	MatrixLine.Replace (colWTon, '-');
	ClrState (stWTon);
    }
}



void DevStateInfo::SetTFE (int State)
{
    if (State) {
	MatrixLine.Replace (colTFE, 'x');
	SetState (stTFE);
    } else {
	MatrixLine.Replace (colTFE, '-');
	ClrState (stTFE);
    }
}



void DevStateInfo::SetRuf (int State)
{
    if (State) {

	// Update the display
	MatrixLine.Replace (colRuf, 'x');

	// Set the state bit
	SetState (stRuf);

	// Clear forced ring
	ClrState (stForcedRing | stForcedRingOn);

    } else {

	// Update the display
	MatrixLine.Replace (colRuf, '-');

	// Clear the state bit
	ClrState (stRuf);

    }
}



void DevStateInfo::AddDigit (char Digit)
// Add a digit to the phone number if the device is in a state where a digit
// is accepted (dialed)
{
    if (Digit != 'R' && (GetState (stAmt) == 0 || HasExt == 0)) {
	MatrixLine.Replace (colPhone + CallPhone.Len (), Digit);
	CallPhone += Digit;
    }
}



void DevStateInfo::ReplaceDigits (int Count, char C)
// Replace the given number of trailing digits of the phone number by
// the character C.
{
    if (Count > 0) {

	// The count may not be greater than the length of the string
	int Len = (int) CallPhone.Len ();
	if (Count > Len) {
	    Count = Len;
	}

	// Replace trailing digits
	while (Count) {
	    CallPhone.Replace (Len - Count, C);
	    Count--;
	}

    }
}



void DevStateInfo::HandleEvent (Event& E)
// Handle incoming events.
{
    switch (E.What) {

	case evForcedRing:
	    // Accept if this is the correct device and the line is idle
	    if (unsigned (DevNum + 21) == unsigned (E.Info.U % 256)) {
		if (GetState (stBusy) == 0) {
		    // Get the duration
		    ForcedRingEnd = Time () + TimeDiff (double (E.Info.U / 256));
		    // Set the new state
		    SetState (stForcedRing | stForcedRingOn);
		    // Send the command to the istec
		    IstecRingOn (DevNum);
		}
		// The event is handled now
		E.Handled = 1;
	    }
	    break;

	case evSecondChange:
	    // Check if we have an active forced ring
	    if (GetState (stForcedRing) && GetState (stBusy) == 0) {
		if (*(Time*) E.Info.O >= ForcedRingEnd) {
		    // End the forced ring
		    ClrState (stForcedRing | stForcedRingOn);
		    IstecRingOff (DevNum);
		} else {
		    // No end. Toggle ring state
		    if (GetState (stForcedRingOn)) {

			// Ring is on, switch it off
			ClrState (stForcedRingOn);
			IstecRingOff (DevNum);

		    } else {

			// Ring is off, switch it on
			SetState (stForcedRingOn);
			IstecRingOn (DevNum);

		    }
		}
	    }
	    break;

	case evExit:
	    // Shutdown forced ring if active
	    if (GetState (stForcedRing) && GetState (stForcedRingOn)) {

		// Forced ring is active, clear it
		ClrState (stForcedRing | stForcedRingOn);
		IstecRingOff (DevNum);

	    }
	    break;
    }
}



