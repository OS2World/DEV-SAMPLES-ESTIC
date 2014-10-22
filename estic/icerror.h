/*****************************************************************************/
/*                                                                           */
/*                                 ICERROR.H                                 */
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



#ifndef _ICERROR_H
#define _ICERROR_H



#include "icmsg.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msErrorIntIgnored     = MSGBASE_ICERROR +  0; // Internal error: ignored



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void IstecError (unsigned MsgNum);
// Display an error message, the message is taken from the application message
// base



// End of ICERROR.H

#endif

