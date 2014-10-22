/*****************************************************************************/
/*                                                                           */
/*                                ICCPRINT.H                                 */
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



// Print the charges



#ifndef _ICCPRINT_H
#define _ICCPRINT_H



#include "str.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



String PrintCharges (const String& Filename);
// Print the charges and return an error string. If all is ok, the empty
// String is returned.



// End of ICCPRINT.H

#endif

