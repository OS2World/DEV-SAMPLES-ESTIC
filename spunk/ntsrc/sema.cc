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



#pragma pack(push)
#include <windows.h>
#pragma pack(pop)

#include "check.h"
#include "sema.h"



/*****************************************************************************/
/*                              class SemHandle                              */
/*****************************************************************************/



class SemHandle {

    friend class Semaphore;

    HANDLE      H;

    SemHandle ();
    // Constructor, set H to something invalid

};



inline SemHandle::SemHandle ():
    H (INVALID_HANDLE_VALUE)
{
}



/*****************************************************************************/
/*                              class Semaphore                              */
/*****************************************************************************/



void Semaphore::Init (const String& Name, unsigned InitialState)
// Does the real work for the constructors
{
    static SECURITY_ATTRIBUTES SA = {
        sizeof (SECURITY_ATTRIBUTES), NULL, 0
    };


    // Get memory for the semaphor handle and initialize it
    Handle = new SemHandle;

    // Create/open if the name is given, create otherwise
    if (Name.NotEmpty ()) {

        // We have a name, create/open a named semaphore
        char pszName [256];

        // Copy the name into the psz buffer
        Name.PSZ (pszName, sizeof (pszName));

        // Try to create the semaphore
        Handle->H = CreateMutex (&SA, InitialState, pszName);

    } else {

        // Create an unnamed semaphore
        Handle->H = CreateMutex (&SA, InitialState, 0);

    }

    // We must have a handle now
    CHECK (Handle->H != 0);
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
    // Delete the semaphore handle
    if (Handle->H != INVALID_HANDLE_VALUE) {
        CloseHandle (Handle->H);
        Handle->H = INVALID_HANDLE_VALUE;
    }

    // Delete the handle data structure
    delete Handle;
}



void Semaphore::Up ()
{
    CHECK (ReleaseMutex (Handle->H) != FALSE);
}



int Semaphore::Down (i32 T)
{
    DWORD Timeout = T >= 0 ? T : INFINITE;

    // Do the down
    switch (WaitForSingleObject (Handle->H, Timeout)) {

        case WAIT_TIMEOUT:
            // Timeout while waiting
            return 0;

        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:
            // we own the semaphore
            return 1;

        default:
            FAIL ("Semaphore::Down: WAIT_FAILED or bad return code");
            return 0;   // Never reached

    }
}



