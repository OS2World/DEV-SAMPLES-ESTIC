/*****************************************************************************/
/*                                                                           */
/*                                 ICDLOG.H                                  */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



#ifndef _ICDLOG_H
#define _ICDLOG_H



#include "str.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void InitDebugLog (const String& Filename);
// Initialize (open) the debug log. If the given name is empty, the call is
// ignored. Notify the user in case of errors.

void DoneDebugLog ();
// Close an open debug log

void WriteDebugLog (const String& Msg);
// Log the given message if the debug log is open



// End of ICDLOG.H

#endif



