/*****************************************************************************/
/*                                                                           */
/*                                    ICCLI.H                                */
/*                                                                           */
/* (C) 1996     Ullrich von Bassewitz                                        */
/*              Wacholderweg 14                                              */
/*              D-70597 Stuttgart                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// Calling line identification stuff



#ifndef _ICCLI_H
#define _ICCLI_H



#include "datetime.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/




// Constants for the TypeOfNumber field
const unsigned ntUnknown                = 0;
const unsigned ntInternational          = 1;
const unsigned ntNational               = 2;
const unsigned ntNetworkDependent       = 3;
const unsigned ntClientSpecific         = 4;
const unsigned ntInvalid                = 6;
const unsigned ntReserved               = 9;

// Constants for the "PresInd" field
const unsigned piPresAllowed            = 0;
const unsigned piPresRestricted         = 1;
const unsigned piNoNumber               = 2;



/*****************************************************************************/
/*                                 class CLI                                 */
/*****************************************************************************/



class CLI: public Object {

public:
    String      Number;                 // Calling partys number
    String      AreaCodeInfo;           // Info on prefix
    String      Alias;                  // Alias for the given number
    unsigned    TypeOfNumber;
    unsigned    PresInd;                // Presentation indicatior
    Time        T;                      // Time of call

public:
    String LogMsg ();
    // Return a log message for the CLI

};



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void HandleCLIMsg (const unsigned char* Data, unsigned Size);
// Handle a CLI message



// End of ICCLI.H

#endif



