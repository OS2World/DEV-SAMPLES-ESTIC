/*****************************************************************************/
/*                                                                           */
/*                                  SEMA.H                                   */
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



#ifndef _SEMA_H
#define _SEMA_H



#include "str.h"



/*****************************************************************************/
/*                              class Semaphore                              */
/*****************************************************************************/



// Initial semaphore states
const unsigned isUnowned        = 0x00;
const unsigned isOwned          = 0x01;



/*****************************************************************************/
/*                              class Semaphore                              */
/*****************************************************************************/



class SemHandle;
class Semaphore: public Object {

private:
    SemHandle*  Handle;


    void Init (const String& Name, unsigned InitialState);
    // Does the real work for the constructors


public:
    Semaphore (unsigned InitialState = isUnowned);
    // Create a new, unnamed semaphore with the given initial state

    Semaphore (const String& Name, unsigned InitialState = isUnowned);
    // Create/open a semaphore with the given name. The function will try to
    // create a semaphore with the given name. If a semaphore with this name
    // does already exist, it is opened instead.
    // InitialState gives the initial state of the semaphore if it is
    // created, the flag is ignored on an open.

    Semaphore (const char* Name, unsigned InitialState = isUnowned);
    // Create/open a semaphore with the given name. The function will try to
    // create a semaphore with the given name. If a semaphore with this name
    // does already exist, it is opened instead.
    // InitialState gives the initial state of the semaphore if it is
    // created, the flag is ignored on an open.

    virtual ~Semaphore ();
    // Cleanup semaphore resources

    void Up ();
    // Do an up on the semaphore (release it)

    int Down (i32 Timeout = -1);
    // Do a down on the semaphore (request it). The function returns 1 (true)
    // if the request was successful and a zero (false) otherwise. If the
    // timeout value is < 0 (the default), the wait will be indefinite and the
    // function will always return 1.

};



// End of SEMA.H

#endif



