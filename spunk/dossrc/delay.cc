/*****************************************************************************/
/*									     */
/*				   DELAY.CC				     */
/*									     */
/* (C) 1995	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// System dependent function for waiting some time.



#include <dos.h>
#ifdef __WATCOMC__
#include <i86.h>
#endif

#include "progutil.h"



/*****************************************************************************/
/*			       Utility functions			     */
/*****************************************************************************/



static void GiveUpTimeslice ()
// Give up the remainder of the current timeslice. Multiplex interrupt works
// under Windows and OS/2
{
    REGS Regs;
#if defined (DOS32) && defined (__WATCOMC__)
    Regs.w.ax = 0x1680;
    int386 (0x2F, &Regs, &Regs);
#else
    Regs.x.ax = 0x1680;
    int86 (0x2F, &Regs, &Regs);
#endif
}



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



u32 Delay (u32 ms)
// System dependent delay function that waits _at_least_ the given time in
// milliseconds. The function is free to choose a longer time, if it is not
// possible, to wait exactly the given time. This is especially true when
// ms exceeds 100, in this case App->Idle () is called in addition to waiting,
// so the _real_ time that is gone may be unpredictable.
// An argument of zero has a special meaning: The function tries to give up
// the current time slice, calls App->Idle () and returns after that.
//
// The function returns the real time passed or just ms.
{
    const ChunkSize = 256;

    // Check the argument...
    if (ms <= ChunkSize) {

	// Give up the time slice
	GiveUpTimeslice ();

	// Wait some time
	if (ms) {
	    delay (ms);
	}

	// Call the applications idle function
	Idle ();

    } else {

	u32 Counter = ms;
	while (Counter) {

	    unsigned TimeToWait = Counter >= ChunkSize ? ChunkSize : Counter;

	    // Recursive call to Delay...
	    u32 WaitTime = Delay (TimeToWait);
	    if (WaitTime > Counter) {
		Counter = 0;
	    } else {
		Counter -= WaitTime;
	    }

	}

    }

    // Return the argument
    return ms;
}



