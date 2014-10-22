/*****************************************************************************/
/*									     */
/*				   EVENTID.H				     */
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



// Definitions for the event codes used inside the spunk library. For
// applications use event codes starting from evUser.



#ifndef _EVENTID_H
#define _EVENTID_H



/*****************************************************************************/
/*				  Event codes				     */
/*****************************************************************************/



// Program startup and end
const unsigned evInit			=   1;	// This event *may* be posted
						// by the application
						// constructor if init is
						// complete.
const unsigned evExit			=   2;	// This event *may* be posted
						// by the programs destructor
						// when the program ends
						// without an error
const unsigned evAbort			=   3;	// The program aborts because
						// of an error.

// Idle function
const unsigned evIdle			=   5;	// The program is idle
const unsigned evSecondChange		=   6;	// Posted every second if the
						// application is idle
const unsigned evMinuteChange		=   7;	// Posted every minute if the
						// application is idle

// Screen size
const unsigned evScreenSizeChange	=   8;	// Change of the screen size

// Window manager
const unsigned evWinMgrNoWindows	=  10;	// No more open windows
const unsigned evWinMgrFirstOpen	=  11;	// One open window
const unsigned evWinMgrLastClose	=  12;	// Max count - 1 reached
const unsigned evWinMgrMaxWindows	=  13;	// Max count of windows reached

// Main menu
const unsigned evDisableCommand		=  20;	// Disable a main menu command
const unsigned evEnableCommand		=  21;	// Enable a main menu command

// User defined events
const unsigned evUser			= 10000;



// End of EVENTID.H

#endif


