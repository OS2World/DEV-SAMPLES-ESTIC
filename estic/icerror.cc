/*****************************************************************************/
/*                                                                           */
/*                                ICERROR.CC                                 */
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



#include "stdmsg.h"
#include "progutil.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void IstecError (unsigned MsgNum)
// Display an error message, the message is taken from the application message
// base
{
    ErrorMsg (LoadAppMsg (MsgNum));
}



