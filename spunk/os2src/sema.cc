/*****************************************************************************/
/*                                                                           */
/*                                  SEMA.CC                                  */
/*                                                                           */
/*                      (C) 1996 MU Softwareentwicklung                      */
/*                                                                           */
/*      Ullrich von Bassewitz                          Michael Peschel       */
/*      Wacholderweg 14                                   Ledergasse 3       */
/*      D-70597 Stuttgart                            D-72555 Metzingen       */
/*      uz@ibb.schwaben.com                      mipe@ibb.schwaben.com       */
/*                                                                           */
/*****************************************************************************/



// This module defines an envelope class for a binary operating system
// supported semaphore. Currently the class is implemented for OS/2 and NT.



#define INCL_DOSERRORS
#define INCL_DOSSEMAPHORES
#pragma pack(push)
#include <os2.h>
#pragma pack(pop)

#include "check.h"
#include "sema.h"



/*****************************************************************************/
/*                              class SemHandle                              */
/*****************************************************************************/



class SemHandle {

    friend class Semaphore;

    HMTX        H;

};



/*****************************************************************************/
/*                              class Semaphore                              */
/*****************************************************************************/



void Semaphore::Init (const String& Name, unsigned InitialState)
// Does the real work for the constructors
{
    APIRET RC;

    // Get memory for the semaphor handle and initialize it
    Handle = new SemHandle;
    Handle->H = (HMTX) -1;

    // Create/open if the name is given, create otherwise
    if (Name.NotEmpty ()) {

        // We have a name, create/open a named semaphore
        char pszName [256];

        // Copy the name into the psz buffer
        Name.PSZ (pszName, sizeof (pszName));

        // Try to create the semaphore
        RC = DosCreateMutexSem (
               pszName,
               &Handle->H,
               DC_SEM_SHARED,
               InitialState
             );

        if (RC == ERROR_DUPLICATE_NAME) {

            // The semaphore is already created, just open it
            RC = DosOpenMutexSem (pszName, &Handle->H);

        }

    } else {

        // Create an unnamed semaphore
        RC = DosCreateMutexSem (
               NULL,
               &Handle->H,
               DC_SEM_SHARED,
               InitialState
             );

    }

    // Return code of the last called function must be zero
    ZCHECK (RC);
}



Semaphore::Semaphore (unsigned InitialState)
// Create a new, unnamed semaphore with the given initial state
{
    Init (EmptyString, InitialState);
}



Semaphore::Semaphore (const String& Name, unsigned InitialState)
// Create/open a semaphore with the given name. The function will try to
// create a semaphore with the given name. If a semaphore with this name
// does already exist, it is opened instead.
// InitialState gives the initial state of the semaphore if it is
// created, the flag is ignored on an open.
{
    Init (Name, InitialState);
}



Semaphore::Semaphore (const char* Name, unsigned InitialState)
// Create/open a semaphore with the given name. The function will try to
// create a semaphore with the given name. If a semaphore with this name
// does already exist, it is opened instead.
// InitialState gives the initial state of the semaphore if it is
// created, the flag is ignored on an open.
{
    Init (Name, InitialState);
}



Semaphore::~Semaphore ()
{
    DosCloseMutexSem (Handle->H);
    delete Handle;
}



void Semaphore::Up ()
{
    ZCHECK (DosReleaseMutexSem (Handle->H));
}



int Semaphore::Down (i32 T)
{
    ULONG Timeout = T >= 0 ? T : SEM_INDEFINITE_WAIT;

    // Do the down
    APIRET RC;
    do {
        RC = DosRequestMutexSem (Handle->H, Timeout);
    } while (RC == ERROR_INTERRUPT);

    if (RC == ERROR_TIMEOUT) {
        // We do not own the semaphore
        return 0;
    } else {
        // Check for errors
        ZCHECK (RC);

        // We own the semaphore now
        return 1;
    }
}



