/*****************************************************************************/
/*                                                                           */
/*                                   TASK.H                                  */
/*                                                                           */
/*                      (C) 1996 MU Softwareentwicklung                      */
/*                                                                           */
/*      Ullrich von Bassewitz                          Michael Peschel       */
/*      Wacholderweg 14                                   Ledergasse 3       */
/*      D-70597 Stuttgart                            D-72555 Metzingen       */
/*      uz@ibb.schwaben.com                      mipe@ibb.schwaben.com       */
/*                                                                           */
/*****************************************************************************/



// Task (thread) object. Currently implemented for OS/2 and NT. There are some
// quirks when working together with the rest of the spunk library, but the
// object has shonw to be too useful to omit it.



#ifndef _TASK_H
#define _TASK_H



#include "object.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Priority class values
const unsigned PrioIdle         = 0;    // Run as idle thread
const unsigned PrioRegular      = 1;    // Run with regular priority
const unsigned PrioServer       = 2;    // Run as server thread
const unsigned PrioTimeCritical = 3;    // Run with time critical priority



/*****************************************************************************/
/*                                class Task                                 */
/*****************************************************************************/



class TaskHandle;
class Task: public Object {

protected:
    size_t      StackSize;
    int         Running;
    int         RunDown;
    TaskHandle* Handle;

private:
    static void CreateFunc (void*);
    // This function is started as a thread

    virtual void Init ();
    // This function is called before Run()

    virtual void Run () = 0;
    // This is the real "worker function" of the task

    virtual void Done ();
    // This function is called after Run()

public:
    Task (size_t aStackSize = 0x8000);
    // Constructor. The Stacksize value may be honored or not.

    virtual ~Task ();
    // Destruktor

    virtual void Start ();
    // Start the task

    virtual void Stop (unsigned WaitTime = 2000);
    // Stop the task. WaitTime is the time, the routine waits for the thread
    // to terminate after the RunDown flag has been set. The Run() function
    // should poll this flag in regular intervalls. If the thread does not
    // recognize this flag, termination is forced. BEWARE: Forced termination
    // of a thread from outside is inherently dangerous, since the thread may
    // have no chance to cleanup resources.
    // The effects of this function differ from one supported operating
    // system to another, when the thread does not honor the RunDown flag and
    // must be killed: Under OS/2, the thread will catch the termination,
    // execute the Done() function and terminate itself. Under NT, the thread
    // will be killed immidiately without any chance for a cleanup.

    int IsRunning () const;
    // Return 1 if the task is currently running

    virtual void SetPriority (unsigned Priority, int Delta = 0);
    // Allows to set the priority of the task. BEWARE: Setting the priority
    // from outside the thread may not be possible from each supported
    // operating system.

};



inline int Task::IsRunning () const
// Return 1 if the task is currently running
{
    return Running;
}



// End of TASK.H

#endif
