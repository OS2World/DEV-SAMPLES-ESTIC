/*****************************************************************************/
/*                                                                           */
/*                                   TASK.CC                                 */
/*                                                                           */
/*                      (C) 1996 MU Softwareentwicklung                      */
/*                                                                           */
/*      Ullrich von Bassewitz                          Michael Peschel       */
/*      Wacholderweg 14                                   Ledergasse 3       */
/*      D-70597 Stuttgart                            D-72555 Metzingen       */
/*      uz@ibb.schwaben.com                      mipe@ibb.schwaben.com       */
/*                                                                           */
/*****************************************************************************/



// Threads



#include <process.h>

#pragma pack(push)
#include <windows.h>
#pragma pack(pop)

#include "delay.h"
#include "check.h"

#include "task.h"



/*****************************************************************************/
/*                             class TaskHandle                              */
/*****************************************************************************/



class TaskHandle {

    friend class Task;

    HANDLE      H;

    TaskHandle ();
    // Constructor, set H to something invalid
};



inline TaskHandle::TaskHandle ():
    H (INVALID_HANDLE_VALUE)
{
}



/*****************************************************************************/
/*                                class Task                                 */
/*****************************************************************************/



Task::Task (size_t aStackSize):
    StackSize (aStackSize),
    Running (0),
    RunDown (0),
    Handle (new TaskHandle)
// Constructor
{
}



Task::~Task ()
// Destructor
{
    // Stop the thread if not already done
    Stop ();

    // Under NT, delete the thread handle
    if (Handle->H != INVALID_HANDLE_VALUE) {
        CloseHandle (Handle->H);
        Handle->H = INVALID_HANDLE_VALUE;
    }

    // Delete the handle data structure
    delete Handle;
}



void Task::CreateFunc (void* ThisObj)
// This function is started as a thread.
{
    // Cast the given pointer into a pointer-to-object. Use the volatile
    // keyword to avoid problems with T when using longjmp
    Task* volatile T = (Task*) ThisObj;

    // Get the thread ID. Be shure to duplicate the pseudohandle to allow
    // access from outside this thread.
    CHECK (DuplicateHandle (GetCurrentProcess (),
                            GetCurrentThread (),
                            GetCurrentProcess (),
                            &T->Handle->H,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS) == TRUE);

    // Set the running flag now.
    T->Running = 1;

    // Call the init routine
    T->Init ();

    // Run the task
    T->Run ();

    // Call the cleanup routine
    T->Done ();

    // If we get back from the run function, we are done - reset the Running flag
    T->Running = 0;
}



void Task::Init ()
// This function is called before Run()
{
}



void Task::Done ()
// This function is called after Run()
{
}



void Task::SetPriority (unsigned Priority, int /*Delta*/)
// Allows the task to set it's priority
{
    // Map the Priority to a system defined value
    int Prio = 0;
    switch (Priority) {
        case PrioIdle:          Prio = THREAD_PRIORITY_IDLE;            break;
        case PrioRegular:       Prio = THREAD_PRIORITY_NORMAL;          break;
        case PrioServer:        Prio = THREAD_PRIORITY_HIGHEST;         break;
        case PrioTimeCritical:  Prio = THREAD_PRIORITY_TIME_CRITICAL;   break;
        default:                FAIL ("Task::SetPriority: Invalid priority value");
    }
    CHECK (SetThreadPriority (Handle->H, Prio) != FALSE);
}



void Task::Start ()
// Start the task
{
    // The task may not run already
    CHECK (!IsRunning ());

    // Create the thread
    CHECK (_beginthread (CreateFunc, 0, StackSize, this) != -1);
}



void Task::Stop (unsigned WaitTime)
// Stop the task
{
    if (IsRunning ()) {

        // Set the rundown flag
        RunDown = 1;

        // Wait until WaitTime is over or the thread has terminated
        while (WaitTime && IsRunning ()) {

            // Calculate time slice to wait
            unsigned Time = 100;
            if (Time > WaitTime) {
                Time = WaitTime;
            }
            WaitTime -= Time;

            // Wait
            Delay (Time);
        }


        // If the thread did not terminate, kill it
        if (IsRunning ()) {
            // Kill the thread
            TerminateThread (Handle->H, 0);
        }

    }
}



