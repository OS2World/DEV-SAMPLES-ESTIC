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



#include <stddef.h>
#include <process.h>
#include <setjmp.h>

#define INCL_BASE
#pragma pack(push)
#include <os2.h>
#pragma pack(pop)

#include "delay.h"
#include "check.h"

#include "task.h"



/*****************************************************************************/
/*                             class TaskHandle                              */
/*****************************************************************************/



class TaskHandle {

    friend class Task;

    ULONG       tid;
};




/*****************************************************************************/
/*                      Exceptionhandler for class Task                      */
/*****************************************************************************/



typedef ULONG APIENTRY (*ExceptionHandler) (PEXCEPTIONREPORTRECORD,
                                            PEXCEPTIONREGISTRATIONRECORD,
                                            PCONTEXTRECORD,
                                            PVOID);

struct TaskRegistrationRecord {

    // Data needed by the operating system
    EXCEPTIONREGISTRATIONRECORD* volatile PrevStruct;
    ExceptionHandler volatile Handler;

    // Private data
    Task*       T;
    jmp_buf     JumpBuf;
};



static ULONG APIENTRY TaskException (PEXCEPTIONREPORTRECORD Report,
                                     PEXCEPTIONREGISTRATIONRECORD Reg,
                                     PCONTEXTRECORD /* Context */,
                                     PVOID /* DispatcherContext */ )
// This exception handler handles the exception XCPT_PROCESS_TERMINATE and
// XCPT_ASYNC_PROCESS_TERMINATE
{
    if (Report->ExceptionNum == XCPT_PROCESS_TERMINATE ||
        Report->ExceptionNum == XCPT_ASYNC_PROCESS_TERMINATE) {

        // Cast the pointer to our own structure
        TaskRegistrationRecord* TR = (TaskRegistrationRecord*) Reg;

        // This is what we have installed the handler for
        longjmp (TR->JumpBuf, 1);

    }

    // Unknown exception - pass it to the next handler
    return XCPT_CONTINUE_SEARCH;
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
    Handle->tid = 0;
}



Task::~Task ()
// Destructor
{
    // Stop the thread if not already done
    Stop ();

    // Delete the handle data
    delete Handle;
}



void Task::CreateFunc (void* ThisObj)
// This function is started as a thread.
{
    // Cast the given pointer into a pointer-to-object. Use the volatile
    // keyword to avoid problems with T when using longjmp
    Task* volatile T = (Task*) ThisObj;

    // Get the thread ID. There is no easy way to do that, the thread ID must
    // be extracted from the thread information block.
    PTIB ptib;
    PPIB ppib;
    ZCHECK (DosGetInfoBlocks (&ptib, &ppib));
    T->Handle->tid = ptib->tib_ptib2->tib2_ultid;

    // Set the running flag now.
    T->Running = 1;

    // Call the init routine
    T->Init ();

    // Register an exception handler
    TaskRegistrationRecord Reg = { 0, TaskException, T };
    if (setjmp (Reg.JumpBuf) == 0) {

        // Register the exception handler
        ZCHECK (DosSetExceptionHandler ((PEXCEPTIONREGISTRATIONRECORD) &Reg));

        // Run the task
        T->Run ();

        // Deregister the exception handler. This is necessary only if we get
        // out of Run() without an exception, since longjmp (used by the
        // handler) will unwind all handlers including the one installed.
        ZCHECK (DosUnsetExceptionHandler ((PEXCEPTIONREGISTRATIONRECORD) &Reg));

    }

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



void Task::SetPriority (unsigned Priority, int Delta)
// Allows the task to set it's priority
{
    // Map the Priority to a system defined value
    ULONG Prio = 0;
    switch (Priority) {
        case PrioIdle:          Prio = PRTYC_IDLETIME;          break;
        case PrioRegular:       Prio = PRTYC_REGULAR;           break;
        case PrioServer:        Prio = PRTYC_FOREGROUNDSERVER;  break;
        case PrioTimeCritical:  Prio = PRTYC_TIMECRITICAL;      break;
        default:                FAIL ("Task::SetPriority: Invalid priority value");
    }
    DosSetPriority (PRTYS_THREAD, Prio, Delta, Handle->tid);
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
            // Send the Kill signal to the thread
            DosKillThread (Handle->tid);
        }

    }
}



