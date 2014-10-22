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



// This is a somewhat more generic "delay" module that should work on more
// than one unix like OS.
// Define the symbol DONT_HAS_USLEEP if usleep() is not available. The code
// will use a implementation based on select() instead. If you don't have
// select(), you are kind of stuck...



#include <unistd.h>
#include <sys/time.h>

#include "../progutil.h"



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



#if defined (DONT_HAS_USLEEP)

// Use an implementation of usleep based on select. This function is taken
// from "Advanced Programming in the Unix Environment" by W. Richard Stevens.
// This function may return earlier if the select function is interrupted -
// this is ignored for simplicity.

static void usleep (u32 usecs)
{
    // Set the timeout
    timeval Timeout;
    Timeout.tv_usec = usecs % 1000000;
    Timeout.tv_sec  = usecs / 1000000;

    // Wait...
    select (0, NULL, NULL, NULL, &Timeout);
}

#endif



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

	// Wait some time
	usleep (ms * 1000);

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




