/*****************************************************************************/
/*                                                                           */
/*                                 DELAY.H                                   */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



// System dependent function for waiting some time.



#ifndef __DELAY_H
#define __DELAY_H



#include "machine.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



u32 Delay (u32 ms);
// System dependent delay function that waits _at_least_ the given time in
// milliseconds. The function is free to choose a longer time, if it is not
// possible, to wait exactly the given time. This is especially true when
// ms exceeds 100, in this case App->Idle () is called in addition to waiting,
// so the _real_ time that is gone may be unpredictable.
// The function returns the real time passed or just ms.



// End of DELAY.H

#endif

